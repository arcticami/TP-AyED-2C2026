# TP-AyED-2C2026

TP LRU Cache - Algoritmos y Estructuras de Datos (3640), Com 01-1900, 2C 2026.

## Estructura

- `lista/` lista simple y lista doble
- `arbol/` arbol binario de busqueda (indice de la Parte 2)
- `parte1/` LRU Cache con lista simple
- `parte2/` LRU Cache con lista doble + indice en arbol
- `parte3/` generador de lotes y simulador de Tweener
- `pruebas/` lote de pruebas de la cache (no es parte de la entrega)
- `informe/` informe de la Parte 3

## Compilar y correr la simulacion

Con Code::Blocks se abre `TP-AyED-2C2026.cbp` (target `Todo` compila todo en `bin/cbp/`).

Para correr la Parte 3 completa en Windows:

    correr_simulacion.bat [semilla] ["capacidades"]

El .bat compila con el `Makefile` (usa el `mingw32-make` que viene con Code::Blocks en
`C:\CodeBlocks\MinGW\bin`), deja los ejecutables en `bin/`, genera los tres lotes en `datos/`
y guarda los resultados en `reporte.txt`. `reporte2.txt` tiene la misma corrida con semilla 42.

Tambien se puede compilar a mano:

    mingw32-make                    (todo)
    mingw32-make generador sim_p1 sim_p2
    mingw32-make test_p1 test_p2
