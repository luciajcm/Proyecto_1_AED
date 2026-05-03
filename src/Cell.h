#pragma once
#include <string>

/**
 * Nodo de la matriz dispersa.
 * Cada celda no vacía es un nodo con punteros hacia la siguiente celda
 * en su misma fila y hacia la siguiente celda en su misma columna.
 */
struct Cell {
    int row;           // Fila (0-indexed internamente)
    int col;           // Columna (0-indexed internamente)
    std::string value; // Valor almacenado (numérico o texto)
    Cell* nextInRow;   // Siguiente nodo en la misma fila
    Cell* nextInCol;   // Siguiente nodo en la misma columna

    Cell(int r, int c, const std::string& v)
        : row(r), col(c), value(v),
          nextInRow(nullptr), nextInCol(nullptr) {}
};
