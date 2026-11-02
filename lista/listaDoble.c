#include "listaDoble.h"

#define MINIMO(X, Y) ((X) <= (Y) ? (X) : (Y))

void crearListaD(tListaD *p)
{
    p->pri = NULL;
    p->ult = NULL;
}

void vaciarListaD(tListaD *p)
{
    tNodoD *aux, *act = p->pri;

    while (act)
    {
        aux = act->sig;
        free(act->info);
        free(act);
        act = aux;
    }

    p->pri = NULL;
    p->ult = NULL;
}

int listaVaciaD(const tListaD *p)
{
    return p->pri == NULL;
}

int ponerPrimeroListaD(tListaD *p, const void *pd, unsigned cantBytes)
{
    tNodoD *aux;

    if ((aux = (tNodoD *)malloc(sizeof(tNodoD))) == NULL ||
        (aux->info = malloc(cantBytes)) == NULL)
    {
        free(aux);
        return SIN_MEM;
    }

    memcpy(aux->info, pd, cantBytes);
    aux->tamInfo = cantBytes;

    aux->sig = p->pri; //el siguiente del nuevo es el que estaba primero
    aux->ant = NULL;   //y no tiene anterior porque va primero

    if (p->pri)
        p->pri->ant = aux; //si la lista no estaba vacia, el que estaba primero ahora tiene anterior
    else
        p->ult = aux; //si estaba vacia, el nuevo tambien es el ultimo

    p->pri = aux;

    return TODO_BIEN;
}

int sacarUltimoListaD(tListaD *p, void *pd, unsigned cantBytes)
{
    tNodoD *aux;

    if (p->ult == NULL)
        return NO_EXISTE;

    aux = p->ult;

    if (pd != NULL) //permite descartar el ultimo sin pedir el dato
        memcpy(pd, aux->info, MINIMO(cantBytes, aux->tamInfo));

    if (aux->ant)
    {
        aux->ant->sig = NULL; //el anterior pasa a ser el ultimo
        p->ult = aux->ant;
    }
    else //era el unico nodo
    {
        p->pri = NULL;
        p->ult = NULL;
    }

    free(aux->info);
    free(aux);

    return TODO_BIEN;
}

void llevarAPrimeroListaD(tListaD *p, tNodoD *mover)
{
    //si ya es el primero esta en su lugar y no hay que tocar nada
    if (p == NULL || mover == NULL || p->pri == mover)
        return;

    //desenganchar (no es el primero, asi que seguro tiene anterior)
    mover->ant->sig = mover->sig;

    if (mover->sig) //si no es el ultimo
        mover->sig->ant = mover->ant;
    else
        p->ult = mover->ant;

    //engancharlo como primero
    mover->ant = NULL;
    mover->sig = p->pri;
    p->pri->ant = mover;
    p->pri = mover;
}

void eliminarNodoListaD(tListaD *p, tNodoD *elim)
{
    if (p == NULL || elim == NULL)
        return;

    //desenganchar
    if (elim->ant) //si no es el primero
        elim->ant->sig = elim->sig;
    else
        p->pri = elim->sig;

    if (elim->sig) //si no es el ultimo
        elim->sig->ant = elim->ant;
    else
        p->ult = elim->ant;

    free(elim->info);
    free(elim);
}
