#ifndef LISTADOBLE_H_INCLUDED
#define LISTADOBLE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIN_MEM 1
#define CLA_DUP 2
#define NO_EXISTE 3
#define ERROR_PARAMETROS 4
#define TODO_BIEN 0

//Lista doblemente enlazada, con puntero al primero y al ultimo

typedef struct sNodo
{
    void *info;
    unsigned tamInfo;
    struct sNodo *ant,*sig;
} tNodoD;

typedef struct
{
    tNodoD * pri;
    tNodoD * ult;
} tListaD;


void crearListaD(tListaD* p);
void vaciarListaD(tListaD* p);
int listaVaciaD(tListaD* p);

int ponerPrimeroListaD(tListaD* p, const void* pd, unsigned cantBytes);
int sacarUltimoListaD(tListaD* p,void* pd, unsigned cantBytes);

//funciones nuevas especificas para cache
void llevarAPrimeroListaD (tListaD *p, tNodoD *mover); //se puede con Nodo ya que si sabes cual es ant y sig
void eliminarNodoListaD (tListaD *p, tNodoD *elim);

/*
void mostrarIzqADerListaD(tListaD* p);
void mostrarDerAIzqListaD(tListaD* p);

int sacarPorClaveListaD(tListaD* p, void* pd, unsigned cantBytes,int (*Cmp)(const void*, const void*));

void invertirListaD(tListaD *p);
*/

#endif // LISTADOBLE_H_INCLUDED
