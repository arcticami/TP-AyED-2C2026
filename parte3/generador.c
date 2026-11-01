#include <stdio.h>
#include <stdlib.h>
#include "tweener.h"
#include "lotes.h"

/*
 * Generador de un lote de prueba para el simulador de Tweener.
 *
 * Uso:  generador <concentrado|disperso|ordenado> <semilla> [cantUsuarios]
 *
 * Genera, en la carpeta de trabajo:
 *   posteos.bin              CANT_POSTEOS registros {id, texto}; el id es el numero de registro
 *   usuarios_<tipo>.bin      cantUsuarios registros {usuario, feed[LARGO_FEED]}
 *
 * La misma semilla genera siempre el mismo lote.
 * (Los simuladores generan los tres lotes por su cuenta; este programa sirve para generar uno solo.)
 */

int main(int argc, char *argv[])
{
    char ruta[TAM_RUTA];
    unsigned semilla;
    unsigned cantUsuarios = CANT_USUARIOS;

    if (argc < 3 || argc > 4 || !tipoLoteValido(argv[1]))
    {
        printf("Uso: %s <concentrado|disperso|ordenado> <semilla> [cantUsuarios]\n", argv[0]);
        return 1;
    }

    semilla = atoi(argv[2]);

    if (argc == 4)
        cantUsuarios = atoi(argv[3]);

    if (cantUsuarios < MIN_USUARIOS)
    {
        printf("Error: el enunciado pide no menos de %d usuarios\n", MIN_USUARIOS);
        return 1;
    }

    if (!generarPosteos(RUTA_POSTEOS))
    {
        printf("Error: no se pudo escribir %s\n", RUTA_POSTEOS);
        return 1;
    }
    printf("Generado %s: %d posteos de %d bytes\n", RUTA_POSTEOS, CANT_POSTEOS, (int)sizeof(tPosteo));

    rutaUsuarios(ruta, argv[1]);
    if (!generarUsuarios(ruta, argv[1], semilla, cantUsuarios))
    {
        printf("Error: no se pudo escribir %s\n", ruta);
        return 1;
    }
    printf("Generado %s: %u usuarios, feed de %d, semilla %u\n", ruta, cantUsuarios, LARGO_FEED, semilla);

    return 0;
}