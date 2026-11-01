#ifndef LRUCACHE_H_INCLUDED
#define LRUCACHE_H_INCLUDED

#include "listaDoble.h"
#include "arbol_bin_busq.h"

#define SIN_MEM 1
#define CLA_DUP 2
#define NO_EXISTE 3
#define ERROR_PARAMETROS 4
#define TODO_BIEN 0

typedef int (*tCmpClave)(const void *, const void *);

typedef struct
{
    tListaD     lista;      //compuesto por [clave|valor]
    tArbol      indice;     //compuesto por [clave|puntero al nodo de la lista que tiene el valor]
    unsigned    capacidad;
    unsigned    cantidad;
    unsigned    tamClave;
    tCmpClave   cmp;
} tLruCache;

int CrearLruCache(tLruCache *c, unsigned capMax, unsigned tamClave, tCmpClave cmp); //tamClave se guarda para poder armar [clave|valor]
int AgregarLruCache(tLruCache *c, const void *clave, const void *valor, unsigned tamValor); 
int ObtenerLruCache(tLruCache *c, const void *clave, void *valor, unsigned tamValor);
int BorrarLruCache(tLruCache *c, const void *clave);
void VaciarLruCache(tLruCache *c); //libera lista e indice, la cache queda reutilizable (capacidad, tamClave y cmp se conservan)
unsigned AlturaIndiceLruCache(const tLruCache *c);

#endif // LRUCACHE_H_INCLUDED
