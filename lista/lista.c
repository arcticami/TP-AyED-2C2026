#include "lista.h"

#define MINIMO(X, Y) ((X) <= (Y) ? (X) : (Y))

void crearLista (tLista *p)
{
    *p = NULL;
}

int listaVacia (const tLista *p)
{
    return *p == NULL;
}


void vaciarLista (tLista *p)
{
    while (*p)
    {
        tNodo *aux = *p;
        *p = aux->sig;
        free(aux->info);
        free(aux);
    }
}

int ponerAlComienzo(tLista *p, const void *d, unsigned cantBytes)
{
    tNodo *nue;

    if ((nue = (tNodo*)malloc(sizeof(tNodo))) == NULL ||
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

int sacarUltimoLista (tLista *p, void *d, unsigned cantBytes)
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


int mostrarLista (const tLista *p,
                 void (*mostrar) (const void *, FILE *), FILE *fp)
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

///funciones especificas para Cache


tLista * BuscarEnLista (tLista *p,const void * clave ,int (*cmp)(const void *, const void *))
{
    int aux;
    if (*p ==NULL)
        return NULL;

    while( *p && (aux = cmp(clave,(*p)->info))!=0 )
        p=&(*p)->sig;

    if (aux == 0)
        return p;

    return NULL;
}

void LlevarAPrimeroLista (tLista *p,tLista * mover)
{
    tNodo * aux;

    if (*p==NULL || *mover ==NULL)
        return;

    aux = *mover ;
    *mover = (*mover)->sig;//reengancha nodo anterior con siguiente


     aux->sig = *p; //reengancha poniendo en primer nodo
    *p = aux; //hace que la lista empieze en el nuevo primero


}

void EliminarNodoLista (tLista *elim)
{
    tNodo * aux;

    if (*elim==NULL)
        return;

    aux = *elim;
    *elim = (*elim)->sig; //re engancha nodo anterior con siguiente

    free(aux->info); //libera
    free(aux);

}
