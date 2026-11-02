#include "simulaciones.h"

/*
 * Simulador de uso de Tweener sin cache, con la LRU Cache de la Parte 1 y con la de la Parte 2.
 *
 * Uso:  simulador [semilla [capacidad ...]]
 *
 * Sin argumentos usa la semilla y las capacidades por defecto. Capacidad 0 = sin cache (siempre lee el archivo).
 * Ejemplos:  simulador                 semilla 1, capacidades por defecto
 *            simulador 7               semilla 7, capacidades por defecto
 *            simulador 7 0 50 500      semilla 7, capacidades 0, 50 y 500
 * Pasos:
 *   1) genera posteos.bin y los tres lotes de usuarios (concentrado, disperso y ordenado) con la semilla;
 *   2) para cada lote, cada parte y cada capacidad procesa todos los usuarios de la primera a la ultima: por cada
 *      id de su feed pide el posteo a la cache y, si no esta, lo lee del archivo y lo guarda en la cache;
 *   3) muestra los resultados por pantalla y los guarda en reporte.txt.
 * Los datos de los posteos no se muestran ni se guardan pq son solo para medir.
 */

const int CAPACIDADES_DEFECTO[] = { 0, 10, 50, 100, 500, 1000, 5000 };

int main(int argc, char *argv[])
{
    FILE *fpPost, *fpRep, *salidas[2];
    int caps[MAX_CAPACIDADES], cantCaps, cantSalidas, i, ok = 1;
    unsigned semilla = SEMILLA_DEFECTO;

    if (argc > 1)
        semilla = atoi(argv[1]);

    if (argc > 2)
    {
        cantCaps = argc - 2;
        if (cantCaps > MAX_CAPACIDADES)
        {
            fprintf(stderr, "Error: maximo %d capacidades\n", MAX_CAPACIDADES);
            return 1;
        }

        for (i = 0; i < cantCaps; i++)
        {
            caps[i] = atoi(argv[i + 2]);
            if (caps[i] < 0)
            {
                fprintf(stderr, "Error: las capacidades no pueden ser negativas\n");
                return 1;
            }
        }
    }
    else
    {
        cantCaps = sizeof(CAPACIDADES_DEFECTO) / sizeof(CAPACIDADES_DEFECTO[0]);
        memcpy(caps, CAPACIDADES_DEFECTO, sizeof(CAPACIDADES_DEFECTO));
    }

    printf("Generando lotes con semilla %u ...\n", semilla);
    if (!generarLotes(semilla))
    {
        fprintf(stderr, "Error: no se pudieron generar los lotes (hay permiso de escritura en la carpeta de trabajo?)\n");
        return 1;
    }

    if ((fpPost = fopen(RUTA_POSTEOS, "rb")) == NULL)
    {
        fprintf(stderr, "Error: no se pudo abrir %s\n", RUTA_POSTEOS);
        return 1;
    }

    //esto se hace para q la comparacion del sin cache sea justa, como al tener un buffer se traen varios registros de golpe
    //dejarlo haria q parezca q el sin cache es mas rapido pq no se tiene en cuenta la cache propia de C
    //basicamente se le indica al SO q para el archivo fpPost no haga un buffer y por cada llamada vaya directo a él a buscar los datos
    if (SIN_BUFFER)
        setvbuf(fpPost, NULL, _IONBF, 0);

    //los resultados salen por pantalla y tambien a un archivo, para no perderlos cuando se cierra la consola
    salidas[0] = stdout;
    cantSalidas = 1;
    if ((fpRep = fopen(ARCHIVO_REPORTE, "w")) != NULL)
        salidas[cantSalidas++] = fpRep;
    else
        fprintf(stderr, "Aviso: no se pudo crear %s, el reporte sale solo por pantalla\n", ARCHIVO_REPORTE);

    for (i = 0; i < cantSalidas; i++)
    {
        fprintf(salidas[i], "\nsemilla  : %u\n", semilla);
        fprintf(salidas[i], "posteos  : %d (registro de %d bytes)\n", CANT_POSTEOS, (int)sizeof(tPosteo));
        fprintf(salidas[i], "usuarios : %d por lote (feed de %d posteos)\n", CANT_USUARIOS, LARGO_FEED);
        fprintf(salidas[i], "tiempo   : promedio de %d corridas; capacidad 0 = sin cache\n", REPETICIONES);
    }

    i = 0;
    while (ok && i < CANT_TIPOS_LOTE)
    {
        ok = simularLote(fpPost, TIPOS_LOTE[i], caps, cantCaps, salidas, cantSalidas);
        i++;
    }

    fclose(fpPost);
    if (fpRep != NULL)
    {
        fclose(fpRep);
        printf("Reporte guardado en %s\n", ARCHIVO_REPORTE);
    }

    return ok ? 0 : 1;
}
