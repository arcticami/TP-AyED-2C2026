#ifndef LRUCACHE_H_INCLUDED
#define LRUCACHE_H_INCLUDED

#include "lista.h"

#define SIN_MEM 1
#define CLA_DUP 2
#define NO_EXISTE 3
#define ERROR_PARAMETROS 4
#define TODO_BIEN 0

typedef int (*tCmpClave)(const void *, const void *);


typedef struct
{
    tLista      l;      // el primero es el mas recientemente usado
    unsigned    capacidad;
    unsigned    cantidad;
    unsigned   tamClave;
    tCmpClave   cmp;
} tLruCache;



int CrearLruCache (tLruCache *c,unsigned capMax,unsigned tamClave, tCmpClave cmp); //pide el tam clave para ya dejarlo definido en LtuCache, es para agregar y obtener

int AgregarLruCache (tLruCache *c,const void *clave, const void * valor, unsigned tamValor); //inserta al principio si no está, clave duplicada actualiza valor y mueve hacia adelante. el cmp lo hace con clave (para buscar). Si cache estaba llena, elimina ultimo

int ObtenerLruCache (tLruCache *c,const void *clave, void * valor, unsigned tamValor);//busca en la cache, si lo encuentra lo mueve adelante, y devuelve el valor (en *valor), min tam y tam en cache.

int BorrarLruCache (tLruCache *c,const void *clave); //Busca por clave, si lo encuentra simplemente lo borra, no lo devuelve.

void VaciarLruCache (tLruCache *c); //Libera la lista; la cache queda reutilizable (capacidad, tamClave y cmp se conservan) . Revisar si es asi o no

#endif // LRUCACHE_H_INCLUDED
