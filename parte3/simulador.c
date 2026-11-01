#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "lrucache.h"
#include "tweener.h"

/*
 * Simulador de uso de Tweener con y sin LRU Cache.
 *
 * Uso:  simulador [-e <semilla>] [-s] <posteos.bin> <usuarios.bin> <capacidad> [capacidad ...]
 *
 *   -e <semilla>  antes de la tabla escribe el encabezado general del reporte (rutas, cantidades, tamano del
 *                 registro y la semilla con la que se generaron los lotes; el simulador no puede deducirla)
 *   -s            antes de la tabla escribe el titulo de la seccion del lote (se deduce de usuarios_<lote>.bin)
 *
 * Un mismo .c se compila dos veces: contra parte1/lrucache.h (lista simple) o contra
 * parte2/lrucache.h (lista doble + arbol). Capacidad 0 = sin cache (siempre lee el archivo).
 *
 * Para cada capacidad procesa todos los usuarios de la primera a la ultima: por cada id de su feed
 * pide el posteo a la cache y, si no esta, lo lee del archivo y lo guarda en la cache.
 * Los datos no se muestran ni se guardan: solo se mide. Los resultados salen por pantalla y se
 * agregan a RUTA_REPORTE.
 */

#define REPETICIONES    3                   //cada capacidad se corre varias veces y se informa el promedio
#define RUTA_REPORTE    "reporte.txt"
#define SIN_BUFFER      1                   //1 = el archivo de posteos se lee sin buffer de stdio
#define LINEA           "----------------------------------------------------------------------------------------------------"

//La Parte 2 incluye al arbol, la Parte 1 no: asi se sabe contra cual se compilo sin tocar nada mas
#ifdef ARBOL_BIN_BUSQ_H_INCLUDED
#define TIENE_ALTURA
#define NOMBRE_VARIANTE "Parte 2 - lista doble + indice arbol"
#else
#define NOMBRE_VARIANTE "Parte 1 - lista simple"
#endif

typedef struct
{
    double        segundos;     //tiempo total promedio de la simulacion
    unsigned long aciertos;
    unsigned long fallos;
    unsigned long lecturas;     //lecturas hechas al archivo de posteos
    unsigned long comparaciones;
    unsigned long errores;      //posteos devueltos con un id distinto al pedido (debe dar 0)
    int           hayAltura;
    unsigned      altura;
} tResultado;

//VARIABLE GLOBAL SOLO PARA CONTAR COMPARACIONES (columna "comparaciones" del reporte). El cmp que recibe la cache
//no puede recibir nada mas que las dos claves, asi que no hay otro lugar de donde colgar el contador.
//Es una metrica opcional: si no gusta se borra junto con la columna y no afecta a nada mas.
unsigned long g_comparaciones = 0;

int cmpId(const void *a, const void *b);
int leerPosteo(FILE *fp, int id, tPosteo *p);
void calentarArchivo(FILE *fp);
int correr(FILE *fpPost, FILE *fpUsu, unsigned capacidad, tResultado *r);
void nombreLote(const char *ruta, char *lote, unsigned tam);
void patronUsuarios(const char *ruta, char *patron, unsigned tam);
void emitirEncabezado(FILE *out, const char *semilla, const char *archPost, const char *archUsu, unsigned posteos, unsigned usuarios);
void emitirSeccion(FILE *out, const char *archUsu);
void emitirCabeceraTabla(FILE *out);
void emitirFila(FILE *out, unsigned capacidad, const tResultado *r, unsigned usuarios);

