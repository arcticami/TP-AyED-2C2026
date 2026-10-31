#include "arbol_bin_busq.h"

void crearArbol(tArbol *pa)
{
    *pa = NULL;
}

int insertarEnArbolRecursiva( tArbol *pa, const void *d,unsigned tam, int(*cmp)(const void * , const void *)) //p o pa, es lo mismo
{

    tNodoArbol *nue;
    int rc;

    if(*pa)
    {
        if((rc=cmp(d,(*pa)->info) )<0) //primero si es mas chico, luego si es mas grande y luego si es duplicado (poco probable)
        {
           return insertarEnArbolRecursiva(&(*pa)->izq,d,tam,cmp); // nos movemos hacia el hijo de la izquierda
        }
        else if (rc>0)
            return insertarEnArbolRecursiva(&(*pa)->der,d,tam,cmp); // nos movemos hacia el hijo de la izquierda

            else
                return CLA_DUP; // o tratamiento de duplicados
    }

    //reservar y chequear memoria
    if ((nue = (tNodoArbol *)malloc(sizeof(tNodoArbol))) == NULL || (nue->info = malloc(tam)) == NULL)
        {
            free(nue);
            return SIN_MEM;
        }

    //copiamos el dato
    nue->tamInfo = tam;
    memcpy(nue->info,d,tam);


    // enganchamos el nodo nuevo
    nue->izq = nue->der = NULL; // o *pa

    *pa=nue; //ponemos en el padre que su hijo (izq - der) es nue

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

tArbol * buscarNodoArbol(tArbol *pa, const void *d, int (*cmp)(const void *, const void *))
{
    int rc;

    if (*pa == NULL)
        return NULL;

    if ((rc = cmp(d, (*pa)->info)) < 0)
        return buscarNodoArbol(&(*pa)->izq, d, cmp);
    if (rc > 0)
        return buscarNodoArbol(&(*pa)->der, d, cmp);
    return pa;
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
