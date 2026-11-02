#ifndef LRUCACHE1_H_INCLUDED
#define LRUCACHE1_H_INCLUDED

#include "../lista/lista.h"

//LRU Cache - Parte 1: implementada con una lista simplemente enlazada.
//Cada nodo guarda un bloque [clave|valor]; el primero es el usado mas recientemente y el ultimo el menos usado.

#define SIN_MEM 1
#define CLA_DUP 2
#define NO_EXISTE 3
#define ERROR_PARAMETROS 4
#define TODO_BIEN 0

//la Parte 2 define el mismo tipo; asi se pueden incluir los dos headers en el mismo .c
#ifndef T_CMP_CLAVE_DEFINIDO
#define T_CMP_CLAVE_DEFINIDO
typedef int (*tCmpClave)(const void *, const void *); //negativo, cero o positivo segun la primera clave sea menor, igual o mayor
#endif

typedef struct
{
    tLista l;
    unsigned capacidad;
    unsigned cantidad;
    unsigned tamClave; //se guarda para poder armar y desarmar el bloque [clave|valor]
    tCmpClave cmp;
} t_lru_cache;

//devuelve ERROR_PARAMETROS si capacidad o tamClave son 0 o cmp es NULL
int crear_lrucache(t_lru_cache *c, unsigned capacidad, unsigned tamClave, tCmpClave cmp);

//agrega al principio; si la clave ya estaba reemplaza el valor y la lleva al principio.
//Si con el nuevo elemento se pasa de la capacidad, elimina el ultimo
int agregar_lrucache(t_lru_cache *c, const void *clave, const void *valor, unsigned tamValor);

//si la encuentra la lleva al principio y copia en valor hasta tamValor bytes del valor guardado; sino devuelve NO_EXISTE
int obtener_lrucache(t_lru_cache *c, const void *clave, void *valor, unsigned tamValor);

//elimina el elemento con esa clave; si no esta devuelve NO_EXISTE
int borrar_lrucache(t_lru_cache *c, const void *clave);

//libera todos los elementos; capacidad, tamClave y cmp se conservan, asi que la cache se puede volver a usar
void vaciar_lrucache(t_lru_cache *c);

#endif // LRUCACHE1_H_INCLUDED