int main(int argc, char *argv[])
{
    FILE *fpPost, *fpUsu, *fpRep;
    FILE *salidas[2];
    tResultado r;
    const char *semilla = NULL;
    unsigned posteos, usuarios;
    unsigned long capacidad;
    int i = 1, s, cantSalidas, hayTitulo = 0, ok = 1, usoOk = 1;

    //opciones
    while (usoOk && i < argc && argv[i][0] == '-')
    {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc)
        {
            semilla = argv[i + 1];
            i += 2;
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            hayTitulo = 1;
            i++;
        }
        else
            usoOk = 0;
    }

    if (!usoOk || argc - i < 3)
    {
        printf("Uso: %s [-e <semilla>] [-s] <posteos.bin> <usuarios.bin> <capacidad> [capacidad ...]\n", argv[0]);
        printf("Capacidad 0 = sin cache. Variante compilada: %s\n", NOMBRE_VARIANTE);
        return 1;
    }

    if ((fpPost = fopen(argv[i], "rb")) == NULL || (fpUsu = fopen(argv[i + 1], "rb")) == NULL)
    {
        printf("Error: no se pudo abrir %s o %s\n", argv[i], argv[i + 1]);
        return 1;
    }

    fseek(fpPost, 0, SEEK_END);
    posteos = (unsigned)(ftell(fpPost) / (long)sizeof(tPosteo));
    rewind(fpPost);

    fseek(fpUsu, 0, SEEK_END);
    usuarios = (unsigned)(ftell(fpUsu) / (long)sizeof(tUsuario));
    rewind(fpUsu);

    if (usuarios == 0)
    {
        printf("Error: %s no tiene usuarios\n", argv[i + 1]);
        return 1;
    }

    if (SIN_BUFFER)
        setvbuf(fpPost, NULL, _IONBF, 0);

    //todo lo que se informa sale por pantalla y tambien al reporte
    salidas[0] = stdout;
    cantSalidas = 1;
    if ((fpRep = fopen(RUTA_REPORTE, "a")) != NULL)
        salidas[cantSalidas++] = fpRep;

    s = 0;
    while (s < cantSalidas)
    {
        if (semilla)
            emitirEncabezado(salidas[s], semilla, argv[i], argv[i + 1], posteos, usuarios);
        if (hayTitulo)
            emitirSeccion(salidas[s], argv[i + 1]);

        fprintf(salidas[s], "%s\n", NOMBRE_VARIANTE);
        emitirCabeceraTabla(salidas[s]);
        s++;
    }

    calentarArchivo(fpPost); //asi la primera corrida no paga la lectura "en frio" del archivo

    i += 2; //desde aca los argumentos son las capacidades
    while (ok && i < argc)
    {
        capacidad = strtoul(argv[i], NULL, 10);

        if ((ok = correr(fpPost, fpUsu, (unsigned)capacidad, &r)) != 0)
        {
            s = 0;
            while (s < cantSalidas)
            {
                emitirFila(salidas[s], (unsigned)capacidad, &r, usuarios);
                s++;
            }
        }
        else
            printf("Error durante la simulacion con capacidad %lu\n", capacidad);
        i++;
    }

    s = 0;
    while (s < cantSalidas)
    {
        fprintf(salidas[s], "\n");
        s++;
    }

    if (fpRep)
        fclose(fpRep);
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

    if (fseek(fp, (long)id * (long)sizeof(tPosteo), SEEK_SET) != 0)
        return 0;

    return fread(p, sizeof(tPosteo), 1, fp) == 1;
}

//lee el archivo entero una vez, sin medir, para que el sistema operativo lo tenga en su cache antes de la primera corrida
void calentarArchivo(FILE *fp)
{
    char buf[65536];

    fseek(fp, 0, SEEK_SET);
    while (fread(buf, 1, sizeof(buf), fp) > 0)
        ;
}

