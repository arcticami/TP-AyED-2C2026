/*
 * Lote de pruebas de la API publica de la cache (NO es parte de la entrega).
 *
 * Un unico .c para Parte 1 y Parte 2: se compila dos veces, cada una contra su
 * propio lrucache.h. Solo usa las primitivas publicas, nunca los campos internos.
 *
 * Conteo de memoria: se linkea con -Wl,--wrap=malloc,--wrap=free, asi todo
 * malloc/free que hacen la cache, la lista y el arbol pasa por las funciones
 * __wrap_* de abajo (sin tocar sus fuentes). Con eso se detectan fugas y se
 * simulan fallas de memoria.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lrucache.h"

/* ------------------------------------------------------------------ memoria */

void *__real_malloc(size_t n);
void __real_free(void *p);

static long g_vivos = 0;       /* bloques pedidos y todavia no liberados */
static long g_base = 0;        /* bloques que ya habia antes de main (los pide la CRT) */
static long g_cont = 0;        /* mallocs desde el ultimo armarFalla */
static long g_falla_en = 0;    /* 0 = nunca falla; N = el N-esimo malloc devuelve NULL */

void *__wrap_malloc(size_t n)
{
    void *p;

    g_cont++;
    if (g_falla_en > 0 && g_cont == g_falla_en)
        return NULL;
    p = __real_malloc(n);
    if (p)
        g_vivos++;
    return p;
}

void __wrap_free(void *p)
{
    if (p)
        g_vivos--;
    __real_free(p);
}

static long vivos(void)
{
    return g_vivos - g_base;
}

static void armarFalla(long n)
{
    g_cont = 0;
    g_falla_en = n;
}

/* ------------------------------------------------------------------ chequeos */

static int g_checks = 0;
static int g_fallas = 0;

#define CHECK(cond, ...) \
    do { \
        g_checks++; \
        if (!(cond)) { \
            g_fallas++; \
            printf("  FALLA (linea %d): ", __LINE__); \
            printf(__VA_ARGS__); \
            printf("\n"); \
        } \
    } while (0)

static void titulo(const char *t)
{
    printf("[%s]\n", t);
}

/* ------------------------------------------------------------------ helpers */

static int cmpInt(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;

    return (x > y) - (x < y);
}

