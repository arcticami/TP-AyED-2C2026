#ifndef LISTADOBLE_H_INCLUDED
#define LISTADOBLE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Lista doblemente enlazada generica, con puntero al primero y al ultimo (la usa la LRU Cache de la Parte 2)

#define SIN_MEM 1
#define CLA_DUP 2
#define NO_EXISTE 3
#define ERROR_PARAMETROS 4
#define TODO_BIEN 0

typedef struct sNodoD
{
    void *info;
    unsigned tamInfo;
    struct sNodoD *ant, *sig;
} tNodoD;

typedef struct
{
    tNodoD *pri;
    tNodoD *ult;
} tListaD;

void crearListaD(tListaD *p);
void vaciarListaD(tListaD *p);
int listaVaciaD(const tListaD *p);

int ponerPrimeroListaD(tListaD *p, const void *pd, unsigned cantBytes);
int sacarUltimoListaD(tListaD *p, void *pd, unsigned cantBytes); //con pd en NULL descarta el ultimo sin copiarlo

//funciones especificas para la cache: reciben el nodo directamente, porque con ant y sig no hace falta recorrer
void llevarAPrimeroListaD(tListaD *p, tNodoD *mover);
void eliminarNodoListaD(tListaD *p, tNodoD *elim);

#endif // LISTADOBLE_H_INCLUDED