//una corrida completa; devuelve 0 si hubo un error de lectura o de memoria
int correr(FILE *fpPost, FILE *fpUsu, unsigned capacidad, tResultado *r)
{
    tLruCache cache;
    tUsuario u;
    tPosteo p;
    clock_t ini, fin;
    double suma = 0;
    int rep = 0, j, id, hit, ok = 1, hayCache = capacidad > 0;

    memset(r, 0, sizeof(tResultado));

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
        suma += (double)(fin - ini) / CLOCKS_PER_SEC;

        if (hayCache)
        {
#ifdef TIENE_ALTURA
            r->hayAltura = 1;
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

//".../usuarios_ordenado.bin" -> "ordenado"; si la ruta no tiene ese formato devuelve la ruta entera
void nombreLote(const char *ruta, char *lote, unsigned tam)
{
    const char *ini = strstr(ruta, "usuarios_");
    const char *fin = ini ? strstr(ini, ".bin") : NULL;
    unsigned n;

    if (fin == NULL)
    {
        strncpy(lote, ruta, tam - 1);
        lote[tam - 1] = '\0';
        return;
    }

    ini += strlen("usuarios_");
    n = (unsigned)(fin - ini);
    if (n > tam - 1)
        n = tam - 1;

    memcpy(lote, ini, n);
    lote[n] = '\0';
}

//".../usuarios_ordenado.bin" -> ".../usuarios_<lote>.bin" (en el encabezado general no va un lote en particular)
void patronUsuarios(const char *ruta, char *patron, unsigned tam)
{
    const char *ini = strstr(ruta, "usuarios_");
    const char *fin = ini ? strstr(ini, ".bin") : NULL;

    if (fin == NULL)
    {
        strncpy(patron, ruta, tam - 1);
        patron[tam - 1] = '\0';
        return;
    }

    snprintf(patron, tam, "%.*s<lote>%s", (int)(ini + strlen("usuarios_") - ruta), ruta, fin);
}

void emitirEncabezado(FILE *out, const char *semilla, const char *archPost, const char *archUsu, unsigned posteos, unsigned usuarios)
{
    char patron[128];

    patronUsuarios(archUsu, patron, sizeof(patron));

    fprintf(out, "semilla  : %s\n", semilla);
    fprintf(out, "posteos  : %s (%u posteos, registro de %u bytes)\n", archPost, posteos, (unsigned)sizeof(tPosteo));
    fprintf(out, "usuarios : %s (%u usuarios, feed de %d posteos)\n", patron, usuarios, LARGO_FEED);
    fprintf(out, "tiempo   : promedio de %d corridas; capacidad 0 = sin cache\n\n", REPETICIONES);
}

void emitirSeccion(FILE *out, const char *archUsu)
{
    char lote[64];
    unsigned i = 0;

    nombreLote(archUsu, lote, sizeof(lote));
    while (lote[i])
    {
        lote[i] = (char)toupper((unsigned char)lote[i]);
        i++;
    }

    fprintf(out, "%s\n%s\n%s\n\n", LINEA, lote, LINEA);
}

void emitirCabeceraTabla(FILE *out)
{
    fprintf(out, "%7s %10s %10s %9s %9s %9s %7s %9s %13s %6s %7s\n",
            "capac.", "total(ms)", "us/usuario", "us/posteo", "aciertos", "fallos", "tasa(%)",
            "lecturas", "comparaciones", "altura", "errores");
}

void emitirFila(FILE *out, unsigned capacidad, const tResultado *r, unsigned usuarios)
{
    double accesos = (double)usuarios * LARGO_FEED;

    fprintf(out, "%7u %10.2f %10.2f %9.3f ", capacidad, r->segundos * 1000.0,
            r->segundos * 1e6 / usuarios, r->segundos * 1e6 / accesos);

    if (capacidad == 0)
        fprintf(out, "%9s %9s %7s ", "-", "-", "-");
    else
        fprintf(out, "%9lu %9lu %7.2f ", r->aciertos, r->fallos,
                100.0 * r->aciertos / (r->aciertos + r->fallos));

    fprintf(out, "%9lu ", r->lecturas);

    if (capacidad == 0)
        fprintf(out, "%13s ", "-");
    else
        fprintf(out, "%13lu ", r->comparaciones);

    if (r->hayAltura)
        fprintf(out, "%6u ", r->altura);
    else
        fprintf(out, "%6s ", "-");

    fprintf(out, "%7lu\n", r->errores);
}
