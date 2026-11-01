#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lrucache.h"
#include "tweener.h"

/*
 * Simulador de uso de Tweener con y sin LRU Cache.
 *
 * Uso:  simulador <posteos.bin> <usuarios.bin>                      muestra los datos de los archivos
 *       simulador <posteos.bin> <usuarios.bin> <capacidad> [...]    corre la simulacion con cada capacidad
 *
 * Un mismo .c se compila dos veces: contra parte1/lrucache.h (lista simple) o contra
 * parte2/lrucache.h (lista doble + arbol). Capacidad 0 = sin cache (siempre lee el archivo).
 *
 * Para cada capacidad procesa todos los usuarios de la primera a la ultima: por cada id de su feed
 * pide el posteo a la cache y, si no esta, lo lee del archivo y lo guarda en la cache.
 * Los datos de los posteos no se muestran ni se guardan: solo se mide.
 * Todo sale por pantalla; correr_simulacion.bat lo redirige a reporte.txt.
 */

#define REPETICIONES    3       //cada capacidad se corre varias veces y se informa el tiempo promedio
#define SIN_BUFFER      1       //1 = el archivo de posteos se lee sin buffer de stdio

//La Parte 2 incluye al arbol, la Parte 1 no: asi se sabe contra cual se compilo sin tocar nada mas
#ifdef ARBOL_BIN_BUSQ_H_INCLUDED
#define TIENE_ALTURA
#define NOMBRE_VARIANTE "Parte 2 - lista doble + indice arbol"
#else
#define NOMBRE_VARIANTE "Parte 1 - lista simple"
#endif

typedef struct
{
    double segundos;            //tiempo total promedio de la simulacion
    int aciertos;
    int fallos;
    int lecturas;               //lecturas hechas al archivo de posteos
    unsigned long comparaciones;
    int errores;                //posteos devueltos con un id distinto al pedido (debe dar 0)
    int altura;                 //altura del arbol indice; -1 si no aplica (Parte 1 o sin cache)
} tResultado;

//VARIABLE GLOBAL SOLO PARA CONTAR COMPARACIONES (columna "comparaciones" del reporte). El cmp que recibe la cache
//no puede recibir nada mas que las dos claves, asi que no hay otro lugar de donde colgar el contador.
//Es una metrica opcional: si no gusta se borra junto con la columna y no afecta a nada mas.
unsigned long g_comparaciones = 0;

int cmpId(const void *a, const void *b);
int leerPosteo(FILE *fp, int id, tPosteo *p);
int correr(FILE *fpPost, FILE *fpUsu, int capacidad, tResultado *r);
void mostrarDatos(int posteos, int usuarios);
void mostrarEncabezado(void);
void mostrarFila(int capacidad, const tResultado *r, int usuarios);

int main(int argc, char *argv[])
{
    FILE *fpPost, *fpUsu;
    tResultado r;
    int posteos, usuarios, capacidad, i, ok = 1;

    if (argc < 3)
    {
        printf("Uso: %s <posteos.bin> <usuarios.bin> [capacidad ...]\n", argv[0]);
        printf("Capacidad 0 = sin cache. Variante compilada: %s\n", NOMBRE_VARIANTE);
        return 1;
    }

    if ((fpPost = fopen(argv[1], "rb")) == NULL || (fpUsu = fopen(argv[2], "rb")) == NULL)
    {
        fprintf(stderr, "Error: no se pudo abrir %s o %s\n", argv[1], argv[2]);
        return 1;
    }

    //la cantidad de registros sale del tamano de cada archivo
    fseek(fpPost, 0, SEEK_END);
    posteos = ftell(fpPost) / sizeof(tPosteo);
    rewind(fpPost);

    fseek(fpUsu, 0, SEEK_END);
    usuarios = ftell(fpUsu) / sizeof(tUsuario);
    rewind(fpUsu);

    if (usuarios == 0)
    {
        fprintf(stderr, "Error: %s no tiene usuarios\n", argv[2]);
        return 1;
    }

    if (argc == 3)
        mostrarDatos(posteos, usuarios);
    else
    {
        if (SIN_BUFFER)
            setvbuf(fpPost, NULL, _IONBF, 0);

        printf("%s\n", NOMBRE_VARIANTE);
        mostrarEncabezado();

        i = 3;
        while (ok && i < argc)
        {
            capacidad = atoi(argv[i]);

            ok = correr(fpPost, fpUsu, capacidad, &r);
            if (ok)
                mostrarFila(capacidad, &r, usuarios);
            else
                fprintf(stderr, "Error durante la simulacion con capacidad %d\n", capacidad);
            i++;
        }
        printf("\n");
    }

    fclose(fpPost);
    fclose(fpUsu);

    return ok ? 0 : 1;
}

