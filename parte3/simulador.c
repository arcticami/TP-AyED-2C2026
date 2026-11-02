#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../parte1/lrucache1.h"
#include "../parte2/lrucache2.h"
#include "tweener.h"
#include "lotes.h"

/*
 * Simulador de uso de Tweener sin cache, con la LRU Cache de la Parte 1 y con la de la Parte 2.
 *
 * Uso:  simulador [semilla [capacidad ...]]
 *
 * Sin argumentos usa la semilla y las capacidades por defecto. Capacidad 0 = sin cache (siempre lee el archivo).
 * Ejemplos:  simulador                 semilla 1, capacidades por defecto
 *            simulador 7               semilla 7, capacidades por defecto
 *            simulador 7 0 50 500      semilla 7, capacidades 0, 50 y 500
 * Pasos:
 *   1) genera posteos.bin y los tres lotes de usuarios (concentrado, disperso y ordenado) con la semilla;
 *   2) para cada lote, cada parte y cada capacidad procesa todos los usuarios de la primera a la ultima: por cada
 *      id de su feed pide el posteo a la cache y, si no esta, lo lee del archivo y lo guarda en la cache;
 *   3) muestra los resultados por pantalla y los guarda en reporte.txt.
 * Los datos de los posteos no se muestran ni se guardan pq son solo para medir.
 */

#define SEMILLA_DEFECTO 1
#define MAX_CAPACIDADES 32
#define REPETICIONES    5       //cada capacidad se corre varias veces y se informa el tiempo promedio
#define SIN_BUFFER      1       //1 = el archivo de posteos se lee sin buffer de stdio
#define ARCHIVO_REPORTE "reporte.txt"
#define CANT_PARTES     2

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

//las dos caches tienen las mismas primitivas pero distinto tipo; esto permite que correr() use cualquiera de las dos
typedef struct
{
    int parte; //1 = lista simple, 2 = lista doble + indice arbol
    t_lru_cache c1;
    t_lru_cache2 c2;
} tCacheSim;

int cmpId(const void *a, const void *b);
int leerPosteo(FILE *fp, int id, tPosteo *p);
//simulaciones de cache
int crearCacheSim(tCacheSim *c, int parte, unsigned capacidad);
int obtenerCacheSim(tCacheSim *c, int id, tPosteo *p);
int agregarCacheSim(tCacheSim *c, int id, const tPosteo *p);
int alturaCacheSim(const tCacheSim *c);
void vaciarCacheSim(tCacheSim *c);
int correr(FILE *fpPost, FILE *fpUsu, int parte, int capacidad, tResultado *r);
int generarLotes(unsigned semilla);
int simularLote(FILE *fpPost, const char *tipo, const int *caps, int cantCaps, FILE *salidas[], int cantSalidas);
void mostrarTitulo(FILE *f, const char *tipo);
void mostrarParte(FILE *f, int parte);
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

    //esto se hace para q la comparacion del sin cache sea justa, como al tener un buffer se traen varios registros de golpe
    //dejarlo haria q parezca q el sin cache es mas rapido pq no se tiene en cuenta la cache propia de C
    //basicamente se le indica al SO q para el archivo fpPost no haga un buffer y por cada llamada vaya directo a él a buscar los datos
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
        fprintf(salidas[i], "\nsemilla  : %u\n", semilla);
        fprintf(salidas[i], "posteos  : %d (registro de %d bytes)\n", CANT_POSTEOS, (int)sizeof(tPosteo));
        fprintf(salidas[i], "usuarios : %d por lote (feed de %d posteos)\n", CANT_USUARIOS, LARGO_FEED);
        fprintf(salidas[i], "tiempo   : promedio de %d corridas; capacidad 0 = sin cache\n", REPETICIONES);
    }

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
    int i, s, parte, ok = 1;

    rutaUsuarios(ruta, tipo);
    if ((fpUsu = fopen(ruta, "rb")) == NULL)
    {
        fprintf(stderr, "Error: no se pudo abrir %s\n", ruta);
        return 0;
    }

    for (s = 0; s < cantSalidas; s++)
        mostrarTitulo(salidas[s], tipo);

    for (parte = 1; ok && parte <= CANT_PARTES; parte++)
    {
        for (s = 0; s < cantSalidas; s++)
        {
            mostrarParte(salidas[s], parte);
            mostrarEncabezado(salidas[s]);
        }

        i = 0;
        while (ok && i < cantCaps)
        {
            ok = correr(fpPost, fpUsu, parte, caps[i], &r);
            if (ok)
            {
                for (s = 0; s < cantSalidas; s++)
                {
                    mostrarFila(salidas[s], caps[i], &r);
                    fflush(salidas[s]);
                }
            }
            else
                fprintf(stderr, "Error durante la simulacion de la Parte %d con capacidad %d\n", parte, caps[i]);
            i++;
        }
    }

    fclose(fpUsu);
    return ok;
}

