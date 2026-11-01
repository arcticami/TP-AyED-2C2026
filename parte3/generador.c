#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tweener.h"

/*
 * Generador de lotes de prueba para el simulador de Tweener.
 *
 * Uso:  generador <concentrado|disperso|ordenado> <semilla> [cantUsuarios]
 *
 * Genera:
 *   datos/posteos.bin            CANT_POSTEOS registros {id, texto}; el id es el numero de registro
 *   datos/usuarios_<tipo>.bin    cantUsuarios registros {usuario, feed[LARGO_FEED]}
 *
 * La misma semilla genera siempre el mismo lote. El texto de los posteos no depende
 * de la semilla (posteos.bin es igual para todos los lotes).
 * La carpeta datos/ tiene que existir.
 */

#define RUTA_POSTEOS    "datos/posteos.bin"
#define SEMILLA_TEXTO   12345u

#define VIRALES         50      //concentrado: cantidad de posteos "virales"
#define PORC_VIRAL      80      //concentrado: % de los lugares del feed que se llenan con virales
#define PASO_VENTANA    10      //ordenado: cuanto avanza la ventana de un usuario al siguiente (feed de 20 => se superpone la mitad)

#define ALFABETO        "abcdefghijklmnopqrstuvwxyz "
#define LARGO_ALFABETO  27

uint32_t siguiente(uint32_t *estado);
unsigned aleatorio(uint32_t *estado, unsigned tope);
int idRepetido(const int *v, int cant, int id);
void armarPosteo(tPosteo *p, int id, uint32_t *estado);
int generarPosteos(void);
void armarFeedConcentrado(int *feed, const int *virales, uint32_t *estado);
void armarFeedDisperso(int *feed, uint32_t *estado);
void armarFeedOrdenado(int *feed, unsigned k);
int generarUsuarios(const char *tipo, uint32_t semilla, unsigned cantUsuarios);

int main(int argc, char *argv[])
{
    uint32_t semilla;
    unsigned cantUsuarios = CANT_USUARIOS;

    if (argc < 3 || argc > 4 ||
        (strcmp(argv[1], "concentrado") != 0 && strcmp(argv[1], "disperso") != 0 && strcmp(argv[1], "ordenado") != 0))
    {
        printf("Uso: %s <concentrado|disperso|ordenado> <semilla> [cantUsuarios]\n", argv[0]);
        return 1;
    }

    semilla = (uint32_t)strtoul(argv[2], NULL, 10);
    if (semilla == 0)
        semilla = 1; //xorshift no puede arrancar en 0

    if (argc == 4)
        cantUsuarios = (unsigned)strtoul(argv[3], NULL, 10);

    if (cantUsuarios < MIN_USUARIOS)
    {
        printf("Error: el enunciado pide no menos de %d usuarios\n", MIN_USUARIOS);
        return 1;
    }

    if (!generarPosteos())
    {
        printf("Error: no se pudo escribir %s (existe la carpeta datos/?)\n", RUTA_POSTEOS);
        return 1;
    }
    printf("Generado %s: %d posteos de %u bytes\n", RUTA_POSTEOS, CANT_POSTEOS, (unsigned)sizeof(tPosteo));

    if (!generarUsuarios(argv[1], semilla, cantUsuarios))
    {
        printf("Error: no se pudo escribir datos/usuarios_%s.bin\n", argv[1]);
        return 1;
    }
    printf("Generado datos/usuarios_%s.bin: %u usuarios, feed de %d, semilla %lu\n",
           argv[1], cantUsuarios, LARGO_FEED, (unsigned long)semilla);

    return 0;
}

//xorshift de 32 bits (rand() en MinGW llega solo a 32767 y no cubre 50000 ids)
uint32_t siguiente(uint32_t *estado)
{
    uint32_t x = *estado;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *estado = x;

    return x;
}

//numero en [0, tope)
unsigned aleatorio(uint32_t *estado, unsigned tope)
{
    return (unsigned)(siguiente(estado) % tope);
}

//1 si id ya esta entre las primeras cant posiciones de v
int idRepetido(const int *v, int cant, int id)
{
    int i = 0;

    while (i < cant && v[i] != id)
        i++;

    return i < cant;
}

/* -------------------------------------------------------------------------- */
/*                                  POSTEOS                                   */
/* -------------------------------------------------------------------------- */

