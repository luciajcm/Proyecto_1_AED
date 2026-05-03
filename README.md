# Hoja de Cálculo — Matrices Dispersas

**Proyecto #1 — Algoritmos y Estructuras de Datos**

## Descripción

Aplicación de hoja de cálculo implementada en C++17 con Qt, que utiliza una **matriz dispersa con listas enlazadas cruzadas** como estructura de datos principal.

---

## Compilación y Ejecución

### Requisitos

- **C++17** o superior (GCC 8+, Clang 7+, MSVC 2019+)
- **Qt5** o **Qt6** (módulos Core y Widgets)
- **CMake 3.16+**

### Linux / macOS

```bash
# Instalar dependencias (Ubuntu/Debian)
sudo apt-get install qt6-base-dev cmake build-essential
# o con Qt5:
sudo apt-get install qtbase5-dev cmake build-essential

# Compilar
mkdir build && cd build
cmake ..
make -j$(nproc)

# Ejecutar
./spreadsheet
```

### Windows (con Qt Creator)

1. Abrir **Qt Creator**.
2. Archivo → Abrir Proyecto → seleccionar `CMakeLists.txt`.
3. Configurar el kit de compilación (Qt5 o Qt6 + MinGW/MSVC).
4. Compilar y ejecutar con **Ctrl+R**.

### Windows (línea de comandos con MinGW)

```cmd
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
spreadsheet.exe
```

---

## Uso de la Aplicación

### Editar celdas

- **Clic** en cualquier celda para seleccionarla.
- Escribe el valor en la **barra de fórmulas** (arriba) y presiona **Enter**.
- También puedes escribir directamente en la celda.
- Las celdas con datos se resaltan en **azul claro**.

### Fórmulas aritméticas

Las fórmulas empiezan con `=` y soportan:

| Ejemplo        | Descripción                          |
|----------------|--------------------------------------|
| `=42`          | Valor literal                        |
| `=A1+B2`       | Suma de celdas                       |
| `=A1*2`        | Multiplicación por escalar           |
| `=A1+B2*3-C4`  | Expresión con precedencia            |
| `=(A1+B1)/2`   | Uso de paréntesis                    |
| `=A1/B1`       | División (error si B1 es cero)       |

### Operaciones de fila/columna

Desde el menú **Edición** o la barra de herramientas:

- **Eliminar fila**: elimina todos los nodos de una fila.
- **Eliminar columna**: elimina todos los nodos de una columna.
- **Eliminar rango**: elimina un bloque rectangular (ej: `A1:C4`).

### Operaciones de agregación

Desde el menú **Fórmulas**:

| Operación       | Descripción                                      |
|-----------------|--------------------------------------------------|
| SUMA fila       | Suma todos los valores numéricos de una fila     |
| SUMA columna    | Suma todos los valores numéricos de una columna  |
| SUMA rango      | Suma un rango rectangular (ej: `A1:C5`)          |
| PROMEDIO fila   | Promedio de valores numéricos en una fila        |
| PROMEDIO col    | Promedio de valores numéricos en una columna     |
| PROMEDIO rango  | Promedio en rango rectangular                    |
| MÁXIMO rango    | Valor máximo en un rango rectangular             |
| MÍNIMO rango    | Valor mínimo en un rango rectangular             |

---

## Estructura de Datos: Lista Enlazada Cruzada

### Diseño del Nodo

```
┌─────────────────────────────────────────┐
│  Cell                                   │
│  ┌───────┬───────┬──────────────────┐   │
│  │  row  │  col  │     value        │   │
│  └───────┴───────┴──────────────────┘   │
│       │ nextInRow ────► (siguiente en fila)   │
│       │ nextInCol ────► (siguiente en col)    │
└─────────────────────────────────────────┘
```

### Diagrama de la Estructura

```
rowHeaders:
  fila 0 ──► [A0,"10"] ──► [C0,"20"] ──► [E0,"30"] ──► null
  fila 1 ──► [B1,"15"] ──► [C1,"25"] ──► null
  fila 3 ──► [A3,"Hola"] ──► null

colHeaders:
  col A ──► [A0,"10"] ──► [A3,"Hola"] ──► null
  col B ──► [B1,"15"] ──► null
  col C ──► [C0,"20"] ──► [C1,"25"] ──► null
  col E ──► [E0,"30"] ──► null
```

Solo existen nodos para celdas con contenido. Las celdas vacías no consumen memoria.

