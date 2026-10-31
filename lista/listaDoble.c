#include "listaDoble.h"

#define MINIMO(X, Y) ((X) <= (Y) ? (X) : (Y))

//Lista doblemente enlazada, con puntero al primero y al ultimo



void crearListaD(tListaD* p)
{
    p->pri=NULL;
    p->ult=NULL;
}
void vaciarListaD(tListaD* p)
{
    tNodoD* aux, *act= p->pri;

        while(act)
        {
            aux=act->sig;
            free(act->info);
            free(act);
            act=aux;
        }
        p->pri=NULL;
        p->ult=NULL;
}

int listaVaciaD(tListaD* p)
{
    return p->pri==NULL;
}


int ponerPrimeroListaD(tListaD* p, const void* pd, unsigned cantBytes)
{
    tNodoD * aux;

    ///////////////////////////////////////////
    //reservo nodo
    if ((aux = (tNodoD*)malloc(sizeof(tNodoD))) == NULL ||
        (aux->info = malloc(cantBytes)) == NULL)
    {
        free(aux);
        return SIN_MEM;
    }

    //copio info
    memcpy(aux->info, pd, cantBytes);
    aux->tamInfo=cantBytes;
    ///////////////////////////////////////////

    //reengancho
    aux->sig= p->pri; //el siguiente del que inserto es el primero actual
    aux->ant=NULL; //el ant del que inserto es nulo (inserto primero)

    if( p->pri)
        p->pri->ant=aux; //si la lista no estaba vacia, el actualizo el anterior del que estaba primero
    else
        p->ult = aux; //si lista estaba vacia, primero es tambien ultimo

    p->pri=aux; //actualizo el primero en listaD

    return TODO_BIEN;
}

int sacarUltimoListaD(tListaD* p,void* pd, unsigned cantBytes)
{
    tNodoD * aux;

    if(p->ult==NULL)
        return NO_EXISTE;

    aux = p->ult;

    if(aux->ant)
        aux->ant->sig =NULL; //si tenia un anterior, le actualizo el sig del anterior a NULL

    //copio info en pd
    memcpy(pd, aux->info, MINIMO(cantBytes,aux->tamInfo));


    if(aux->ant == NULL) //si era el unico nodo en lista
    {
        p->pri = NULL;
        p->ult = NULL;
    }
    else    //actualizo ultimo
        p->ult=aux->ant;


    //libero memoria
    free(aux->info);
    free(aux);

    return TODO_BIEN;
}

void llevarAPrimeroListaD (tListaD *p, tNodoD *mover)
{
    //si el nodo ya es el primero ya esta en su lugar y salimos sin tocar nada
    if(p == NULL || mover == NULL || p->pri == mover)
        return;

    // desenganchar

        //anterior
    if(mover->ant)//si no es el primero
        mover->ant->sig = mover->sig;
    else
        p->pri = mover->sig;//si es el primero

        //siguiente
    if (mover->sig) //si no es el ultimo
        mover->sig->ant = mover->ant;
    else
        p->ult = mover->ant; //si es el ultimo

    // engancharlo como primero
    mover->ant = NULL;
    mover->sig = p->pri;
    p->pri->ant = mover;
    p->pri = mover;
}

void eliminarNodoListaD (tListaD *p, tNodoD *elim)
{
    if(p == NULL || elim == NULL)
        return;

    // desenganchar

        //anterior
    if(elim->ant)//si no es el primero
        elim->ant->sig = elim->sig;
    else
        p->pri = elim->sig;

        //siguiente
    if (elim->sig) //si no es el ultimo
        elim->sig->ant = elim->ant;
    else
        p->ult = elim->ant; //si es el ultimo


    //liberar nodo
    free(elim->info);
    free (elim);

}
