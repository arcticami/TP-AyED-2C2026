#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lrucache.h"
#include "tweener.h"
#include "lotes.h"

/*
 * Simulador de uso de Tweener con y sin LRU Cache.
 *
 * Uso:  simulador [semilla [capacidad ...]]
 *
 * Sin argumentos usa la semilla y las capacidades por defecto. Capacidad 0 = sin cache (siempre lee el archivo).
 * Ejemplos:  simulador                 semilla 1, capacidades por defecto
 *            simulador 7               semilla 7, capacidades por defecto
 *            simulador 7 0 50 500      semilla 7, capacidades 0, 50 y 500
 *
 * Un mismo .c se compila dos veces: contra parte1/lrucache.h (lista simple) o contra
 * parte2/lrucache.h (lista doble + arbol). Cada compilacion es un programa completo:
 *   1) genera posteos.bin y los tres lotes de usuarios (concentrado, disperso y ordenado) con la semilla;
 *   2) para cada lote y cada capacidad procesa todos los usuarios de la primera a la ultima: por cada id de su
 *      feed pide el posteo a la cache y, si no esta, lo lee del archivo y lo guarda en la cache;
 *   3) muestra los resultados por pantalla y los guarda en el archivo de reporte de la variante.
 * Los datos de los posteos no se muestran ni se guardan: solo se mide.
 */

#define SEMILLA_DEFECTO 1
#define MAX_CAPACIDADES 32
#define REPETICIONES    5       //cada capacidad se corre varias veces y se informa el tiempo promedio
#define SIN_BUFFER      1       //1 = el archivo de posteos se lee sin buffer de stdio

//La Parte 2 incluye al arbol, la Parte 1 no: asi se sabe contra cual se compilo sin tocar nada mas
#ifdef ARBOL_BIN_BUSQ_H_INCLUDED
#define TIENE_ALTURA
#define NOMBRE_VARIANTE "Parte 2 - lista doble + indice arbol"
#define ARCHIVO_REPORTE "reporte_parte2.txt"
#else
#define NOMBRE_VARIANTE "Parte 1 - lista simple"
#define ARCHIVO_REPORTE "reporte_parte1.txt"
#endif

const int CAPACIDADES_DEFECTO[] = { 0, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000 };

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
void calentarArchivo(FILE *fp);
int correr(FILE *fpPost, FILE *fpUsu, int capacidad, tResultado *r);
int generarLotes(unsigned semilla);
int simularLote(FILE *fpPost, const char *tipo, const int *caps, int cantCaps, FILE *salidas[], int cantSalidas);
void mostrarTitulo(FILE *f, const char *tipo);
void mostrarEncabezado(FILE *f);
void mostrarFila(FILE *f, int capacidad, const tResultado *r);

int main(int argc, char *argv[])
{
    FILE *fpPost, *fpRep, *salidas[2];
    int caps[MAX_CAPACIDADES], cantCaps, cantSalidas, i, ok = 1;
    unsigned semilla = SEMILLA_DEFECTO;

    if (argc > 1)
        semilla = atoi(argv[1]);

    if (argc > 2)
    {
        cantCaps = argc - 2;
        if (cantCaps > MAX_CAPACIDADES)
        {
            fprintf(stderr, "Error: maximo %d capacidades\n", MAX_CAPACIDADES);
            return 1;
        }

        for (i = 0; i < cantCaps; i++)
        {
            caps[i] = atoi(argv[i + 2]);
            if (caps[i] < 0)
            {
                fprintf(stderr, "Error: las capacidades no pueden ser negativas\n");
                return 1;
            }
        }
    }
    else
    {
        cantCaps = sizeof(CAPACIDADES_DEFECTO) / sizeof(CAPACIDADES_DEFECTO[0]);
        memcpy(caps, CAPACIDADES_DEFECTO, sizeof(CAPACIDADES_DEFECTO));
    }

    printf("Generando lotes con semilla %u ...\n", semilla);
    if (!generarLotes(semilla))
    {
        fprintf(stderr, "Error: no se pudieron generar los lotes (hay permiso de escritura en la carpeta de trabajo?)\n");
        return 1;
    }

    if ((fpPost = fopen(RUTA_POSTEOS, "rb")) == NULL)
    {
        fprintf(stderr, "Error: no se pudo abrir %s\n", RUTA_POSTEOS);
        return 1;
    }

    if (SIN_BUFFER)
        setvbuf(fpPost, NULL, _IONBF, 0);

    //los resultados salen por pantalla y tambien a un archivo, para no perderlos cuando se cierra la consola
    salidas[0] = stdout;
    cantSalidas = 1;
    if ((fpRep = fopen(ARCHIVO_REPORTE, "w")) != NULL)
        salidas[cantSalidas++] = fpRep;
    else
        fprintf(stderr, "Aviso: no se pudo crear %s, el reporte sale solo por pantalla\n", ARCHIVO_REPORTE);

    for (i = 0; i < cantSalidas; i++)
    {
        fprintf(salidas[i], "\n%s\n", NOMBRE_VARIANTE);
        fprintf(salidas[i], "semilla  : %u\n", semilla);
        fprintf(salidas[i], "posteos  : %d (registro de %d bytes)\n", CANT_POSTEOS, (int)sizeof(tPosteo));
        fprintf(salidas[i], "usuarios : %d por lote (feed de %d posteos)\n", CANT_USUARIOS, LARGO_FEED);
        fprintf(salidas[i], "tiempo   : promedio de %d corridas; capacidad 0 = sin cache\n", REPETICIONES);
    }

    calentarArchivo(fpPost); //asi la primera corrida no paga la lectura "en frio" del archivo

    for (i = 0; ok && i < CANT_TIPOS_LOTE; i++)
        ok = simularLote(fpPost, TIPOS_LOTE[i], caps, cantCaps, salidas, cantSalidas);

    fclose(fpPost);
    if (fpRep != NULL)
    {
        fclose(fpRep);
        printf("Reporte guardado en %s\n", ARCHIVO_REPORTE);
    }

    return ok ? 0 : 1;
}

