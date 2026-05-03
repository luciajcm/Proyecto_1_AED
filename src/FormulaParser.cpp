#include "FormulaParser.h"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
//  Punto de entrada público
// ─────────────────────────────────────────────────────────────────────────────

std::string FormulaParser::evaluate(const std::string& formula,
                                    const SparseMatrix& matrix) {
    if (formula.empty() || formula[0] != '=')
        throw std::runtime_error("La fórmula debe comenzar con '='.");

    // Extraer la expresión (sin el '=')
    std::string expr = formula.substr(1);
    FormulaParser parser(expr, matrix);

    double result = parser.parseExpression();
    parser.skipSpaces();
    if (parser.pos != parser.src.size())
        throw std::runtime_error("Carácter inesperado en posición " +
                                 std::to_string(parser.pos));

    // Formatear: si es entero exacto, mostrar sin decimales
    std::ostringstream oss;
    if (result == static_cast<long long>(result))
        oss << static_cast<long long>(result);
    else
        oss << std::setprecision(10) << result;
    return oss.str();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Parser (descenso recursivo)
// ─────────────────────────────────────────────────────────────────────────────

char FormulaParser::peek() {
    skipSpaces();
    if (pos >= src.size()) return '\0';
    return src[pos];
}

char FormulaParser::consume() {
    skipSpaces();
    if (pos >= src.size()) throw std::runtime_error("Fin inesperado de expresión.");
    return src[pos++];
}

void FormulaParser::skipSpaces() {
    while (pos < src.size() && src[pos] == ' ') pos++;
}

double FormulaParser::parseExpression() {
    double result = parseTerm();
    while (true) {
        char op = peek();
        if (op == '+' || op == '-') {
            consume();
            double right = parseTerm();
            result = (op == '+') ? result + right : result - right;
        } else {
            break;
        }
    }
    return result;
}

double FormulaParser::parseTerm() {
    double result = parseFactor();
    while (true) {
        char op = peek();
        if (op == '*' || op == '/') {
            consume();
            double right = parseFactor();
            if (op == '/' && right == 0.0)
                throw std::runtime_error("División por cero.");
            result = (op == '*') ? result * right : result / right;
        } else {
            break;
        }
    }
    return result;
}

double FormulaParser::parseFactor() {
    skipSpaces();
    char c = peek();

    // Subexpresión entre paréntesis
    if (c == '(') {
        consume(); // '('
        double val = parseExpression();
        skipSpaces();
        if (peek() != ')')
            throw std::runtime_error("Se esperaba ')'.");
        consume(); // ')'
        return val;
    }

    // Signo negativo unario
    if (c == '-') {
        consume();
        return -parseFactor();
    }

    // Referencia de celda: empieza con letra
    if (isalpha(c)) return readCellRef();

    // Número
    if (isdigit(c) || c == '.') return readNumber();

    throw std::runtime_error(std::string("Carácter inesperado: '") + c + "'");
}

double FormulaParser::readNumber() {
    size_t start = pos;
    while (pos < src.size() && (isdigit(src[pos]) || src[pos] == '.')) pos++;
    if (start == pos) throw std::runtime_error("Se esperaba un número.");
    return std::stod(src.substr(start, pos - start));
}

double FormulaParser::readCellRef() {
    // Leer letras
    size_t start = pos;
    while (pos < src.size() && isalpha(src[pos])) pos++;
    std::string colStr = src.substr(start, pos - start);

    // Leer dígitos
    size_t numStart = pos;
    while (pos < src.size() && isdigit(src[pos])) pos++;
    if (pos == numStart)
        throw std::runtime_error("Referencia de celda inválida: falta número de fila.");

    std::string ref = colStr + src.substr(numStart, pos - numStart);
    auto [row, col] = CellAddress::parse(ref);

    std::string val = matrix.queryCell(row, col);
    if (val.empty()) return 0.0; // Celda vacía cuenta como 0

    double result;
    // Intentar parsear recursivamente si es fórmula
    if (!val.empty() && val[0] == '=') {
        std::string sub = FormulaParser::evaluate(val, matrix);
        try { result = std::stod(sub); } catch (...) { result = 0.0; }
    } else {
        try { result = std::stod(val); }
        catch (...) {
            throw std::runtime_error("La celda " + ref +
                                     " contiene texto, no un número.");
        }
    }
    return result;
}
