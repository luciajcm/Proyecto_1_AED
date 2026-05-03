#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QStatusBar>
#include <QAction>
#include <QToolBar>
#include "SparseMatrix.h"
#include "FormulaParser.h"

/**
 * Ventana principal de la hoja de cálculo.
 *
 * Componentes de la UI:
 *  - Barra de herramientas: celda activa + barra de fórmulas
 *  - QTableWidget: cuadrícula principal (celdas vacías no tienen datos)
 *  - Barra de menú: operaciones de fila/columna y agregaciones
 *  - Barra de estado: mensajes al usuario
 *
 * Todas las operaciones van a través de SparseMatrix; QTableWidget
 * es solo una vista que se refresca tras cada cambio.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    // ── Edición de celdas ────────────────────────────────────────
    void onCellSelected(int row, int col);
    void onFormulaBarReturnPressed();
    void onTableCellChanged(int row, int col);

    // ── Operaciones de fila/columna (menú) ───────────────────────
    void onDeleteRow();
    void onDeleteColumn();
    void onDeleteRange();

    // ── Agregaciones ─────────────────────────────────────────────
    void onSumRow();
    void onSumColumn();
    void onSumRange();
    void onAvgRow();
    void onAvgColumn();
    void onAvgRange();
    void onMaxRange();
    void onMinRange();

    // ── Ayuda ────────────────────────────────────────────────────
    void onAbout();

private:
    // ── Datos ────────────────────────────────────────────────────
    SparseMatrix matrix;

    // ── Widgets ──────────────────────────────────────────────────
    QTableWidget*  table;
    QLineEdit*     formulaBar;
    QLabel*        cellLabel;
    QStatusBar*    statusBar;

    // ── Control de señales ───────────────────────────────────────
    bool blockTableSignals = false;

    // ── Dimensiones de la cuadrícula ─────────────────────────────
    static constexpr int INIT_ROWS = 50;
    static constexpr int INIT_COLS = 26;

    // ── Helpers ──────────────────────────────────────────────────
    void setupUi();
    void setupMenuBar();
    void setupToolBar();

    /** Refresca la cuadrícula completa desde la matriz. */
    void refreshTable();

    /** Obtiene la celda actualmente seleccionada. */
    std::pair<int,int> currentCell() const;

    /** Muestra un mensaje en la barra de estado. */
    void status(const QString& msg);

    /** Convierte índice de columna 0-indexed a letra(s). */
    static QString colHeader(int col);

    /** Aplica estilo visual a una celda de la tabla. */
    void styleCell(int row, int col, bool occupied);

    /** Expande la tabla si row/col superan el tamaño actual. */
    void ensureSize(int row, int col);

    /** Retorna el valor mostrado en la celda (puede ser resultado de fórmula). */
    std::string displayValue(int row, int col);

    /** Pide al usuario una referencia de celda única. */
    std::pair<int,int> askCellRef(const QString& title, bool* ok = nullptr);

    /** Pide al usuario un número de fila (1-indexed). */
    int askRow(const QString& title, bool* ok = nullptr);

    /** Pide al usuario un número de columna (letras). */
    int askCol(const QString& title, bool* ok = nullptr);

    /** Pide al usuario un rango "A1:C4". Retorna {r1,c1,r2,c2}. */
    std::tuple<int,int,int,int> askRange(const QString& title, bool* ok = nullptr);
};
