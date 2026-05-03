#pragma once
#include "SparseMatrix.h"
#include <string>
#include <stdexcept>

/**
 * Utilidades de conversión entre notación Excel (A1, B3, AA10...)
 * y coordenadas (row, col) 0-indexadas.
 */
namespace CellAddress {

    /** Convierte índice de columna 0-indexed a letras: 0->"A", 25->"Z", 26->"AA" */
    inline std::string colToStr(int col) {
        std::string result;
        col++;
        while (col > 0) {
            col--;
            result = char('A' + col % 26) + result;
            col /= 26;
        }
        return result;
    }

    /** Convierte letras de columna a índice 0-indexed: "A"->0, "Z"->25, "AA"->26 */
    inline int strToCol(const std::string& s) {
        int result = 0;
        for (char c : s) result = result * 26 + (toupper(c) - 'A' + 1);
        return result - 1;
    }

    /**
     * Parsea una referencia de celda tipo "A1", "B23", "AA100".
     * Retorna {row, col} 0-indexados.
     * Lanza std::invalid_argument si el formato es inválido.
     */
    inline std::pair<int,int> parse(const std::string& ref) {
        if (ref.empty()) throw std::invalid_argument("Referencia vacía.");
        size_t i = 0;
        std::string colStr;
        while (i < ref.size() && isalpha(ref[i])) colStr += toupper(ref[i++]);
        if (colStr.empty() || i == ref.size())
            throw std::invalid_argument("Referencia de celda inválida: " + ref);
        int rowNum = std::stoi(ref.substr(i)) - 1;
        int colNum = strToCol(colStr);
        if (rowNum < 0 || colNum < 0)
            throw std::invalid_argument("Coordenadas negativas en: " + ref);
        return {rowNum, colNum};
    }

    /** Serializa coordenadas 0-indexed a notación Excel: (0,0)->"A1" */
    inline std::string toString(int row, int col) {
        return colToStr(col) + std::to_string(row + 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────

/**
 * Parser de fórmulas aritméticas.
 *
 * Gramática soportada:
 *   formula    → '=' expression
 *   expression → term (('+' | '-') term)*
 *   term       → factor (('*' | '/') factor)*
 *   factor     → NUMBER | CELL_REF | '(' expression ')'
 *
 * Los operandos pueden ser literales numéricos o referencias a celdas.
 * Ejemplo: =A1+B2*3-C4/2
 */
class FormulaParser {
public:
    /**
     * Evalúa la fórmula dada usando la matriz para resolver referencias.
     * La cadena debe comenzar con '='.
     * Retorna el resultado como string (double formateado).
     * Lanza std::runtime_error si hay error de sintaxis o referencia circular.
     */
    static std::string evaluate(const std::string& formula,
                                const SparseMatrix& matrix);

private:
    const std::string& src;
    size_t pos;
    const SparseMatrix& matrix;

    FormulaParser(const std::string& s, const SparseMatrix& m)
        : src(s), pos(0), matrix(m) {}

    double parseExpression();
    double parseTerm();
    double parseFactor();

    void skipSpaces();
    double readNumber();
    double readCellRef();
    char peek();
    char consume();
};
