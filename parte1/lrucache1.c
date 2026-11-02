#include "lrucache1.h"

#define MINIMO(a, b) ((a) < (b) ? (a) : (b))


int crear_lrucache(t_lru_cache *c, unsigned capacidad, unsigned tamClave, tCmpClave cmp)
{
    //valida parametros
    if (c == NULL || capacidad == 0 || tamClave == 0 || cmp == NULL)
        return ERROR_PARAMETROS;
    //deja la lista vacia
    crearLista(&c->l);
    //rellena los parametros
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
    //uso la primitiva
    vaciarLista(&c->l);
    //capacidad, tamClave y cmp se conservan pero la cantidad se actualiza
    c->cantidad = 0;
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

    //busca en lista x la clave
    aux = buscarEnLista(&c->l, clave, c->cmp);

    if (aux != NULL)
    {
        //si devolvio algo distinto de null: la clave ya estaba, reemplazamos el bloque y lo llevamos al principio
        //primero libero el bloque de info viejo
        free((*aux)->info);
        //ahora copio el bloque nuevo en donde estaba el viejo
        (*aux)->info = info;
        //actualizo el tamaño de la info
        (*aux)->tamInfo = c->tamClave + tamValor;
        //ahora muevo el nodo al principio
        llevarAPrimeroLista(&c->l, aux);
        return TODO_BIEN;
    }

    //si no lo encontro entonces hay q insertar un dato nuevo, uso resultado como indicador de q la operacion salió bien
    resultado = ponerAlComienzo(&c->l, info, c->tamClave + tamValor);
    //ponerAlComienzo guarda su propia copia del dato asi q puedo liberar el bloque nuevo q armé
    free(info);
    //si algo fallo, retorno
    if (resultado != TODO_BIEN)
        return resultado;
    //si todo salio bien, la cantidad de datos en la cache aumenta
    c->cantidad++;
    //hay q verificar si con el nuevo la cache se desborda, se elimina el ultimo (el menos usado)
    if (c->cantidad > c->capacidad)
    {
        //no necesito traerme el dato
        sacarUltimoLista(&c->l, NULL, 0);
        //la actualizo pq esta excedida
        c->cantidad--;
    }

    return TODO_BIEN;
}

int obtener_lrucache(t_lru_cache *c, const void *clave, void *valor, unsigned tamValor)
{
    tLista *aux;
    //valido parametros
    if (c == NULL || clave == NULL || valor == NULL)
        return ERROR_PARAMETROS;
    //busco la clave en la lista y si no esta retorno
    if ((aux = buscarEnLista(&c->l, clave, c->cmp)) == NULL)
        return NO_EXISTE;
    //si pase de aca entonces la clave esta, necesito su valor
    //el valor esta despues de la clave; se copia lo que entre en el buffer
    memcpy(valor, (char *)(*aux)->info + c->tamClave, MINIMO(tamValor, (*aux)->tamInfo - c->tamClave));
    //lo pongo primero pq es el ultimo q se accedio
    llevarAPrimeroLista(&c->l, aux);

    return TODO_BIEN;
}

int borrar_lrucache(t_lru_cache *c, const void *clave)
{
    tLista *aux;

    if (c == NULL || clave == NULL)
        return ERROR_PARAMETROS;
    //busco la clave en la lista, si no esta no hay nada q borrar
    if ((aux = buscarEnLista(&c->l, clave, c->cmp)) == NULL)
        return NO_EXISTE;
    //si esta, lo borro
    eliminarNodoLista(aux);
    //actualizo la cantidad
    c->cantidad--;

    return TODO_BIEN;
}
