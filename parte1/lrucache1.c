#include "lrucache.h"

#define MINIMO(a, b) ((a) < (b) ? (a) : (b))

int CrearLruCache (tLruCache *c,unsigned capMax,unsigned tamClave, tCmpClave cmp)
{

    if( capMax==0 || tamClave == 0 || cmp == NULL)
        return ERROR_PARAMETROS;

    crearLista(&(c->l));

    c->cantidad = 0;
    c->capacidad=capMax;
    c->tamClave= tamClave;
    c->cmp = cmp;

    return TODO_BIEN;
}

void VaciarLruCache (tLruCache *c)
{
    if( c == NULL)
        return ;

    vaciarLista(&(c->l));
    c->cantidad = 0; // solo estos dos, la cache queda reutilizable
}

int AgregarLruCache (tLruCache *c,const void *clave, const void * valor, unsigned tamValor) //inserta al principio si no está, clave duplicada actualiza valor y mueve hacia adelante. el cmp lo hace con clave (para buscar). Si cache estaba llena, elimina ultimo
{
    int resultado;
    tLista * aux;
    void * info;

    if(c== NULL || clave ==NULL )
        return ERROR_PARAMETROS;

        //hay que concatenar clave y valor

    if((info=malloc( c->tamClave + tamValor))==NULL)
        return SIN_MEM;

    memcpy(info,clave,(size_t) c->tamClave ); //copiamos clave
    memcpy( (char *)info + c->tamClave ,valor,(size_t) tamValor ); //concatenamos valor



    aux = BuscarEnLista(&(c->l),clave,c->cmp);

    if(aux==NULL)
    {
        //no estaba en cache


        //si la cache estaba llena hay que borrar el ultimo
        if(c->cantidad >= c->capacidad) //solo deberia poder ser igual
        {
            sacarUltimoLista(&(c->l),NULL,0); // le paso una variable cualquiera para que el memcpy no rompa
            c->cantidad --;
        }


        //insertamos nuevo

        resultado = ponerAlComienzo(&(c->l),info,c->tamClave + tamValor);
        free (info);
        if (resultado== TODO_BIEN)
            c->cantidad++;

        return resultado;
    }
    free( (*aux)->info );
    (*aux)->info = info;
    (*aux)->tamInfo = c->tamClave + tamValor;


    LlevarAPrimeroLista(&(c->l),aux);
    return TODO_BIEN;
}

int ObtenerLruCache (tLruCache *c,const void *clave, void * valor, unsigned tamValor)
{
    tLista * aux;

    if(c== NULL || clave ==NULL || valor ==NULL )
        return ERROR_PARAMETROS;

    aux = BuscarEnLista(&(c->l),clave,c->cmp);

    if(aux ==NULL)
            return NO_EXISTE;

    memcpy(valor,(char*) (*aux)->info + c->tamClave, MINIMO(tamValor, (*aux)->tamInfo - c->tamClave)); //devuelve valor

    LlevarAPrimeroLista(&(c->l),aux); //mueve hacia adelante de todo

    return TODO_BIEN;
}

int BorrarLruCache (tLruCache *c,const void *clave)
{
    tLista * aux;

    if(c== NULL || clave ==NULL)
        return ERROR_PARAMETROS;

    aux = BuscarEnLista(&(c->l),clave,c->cmp);

    if(aux == NULL)
        return NO_EXISTE;

    EliminarNodoLista(aux);
    c->cantidad--;

    return TODO_BIEN;
}