---

## Análisis de Complejidad

| Operación               | Complejidad Temporal | Complejidad Espacial |
|-------------------------|----------------------|----------------------|
| Insertar celda          | O(k_r + k_c)         | O(1) por nodo        |
| Consultar celda         | O(k_r)               | O(1)                 |
| Modificar celda         | O(k_r)               | O(1)                 |
| Eliminar celda          | O(k_r + k_c)         | O(1)                 |
| Eliminar fila           | O(k_r · k_c_max)     | O(1)                 |
| Eliminar columna        | O(k_c · k_r_max)     | O(1)                 |
| Eliminar rango r×c      | O(r·c · (k_r+k_c))   | O(1)                 |
| SUMA / PROMEDIO fila    | O(k_r)               | O(1)                 |
| SUMA / PROMEDIO col     | O(k_c)               | O(1)                 |
| SUMA / PROMEDIO rango   | O(celdas en rango)   | O(1)                 |
| MÁXIMO / MÍNIMO rango   | O(celdas en rango)   | O(1)                 |

Donde:
- **k_r** = número de elementos no vacíos en la fila
- **k_c** = número de elementos no vacíos en la columna

### Comparación con Matriz Densa

Para una hoja de 1000×1000 con solo 500 celdas ocupadas (0.05% densidad):

| Métrica              | Matriz Densa     | Matriz Dispersa |
|----------------------|------------------|-----------------|
| Memoria              | 1,000,000 celdas | 500 nodos       |
| Inserción/eliminación| O(1) acceso      | O(k) recorrido  |
| Iteración de fila    | O(n)             | O(k_r)          |

La matriz dispersa es **óptima en memoria** y en **tiempo de iteración** cuando la densidad es baja (< 10%).

---

## Casos Borde Manejados

- **Consultar celda inexistente**: retorna `""` sin error.
- **Eliminar celda inexistente**: no hace nada, retorna `false`.
- **Eliminar fila/columna vacía**: operación segura, sin crash.
- **Agregación en rango sin números**: retorna 0 (suma/promedio) o lanza excepción capturada (max/min).
- **Insertar texto en lugar de número**: almacenado normalmente, ignorado en agregaciones.
- **Fórmula con división por cero**: muestra `#ERR` en la celda.
- **Fórmula con referencia a celda vacía**: la celda vale 0.

---

## Estructura del Proyecto

```
spreadsheet/
├── CMakeLists.txt          # Sistema de compilación
├── README.md               # Este archivo
└── src/
    ├── Cell.h              # Nodo de la matriz dispersa
    ├── SparseMatrix.h      # Interfaz de la matriz
    ├── SparseMatrix.cpp    # Implementación de la matriz
    ├── FormulaParser.h     # Parser de fórmulas (interfaz)
    ├── FormulaParser.cpp   # Parser de fórmulas (implementación)
    ├── MainWindow.h        # Ventana principal Qt (interfaz)
    ├── MainWindow.cpp      # Ventana principal Qt (implementación)
    └── main.cpp            # Punto de entrada
```

---

## Justificación de la Estructura de Datos (máx. 1 página)

### ¿Por qué lista enlazada cruzada?

Una hoja de cálculo real (ej. Excel con 1,048,576 filas × 16,384 columnas) tiene más de **17 mil millones** de celdas posibles. En la práctica, un usuario típico utiliza solo algunos cientos o miles de celdas. Usar una matriz densa bidimensional:

1. **Desperdiciaría memoria** proporcional al área total, no al contenido real.
2. **Dificultaría la iteración** al tener que recorrer todas las celdas para encontrar las no vacías.

La lista enlazada cruzada resuelve ambos problemas:
- **Solo se crean nodos para celdas con datos.** Una hoja con 500 celdas usa exactamente 500 nodos, independientemente del tamaño de la cuadrícula.
- **Las operaciones de fila/columna recorren solo los elementos existentes** en O(k), no toda la fila O(n).

### Alternativas descartadas

- `std::map<pair<int,int>, string>`: O(log n) por operación, sin estructura de fila/columna eficiente.
- `std::unordered_map`: O(1) promedio para acceso puntual, pero sin iterar por fila/columna eficientemente.
- Arreglo 2D estático: O(1) acceso, pero O(n·m) memoria y sin soporte para hojas grandes.

La lista enlazada cruzada es el equilibrio óptimo para el patrón de uso de una hoja de cálculo dispersa.
