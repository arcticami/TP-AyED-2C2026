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

unsigned alturaArbol (const tArbol * p)
{
    int hi,hd;

    if(!*p)
    return 0;

    hi= alturaArbolBin (& (*p)->izq);
    hd= alturaArbolBin (& (*p)->der);

    return (hi>hd ? hi : hd) +1;
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
//falta eliminarNodoArbol
