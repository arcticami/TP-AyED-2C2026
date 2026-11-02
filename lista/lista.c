#include "lista.h"

#define MINIMO(X, Y) ((X) <= (Y) ? (X) : (Y))

void crearLista(tLista *p)
{
    *p = NULL;
}

int listaVacia(const tLista *p)
{
    return *p == NULL;
}

void vaciarLista(tLista *p)
{
    tNodo *aux;

    while (*p)
    {
        aux = *p;
        *p = aux->sig;
        free(aux->info);
        free(aux);
    }
}

int ponerAlComienzo(tLista *p, const void *d, unsigned cantBytes)
{
    tNodo *nue;

    if ((nue = (tNodo *)malloc(sizeof(tNodo))) == NULL ||
        (nue->info = malloc(cantBytes)) == NULL)
    {
        free(nue);
        return SIN_MEM;
    }

    memcpy(nue->info, d, cantBytes);
    nue->tamInfo = cantBytes;
    nue->sig = *p;
    *p = nue;

    return TODO_BIEN;
}

int sacarUltimoLista(tLista *p, void *d, unsigned cantBytes)
{
    if (*p == NULL)
        return NO_EXISTE;

    while ((*p)->sig)
        p = &(*p)->sig;

    if (d != NULL) //permite descartar el ultimo sin pedir el dato
        memcpy(d, (*p)->info, MINIMO(cantBytes, (*p)->tamInfo));

    free((*p)->info);
    free(*p);
    *p = NULL;

    return TODO_BIEN;
}

int mostrarLista(const tLista *p, void (*mostrar)(const void *, FILE *), FILE *fp)
{
    int cant = 0;

    while (*p)
    {
        mostrar((*p)->info, fp);
        p = &(*p)->sig;
        cant++;
    }

    return cant;
}

/* -------------------------------------------------------------------------- */
/*                     FUNCIONES ESPECIFICAS PARA LA CACHE                    */
/* -------------------------------------------------------------------------- */

//devuelve el enlace que apunta al nodo con esa clave (asi se puede mover o eliminar sin volver a recorrer)
tLista *buscarEnLista(tLista *p, const void *clave, int (*cmp)(const void *, const void *))
{
    while (*p && cmp(clave, (*p)->info) != 0)
        p = &(*p)->sig;

    return *p ? p : NULL;
}

//mover es el enlace que apunta al nodo (el sig del anterior, o p si ya es el primero)
void llevarAPrimeroLista(tLista *p, tLista *mover)
{
    tNodo *aux;

    if (*p == NULL || *mover == NULL)
        return;

    aux = *mover;
    *mover = aux->sig; //el anterior queda enganchado con el siguiente

    aux->sig = *p; //el nodo pasa a ser el primero
    *p = aux;
}

void eliminarNodoLista(tLista *elim)
{
    tNodo *aux;

    if (*elim == NULL)
        return;

    aux = *elim;
    *elim = aux->sig; //el anterior queda enganchado con el siguiente

    free(aux->info);
    free(aux);
}