//escribe posteos.bin y los tres lotes de usuarios con la misma semilla
int generarLotes(unsigned semilla)
{
    char ruta[TAM_RUTA];
    int i = 0, ok = generarPosteos(RUTA_POSTEOS);

    while (ok && i < CANT_TIPOS_LOTE)
    {
        rutaUsuarios(ruta, TIPOS_LOTE[i]);
        ok = generarUsuarios(ruta, TIPOS_LOTE[i], semilla, CANT_USUARIOS);
        i++;
    }

    return ok;
}

//simula un lote con cada capacidad y va mostrando las filas a medida que terminan
int simularLote(FILE *fpPost, const char *tipo, const int *caps, int cantCaps, FILE *salidas[], int cantSalidas)
{
    char ruta[TAM_RUTA];
    FILE *fpUsu;
    tResultado r;
    int i, s, ok = 1;

    rutaUsuarios(ruta, tipo);
    if ((fpUsu = fopen(ruta, "rb")) == NULL)
    {
        fprintf(stderr, "Error: no se pudo abrir %s\n", ruta);
        return 0;
    }

    for (s = 0; s < cantSalidas; s++)
    {
        mostrarTitulo(salidas[s], tipo);
        mostrarEncabezado(salidas[s]);
    }

    i = 0;
    while (ok && i < cantCaps)
    {
        ok = correr(fpPost, fpUsu, caps[i], &r);
        if (ok)
        {
            for (s = 0; s < cantSalidas; s++)
            {
                mostrarFila(salidas[s], caps[i], &r);
                fflush(salidas[s]);
            }
        }
        else
            fprintf(stderr, "Error durante la simulacion con capacidad %d\n", caps[i]);
        i++;
    }

    fclose(fpUsu);
    return ok;
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

//lee el archivo de posteos completo una vez, sin medir nada, para que el sistema operativo lo tenga en memoria
void calentarArchivo(FILE *fp)
{
    tPosteo p;

    rewind(fp);
    while (fread(&p, sizeof(tPosteo), 1, fp) == 1)
        ;
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

void mostrarTitulo(FILE *f, const char *tipo)
{
    int i;

    fprintf(f, "\n");
    for (i = 0; i < 100; i++)
        fputc('-', f);
    fprintf(f, "\nLOTE %s\n", tipo);
    for (i = 0; i < 100; i++)
        fputc('-', f);
    fprintf(f, "\n");
}

void mostrarEncabezado(FILE *f)
{
    fprintf(f, "%7s %10s %10s %9s %9s %9s %7s %9s %13s %6s %7s\n",
            "capac.", "total(ms)", "us/usuario", "us/posteo", "aciertos", "fallos", "tasa(%)",
            "lecturas", "comparaciones", "altura", "errores");
}

void mostrarFila(FILE *f, int capacidad, const tResultado *r)
{
    double accesos = (double)CANT_USUARIOS * LARGO_FEED;

    fprintf(f, "%7d %10.2f %10.2f %9.3f ", capacidad, r->segundos * 1000.0, r->segundos * 1e6 / CANT_USUARIOS, r->segundos * 1e6 / accesos);

    if (capacidad == 0)
        fprintf(f, "%9s %9s %7s ", "-", "-", "-");
    else
        fprintf(f, "%9d %9d %7.2f ", r->aciertos, r->fallos, 100.0 * r->aciertos / (r->aciertos + r->fallos));

    fprintf(f, "%9d ", r->lecturas);

    if (capacidad == 0)
        fprintf(f, "%13s ", "-");
    else
        fprintf(f, "%13lu ", r->comparaciones);

    if (r->altura < 0)
        fprintf(f, "%6s ", "-");
    else
        fprintf(f, "%6d ", r->altura);

    fprintf(f, "%7d\n", r->errores);
}