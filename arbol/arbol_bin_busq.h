#ifndef ARBOL_BIN_BUSQ_H_INCLUDED
#define ARBOL_BIN_BUSQ_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
int insertarEnArbol(tArbol *pa, const void *d, unsigned tam, int (*cmp)(const void *, const void *));
tArbol *buscarNodoArbol(tArbol *pa, const void *d, int (*cmp)(const void *, const void *)); //devuelve el tArbol * , no te pasa una copia del dato
unsigned alturaArbol(const tArbol * p);
void vaciarArbol(tArbol *pa);
void eliminarNodoArbol(tArbol *pa);

#endif // ARBOL_BIN_BUSQ_H_INCLUDED