//texto de relleno con el id adelante ("Posteo 123: ..."), solo sirve para poder revisar el archivo a ojo
void armarPosteo(tPosteo *p, int id, uint32_t *estado)
{
    char cab[32];
    int n, i = 0;

    memset(p, 0, sizeof(tPosteo)); //deja en cero el relleno del struct, asi el archivo siempre sale igual

    p->id = id;
    n = sprintf(cab, "Posteo %d: ", id);

    while (i < TAM_TEXTO - 2)
    {
        if (i < n)
            p->texto[i] = cab[i];
        else
            p->texto[i] = ALFABETO[aleatorio(estado, LARGO_ALFABETO)];
        i++;
    }
}

int generarPosteos(void)
{
    FILE *pf;
    tPosteo p;
    uint32_t estado = SEMILLA_TEXTO;
    int id = 0, ok = 1;

    if ((pf = fopen(RUTA_POSTEOS, "wb")) == NULL)
        return 0;

    while (ok && id < CANT_POSTEOS)
    {
        armarPosteo(&p, id, &estado);
        ok = fwrite(&p, sizeof(tPosteo), 1, pf) == 1;
        id++;
    }

    fclose(pf);
    return ok;
}

/* -------------------------------------------------------------------------- */
/*                                  USUARIOS                                  */
/* -------------------------------------------------------------------------- */

//cada lugar del feed es un viral con probabilidad PORC_VIRAL %, sino un posteo cualquiera; sin repetir ids dentro del feed
void armarFeedConcentrado(int *feed, const int *virales, uint32_t *estado)
{
    int j = 0, id;

    while (j < LARGO_FEED)
    {
        if (aleatorio(estado, 100) < PORC_VIRAL)
            id = virales[aleatorio(estado, VIRALES)];
        else
            id = (int)aleatorio(estado, CANT_POSTEOS);

        if (!idRepetido(feed, j, id))
        {
            feed[j] = id;
            j++;
        }
    }
}

//posteos cualquiera de toda la base, sin repetir ids dentro del feed
void armarFeedDisperso(int *feed, uint32_t *estado)
{
    int j = 0, id;

    while (j < LARGO_FEED)
    {
        id = (int)aleatorio(estado, CANT_POSTEOS);

        if (!idRepetido(feed, j, id))
        {
            feed[j] = id;
            j++;
        }
    }
}

//ventana de ids consecutivos que avanza PASO_VENTANA por usuario (se superpone con la del usuario anterior);
//si llega al final de la base vuelve a empezar, asi cada feed siempre queda creciente
void armarFeedOrdenado(int *feed, unsigned k)
{
    int base = (int)((k * PASO_VENTANA) % (CANT_POSTEOS - LARGO_FEED + 1));
    int j = 0;

    while (j < LARGO_FEED)
    {
        feed[j] = base + j;
        j++;
    }
}

int generarUsuarios(const char *tipo, uint32_t semilla, unsigned cantUsuarios)
{
    char ruta[64];
    FILE *pf;
    tUsuario u;
    int virales[VIRALES];
    uint32_t estado = semilla;
    unsigned k = 0;
    int i = 0, ok = 1;

    sprintf(ruta, "datos/usuarios_%s.bin", tipo);
    if ((pf = fopen(ruta, "wb")) == NULL)
        return 0;

    //los virales se eligen una vez por lote (distintos entre si)
    while (strcmp(tipo, "concentrado") == 0 && i < VIRALES)
    {
        virales[i] = (int)aleatorio(&estado, CANT_POSTEOS);
        if (!idRepetido(virales, i, virales[i]))
            i++;
    }

    while (ok && k < cantUsuarios)
    {
        memset(&u, 0, sizeof(tUsuario));
        snprintf(u.usuario, TAM_USUARIO, "user%05u", k + 1);

        if (strcmp(tipo, "concentrado") == 0)
            armarFeedConcentrado(u.feed, virales, &estado);
        else if (strcmp(tipo, "disperso") == 0)
            armarFeedDisperso(u.feed, &estado);
        else
            armarFeedOrdenado(u.feed, k);

        ok = fwrite(&u, sizeof(tUsuario), 1, pf) == 1;
        k++;
    }

    fclose(pf);
    return ok;
}
