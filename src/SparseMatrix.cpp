#include "SparseMatrix.h"
#include <stdexcept>
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────

SparseMatrix::SparseMatrix() {}

SparseMatrix::~SparseMatrix() {
    // Liberar todos los nodos recorriendo fila por fila
    for (auto& [row, head] : rowHeaders) {
        Cell* cur = head;
        while (cur) {
            Cell* next = cur->nextInRow;
            delete cur;
            cur = next;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers privados
// ─────────────────────────────────────────────────────────────────────────────

Cell* SparseMatrix::findCell(int row, int col) const {
    auto it = rowHeaders.find(row);
    if (it == rowHeaders.end()) return nullptr;
    Cell* cur = it->second;
    while (cur && cur->col < col) cur = cur->nextInRow;
    if (cur && cur->col == col) return cur;
    return nullptr;
}

Cell* SparseMatrix::findPrevInRow(int row, int col) const {
    auto it = rowHeaders.find(row);
    if (it == rowHeaders.end() || it->second == nullptr) return nullptr;
    if (it->second->col >= col) return nullptr; // el buscado es el primero
    Cell* cur = it->second;
    while (cur->nextInRow && cur->nextInRow->col < col)
        cur = cur->nextInRow;
    return cur;
}

Cell* SparseMatrix::findPrevInCol(int row, int col) const {
    auto it = colHeaders.find(col);
    if (it == colHeaders.end() || it->second == nullptr) return nullptr;
    if (it->second->row >= row) return nullptr;
    Cell* cur = it->second;
    while (cur->nextInCol && cur->nextInCol->row < row)
        cur = cur->nextInCol;
    return cur;
}

void SparseMatrix::unlinkFromRow(Cell* node) {
    int row = node->row, col = node->col;
    Cell* prev = findPrevInRow(row, col);
    if (prev) {
        prev->nextInRow = node->nextInRow;
    } else {
        // Era el primero de la fila
        if (node->nextInRow)
            rowHeaders[row] = node->nextInRow;
        else
            rowHeaders.erase(row);
    }
}

void SparseMatrix::unlinkFromCol(Cell* node) {
    int row = node->row, col = node->col;
    Cell* prev = findPrevInCol(row, col);
    if (prev) {
        prev->nextInCol = node->nextInCol;
    } else {
        if (node->nextInCol)
            colHeaders[col] = node->nextInCol;
        else
            colHeaders.erase(col);
    }
}

bool SparseMatrix::toDouble(const std::string& value, double& out) {
    if (value.empty()) return false;
    try {
        size_t pos;
        out = std::stod(value, &pos);
        return pos == value.size(); // Toda la cadena es numérica
    } catch (...) {
        return false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Operaciones básicas
// ─────────────────────────────────────────────────────────────────────────────

void SparseMatrix::insertCell(int row, int col, const std::string& value) {
    // Si ya existe, solo actualizar
    Cell* existing = findCell(row, col);
    if (existing) {
        existing->value = value;
        return;
    }

    Cell* node = new Cell(row, col, value);

    // ── Insertar en la lista de la fila (ordenado por col) ──
    auto rowIt = rowHeaders.find(row);
    if (rowIt == rowHeaders.end() || rowIt->second == nullptr || rowIt->second->col > col) {
        // Insertar al inicio de la fila
        node->nextInRow = (rowIt != rowHeaders.end()) ? rowIt->second : nullptr;
        rowHeaders[row] = node;
    } else {
        // Recorrer para encontrar la posición de inserción ordenada por col
        Cell* p = rowHeaders[row];
        while (p->nextInRow && p->nextInRow->col < col) p = p->nextInRow;
        node->nextInRow = p->nextInRow;
        p->nextInRow = node;
    }

    // ── Insertar en la lista de la columna (ordenado por row) ──
    auto colIt = colHeaders.find(col);
    if (colIt == colHeaders.end() || colIt->second == nullptr || colIt->second->row > row) {
        node->nextInCol = (colIt != colHeaders.end()) ? colIt->second : nullptr;
        colHeaders[col] = node;
    } else {
        Cell* p = colHeaders[col];
        while (p->nextInCol && p->nextInCol->row < row) p = p->nextInCol;
        node->nextInCol = p->nextInCol;
        p->nextInCol = node;
    }
}

std::string SparseMatrix::queryCell(int row, int col) const {
    Cell* node = findCell(row, col);
    return node ? node->value : "";
}

bool SparseMatrix::modifyCell(int row, int col, const std::string& value) {
    Cell* node = findCell(row, col);
    if (!node) return false;
    node->value = value;
    return true;
}

bool SparseMatrix::deleteCell(int row, int col) {
    Cell* node = findCell(row, col);
    if (!node) return false;

    unlinkFromRow(node);
    unlinkFromCol(node);
    delete node;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Operaciones sobre filas y columnas
// ─────────────────────────────────────────────────────────────────────────────

void SparseMatrix::deleteRow(int row) {
    auto it = rowHeaders.find(row);
    if (it == rowHeaders.end()) return;

    Cell* cur = it->second;
    while (cur) {
        Cell* next = cur->nextInRow;
        unlinkFromCol(cur);
        delete cur;
        cur = next;
    }
    rowHeaders.erase(row);
}

void SparseMatrix::deleteColumn(int col) {
    auto it = colHeaders.find(col);
    if (it == colHeaders.end()) return;

    Cell* cur = it->second;
    while (cur) {
        Cell* next = cur->nextInCol;
        unlinkFromRow(cur);
        delete cur;
        cur = next;
    }
    colHeaders.erase(col);
}

void SparseMatrix::deleteRange(int r1, int c1, int r2, int c2) {
    if (r1 > r2) std::swap(r1, r2);
    if (c1 > c2) std::swap(c1, c2);

    // Recolectar nodos a eliminar para no invalidar iteradores
    std::vector<std::pair<int,int>> toDelete;
    for (auto& [row, head] : rowHeaders) {
        if (row < r1 || row > r2) continue;
        Cell* cur = head;
        while (cur) {
            if (cur->col >= c1 && cur->col <= c2)
                toDelete.push_back({cur->row, cur->col});
            cur = cur->nextInRow;
        }
    }
    for (auto& [r, c] : toDelete)
        deleteCell(r, c);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Agregaciones
// ─────────────────────────────────────────────────────────────────────────────

double SparseMatrix::sumRow(int row) const {
    auto it = rowHeaders.find(row);
    if (it == rowHeaders.end()) return 0.0;
    double sum = 0.0;
    for (Cell* cur = it->second; cur; cur = cur->nextInRow) {
        double v; if (toDouble(cur->value, v)) sum += v;
    }
    return sum;
}

double SparseMatrix::sumColumn(int col) const {
    auto it = colHeaders.find(col);
    if (it == colHeaders.end()) return 0.0;
    double sum = 0.0;
    for (Cell* cur = it->second; cur; cur = cur->nextInCol) {
        double v; if (toDouble(cur->value, v)) sum += v;
    }
    return sum;
}

double SparseMatrix::sumRange(int r1, int c1, int r2, int c2) const {
    if (r1 > r2) std::swap(r1, r2);
    if (c1 > c2) std::swap(c1, c2);
    double sum = 0.0;
    for (auto& [row, head] : rowHeaders) {
        if (row < r1 || row > r2) continue;
        for (Cell* cur = head; cur; cur = cur->nextInRow) {
            if (cur->col < c1 || cur->col > c2) continue;
            double v; if (toDouble(cur->value, v)) sum += v;
        }
    }
    return sum;
}

double SparseMatrix::avgRow(int row) const {
    auto it = rowHeaders.find(row);
    if (it == rowHeaders.end()) return 0.0;
    double sum = 0.0; int count = 0;
    for (Cell* cur = it->second; cur; cur = cur->nextInRow) {
        double v; if (toDouble(cur->value, v)) { sum += v; count++; }
    }
    return count ? sum / count : 0.0;
}

double SparseMatrix::avgColumn(int col) const {
    auto it = colHeaders.find(col);
    if (it == colHeaders.end()) return 0.0;
    double sum = 0.0; int count = 0;
    for (Cell* cur = it->second; cur; cur = cur->nextInCol) {
        double v; if (toDouble(cur->value, v)) { sum += v; count++; }
    }
    return count ? sum / count : 0.0;
}

double SparseMatrix::avgRange(int r1, int c1, int r2, int c2) const {
    if (r1 > r2) std::swap(r1, r2);
    if (c1 > c2) std::swap(c1, c2);
    double sum = 0.0; int count = 0;
    for (auto& [row, head] : rowHeaders) {
        if (row < r1 || row > r2) continue;
        for (Cell* cur = head; cur; cur = cur->nextInRow) {
            if (cur->col < c1 || cur->col > c2) continue;
            double v; if (toDouble(cur->value, v)) { sum += v; count++; }
        }
    }
    return count ? sum / count : 0.0;
}

double SparseMatrix::maxRange(int r1, int c1, int r2, int c2) const {
    if (r1 > r2) std::swap(r1, r2);
    if (c1 > c2) std::swap(c1, c2);
    double maxVal = -std::numeric_limits<double>::infinity();
    bool found = false;
    for (auto& [row, head] : rowHeaders) {
        if (row < r1 || row > r2) continue;
        for (Cell* cur = head; cur; cur = cur->nextInRow) {
            if (cur->col < c1 || cur->col > c2) continue;
            double v;
            if (toDouble(cur->value, v)) { maxVal = std::max(maxVal, v); found = true; }
        }
    }
    if (!found) throw std::runtime_error("No hay valores numéricos en el rango.");
    return maxVal;
}

double SparseMatrix::minRange(int r1, int c1, int r2, int c2) const {
    if (r1 > r2) std::swap(r1, r2);
    if (c1 > c2) std::swap(c1, c2);
    double minVal = std::numeric_limits<double>::infinity();
    bool found = false;
    for (auto& [row, head] : rowHeaders) {
        if (row < r1 || row > r2) continue;
        for (Cell* cur = head; cur; cur = cur->nextInRow) {
            if (cur->col < c1 || cur->col > c2) continue;
            double v;
            if (toDouble(cur->value, v)) { minVal = std::min(minVal, v); found = true; }
        }
    }
    if (!found) throw std::runtime_error("No hay valores numéricos en el rango.");
    return minVal;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Utilidades
// ─────────────────────────────────────────────────────────────────────────────

std::vector<Cell*> SparseMatrix::getAllCells() const {
    std::vector<Cell*> cells;
    for (auto& [row, head] : rowHeaders)
        for (Cell* cur = head; cur; cur = cur->nextInRow)
            cells.push_back(cur);
    return cells;
}

int SparseMatrix::getMaxRow() const {
    int maxRow = -1;
    for (auto& [row, _] : rowHeaders) maxRow = std::max(maxRow, row);
    return maxRow;
}

int SparseMatrix::getMaxCol() const {
    int maxCol = -1;
    for (auto& [col, _] : colHeaders) maxCol = std::max(maxCol, col);
    return maxCol;
}

bool SparseMatrix::isEmpty() const {
    return rowHeaders.empty();
}
