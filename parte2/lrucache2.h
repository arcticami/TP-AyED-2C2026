#ifndef LRUCACHE2_H_INCLUDED
#define LRUCACHE2_H_INCLUDED

#include "../lista/listaDoble.h"
#include "../arbol/arbol_bin_busq.h"

//LRU Cache - Parte 2: mismas primitivas que la Parte 1 (con un 2 al final para que las dos versiones
//puedan estar en el mismo proyecto), implementada con:
//  - una lista doble con los bloques [clave|valor] ordenados por uso (el primero es el mas reciente);
//  - un arbol binario de busqueda por clave que funciona de indice: cada nodo guarda [clave|puntero al nodo de la lista].

#define SIN_MEM 1
#define CLA_DUP 2
#define NO_EXISTE 3
#define ERROR_PARAMETROS 4
#define TODO_BIEN 0

//la Parte 1 define el mismo tipo; asi se pueden incluir los dos headers en el mismo .c
#ifndef T_CMP_CLAVE_DEFINIDO
#define T_CMP_CLAVE_DEFINIDO
typedef int (*tCmpClave)(const void *, const void *); //negativo, cero o positivo segun la primera clave sea menor, igual o mayor
#endif

typedef struct
{
    tListaD lista;
    tArbol indice;
    unsigned capacidad;
    unsigned cantidad;
    unsigned tamClave; //se guarda para poder armar [clave|valor] y [clave|puntero]
    tCmpClave cmp;
} t_lru_cache2;

int crear_lrucache2(t_lru_cache2 *c, unsigned capacidad, unsigned tamClave, tCmpClave cmp);
int agregar_lrucache2(t_lru_cache2 *c, const void *clave, const void *valor, unsigned tamValor);
int obtener_lrucache2(t_lru_cache2 *c, const void *clave, void *valor, unsigned tamValor);
int borrar_lrucache2(t_lru_cache2 *c, const void *clave);
void vaciar_lrucache2(t_lru_cache2 *c); //libera la lista y el indice; capacidad, tamClave y cmp se conservan

//altura del arbol indice (0 si la cache esta vacia)
unsigned altura_indice_lrucache2(const t_lru_cache2 *c);

#endif // LRUCACHE2_H_INCLUDED
