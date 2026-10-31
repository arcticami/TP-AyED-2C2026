#include "arbol_bin_busq.h"

void crearArbol(tArbol *pa)
{
    *pa = NULL;
}

int insertarEnArbol(tArbol *pa, const void *d, unsigned tam, int (*cmp)(const void *, const void *))
{
    tNodoArbol *nue;
    int rc;

    //bajamos hasta encontrar el enlace vacio donde va el nodo (pa siempre apunta al enlace, no al nodo)
    while (*pa)
    {
        if ((rc = cmp(d, (*pa)->info)) < 0)
            pa = &(*pa)->izq;
        else if (rc > 0)
            pa = &(*pa)->der;
        else
            return CLA_DUP;
    }

    //reservar y chequear memoria
    if ((nue = (tNodoArbol *)malloc(sizeof(tNodoArbol))) == NULL || (nue->info = malloc(tam)) == NULL)
    {
        free(nue);
        return SIN_MEM;
    }

    //copiamos el dato y enganchamos el nodo nuevo
    nue->tamInfo = tam;
    memcpy(nue->info, d, tam);
    nue->izq = nue->der = NULL;

    *pa = nue; //el enlace que estaba vacio ahora apunta al nodo nuevo

    return TODO_BIEN;
}

unsigned alturaArbol(const tArbol * p)
{
    unsigned hi, hd;

    if(!*p)
        return 0;

    hi = alturaArbol(&(*p)->izq);
    hd = alturaArbol(&(*p)->der);

    return (hi > hd ? hi : hd) + 1;
}

tArbol *buscarNodoArbol(tArbol *pa, const void *d, int (*cmp)(const void *, const void *))
{
    int rc;

    while (*pa && (rc = cmp(d, (*pa)->info)) != 0)
        pa = (rc < 0) ? &(*pa)->izq : &(*pa)->der;

    return *pa ? pa : NULL;
}

void vaciarArbol(tArbol *pa) //pensar si iterativa o recursiva
{
    if (*pa == NULL)
        return;

    //recorrido posorden

    vaciarArbol(&(*pa)->izq);
    vaciarArbol(&(*pa)->der);

    free((*pa)->info);
    free(*pa);
    *pa = NULL;
}

void eliminarNodoArbol(tArbol *pa)
{
    tNodoArbol *aux, *suc;
    tArbol *menor;

    if (*pa == NULL)
        return;

    aux = *pa;

    if (aux->izq == NULL)       //0 hijos o solo hijo derecho: el hijo (o NULL) ocupa su lugar
        *pa = aux->der;
    else if (aux->der == NULL)  //solo hijo izquierdo
        *pa = aux->izq;
    else                        //2 hijos: lo reemplaza el menor del subarbol derecho (su sucesor)
    {
        menor = &aux->der;
        while ((*menor)->izq)
            menor = &(*menor)->izq;

        suc = *menor;
        *menor = suc->der;      //el sucesor se desengancha y su hijo derecho ocupa su lugar
        suc->izq = aux->izq;    //el sucesor hereda los dos hijos del nodo eliminado
        suc->der = aux->der;
        *pa = suc;
    }

    free(aux->info);
    free(aux);
}