static int cmpCad(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

static int agregarInt(tLruCache *c, int k, int v)
{
    return AgregarLruCache(c, &k, &v, sizeof(v));
}

/* devuelve el codigo de Obtener; el valor queda en *v (si lo encontro) */
static int obtenerInt(tLruCache *c, int k, int *v)
{
    return ObtenerLruCache(c, &k, v, sizeof(*v));
}

static int borrarInt(tLruCache *c, int k)
{
    return BorrarLruCache(c, &k);
}

/* 1 si esta. OJO: si esta, lo lleva al principio (efecto de Obtener) */
static int estaInt(tLruCache *c, int k)
{
    int v;

    return obtenerInt(c, k, &v) == TODO_BIEN;
}

/* ------------------------------------------------------------------ pruebas */

static void testParametros(void)
{
    tLruCache c;

    titulo("crear: parametros invalidos");
    CHECK(CrearLruCache(&c, 0, sizeof(int), cmpInt) == ERROR_PARAMETROS, "capacidad 0");
    CHECK(CrearLruCache(&c, 3, 0, cmpInt) == ERROR_PARAMETROS, "tamClave 0");
    CHECK(CrearLruCache(&c, 3, sizeof(int), NULL) == ERROR_PARAMETROS, "cmp NULL");
    CHECK(CrearLruCache(&c, 3, sizeof(int), cmpInt) == TODO_BIEN, "crear valido");
    VaciarLruCache(&c);
}

static void testVacia(void)
{
    tLruCache c;
    int v = 123;

    titulo("cache vacia");
    CrearLruCache(&c, 3, sizeof(int), cmpInt);
    CHECK(obtenerInt(&c, 1, &v) == NO_EXISTE, "obtener en vacia");
    CHECK(v == 123, "obtener inexistente no debe tocar el destino (v=%d)", v);
    CHECK(borrarInt(&c, 1) == NO_EXISTE, "borrar en vacia");
    VaciarLruCache(&c);
    CHECK(vivos() == 0, "vaciar una cache vacia deja %ld bloques", vivos());
}

static void testBasico(void)
{
    tLruCache c;
    int v = 0;

    titulo("agregar y obtener");
    CrearLruCache(&c, 3, sizeof(int), cmpInt);
    CHECK(agregarInt(&c, 1, 10) == TODO_BIEN, "agregar 1");
    CHECK(agregarInt(&c, 2, 20) == TODO_BIEN, "agregar 2");
    CHECK(agregarInt(&c, 3, 30) == TODO_BIEN, "agregar 3");
    CHECK(obtenerInt(&c, 1, &v) == TODO_BIEN && v == 10, "obtener 1 -> %d", v);
    CHECK(obtenerInt(&c, 2, &v) == TODO_BIEN && v == 20, "obtener 2 -> %d", v);
    CHECK(obtenerInt(&c, 3, &v) == TODO_BIEN && v == 30, "obtener 3 -> %d", v);
    CHECK(obtenerInt(&c, 9, &v) == NO_EXISTE, "obtener 9");
    VaciarLruCache(&c);
    CHECK(vivos() == 0, "fuga tras vaciar: %ld", vivos());
}

static void testDesborde(void)
{
    tLruCache c;

    titulo("desborde: sale el menos recientemente usado");
    CrearLruCache(&c, 3, sizeof(int), cmpInt);
    agregarInt(&c, 1, 10);
    agregarInt(&c, 2, 20);
    agregarInt(&c, 3, 30);          /* orden: 3 2 1 */
    agregarInt(&c, 4, 40);          /* sale el 1 */
    CHECK(!estaInt(&c, 1), "el 1 deberia haber salido");
    CHECK(estaInt(&c, 2), "el 2 deberia seguir");
    CHECK(estaInt(&c, 3), "el 3 deberia seguir");
    CHECK(estaInt(&c, 4), "el 4 deberia estar");
    VaciarLruCache(&c);
    CHECK(vivos() == 0, "fuga: %ld", vivos());
}

static void testObtenerRefresca(void)
{
    tLruCache c;
    int v;

    titulo("obtener refresca la recencia");
    CrearLruCache(&c, 3, sizeof(int), cmpInt);
    agregarInt(&c, 1, 10);
    agregarInt(&c, 2, 20);
    agregarInt(&c, 3, 30);          /* orden: 3 2 1 */
    obtenerInt(&c, 1, &v);          /* orden: 1 3 2 */
    agregarInt(&c, 4, 40);          /* sale el 2, no el 1 */
    CHECK(!estaInt(&c, 2), "el 2 deberia haber salido");
    CHECK(estaInt(&c, 1), "el 1 se uso, deberia seguir");
    CHECK(estaInt(&c, 3), "el 3 deberia seguir");
    CHECK(estaInt(&c, 4), "el 4 deberia estar");
    VaciarLruCache(&c);
}

static void testObtenerMiss(void)
{
    tLruCache c;
    int v;

    titulo("obtener que no encuentra no altera el orden");
    CrearLruCache(&c, 2, sizeof(int), cmpInt);
    agregarInt(&c, 1, 10);
    agregarInt(&c, 2, 20);          /* orden: 2 1 */
    obtenerInt(&c, 9, &v);          /* miss: no cambia nada */
    agregarInt(&c, 3, 30);          /* sale el 1 */
    CHECK(!estaInt(&c, 1), "el 1 deberia haber salido");
    CHECK(estaInt(&c, 2), "el 2 deberia seguir");
    VaciarLruCache(&c);
}

static void testReemplazo(void)
{
    tLruCache c;
    int v = 0;

    titulo("reemplazo (misma clave)");
    CrearLruCache(&c, 2, sizeof(int), cmpInt);
    agregarInt(&c, 1, 10);
    agregarInt(&c, 2, 20);          /* orden: 2 1 */
    CHECK(agregarInt(&c, 1, 99) == TODO_BIEN, "reemplazar 1");  /* orden: 1 2 */
    agregarInt(&c, 3, 30);          /* sale el 2, no hay duplicado del 1 */
    CHECK(!estaInt(&c, 2), "el 2 deberia haber salido");
    CHECK(obtenerInt(&c, 1, &v) == TODO_BIEN && v == 99, "el 1 deberia valer 99, vale %d", v);
    CHECK(estaInt(&c, 3), "el 3 deberia estar");
    VaciarLruCache(&c);
    CHECK(vivos() == 0, "fuga: %ld", vivos());
}

static void testReemplazoNoCrece(void)
{
    tLruCache c;
    int i;

    titulo("reemplazar muchas veces no consume capacidad");
    CrearLruCache(&c, 2, sizeof(int), cmpInt);
    agregarInt(&c, 1, 10);
    agregarInt(&c, 2, 20);
    for (i = 0; i < 10; i++)
        agregarInt(&c, 1, i);
    CHECK(estaInt(&c, 1), "el 1 deberia estar");
    CHECK(estaInt(&c, 2), "el 2 no deberia haber salido por reemplazar el 1");
    VaciarLruCache(&c);
    CHECK(vivos() == 0, "fuga: %ld", vivos());
}

static void testBorrar(void)
{
    tLruCache c;

    titulo("borrar primero, medio y ultimo");
    CrearLruCache(&c, 5, sizeof(int), cmpInt);
    agregarInt(&c, 1, 10);
    agregarInt(&c, 2, 20);
    agregarInt(&c, 3, 30);
    agregarInt(&c, 4, 40);
    agregarInt(&c, 5, 50);          /* orden: 5 4 3 2 1 */
    CHECK(borrarInt(&c, 5) == TODO_BIEN, "borrar primero");
    CHECK(borrarInt(&c, 3) == TODO_BIEN, "borrar del medio");
    CHECK(borrarInt(&c, 1) == TODO_BIEN, "borrar ultimo");
    CHECK(borrarInt(&c, 9) == NO_EXISTE, "borrar inexistente");
    CHECK(borrarInt(&c, 5) == NO_EXISTE, "borrar dos veces");
    CHECK(!estaInt(&c, 5) && !estaInt(&c, 3) && !estaInt(&c, 1), "borrados no deben estar");
    CHECK(estaInt(&c, 4) && estaInt(&c, 2), "los otros deben seguir");
    VaciarLruCache(&c);
    CHECK(vivos() == 0, "fuga: %ld", vivos());
}

static void testBorrarLiberaLugar(void)
{
    tLruCache c;

    titulo("borrar libera un lugar (cantidad bien llevada)");
    CrearLruCache(&c, 3, sizeof(int), cmpInt);
    agregarInt(&c, 1, 10);
    agregarInt(&c, 2, 20);
    agregarInt(&c, 3, 30);
    borrarInt(&c, 2);
    agregarInt(&c, 4, 40);          /* hay lugar: no debe salir nadie */
    CHECK(estaInt(&c, 1), "el 1 no deberia haber salido");
    CHECK(estaInt(&c, 3), "el 3 no deberia haber salido");
    CHECK(estaInt(&c, 4), "el 4 deberia estar");
    VaciarLruCache(&c);
}

static void testCapacidadUno(void)
{
    tLruCache c;
    int v = 0;

    titulo("capacidad 1");
    CrearLruCache(&c, 1, sizeof(int), cmpInt);
    agregarInt(&c, 1, 10);
    agregarInt(&c, 2, 20);
    agregarInt(&c, 3, 30);
    CHECK(!estaInt(&c, 1) && !estaInt(&c, 2), "solo debe quedar el 3");
    CHECK(obtenerInt(&c, 3, &v) == TODO_BIEN && v == 30, "el 3 vale %d", v);
    CHECK(borrarInt(&c, 3) == TODO_BIEN, "borrar el unico");
    CHECK(!estaInt(&c, 3), "ya no esta");
    CHECK(agregarInt(&c, 7, 70) == TODO_BIEN && estaInt(&c, 7), "se puede volver a agregar");
    VaciarLruCache(&c);
    CHECK(vivos() == 0, "fuga: %ld", vivos());
}

static void testVaciarReutilizar(void)
{
    tLruCache c;
    int i, v = 0;

    titulo("vaciar y reutilizar");
    CrearLruCache(&c, 3, sizeof(int), cmpInt);
    for (i = 1; i <= 5; i++)
        agregarInt(&c, i, i * 10);
    VaciarLruCache(&c);
    CHECK(vivos() == 0, "fuga tras vaciar: %ld", vivos());
    CHECK(obtenerInt(&c, 5, &v) == NO_EXISTE, "tras vaciar no hay nada");
    for (i = 1; i <= 3; i++)
        agregarInt(&c, i, i * 100);
    CHECK(estaInt(&c, 1) && estaInt(&c, 2) && estaInt(&c, 3),
          "tras vaciar entran 3 sin que salga ninguno (cantidad en 0)");
    VaciarLruCache(&c);
    VaciarLruCache(&c);             /* vaciar dos veces seguidas no debe romper */
    CHECK(vivos() == 0, "fuga: %ld", vivos());
}

static void testValoresDeTamanioVariable(void)
{
    tLruCache c;
    int k = 1;
    char chico[3] = {'a', 'b', 'c'};
    char grande[40];
    char buf[64];
    char vacio = 'x';
    int i;

    titulo("valores de distinto tamano");
    for (i = 0; i < 40; i++)
        grande[i] = (char)('A' + (i % 26));

    CrearLruCache(&c, 3, sizeof(int), cmpInt);
    CHECK(AgregarLruCache(&c, &k, chico, sizeof(chico)) == TODO_BIEN, "agregar valor chico");

    memset(buf, '.', sizeof(buf));
    CHECK(ObtenerLruCache(&c, &k, buf, sizeof(chico)) == TODO_BIEN, "obtener chico");
    CHECK(memcmp(buf, chico, 3) == 0, "valor chico mal copiado");

    memset(buf, '.', sizeof(buf));
    ObtenerLruCache(&c, &k, buf, 40);   /* buffer mas grande que el valor guardado */
    CHECK(memcmp(buf, chico, 3) == 0 && buf[3] == '.',
          "con buffer grande solo debe copiar los 3 bytes guardados (buf[3]='%c')", buf[3]);

    CHECK(AgregarLruCache(&c, &k, grande, sizeof(grande)) == TODO_BIEN, "reemplazar por valor grande");
    memset(buf, '.', sizeof(buf));
    ObtenerLruCache(&c, &k, buf, sizeof(grande));
    CHECK(memcmp(buf, grande, 40) == 0, "valor grande mal copiado tras reemplazo");

    memset(buf, '.', sizeof(buf));
    ObtenerLruCache(&c, &k, buf, 5);    /* buffer mas chico que el valor */
    CHECK(memcmp(buf, grande, 5) == 0 && buf[5] == '.',
          "con buffer chico solo debe copiar 5 bytes (buf[5]='%c')", buf[5]);

    CHECK(AgregarLruCache(&c, &k, chico, sizeof(chico)) == TODO_BIEN, "reemplazar por valor chico");
    memset(buf, '.', sizeof(buf));
    ObtenerLruCache(&c, &k, buf, sizeof(buf));
    CHECK(memcmp(buf, chico, 3) == 0 && buf[3] == '.', "no debe quedar resto del valor grande");

    titulo("valor vacio");
    k = 2;
    CHECK(AgregarLruCache(&c, &k, NULL, 0) == TODO_BIEN, "agregar valor vacio (NULL, 0)");
    CHECK(ObtenerLruCache(&c, &k, &vacio, 0) == TODO_BIEN, "obtener valor vacio");
    CHECK(vacio == 'x', "obtener de 0 bytes no debe escribir");
    CHECK(BorrarLruCache(&c, &k) == TODO_BIEN, "borrar valor vacio");

    VaciarLruCache(&c);
    CHECK(vivos() == 0, "fuga: %ld", vivos());
}

typedef struct
{
    int dni;
    char tipo;
} tClaveComp;

static int cmpComp(const void *a, const void *b)
{
    const tClaveComp *x = (const tClaveComp *)a;
    const tClaveComp *y = (const tClaveComp *)b;

    if (x->dni != y->dni)
        return (x->dni > y->dni) - (x->dni < y->dni);
    return (x->tipo > y->tipo) - (x->tipo < y->tipo);
}

static void testClavesDeOtrosTipos(void)
{
    tLruCache c;
    char k1[16] = "ana";
    char k2[16] = "luis";
    char k3[16] = "ana2";
    char v[8];
    tClaveComp a, b;
    double pi = 3.14, d = 0;

    titulo("claves de cadena (char[16])");
    CrearLruCache(&c, 2, sizeof(k1), cmpCad);
    AgregarLruCache(&c, k1, "hola", 5);
    AgregarLruCache(&c, k2, "chau", 5);
    CHECK(ObtenerLruCache(&c, k1, v, sizeof(v)) == TODO_BIEN && strcmp(v, "hola") == 0,
          "ana -> %s", v);
    AgregarLruCache(&c, k3, "xx", 3);   /* sale luis */
    CHECK(ObtenerLruCache(&c, k2, v, sizeof(v)) == NO_EXISTE, "luis deberia haber salido");
    CHECK(ObtenerLruCache(&c, k1, v, sizeof(v)) == TODO_BIEN, "ana deberia seguir");
    VaciarLruCache(&c);

    titulo("claves compuestas (struct)");
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.dni = 10; a.tipo = 'A';
    b.dni = 10; b.tipo = 'B';
    CrearLruCache(&c, 3, sizeof(tClaveComp), cmpComp);
    AgregarLruCache(&c, &a, &pi, sizeof(pi));
    CHECK(ObtenerLruCache(&c, &b, &d, sizeof(d)) == NO_EXISTE, "misma dni, distinto tipo: no es la misma clave");
    AgregarLruCache(&c, &b, &pi, sizeof(pi));
    CHECK(ObtenerLruCache(&c, &b, &d, sizeof(d)) == TODO_BIEN && d == 3.14, "clave compuesta");
    VaciarLruCache(&c);
    CHECK(vivos() == 0, "fuga: %ld", vivos());
}

static void testSinMemoria(void)
{
    tLruCache c;
    long n;
    int r;
    int v;

    titulo("falla de memoria en agregar: no hay fugas ni elemento a medias");
    for (n = 1; n <= 6; n++)
    {
        CrearLruCache(&c, 3, sizeof(int), cmpInt);
        armarFalla(n);
        r = agregarInt(&c, 1, 10);
        armarFalla(0);
        CHECK(r == TODO_BIEN || r == SIN_MEM, "agregar con falla en malloc %ld devolvio %d", n, r);
        if (r == SIN_MEM)
            CHECK(obtenerInt(&c, 1, &v) == NO_EXISTE, "malloc %ld fallo pero el elemento quedo", n);
        else
            CHECK(estaInt(&c, 1), "malloc %ld: dijo TODO_BIEN pero no esta", n);
        CHECK(agregarInt(&c, 2, 20) == TODO_BIEN && estaInt(&c, 2),
              "tras la falla %ld la cache debe seguir usable", n);
        VaciarLruCache(&c);
        CHECK(vivos() == 0, "falla en malloc %ld dejo %ld bloques", n, vivos());
    }
}

/* ---------------------------------------------- contraste contra un modelo */

#define REF_MAX 64

typedef struct
{
    int k[REF_MAX];
    int v[REF_MAX];
    int n;
    int cap;
} tRef;

static int refBuscar(const tRef *r, int k)
{
    int i = 0;

    while (i < r->n && r->k[i] != k)
        i++;
    return i < r->n ? i : -1;
}

static void refAlFrente(tRef *r, int i)
{
    int k = r->k[i];
    int v = r->v[i];

    while (i > 0)
    {
        r->k[i] = r->k[i - 1];
        r->v[i] = r->v[i - 1];
        i--;
    }
    r->k[0] = k;
    r->v[0] = v;
}

static void refPut(tRef *r, int k, int v)
{
    int i = refBuscar(r, k);

    if (i >= 0)
    {
        r->v[i] = v;
        refAlFrente(r, i);
        return;
    }
    if (r->n == r->cap)
        r->n--;
    r->n++;
    r->k[r->n - 1] = k;
    r->v[r->n - 1] = v;
    refAlFrente(r, r->n - 1);
}

static int refGet(tRef *r, int k, int *v)
{
    int i = refBuscar(r, k);

    if (i < 0)
        return 0;
    refAlFrente(r, i);
    *v = r->v[0];
    return 1;
}

static int refDel(tRef *r, int k)
{
    int i = refBuscar(r, k);

    if (i < 0)
        return 0;
    while (i < r->n - 1)
    {
        r->k[i] = r->k[i + 1];
        r->v[i] = r->v[i + 1];
        i++;
    }
    r->n--;
    return 1;
}

static unsigned long long g_semilla = 88172645463325252ULL;

static unsigned azar(void)
{
    g_semilla ^= g_semilla << 13;
    g_semilla ^= g_semilla >> 7;
    g_semilla ^= g_semilla << 17;
    return (unsigned)(g_semilla >> 8);
}

static void testContraModelo(int cap, int rango, int ops)
{
    tLruCache c;
    tRef ref;
    int i, j, op, k, v, vref, r, rref;
    int malos = 0, finalMalos = 0;

    printf("[contraste con modelo: capacidad %d, claves 0..%d, %d operaciones]\n", cap, rango - 1, ops);
    ref.n = 0;
    ref.cap = cap;
    CrearLruCache(&c, (unsigned)cap, sizeof(int), cmpInt);

    for (i = 0; i < ops; i++)
    {
        op = (int)(azar() % 100);
        k = (int)(azar() % (unsigned)rango);
        v = (int)(azar() % 100000);
        if (op < 40)
        {
            refPut(&ref, k, v);
            r = agregarInt(&c, k, v);
            if (r != TODO_BIEN)
                malos++;
        }
        else if (op < 85)
        {
            v = -1;
            vref = -1;
            rref = refGet(&ref, k, &vref);
            r = obtenerInt(&c, k, &v);
            if ((rref && (r != TODO_BIEN || v != vref)) || (!rref && r != NO_EXISTE))
                malos++;
        }
        else
        {
            rref = refDel(&ref, k);
            r = borrarInt(&c, k);
            if ((rref && r != TODO_BIEN) || (!rref && r != NO_EXISTE))
                malos++;
        }
    }
    CHECK(malos == 0, "%d operaciones difieren del modelo", malos);

    /* al final, el contenido completo debe coincidir (de atras para adelante,
       para no alterar el orden relativo que se esta comprobando) */
    for (j = ref.n - 1; j >= 0; j--)
    {
        v = -1;
        if (obtenerInt(&c, ref.k[j], &v) != TODO_BIEN || v != ref.v[j])
            finalMalos++;
    }
    CHECK(finalMalos == 0, "%d elementos del modelo no coinciden al final", finalMalos);

    VaciarLruCache(&c);
    CHECK(vivos() == 0, "fuga tras el contraste: %ld", vivos());
}

/* ------------------------------------------------------------------ main */

int main(void)
{
    g_base = g_vivos;
    testParametros();
    testVacia();
    testBasico();
    testDesborde();
    testObtenerRefresca();
    testObtenerMiss();
    testReemplazo();
    testReemplazoNoCrece();
    testBorrar();
    testBorrarLiberaLugar();
    testCapacidadUno();
    testVaciarReutilizar();
    testValoresDeTamanioVariable();
    testClavesDeOtrosTipos();
    testSinMemoria();
    testContraModelo(1, 3, 2000);
    testContraModelo(5, 12, 5000);
    testContraModelo(50, 120, 20000);

    printf("\n%d chequeos, %d fallas\n", g_checks, g_fallas);
    return g_fallas == 0 ? 0 : 1;
}
