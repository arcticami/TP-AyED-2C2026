#include "lrucache.h"

#define MINIMO(X, Y) ((X) < (Y) ? (X) : (Y))

/* -------------------------------------------------------------------------- */
/*                            FUNCIONES AUXILIARES                            */
/* -------------------------------------------------------------------------- */

void armarEntradaIndice(void *buf, const void *clave, unsigned tamClave, tNodoD *nodo);
tNodoD * nodoDeEntradaIndice(const tArbol *enlace, unsigned tamClave);
void descartarUltimo(tLruCache *c);

//arma la entrada del indice: la clave y a continuacion el puntero al nodo de la lista
void armarEntradaIndice(void *buf, const void *clave, unsigned tamClave, tNodoD *nodo)
{
    memcpy(buf, clave, tamClave);
    memcpy((char *)buf + tamClave, &nodo, sizeof(tNodoD *)); //con memcpy porque el puntero puede quedar desalineado
}

//devuelve el nodo de la lista al que apunta una entrada del indice
tNodoD * nodoDeEntradaIndice(const tArbol *enlace, unsigned tamClave)
{
    tNodoD *nodo;

    memcpy(&nodo, (char *)(*enlace)->info + tamClave, sizeof(tNodoD *));
    return nodo;
}

//saca el ultimo de la lista y su entrada del indice
void descartarUltimo(tLruCache *c)
{
    tNodoD *ult = c->lista.ult;
    tArbol *enlace;

    //la clave esta al principio del info, asi que se usa para buscar en el indice
    //primero se saca del indice y despues de la lista, porque al liberar el nodo se pierde la clave
    if ((enlace = buscarNodoArbol(&c->indice, ult->info, c->cmp)) != NULL)
        eliminarNodoArbol(enlace);

    eliminarNodoListaD(&c->lista, ult);
    c->cantidad--;
}

/* -------------------------------------------------------------------------- */
/*                                 PRIMITIVAS                                 */
/* -------------------------------------------------------------------------- */

int CrearLruCache(tLruCache *c, unsigned capMax, unsigned tamClave, tCmpClave cmp)
{
    if (c == NULL || capMax == 0 || tamClave == 0 || cmp == NULL)
        return ERROR_PARAMETROS;

    crearListaD(&c->lista);
    crearArbol(&c->indice);

    c->cantidad = 0;
    c->capacidad = capMax;
    c->tamClave = tamClave;
    c->cmp = cmp;

    return TODO_BIEN;
}

void VaciarLruCache(tLruCache *c)
{
    if (c == NULL)
        return;

    //cada estructura libera lo suyo: la lista los datos [clave|valor], el arbol sus bloques [clave|puntero]
    vaciarListaD(&c->lista);
    vaciarArbol(&c->indice);
    c->cantidad = 0; //capacidad, tamClave y cmp se conservan
}

int AgregarLruCache(tLruCache *c, const void *clave, const void * valor, unsigned tamValor)
{
    tArbol *enlace;
    tNodoD *nodo;
    void *info, *entrada;
    unsigned tamInfo, tamEntrada;
    int rc;

    if (c == NULL || clave == NULL || (valor == NULL && tamValor > 0))
        return ERROR_PARAMETROS;

    //concatenamos clave y valor
    tamInfo = c->tamClave + tamValor;
    if ((info = malloc(tamInfo)) == NULL)
        return SIN_MEM;

    memcpy(info, clave, c->tamClave);
    if (tamValor > 0)
        memcpy((char *)info + c->tamClave, valor, tamValor);

    //la busqueda se hace en el arbol
    if ((enlace = buscarNodoArbol(&c->indice, clave, c->cmp)) != NULL)
    {
        //la clave ya existia: reemplazamos el dato en el MISMO nodo de la lista, asi el indice sigue apuntando bien
        nodo = nodoDeEntradaIndice(enlace, c->tamClave);
        free(nodo->info);
        nodo->info = info;
        nodo->tamInfo = tamInfo;
        llevarAPrimeroListaD(&c->lista, nodo);
        return TODO_BIEN;
    }

    //clave nueva: primero a la lista (queda como primero)
    rc = ponerPrimeroListaD(&c->lista, info, tamInfo); //copia el info, por eso liberamos el nuestro
    free(info);
    if (rc != TODO_BIEN)
        return rc;

    nodo = c->lista.pri;

    //despues al indice, referenciando ese nodo
    tamEntrada = c->tamClave + (unsigned)sizeof(tNodoD *);
    if ((entrada = malloc(tamEntrada)) == NULL)
    {
        eliminarNodoListaD(&c->lista, nodo); //deshacemos para que las dos estructuras sigan consistentes
        return SIN_MEM;
    }

    armarEntradaIndice(entrada, clave, c->tamClave, nodo);
    rc = insertarEnArbol(&c->indice, entrada, tamEntrada, c->cmp); //copia la entrada
    free(entrada);
    if (rc != TODO_BIEN)
    {
        eliminarNodoListaD(&c->lista, nodo);
        return rc;
    }

    c->cantidad++;

    //si nos pasamos de la capacidad se descarta el menos usado (con el nuevo ya insertado no se pierde nada si algo fallo antes)
    if (c->cantidad > c->capacidad)
        descartarUltimo(c);

    return TODO_BIEN;
}

int ObtenerLruCache(tLruCache *c, const void *clave, void *valor, unsigned tamValor)
{
    tArbol *enlace;
    tNodoD *nodo;

    if (c == NULL || clave == NULL || (valor == NULL && tamValor > 0))
        return ERROR_PARAMETROS;

    if ((enlace = buscarNodoArbol(&c->indice, clave, c->cmp)) == NULL)
        return NO_EXISTE;

    nodo = nodoDeEntradaIndice(enlace, c->tamClave);

    if (tamValor > 0)
        memcpy(valor, (char *)nodo->info + c->tamClave, MINIMO(tamValor, nodo->tamInfo - c->tamClave));

    llevarAPrimeroListaD(&c->lista, nodo); //el indice no cambia: el nodo sigue en la misma direccion

    return TODO_BIEN;
}

int BorrarLruCache(tLruCache *c, const void *clave)
{
    tArbol *enlace;
    tNodoD *nodo;

    if (c == NULL || clave == NULL)
        return ERROR_PARAMETROS;

    if ((enlace = buscarNodoArbol(&c->indice, clave, c->cmp)) == NULL)
        return NO_EXISTE;

    nodo = nodoDeEntradaIndice(enlace, c->tamClave); //guardamos el puntero ANTES de eliminar la entrada (eliminarNodoArbol libera su info)

    eliminarNodoArbol(enlace);
    eliminarNodoListaD(&c->lista, nodo);
    c->cantidad--;

    return TODO_BIEN;
}

unsigned AlturaIndiceLruCache(const tLruCache *c)
{
    if (c == NULL)
        return 0;

    return alturaArbol(&c->indice);
}