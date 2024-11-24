# Proyecto 2: Sistemas Operativos

## Simuladores de Cola Circular y Memoria Virtual

Este repositorio contiene implementaciones de dos simuladores: un simulador de cola circular y un simulador de memoria virtual. Ambos simuladores están diseñados para ayudar a comprender conceptos de programación concurrente y gestión de memoria en sistemas operativos.

### Contenido

- `simulapc.cpp`: Código fuente del simulador de cola circular.
- `mvirtual.cpp`: Código fuente del simulador de memoria virtual.
- `referencias.txt`: Archivo de texto que contiene la secuencia de referencias a páginas para el simulador de memoria virtual.
- `log.txt`: Archivo de log generado por el simulador de cola circular.

### Simulador de Cola Circular

#### Descripción

El simulador de cola circular permite que múltiples productores y consumidores interactúen con una cola compartida. Los productores añaden elementos a la cola, mientras que los consumidores los extraen. La cola se ajusta dinámicamente en tamaño según la carga de trabajo.

#### Uso

Compila el código utilizando `g++`:

```bash
g++ -o simulapc simulapc.cpp -lpthread
```

Ejecuta el simulador con los siguientes parámetros:

```bash
./simulapc -p <num_producers> -c <num_consumers> -s <initial_queue_size> -t <max_wait_time>
```

**Ejemplo:**

```bash
./simulapc -p 1 -c 1 -s 5 -t 5
```

### Simulador de Memoria Virtual

#### Descripción

El simulador de memoria virtual implementa algoritmos de reemplazo de páginas para gestionar la memoria virtual. Los algoritmos implementados incluyen FIFO, LRU, Óptimo, Clock y LRU Clock.

#### Uso

Compila el código utilizando `g++`:

```bash
g++ -o mvirtual mvirtual.cpp
```

Ejecuta el simulador con los siguientes parámetros:

```bash
./mvirtual -m <numFrames> -a <algorithm> -f <file>
```

**Ejemplo:**

```bash
./mvirtual -m 3 -a FIFO -f referencias.txt
```


