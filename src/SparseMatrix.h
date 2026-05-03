#pragma once
#include "Cell.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <limits>
#include <stdexcept>

/**
 * Matriz dispersa implementada con listas enlazadas cruzadas.
 *
 * Estructura:
 *   - rowHeaders[r] apunta al primer nodo de la fila r.
 *   - colHeaders[c] apunta al primer nodo de la columna c.
 *   - Cada nodo tiene punteros nextInRow y nextInCol, mantenidos
 *     ordenados por índice creciente dentro de cada fila/columna.
 *
 * Complejidad:
 *   - Inserción/búsqueda/eliminación en fila: O(k_r) donde k_r es el
 *     número de elementos en esa fila. Ídem para columna.
 *   - Mucho mejor que O(n·m) de una matriz densa.
 */
class SparseMatrix {
public:
    SparseMatrix();
    ~SparseMatrix();

    // ── Operaciones básicas de celda ──────────────────────────────
    /** Inserta o actualiza el valor en (row, col). O(k_r + k_c). */
    void insertCell(int row, int col, const std::string& value);

    /** Retorna el valor en (row, col) o "" si no existe. O(k_r). */
    std::string queryCell(int row, int col) const;

    /** Modifica in situ el valor de un nodo existente. O(k_r). */
    bool modifyCell(int row, int col, const std::string& value);

    /** Elimina el nodo en (row, col). O(k_r + k_c). */
    bool deleteCell(int row, int col);

    // ── Operaciones sobre filas y columnas ───────────────────────
    /** Elimina todos los nodos de la fila dada. */
    void deleteRow(int row);

    /** Elimina todos los nodos de la columna dada. */
    void deleteColumn(int col);

    /** Elimina todos los nodos dentro del rango rectangular. */
    void deleteRange(int r1, int c1, int r2, int c2);

    // ── Operaciones de agregación ─────────────────────────────────
    double sumRow(int row) const;
    double sumColumn(int col) const;
    double sumRange(int r1, int c1, int r2, int c2) const;

    double avgRow(int row) const;
    double avgColumn(int col) const;
    double avgRange(int r1, int c1, int r2, int c2) const;

    double maxRange(int r1, int c1, int r2, int c2) const;
    double minRange(int r1, int c1, int r2, int c2) const;

    // ── Utilidades ────────────────────────────────────────────────
    /** Retorna todos los nodos existentes. */
    std::vector<Cell*> getAllCells() const;

    int getMaxRow() const;
    int getMaxCol() const;
    bool isEmpty() const;

private:
    // Cabeceras de filas: rowHeaders[fila] -> primer Cell de esa fila
    std::unordered_map<int, Cell*> rowHeaders;
    // Cabeceras de columnas: colHeaders[col] -> primer Cell de esa columna
    std::unordered_map<int, Cell*> colHeaders;

    /** Busca el nodo en (row,col). Retorna nullptr si no existe. */
    Cell* findCell(int row, int col) const;

    /** Retorna el nodo anterior al de columna col en la fila row,
     *  o nullptr si el nodo es el primero. */
    Cell* findPrevInRow(int row, int col) const;

    /** Retorna el nodo anterior al de fila row en la columna col,
     *  o nullptr si el nodo es el primero. */
    Cell* findPrevInCol(int row, int col) const;

    /** Desvincula el nodo de la lista de su fila. No lo libera. */
    void unlinkFromRow(Cell* node);

    /** Desvincula el nodo de la lista de su columna. No lo libera. */
    void unlinkFromCol(Cell* node);

    /** Intenta convertir value a double. Retorna false si no es numérico. */
    static bool toDouble(const std::string& value, double& out);
};
