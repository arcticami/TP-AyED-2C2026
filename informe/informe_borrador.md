<!--
BORRADOR para que el grupo lo revise y lo reescriba con sus palabras antes de entregarlo.
Los numeros salen de reporte.txt (semilla 1) y reporte2.txt (semilla 42). Completar lo que esta entre [corchetes].
Para entregarlo pasarlo a Word/PDF y chequear que no pase de 3 paginas.
-->

# Trabajo Práctico: LRU Cache - Informe Parte 3

Algoritmos y Estructuras de Datos (3640) - Com 01-1900 - 2C 2026
Integrantes: [completar]

## 1. Cómo se hizo la simulación

- **Posteos:** `datos/posteos.bin`, 50000 registros `{id, texto[141]}` (148 bytes cada uno). El id es el número de registro, así que cada lectura es un `fseek` directo. El archivo se lee sin buffer de stdio para que cada fallo de la caché sea una lectura real.
- **Usuarios:** 2500 usuarios con un feed de 20 posteos (50000 accesos por corrida). Se generaron tres lotes con el generador (misma semilla, mismo lote):
  - **concentrado:** se eligen 50 posteos "virales" y cada lugar del feed es viral con 80% de probabilidad;
  - **disperso:** cada lugar del feed es un posteo cualquiera de los 50000;
  - **ordenado:** el usuario k ve los ids consecutivos desde 10·k, así que su feed es creciente y se superpone en 10 posteos con el del usuario anterior.
- Cada lote se corrió sin caché (capacidad 0) y con capacidades 10, 100, 1000 y 5000, con la Parte 1 (lista simple) y la Parte 2 (lista doble + índice en ABB). El tiempo es el promedio de 3 corridas medido con `clock()`; el tiempo por usuario y por posteo se saca dividiendo el total. Además se cuentan aciertos, fallos, lecturas al archivo y comparaciones de claves.
- PC donde se midió: [completar procesador / sistema operativo].

## 2. Resultados (semilla 1)

Tiempo total en ms. Sin caché se hacen siempre 50000 lecturas. Lecturas con caché = fallos.

**Lote concentrado**

| Capacidad | Aciertos | Tasa | Lecturas | P1 (ms) | P2 (ms) | Comparaciones P1 | Comparaciones P2 |
|---:|---:|---:|---:|---:|---:|---:|---:|
| sin caché | - | - | 50000 | 247 | 268 | - | - |
| 10   | 1658  | 3,3%  | 48342 | 273 | 300 | 978.369    | 679.611 |
| 100  | 37640 | 75,3% | 12360 | 100 | 87  | 4.059.182  | 579.054 |
| 1000 | 38705 | 77,4% | 11295 | 229 | 84  | 23.401.145 | 739.189 |
| 5000 | 39387 | 78,8% | 10613 | 671 | 90  | 84.837.571 | 780.218 |

**Lote disperso**

| Capacidad | Aciertos | Tasa | Lecturas | P1 (ms) | P2 (ms) | Comparaciones P1 | Comparaciones P2 |
|---:|---:|---:|---:|---:|---:|---:|---:|
| sin caché | - | - | 50000 | 264 | 276 | - | - |
| 10   | 5    | 0,01% | 49995 | 288  | 313 | 999.821     | 695.436 |
| 100  | 108  | 0,2%  | 49892 | 354  | 320 | 9.974.159   | 1.530.878 |
| 1000 | 1001 | 2,0%  | 48999 | 973  | 329 | 97.490.673  | 2.088.477 |
| 5000 | 4762 | 9,5%  | 45238 | 3673 | 328 | 439.007.976 | 2.569.261 |

**Lote ordenado**

| Capacidad | Aciertos | Tasa | Lecturas | P1 (ms) | P2 (ms) | Altura del índice | Comparaciones P1 | Comparaciones P2 |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| sin caché | - | - | 50000 | 117 | 118 | - | - | - |
| 10   | 24990 | 50,0% | 25010 | 71   | 78   | 10   | 749.990     | 912.580 |
| 100  | 24990 | 50,0% | 25010 | 103  | 129  | 100  | 5.241.800   | 9.895.705 |
| 1000 | 24990 | 50,0% | 25010 | 399  | 613  | 1000 | 49.268.900  | 97.944.955 |
| 5000 | 24990 | 50,0% | 25010 | 1666 | 2830 | 5000 | 225.344.900 | 450.074.955 |

Con semilla 42 (`reporte2.txt`) los resultados son prácticamente iguales: en el concentrado la tasa va de 3,5% a 79%, en el disperso de 0% a 9,6%, y el ordenado da los mismos aciertos y comparaciones porque no usa la semilla. En los lotes aleatorios la altura del árbol con capacidad 5000 quedó en 24-27.

## 3. Preguntas

### ¿En qué situaciones el uso de la caché mejora el tiempo de procesamiento y en cuáles no?

La caché mejora cuando hay muchos aciertos, o sea cuando los mismos posteos se piden varias veces en poco tiempo y entran en la caché:

