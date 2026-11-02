# TP-AyED-2C2026

TP LRU Cache - Algoritmos y Estructuras de Datos (3640), Com 01-1900, 2C 2026.

## Estructura

- `lista/` lista simple (Parte 1) y lista doble (Parte 2)
- `arbol/` arbol binario de busqueda (indice de la Parte 2)
- `parte1/` TDA `t_lru_cache` con lista simple: `crear_lrucache`, `agregar_lrucache`, `obtener_lrucache`, `borrar_lrucache`, `vaciar_lrucache`
- `parte2/` TDA `t_lru_cache2` con lista doble + indice en arbol: las mismas primitivas con un 2 al final, mas `altura_indice_lrucache2`
- `parte3/` simulador de Tweener (genera los lotes con una semilla y los corre sin cache, con la Parte 1 y con la Parte 2): `simulador.c` tiene el `main`,
  `simulaciones.h`/`simulaciones.c` las funciones de la simulacion y `lotes.h`/`lotes.c` el generador de lotes
- `pruebas/` programas aparte, cada uno con su propio `main`: lote de pruebas de la cache y generador de un lote suelto

La Parte 2 lleva un 2 al final de los nombres para que las dos versiones puedan estar en el mismo proyecto
(el simulador usa las dos a la vez).

## Correr con un proyecto nuevo de Code::Blocks

1. File > New > Project > Console application (C), en cualquier carpeta.
2. Borrar el `main.c` que crea Code::Blocks.
3. Agregar al proyecto (arrastrando o con Project > Add files recursively) las carpetas `lista`, `arbol`,
   `parte1`, `parte2` y `parte3`, sin moverlas de lugar. **No agregar `pruebas/`**, porque sus archivos tienen otro `main`.
4. Compilar y ejecutar. Sin argumentos usa la semilla 1; para cambiarla, Project > Set programs' arguments
   (por ejemplo `7`, o `7 0 50 500` para elegir tambien las capacidades).

Los includes son relativos a cada archivo (`../lista/lista.h`, etc.), asi que no hace falta configurar
carpetas de busqueda. El simulador escribe `posteos.bin`, `usuarios_<lote>.bin` y `reporte.txt`
en la carpeta de trabajo (la del proyecto).

Tambien esta `TP-AyED-2C2026.cbp` con los targets `Simulador`, `Test_P1`, `Test_P2` y `Generador`.
Los tests se linkean con `-Wl,--wrap=malloc -Wl,--wrap=free` y `Test_P2` ademas con `-DPARTE2`.
