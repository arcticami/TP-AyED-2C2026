#ifndef SIMULACIONES_H_INCLUDED
#define SIMULACIONES_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../parte1/lrucache1.h"
#include "../parte2/lrucache2.h"
#include "tweener.h"
#include "lotes.h"

#define SEMILLA_DEFECTO 1
#define MAX_CAPACIDADES 32
#define REPETICIONES    5       //cada capacidad se corre varias veces y se informa el tiempo promedio
#define SIN_BUFFER      1       //1 = el archivo de posteos se lee sin buffer de stdio
#define ARCHIVO_REPORTE "reporte.txt"
#define CANT_PARTES     2


typedef struct
{
    double segundos;            //tiempo total promedio de la simulacion
    int aciertos;
    int fallos;
    int lecturas;               //lecturas hechas al archivo de posteos
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
//funcion core de la simulacion
int correr(FILE *fpPost, FILE *fpUsu, int parte, int capacidad, tResultado *r);
//funcion q recibe la semilla y crea los lotes de posteos y los 3 tipos de usuarios
int generarLotes(unsigned semilla);
//arma la tabla comparativa del reporte por cada tipo de lote
int simularLote(FILE *fpPost, const char *tipo, const int *caps, int cantCaps, FILE *salidas[], int cantSalidas);
//funciones de impresion
void mostrarTitulo(FILE *f, const char *tipo);
void mostrarParte(FILE *f, int parte);
void mostrarEncabezado(FILE *f);
void mostrarFila(FILE *f, int capacidad, const tResultado *r);

#endif // SIMULACIONES_H_INCLUDED
