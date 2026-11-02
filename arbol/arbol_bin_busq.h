#ifndef ARBOL_BIN_BUSQ_H_INCLUDED
#define ARBOL_BIN_BUSQ_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Arbol binario de busqueda generico (es el indice de la LRU Cache de la Parte 2).
//Las funciones reciben el enlace (tArbol *) y no el nodo, asi pueden modificar el puntero que lo apunta.

#define SIN_MEM 1
#define CLA_DUP 2
#define NO_EXISTE 3
#define ERROR_PARAMETROS 4
#define TODO_BIEN 0

typedef struct sNodoArbol
{
    void *info;
    unsigned tamInfo;
    struct sNodoArbol *izq;
    struct sNodoArbol *der;
} tNodoArbol;

typedef tNodoArbol *tArbol;

void crearArbol(tArbol *pa);
int insertarEnArbol(tArbol *pa, const void *d, unsigned tam, int (*cmp)(const void *, const void *)); //CLA_DUP si ya estaba
tArbol *buscarNodoArbol(tArbol *pa, const void *d, int (*cmp)(const void *, const void *)); //devuelve el enlace al nodo, o NULL
unsigned alturaArbol(const tArbol *pa);
void vaciarArbol(tArbol *pa);
void eliminarNodoArbol(tArbol *pa); //pa es el enlace al nodo a eliminar (el que devuelve buscarNodoArbol)

#endif // ARBOL_BIN_BUSQ_H_INCLUDED
