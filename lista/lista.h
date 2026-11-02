#ifndef LISTA_H_INCLUDED
#define LISTA_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Lista simplemente enlazada generica (la usa la LRU Cache de la Parte 1)

#define SIN_MEM 1
#define CLA_DUP 2
#define NO_EXISTE 3
#define ERROR_PARAMETROS 4
#define TODO_BIEN 0

typedef struct sNodo
{
    void *info;
    unsigned tamInfo;
    struct sNodo *sig;
} tNodo;

typedef tNodo *tLista;

void crearLista(tLista *p);
int listaVacia(const tLista *p);
void vaciarLista(tLista *p);
int ponerAlComienzo(tLista *p, const void *d, unsigned cantBytes);
int sacarUltimoLista(tLista *p, void *d, unsigned cantBytes); //con d en NULL descarta el ultimo sin copiarlo
int mostrarLista(const tLista *p, void (*mostrar)(const void *, FILE *), FILE *fp);

//funciones especificas para la cache: trabajan con el enlace (tLista *) que apunta al nodo

tLista *buscarEnLista(tLista *p, const void *clave, int (*cmp)(const void *, const void *)); //NULL si no esta
void llevarAPrimeroLista(tLista *p, tLista *mover);
void eliminarNodoLista(tLista *elim);

#endif // LISTA_H_INCLUDED
