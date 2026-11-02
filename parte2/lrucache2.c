#include "lrucache2.h"

#define MINIMO(X, Y) ((X) < (Y) ? (X) : (Y))

/* -------------------------------------------------------------------------- */
/*                            FUNCIONES AUXILIARES                            */
/* -------------------------------------------------------------------------- */

void armarEntradaIndice(void *buf, const void *clave, unsigned tamClave, tNodoD *nodo);
tNodoD *nodoDeEntradaIndice(const tArbol *enlace, unsigned tamClave);
void descartarUltimo(t_lru_cache2 *c);

//arma la entrada del indice: la clave y a continuacion el puntero al nodo de la lista
void armarEntradaIndice(void *buf, const void *clave, unsigned tamClave, tNodoD *nodo)
{
    memcpy(buf, clave, tamClave);
    memcpy((char *)buf + tamClave, &nodo, sizeof(tNodoD *)); //con memcpy porque el puntero puede quedar desalineado
}

//devuelve el nodo de la lista al que apunta una entrada del indice
tNodoD *nodoDeEntradaIndice(const tArbol *enlace, unsigned tamClave)
{
    tNodoD *nodo;

    memcpy(&nodo, (char *)(*enlace)->info + tamClave, sizeof(tNodoD *));
    return nodo;
}

//saca el ultimo de la lista y su entrada del indice
void descartarUltimo(t_lru_cache2 *c)
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

int crear_lrucache2(t_lru_cache2 *c, unsigned capacidad, unsigned tamClave, tCmpClave cmp)
{
    if (c == NULL || capacidad == 0 || tamClave == 0 || cmp == NULL)
        return ERROR_PARAMETROS;

    crearListaD(&c->lista);
    crearArbol(&c->indice);

    c->cantidad = 0;
    c->capacidad = capacidad;
    c->tamClave = tamClave;
    c->cmp = cmp;

    return TODO_BIEN;
}

void vaciar_lrucache2(t_lru_cache2 *c)
{
    if (c == NULL)
        return;

    //cada estructura libera lo suyo: la lista los datos [clave|valor], el arbol sus bloques [clave|puntero]
    vaciarListaD(&c->lista);
    vaciarArbol(&c->indice);
    c->cantidad = 0; //capacidad, tamClave y cmp se conservan
}

int agregar_lrucache2(t_lru_cache2 *c, const void *clave, const void *valor, unsigned tamValor)
{
    tArbol *enlace;
    tNodoD *nodo;
    void *info, *entrada;
    unsigned tamInfo, tamEntrada;
    int rc;

    if (c == NULL || clave == NULL || (valor == NULL && tamValor > 0))
        return ERROR_PARAMETROS;

    //armo bloque clave valor
    tamInfo = c->tamClave + tamValor;
    if ((info = malloc(tamInfo)) == NULL)
        return SIN_MEM;

    memcpy(info, clave, c->tamClave);
    if (tamValor > 0)
        memcpy((char *)info + c->tamClave, valor, tamValor);

    //busco la clave en el arbol, si esta:
    if ((enlace = buscarNodoArbol(&c->indice, clave, c->cmp)) != NULL)
    {
        //si el enlace es !null entonces la clave está en el arbol y enlace ahora contiene la ubicacion en la lista del nodo
        //traigo los datos del nodo
        nodo = nodoDeEntradaIndice(enlace, c->tamClave);
        //libero la info vieja q tenia en la lista
        free(nodo->info);
        //actualizo la info
        nodo->info = info;
        //actualizo el tam
        nodo->tamInfo = tamInfo;
        //lo muevo al principio
        llevarAPrimeroListaD(&c->lista, nodo);
        return TODO_BIEN;
    }

    //si no estaba la clave, es un nodo nuevo, mando el nodo a poner primero y este copia los datos
    rc = ponerPrimeroListaD(&c->lista, info, tamInfo);
    //libero el nodo pq se hizo una copia en la primitiva
    free(info);
    if (rc != TODO_BIEN)
        return rc;
    //agarro el primer nodo
    nodo = c->lista.pri;

    //despues al indice, referenciando ese nodo
    tamEntrada = c->tamClave + (unsigned)sizeof(tNodoD *);
    if ((entrada = malloc(tamEntrada)) == NULL)
    {
        eliminarNodoListaD(&c->lista, nodo); //deshacemos para que las dos estructuras sigan consistentes
        return SIN_MEM;
    }
    //armo el bloquecito para poner en el arbol
    armarEntradaIndice(entrada, clave, c->tamClave, nodo);
    //inserto el bloque en el arbol y uso rc como indicador para ver si salio todo bien
    rc = insertarEnArbol(&c->indice, entrada, tamEntrada, c->cmp);
    //libero el bloque pq se inserto una copia en el arbol
    free(entrada);
    if (rc != TODO_BIEN)
    {
        eliminarNodoListaD(&c->lista, nodo);
        return rc;
    }
    //actualizo la cantidad
    c->cantidad++;

    //si nos pasamos de la capacidad se descarta el menos usado (con el nuevo ya insertado no se pierde nada si algo fallo antes)
    if (c->cantidad > c->capacidad)
        descartarUltimo(c);

    return TODO_BIEN;
}

int obtener_lrucache2(t_lru_cache2 *c, const void *clave, void *valor, unsigned tamValor)
{
    tArbol *enlace;
    tNodoD *nodo;
    //check parametros
    if (c == NULL || clave == NULL || (valor == NULL && tamValor > 0))
        return ERROR_PARAMETROS;
    //busco el bloquecito en el arbol
    if ((enlace = buscarNodoArbol(&c->indice, clave, c->cmp)) == NULL)
        return NO_EXISTE;
    //a partir de la ficha o bloquecito q tengo en el arbol,v oy a la lista a buscar el nodo
    nodo = nodoDeEntradaIndice(enlace, c->tamClave);
    //esta funcion puede usarse tambien solo para saber si el nodo esta en al lista sin la necesidad de devolver el dato
    //entonces si quiero devolver el dato, pregunto si me pidieron q el valor sea >0 y lo copio
    if (tamValor > 0)
        memcpy(valor, (char *)nodo->info + c->tamClave, MINIMO(tamValor, nodo->tamInfo - c->tamClave));
    //muevo el nodo al principio
    llevarAPrimeroListaD(&c->lista, nodo); //el indice no cambia: el nodo sigue en la misma direccion

    return TODO_BIEN;
}

int borrar_lrucache2(t_lru_cache2 *c, const void *clave)
{
    tArbol *enlace;
    tNodoD *nodo;
    //chack parametros
    if (c == NULL || clave == NULL)
        return ERROR_PARAMETROS;
    //busco si ta en el arbol
    if ((enlace = buscarNodoArbol(&c->indice, clave, c->cmp)) == NULL)
        return NO_EXISTE;
    //extraigo el dato de la lista
    nodo = nodoDeEntradaIndice(enlace, c->tamClave); //guardamos el puntero ANTES de eliminar la entrada (eliminarNodoArbol libera su info)
    //lo borro del arbol y de la lista y actualizo la cantidad
    eliminarNodoArbol(enlace);
    eliminarNodoListaD(&c->lista, nodo);
    c->cantidad--;

    return TODO_BIEN;
}

unsigned altura_indice_lrucache2(const t_lru_cache2 *c)
{
    if (c == NULL)
        return 0;

    return alturaArbol(&c->indice);
}
