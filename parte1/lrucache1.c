#include "lrucache1.h"

#define MINIMO(a, b) ((a) < (b) ? (a) : (b))

int crear_lrucache(t_lru_cache *c, unsigned capacidad, unsigned tamClave, tCmpClave cmp)
{
    if (c == NULL || capacidad == 0 || tamClave == 0 || cmp == NULL)
        return ERROR_PARAMETROS;

    crearLista(&c->l);

    c->cantidad = 0;
    c->capacidad = capacidad;
    c->tamClave = tamClave;
    c->cmp = cmp;

    return TODO_BIEN;
}

void vaciar_lrucache(t_lru_cache *c)
{
    if (c == NULL)
        return;

    vaciarLista(&c->l);
    c->cantidad = 0; //capacidad, tamClave y cmp se conservan
}

int agregar_lrucache(t_lru_cache *c, const void *clave, const void *valor, unsigned tamValor)
{
    tLista *aux;
    void *info;
    int resultado;

    if (c == NULL || clave == NULL)
        return ERROR_PARAMETROS;

    //concatenamos clave y valor en un solo bloque
    if ((info = malloc(c->tamClave + tamValor)) == NULL)
        return SIN_MEM;

    memcpy(info, clave, c->tamClave);
    memcpy((char *)info + c->tamClave, valor, tamValor);

    aux = buscarEnLista(&c->l, clave, c->cmp);

    if (aux != NULL)
    {
        //la clave ya estaba: reemplazamos el bloque y lo llevamos al principio
        free((*aux)->info);
        (*aux)->info = info;
        (*aux)->tamInfo = c->tamClave + tamValor;

        llevarAPrimeroLista(&c->l, aux);
        return TODO_BIEN;
    }

    //clave nueva: primero se inserta y despues se descarta, como pide el enunciado
    //(asi, si ponerAlComienzo falla, no se pierde el ultimo)
    resultado = ponerAlComienzo(&c->l, info, c->tamClave + tamValor);
    free(info); //ponerAlComienzo guarda su propia copia
    if (resultado != TODO_BIEN)
        return resultado;

    c->cantidad++;

    //si con el nuevo la cache se desborda, se elimina el ultimo (el menos usado)
    if (c->cantidad > c->capacidad)
    {
        sacarUltimoLista(&c->l, NULL, 0); //el dato no nos interesa
        c->cantidad--;
    }

    return TODO_BIEN;
}

int obtener_lrucache(t_lru_cache *c, const void *clave, void *valor, unsigned tamValor)
{
    tLista *aux;

    if (c == NULL || clave == NULL || valor == NULL)
        return ERROR_PARAMETROS;

    if ((aux = buscarEnLista(&c->l, clave, c->cmp)) == NULL)
        return NO_EXISTE;

    //el valor esta despues de la clave; se copia lo que entre en el buffer
    memcpy(valor, (char *)(*aux)->info + c->tamClave, MINIMO(tamValor, (*aux)->tamInfo - c->tamClave));

    llevarAPrimeroLista(&c->l, aux);

    return TODO_BIEN;
}

int borrar_lrucache(t_lru_cache *c, const void *clave)
{
    tLista *aux;

    if (c == NULL || clave == NULL)
        return ERROR_PARAMETROS;

    if ((aux = buscarEnLista(&c->l, clave, c->cmp)) == NULL)
        return NO_EXISTE;

    eliminarNodoLista(aux);
    c->cantidad--;

    return TODO_BIEN;
}