- **Concentrado con capacidad ≥ 100:** los 50 virales entran en la caché y la tasa de aciertos sube a 75-79%. Las lecturas al archivo bajan de 50000 a unas 11000 y con la Parte 2 el tiempo baja de ~268 ms a ~85 ms (unas 3 veces más rápido). Con capacidad 10 no entran los 50 virales, se van pisando entre ellos y la tasa queda en 3%: el tiempo es peor que sin caché.
- **Ordenado:** cada usuario repite 10 posteos del anterior, que son justo los últimos que se cargaron, así que con capacidad 10 ya se consigue el 50% de aciertos y la Parte 1 baja de 117 ms a 71 ms. Agrandar la caché no suma aciertos (el usuario nunca vuelve a ver posteos de dos usuarios atrás) y solo agrega costo.

No mejora cuando casi no hay aciertos:

- **Disperso:** la tasa no pasa del 9,5% ni con 5000 elementos, así que se sigue leyendo casi todo del archivo y además se paga buscar en la caché y agregar cada posteo. Con la Parte 2 el tiempo sube de 276 a ~320 ms; con la Parte 1 se dispara hasta 3673 ms.
- **Caché demasiado chica** para lo que se repite (concentrado con 10).

En resumen, la caché conviene si el costo de buscar en ella es mucho menor que una lectura al archivo **y** la tasa de aciertos es alta. Si una de las dos no se cumple, el programa queda igual o peor que sin caché.

### ¿A partir de qué capacidad deja de convenir la implementación de la Parte 1? ¿Por qué?

Con nuestros datos, a partir de una capacidad de alrededor de 100 la Parte 1 deja de convenir:

- Con capacidad 10 la Parte 1 es un poco más rápida que la Parte 2 en todos los lotes (273 vs 300, 288 vs 313, 71 vs 78 ms), porque con pocos elementos recorrer la lista es barato y la Parte 2 tiene más trabajo fijo por operación (mantiene lista y árbol y pide más memoria).
- Con capacidad 100 la Parte 2 ya gana en el concentrado (87 vs 100 ms) y en el disperso (320 vs 354 ms).
- Con 1000 y 5000 la diferencia es enorme: en el concentrado la Parte 1 tarda 229 y 671 ms contra 84 y 90 de la Parte 2, y con 5000 la Parte 1 es más lenta que no usar caché. En el disperso con 5000 la Parte 1 tarda 3673 ms contra 328.

El motivo es que en la Parte 1 buscar una clave es recorrer la lista, O(n) en la capacidad. En cada fallo se recorre la lista entera en `ObtenerLruCache` y otra vez en `AgregarLruCache` (para ver si la clave ya estaba), y eso se ve en las comparaciones: en el disperso con 5000 son 439 millones, casi 2 × 5000 por fallo. En la Parte 2 la búsqueda es en el árbol, que con claves aleatorias tiene altura 24-26 para 5000 elementos, y las comparaciones totales se quedan en 2,5 millones. Mover al principio y sacar el último en la lista doble no requiere recorrer. Por eso el tiempo de la Parte 2 casi no cambia con la capacidad y el de la Parte 1 crece casi proporcional a ella.

[Si quieren el punto exacto de cruce se puede correr, por ejemplo, `correr_simulacion.bat 1 "10 25 50 75 100"`.]

La excepción es el lote ordenado, donde la Parte 1 es mejor en todas las capacidades por lo que pasa con el árbol (siguiente pregunta).

### ¿Qué ocurre con el árbol índice en el lote ordenado y cómo afecta al rendimiento?

En el lote ordenado los ids llegan a la caché en orden creciente, así que cada clave nueva es mayor que todas las del árbol y se inserta siempre como hijo derecho del último. Como el árbol no se balancea, se degenera en una lista: la altura queda igual a la capacidad (10, 100, 1000 y 5000 en la tabla), en lugar de las 24-27 de los lotes aleatorios.

Con eso la búsqueda en el índice pasa de ser O(log n) a O(n), igual que en la Parte 1, pero además es peor que la Parte 1:

- En cada fallo se recorre toda la rama en `ObtenerLruCache`, otra vez en `AgregarLruCache` para ver si la clave existe y otra para insertarla, porque la clave nueva va al fondo.
- Los aciertos son los posteos más nuevos (los 10 últimos del usuario anterior), que en el árbol son las claves más grandes y están al fondo de la rama. En cambio en la lista de la Parte 1 son los primeros, así que se encuentran enseguida.

Por eso con capacidad 5000 la Parte 2 hace 450 millones de comparaciones contra 225 de la Parte 1 y tarda 2830 ms contra 1666. O sea que en este caso el índice no sirve y además agrega el costo de mantener dos estructuras. Para evitarlo habría que usar un árbol que se mantenga balanceado (por ejemplo AVL), así la altura queda en el orden de log2(n) sin importar el orden en que lleguen las claves.

## 4. Conclusión

[Completar con las conclusiones del grupo. Ideas: la caché solo sirve si los datos se repiten y la caché alcanza para guardar lo que se repite; la Parte 1 alcanza para cachés chicas; la Parte 2 escala mucho mejor mientras el árbol no se degenere.]
