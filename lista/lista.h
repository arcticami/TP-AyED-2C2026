#ifndef LISTA_H
#define LISTA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIN_MEM 1
#define CLA_DUP 2
#define NO_EXISTE 3
#define ERROR_PARAMETROS 4
#define TODO_BIEN 0

typedef struct sNodo
{
    void            *info;
    unsigned        tamInfo;
    struct sNodo    *sig;
} tNodo;

typedef tNodo *tLista;

void crearLista (tLista *p);
int listaVacia (const tLista *p);
//int listaLlena (const tLista *p, unsigned cantBytes); creo que no se usa y da un warning
void vaciarLista (tLista *p);
int ponerAlComienzo(tLista *p, const void *d, unsigned cantBytes); //poner el más reciente usado
int sacarUltimoLista (tLista *p, void *d, unsigned cantBytes); //sacar el más viejo usado
int mostrarLista (const tLista *p, void (*mostrar) (const void *, FILE *), FILE *fp);

//funciones especificas para Cache

tLista * BuscarEnLista (tLista *p,const void* clave,int (*cmp)(const void *, const void *));
void LlevarAPrimeroLista (tLista *p,tLista * mover);
void EliminarNodoLista (tLista *elim);

#endif // LISTA_H