//compara por id; la clave esta al principio del bloque de la cache, asi que sirve para comparar clave contra bloque
int cmpId(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;

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

/* -------------------------------------------------------------------------- */
/*                 CACHE DE LA SIMULACION (PARTE 1 O PARTE 2)                 */
/* -------------------------------------------------------------------------- */

int crearCacheSim(tCacheSim *c, int parte, unsigned capacidad)
{
    c->parte = parte;

    if (parte == 1)
        return crear_lrucache(&c->c1, capacidad, sizeof(int), cmpId);

    return crear_lrucache2(&c->c2, capacidad, sizeof(int), cmpId);
}

int obtenerCacheSim(tCacheSim *c, int id, tPosteo *p)
{
    if (c->parte == 1)
        return obtener_lrucache(&c->c1, &id, p, sizeof(tPosteo));

    return obtener_lrucache2(&c->c2, &id, p, sizeof(tPosteo));
}

int agregarCacheSim(tCacheSim *c, int id, const tPosteo *p)
{
    if (c->parte == 1)
        return agregar_lrucache(&c->c1, &id, p, sizeof(tPosteo));

    return agregar_lrucache2(&c->c2, &id, p, sizeof(tPosteo));
}

//altura del arbol indice; -1 en la Parte 1, que no tiene arbol
int alturaCacheSim(const tCacheSim *c)
{
    if (c->parte == 1)
        return -1;

    return altura_indice_lrucache2(&c->c2);
}

void vaciarCacheSim(tCacheSim *c)
{
    if (c->parte == 1)
        vaciar_lrucache(&c->c1);
    else
        vaciar_lrucache2(&c->c2);
}

/* -------------------------------------------------------------------------- */
/*                                 SIMULACION                                 */
/* -------------------------------------------------------------------------- */

//una corrida completa con la cache de la parte indicada, repetida REPETICIONES veces;
//devuelve 0 si hubo un error de lectura o de memoria
int correr(FILE *fpPost, FILE *fpUsu, int parte, int capacidad, tResultado *r)
{
    tCacheSim cache;
    tUsuario u;
    tPosteo p;
    double ini, fin, suma = 0;
    int rep = 0, j, id, hit, ok = 1, hayCache = capacidad > 0;

    //la altura del arbol es -1 pq no corresponde
    //pero cambia si hay cache de la parte 2
    r->altura = -1;

    while (ok && rep < REPETICIONES)
    {
        //se pone todo lo del resultado en cero
        r->aciertos = r->fallos = r->lecturas = r->errores = 0;

        //si esta corrida se va a hacer con cache (hay carre) se crea de la parte pedida
        if (hayCache && crearCacheSim(&cache, parte, capacidad) != TODO_BIEN)
            return 0;
        //voy al principio del archivo de usuarios
        rewind(fpUsu);
        //y esto es para guardar cuanto tiempo de procesador uso el programa hasta ahora (para arrancar la medicion desde aca)
        ini = clock();
        //leyendo de a un usuario por vez...
        while (ok && fread(&u, sizeof(tUsuario), 1, fpUsu) == 1)
        {
            j = 0;
            //por cada posteo....
            while (ok && j < LARGO_FEED)
            {
                id = u.feed[j];

                //guardo si hubo acierto o no con el resultado q devuelva obtenercache si corresponde
                hit = hayCache && obtenerCacheSim(&cache, id, &p) == TODO_BIEN;

                if (hit)
                    r->aciertos++;
                else
                {
                    //si entraste a este else es pq algo fallo, si hay cache entonces la falla es q no esta en cache el dato
                    if (hayCache)
                        r->fallos++;
                    //leo el posteo del archivo con acceso directo y cuento una lectura
                    ok = leerPosteo(fpPost, id, &p);
                    r->lecturas++;
                    //si lo pude leer, como estoy aca pq no estaba en cache, lo guardo en cache
                    if (ok && hayCache)
                        ok = agregarCacheSim(&cache, id, &p) == TODO_BIEN;
                }
                //checkeo q el id de lo q lei coincida con el id pedido
                if (ok && p.id != id)
                    r->errores++;
                j++;
            }
        }
        //aca tengo el tiempo de finalizacion de hacer las pasadas de todos los usuarios en 1 repeticion, lo pasoa s egundos
        fin = clock();
        suma += (fin - ini) / CLOCKS_PER_SEC;
        //guardo la altura del arbol y vacio la cache
        if (hayCache)
        {
            r->altura = alturaCacheSim(&cache);
            vaciarCacheSim(&cache);
        }
        rep++;
    }
    //la cantidad de segundos q fui sumando dividido las repeticiones da el tiempo promedio de las mismas
    r->segundos = suma / REPETICIONES;

    return ok;
}

/* -------------------------------------------------------------------------- */
/*                                   SALIDA (son todas cosas de printeo)      */
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

void mostrarParte(FILE *f, int parte)
{
    if (parte == 1)
        fprintf(f, "\nParte 1 - lista simple\n");
    else
        fprintf(f, "\nParte 2 - lista doble + indice arbol\n");
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