//compara por id; la clave esta al principio del bloque de la cache, asi que sirve para comparar clave contra bloque
int cmpId(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;

    g_comparaciones++; //solo para la metrica de comparaciones

    return (x > y) - (x < y);
}

//el id coincide con el numero de registro, asi que se llega directo con fseek
int leerPosteo(FILE *fp, int id, tPosteo *p)
{
    if (id < 0 || id >= CANT_POSTEOS)
        return 0;

    if (fseek(fp, id * sizeof(tPosteo), SEEK_SET) != 0)
        return 0;

    return fread(p, sizeof(tPosteo), 1, fp) == 1;
}

//una corrida completa, repetida REPETICIONES veces; devuelve 0 si hubo un error de lectura o de memoria
int correr(FILE *fpPost, FILE *fpUsu, int capacidad, tResultado *r)
{
    tLruCache cache;
    tUsuario u;
    tPosteo p;
    double ini, fin, suma = 0;
    int rep = 0, j, id, hit, ok = 1, hayCache = capacidad > 0;

    memset(r, 0, sizeof(tResultado));
    r->altura = -1;

    while (ok && rep < REPETICIONES)
    {
        r->aciertos = r->fallos = r->lecturas = r->errores = 0;
        g_comparaciones = 0;

        if (hayCache && CrearLruCache(&cache, capacidad, sizeof(int), cmpId) != TODO_BIEN)
            return 0;

        rewind(fpUsu);
        ini = clock();

        while (ok && fread(&u, sizeof(tUsuario), 1, fpUsu) == 1)
        {
            j = 0;
            while (ok && j < LARGO_FEED)
            {
                id = u.feed[j];
                hit = hayCache && ObtenerLruCache(&cache, &id, &p, sizeof(tPosteo)) == TODO_BIEN;

                if (hit)
                    r->aciertos++;
                else
                {
                    if (hayCache)
                        r->fallos++;

                    ok = leerPosteo(fpPost, id, &p);
                    r->lecturas++;

                    if (ok && hayCache)
                        ok = AgregarLruCache(&cache, &id, &p, sizeof(tPosteo)) == TODO_BIEN;
                }

                if (ok && p.id != id)
                    r->errores++;
                j++;
            }
        }

        fin = clock();
        suma += (fin - ini) / CLOCKS_PER_SEC;

        if (hayCache)
        {
#ifdef TIENE_ALTURA
            r->altura = AlturaIndiceLruCache(&cache);
#endif
            VaciarLruCache(&cache);
        }
        rep++;
    }

    r->segundos = suma / REPETICIONES;
    r->comparaciones = g_comparaciones;

    return ok;
}

/* -------------------------------------------------------------------------- */
/*                                   SALIDA                                   */
/* -------------------------------------------------------------------------- */

//datos que se informan una sola vez arriba del reporte
void mostrarDatos(int posteos, int usuarios)
{
    printf("posteos  : %d (registro de %d bytes)\n", posteos, (int)sizeof(tPosteo));
    printf("usuarios : %d (feed de %d posteos)\n", usuarios, LARGO_FEED);
    printf("tiempo   : promedio de %d corridas; capacidad 0 = sin cache\n\n", REPETICIONES);
}

void mostrarEncabezado(void)
{
    printf("%7s %10s %10s %9s %9s %9s %7s %9s %13s %6s %7s\n",
           "capac.", "total(ms)", "us/usuario", "us/posteo", "aciertos", "fallos", "tasa(%)",
           "lecturas", "comparaciones", "altura", "errores");
}

void mostrarFila(int capacidad, const tResultado *r, int usuarios)
{
    double accesos = usuarios * LARGO_FEED;

    printf("%7d %10.2f %10.2f %9.3f ", capacidad, r->segundos * 1000.0, r->segundos * 1e6 / usuarios, r->segundos * 1e6 / accesos);

    if (capacidad == 0)
        printf("%9s %9s %7s ", "-", "-", "-");
    else
        printf("%9d %9d %7.2f ", r->aciertos, r->fallos, 100.0 * r->aciertos / (r->aciertos + r->fallos));

    printf("%9d ", r->lecturas);

    if (capacidad == 0)
        printf("%13s ", "-");
    else
        printf("%13lu ", r->comparaciones);

    if (r->altura < 0)
        printf("%6s ", "-");
    else
        printf("%6d ", r->altura);

    printf("%7d\n", r->errores);
}
