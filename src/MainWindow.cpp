#include "MainWindow.h"
#include "FormulaParser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QHeaderView>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QFont>
#include <QColor>
#include <QBrush>
#include <QTableWidgetItem>
#include <QSizePolicy>
#include <stdexcept>
#include <sstream>
#include <iomanip>

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Hoja de Cálculo — Matrices Dispersas");
    resize(1100, 700);
    setupUi();
    setupMenuBar();
    setupToolBar();
    refreshTable();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Setup UI
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::setupUi() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // ── Barra de fórmulas ──
    QHBoxLayout* formulaLayout = new QHBoxLayout();
    cellLabel = new QLabel("A1", this);
    cellLabel->setFixedWidth(60);
    cellLabel->setAlignment(Qt::AlignCenter);
    cellLabel->setStyleSheet("font-weight:bold; border:1px solid #aaa; "
                              "background:#f0f0f0; padding:2px;");

    formulaBar = new QLineEdit(this);
    formulaBar->setPlaceholderText("Ingresa un valor o fórmula (ej: =A1+B2)");
    formulaBar->setStyleSheet("font-size:13px; padding:2px 6px;");

    formulaLayout->addWidget(cellLabel);
    formulaLayout->addWidget(formulaBar);
    mainLayout->addLayout(formulaLayout);

    // ── Tabla principal ──
    table = new QTableWidget(INIT_ROWS, INIT_COLS, this);
    table->setFont(QFont("Monospace", 10));
    table->horizontalHeader()->setDefaultSectionSize(80);
    table->verticalHeader()->setDefaultSectionSize(22);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);

    QStringList colHdrs;
    for (int c = 0; c < INIT_COLS; c++) colHdrs << colHeader(c);
    table->setHorizontalHeaderLabels(colHdrs);

    QStringList rowHdrs;
    for (int r = 0; r < INIT_ROWS; r++) rowHdrs << QString::number(r + 1);
    table->setVerticalHeaderLabels(rowHdrs);

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    mainLayout->addWidget(table);

    // ── Barra de estado ──
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    status("Listo. Haz clic en una celda para editar.");

    // ── Señales ──
    connect(table, &QTableWidget::currentCellChanged,
            this, &MainWindow::onCellSelected);
    connect(formulaBar, &QLineEdit::returnPressed,
            this, &MainWindow::onFormulaBarReturnPressed);
    connect(table, &QTableWidget::cellChanged,
            this, &MainWindow::onTableCellChanged);
}

void MainWindow::setupMenuBar() {
    QMenuBar* mb = menuBar();

    // ── Edición ──
    QMenu* editMenu = mb->addMenu("&Edición");
    QAction* actDelRow = editMenu->addAction("Eliminar fila actual");
    connect(actDelRow, &QAction::triggered, this, &MainWindow::onDeleteRow);
    QAction* actDelCol = editMenu->addAction("Eliminar columna actual");
    connect(actDelCol, &QAction::triggered, this, &MainWindow::onDeleteColumn);
    QAction* actDelRng = editMenu->addAction("Eliminar rango...");
    connect(actDelRng, &QAction::triggered, this, &MainWindow::onDeleteRange);

    // ── Fórmulas ──
    QMenu* formulasMenu = mb->addMenu("&Fórmulas");
    QAction* actSumRow = formulasMenu->addAction("SUMA de fila...");
    QAction* actSumCol = formulasMenu->addAction("SUMA de columna...");
    QAction* actSumRng = formulasMenu->addAction("SUMA de rango...");
    formulasMenu->addSeparator();
    QAction* actAvgRow = formulasMenu->addAction("PROMEDIO de fila...");
    QAction* actAvgCol = formulasMenu->addAction("PROMEDIO de columna...");
    QAction* actAvgRng = formulasMenu->addAction("PROMEDIO de rango...");
    formulasMenu->addSeparator();
    QAction* actMaxRng = formulasMenu->addAction("MÁXIMO de rango...");
    QAction* actMinRng = formulasMenu->addAction("MÍNIMO de rango...");

    connect(actSumRow, &QAction::triggered, this, &MainWindow::onSumRow);
    connect(actSumCol, &QAction::triggered, this, &MainWindow::onSumColumn);
    connect(actSumRng, &QAction::triggered, this, &MainWindow::onSumRange);
    connect(actAvgRow, &QAction::triggered, this, &MainWindow::onAvgRow);
    connect(actAvgCol, &QAction::triggered, this, &MainWindow::onAvgColumn);
    connect(actAvgRng, &QAction::triggered, this, &MainWindow::onAvgRange);
    connect(actMaxRng, &QAction::triggered, this, &MainWindow::onMaxRange);
    connect(actMinRng, &QAction::triggered, this, &MainWindow::onMinRange);

    // ── Ayuda ──
    QMenu* helpMenu = mb->addMenu("A&yuda");
    QAction* actAbout = helpMenu->addAction("Acerca de...");
    connect(actAbout, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar() {
    QToolBar* tb = addToolBar("Operaciones rápidas");
    tb->setMovable(false);

    QAction* a;
    a = tb->addAction(QString::fromUtf8("Σ Fila"));
    a->setToolTip("Suma de la fila actual"); connect(a, &QAction::triggered, this, &MainWindow::onSumRow);
    a = tb->addAction(QString::fromUtf8("Σ Col"));
    a->setToolTip("Suma de la columna actual"); connect(a, &QAction::triggered, this, &MainWindow::onSumColumn);
    tb->addSeparator();
    a = tb->addAction("Del Fila");
    a->setToolTip("Eliminar fila actual"); connect(a, &QAction::triggered, this, &MainWindow::onDeleteRow);
    a = tb->addAction("Del Col");
    a->setToolTip("Eliminar columna actual"); connect(a, &QAction::triggered, this, &MainWindow::onDeleteColumn);
    tb->addSeparator();
    a = tb->addAction("Del Rango");
    a->setToolTip("Eliminar rango de celdas"); connect(a, &QAction::triggered, this, &MainWindow::onDeleteRange);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────

QString MainWindow::colHeader(int col) {
    return QString::fromStdString(CellAddress::colToStr(col));
}

void MainWindow::status(const QString& msg) {
    statusBar->showMessage(msg);
}

void MainWindow::styleCell(int row, int col, bool occupied) {
    QTableWidgetItem* item = table->item(row, col);
    if (!item) return;
    if (occupied) {
        item->setBackground(QBrush(QColor(220, 240, 255)));
        item->setForeground(QBrush(QColor(20, 50, 110)));
    } else {
        item->setBackground(QBrush(Qt::white));
        item->setForeground(QBrush(Qt::black));
    }
}

void MainWindow::ensureSize(int row, int col) {
    if (row >= table->rowCount()) {
        table->setRowCount(row + 10);
        QStringList rh;
        for (int r = 0; r < table->rowCount(); r++) rh << QString::number(r+1);
        table->setVerticalHeaderLabels(rh);
    }
    if (col >= table->columnCount()) {
        table->setColumnCount(col + 5);
        QStringList ch;
        for (int c = 0; c < table->columnCount(); c++) ch << colHeader(c);
        table->setHorizontalHeaderLabels(ch);
    }
}

std::pair<int,int> MainWindow::currentCell() const {
    return {table->currentRow(), table->currentColumn()};
}

std::string MainWindow::displayValue(int row, int col) {
    std::string raw = matrix.queryCell(row, col);
    if (!raw.empty() && raw[0] == '=') {
        try { return FormulaParser::evaluate(raw, matrix); }
        catch (...) { return "#ERR"; }
    }
    return raw;
}

void MainWindow::refreshTable() {
    blockTableSignals = true;

    for (int r = 0; r < table->rowCount(); r++) {
        for (int c = 0; c < table->columnCount(); c++) {
            if (!table->item(r, c))
                table->setItem(r, c, new QTableWidgetItem(""));
            else
                table->item(r, c)->setText("");
            styleCell(r, c, false);
        }
    }

    for (Cell* cell : matrix.getAllCells()) {
        int r = cell->row, c = cell->col;
        ensureSize(r, c);
        if (!table->item(r, c))
            table->setItem(r, c, new QTableWidgetItem(""));
        table->item(r, c)->setText(QString::fromStdString(displayValue(r, c)));
        styleCell(r, c, true);
    }

    blockTableSignals = false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Diálogos de entrada
// ─────────────────────────────────────────────────────────────────────────────

std::pair<int,int> MainWindow::askCellRef(const QString& title, bool* ok) {
    QString ref = QInputDialog::getText(this, title, "Referencia de celda (ej: B3):",
                                        QLineEdit::Normal, "", ok);
    if (ok && !*ok) return {-1,-1};
    try {
        return CellAddress::parse(ref.trimmed().toStdString());
    } catch (...) {
        QMessageBox::warning(this, "Error", "Referencia inválida: " + ref);
        if (ok) *ok = false;
        return {-1,-1};
    }
}

int MainWindow::askRow(const QString& title, bool* ok) {
    auto [curRow, curCol] = currentCell();
    int row = QInputDialog::getInt(this, title, "Número de fila (1-indexed):",
                                    curRow + 1, 1, 9999, 1, ok);
    return row - 1;
}

int MainWindow::askCol(const QString& title, bool* ok) {
    auto [curRow, curCol] = currentCell();
    QString col = QInputDialog::getText(this, title,
                    "Letra(s) de columna (ej: A, B, AA):",
                    QLineEdit::Normal,
                    QString::fromStdString(CellAddress::colToStr(curCol)), ok);
    if (ok && !*ok) return -1;
    try {
        return CellAddress::strToCol(col.trimmed().toStdString());
    } catch (...) {
        QMessageBox::warning(this, "Error", "Columna inválida: " + col);
        if (ok) *ok = false;
        return -1;
    }
}

std::tuple<int,int,int,int> MainWindow::askRange(const QString& title, bool* ok) {
    QString range = QInputDialog::getText(this, title,
                        "Rango de celdas (ej: A1:C4):",
                        QLineEdit::Normal, "", ok);
    if (ok && !*ok) return {-1,-1,-1,-1};
    QString s = range.trimmed().toUpper();
    int colon = s.indexOf(':');
    if (colon < 0) {
        QMessageBox::warning(this, "Error", "Formato inválido. Use A1:C4");
        if (ok) *ok = false;
        return {-1,-1,-1,-1};
    }
    try {
        auto [r1,c1] = CellAddress::parse(s.left(colon).toStdString());
        auto [r2,c2] = CellAddress::parse(s.mid(colon+1).toStdString());
        return {r1,c1,r2,c2};
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
        if (ok) *ok = false;
        return {-1,-1,-1,-1};
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Slots — Edición de celdas
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onCellSelected(int row, int col) {
    if (row < 0 || col < 0) return;
    cellLabel->setText(QString::fromStdString(CellAddress::toString(row, col)));
    std::string raw = matrix.queryCell(row, col);
    blockTableSignals = true;
    formulaBar->setText(QString::fromStdString(raw));
    blockTableSignals = false;
}

void MainWindow::onFormulaBarReturnPressed() {
    auto [row, col] = currentCell();
    if (row < 0 || col < 0) { status("Selecciona una celda primero."); return; }

    std::string raw = formulaBar->text().trimmed().toStdString();
    if (raw.empty()) {
        matrix.deleteCell(row, col);
        status(QString("Celda %1 eliminada.").arg(
               QString::fromStdString(CellAddress::toString(row, col))));
    } else {
        matrix.insertCell(row, col, raw);
        status(QString("Celda %1 actualizada.").arg(
               QString::fromStdString(CellAddress::toString(row, col))));
    }
    refreshTable();
    table->setCurrentCell(row, col);
}

void MainWindow::onTableCellChanged(int row, int col) {
    if (blockTableSignals) return;
    QTableWidgetItem* item = table->item(row, col);
    std::string text = item ? item->text().trimmed().toStdString() : "";

    if (text.empty())
        matrix.deleteCell(row, col);
    else
        matrix.insertCell(row, col, text);

    blockTableSignals = true;
    std::string disp = displayValue(row, col);
    if (item) { item->setText(QString::fromStdString(disp)); styleCell(row, col, !text.empty()); }
    blockTableSignals = false;

    formulaBar->setText(QString::fromStdString(matrix.queryCell(row, col)));
    status(QString("Celda %1 modificada.").arg(
           QString::fromStdString(CellAddress::toString(row, col))));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Slots — Operaciones de fila/columna
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onDeleteRow() {
    bool ok; int row = askRow("Eliminar fila", &ok);
    if (!ok) return;
    matrix.deleteRow(row); refreshTable();
    status(QString("Fila %1 eliminada.").arg(row + 1));
}

void MainWindow::onDeleteColumn() {
    bool ok; int col = askCol("Eliminar columna", &ok);
    if (!ok) return;
    matrix.deleteColumn(col); refreshTable();
    status("Columna " + QString::fromStdString(CellAddress::colToStr(col)) + " eliminada.");
}

void MainWindow::onDeleteRange() {
    bool ok; auto [r1,c1,r2,c2] = askRange("Eliminar rango", &ok);
    if (!ok) return;
    matrix.deleteRange(r1,c1,r2,c2); refreshTable();
    status("Rango eliminado.");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Slots — Agregaciones
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onSumRow() {
    bool ok; int row = askRow("SUMA de fila", &ok); if (!ok) return;
    double r = matrix.sumRow(row);
    QMessageBox::information(this, "SUMA fila " + QString::number(row+1),
                             QString("Resultado: %1").arg(r));
    status(QString("SUMA fila %1 = %2").arg(row+1).arg(r));
}

void MainWindow::onSumColumn() {
    bool ok; int col = askCol("SUMA de columna", &ok); if (!ok) return;
    double r = matrix.sumColumn(col);
    QMessageBox::information(this, "SUMA columna " +
        QString::fromStdString(CellAddress::colToStr(col)),
        QString("Resultado: %1").arg(r));
    status(QString("SUMA col %1 = %2").arg(
           QString::fromStdString(CellAddress::colToStr(col))).arg(r));
}

void MainWindow::onSumRange() {
    bool ok; auto [r1,c1,r2,c2] = askRange("SUMA de rango", &ok); if (!ok) return;
    double r = matrix.sumRange(r1,c1,r2,c2);
    QMessageBox::information(this, "SUMA rango", QString("Resultado: %1").arg(r));
    status(QString("SUMA rango = %1").arg(r));
}

void MainWindow::onAvgRow() {
    bool ok; int row = askRow("PROMEDIO de fila", &ok); if (!ok) return;
    double r = matrix.avgRow(row);
    QMessageBox::information(this, "PROMEDIO fila " + QString::number(row+1),
                             QString("Resultado: %1").arg(r));
    status(QString("PROMEDIO fila %1 = %2").arg(row+1).arg(r));
}

void MainWindow::onAvgColumn() {
    bool ok; int col = askCol("PROMEDIO de columna", &ok); if (!ok) return;
    double r = matrix.avgColumn(col);
    QMessageBox::information(this, "PROMEDIO columna " +
        QString::fromStdString(CellAddress::colToStr(col)),
        QString("Resultado: %1").arg(r));
    status(QString("PROMEDIO col %1 = %2").arg(
           QString::fromStdString(CellAddress::colToStr(col))).arg(r));
}

void MainWindow::onAvgRange() {
    bool ok; auto [r1,c1,r2,c2] = askRange("PROMEDIO de rango", &ok); if (!ok) return;
    double r = matrix.avgRange(r1,c1,r2,c2);
    QMessageBox::information(this, "PROMEDIO rango", QString("Resultado: %1").arg(r));
    status(QString("PROMEDIO rango = %1").arg(r));
}

void MainWindow::onMaxRange() {
    bool ok; auto [r1,c1,r2,c2] = askRange("MÁXIMO de rango", &ok); if (!ok) return;
    try {
        double r = matrix.maxRange(r1,c1,r2,c2);
        QMessageBox::information(this, "MÁXIMO rango", QString("Resultado: %1").arg(r));
        status(QString("MÁXIMO rango = %1").arg(r));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Aviso", QString::fromStdString(e.what()));
    }
}

void MainWindow::onMinRange() {
    bool ok; auto [r1,c1,r2,c2] = askRange("MÍNIMO de rango", &ok); if (!ok) return;
    try {
        double r = matrix.minRange(r1,c1,r2,c2);
        QMessageBox::information(this, "MÍNIMO rango", QString("Resultado: %1").arg(r));
        status(QString("MÍNIMO rango = %1").arg(r));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Aviso", QString::fromStdString(e.what()));
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "Acerca del proyecto",
        "<b>Hoja de Cálculo con Matrices Dispersas</b><br><br>"
        "Proyecto #1 — Algoritmos y Estructuras de Datos<br><br>"
        "Implementa una matriz dispersa con listas enlazadas cruzadas.<br>"
        "Cada nodo almacena (fila, columna, valor) con punteros<br>"
        "<tt>nextInRow</tt> y <tt>nextInCol</tt>.<br><br>"
        "<b>Fórmulas:</b> comienza con '=', soporta +, -, *, / y refs de celda.<br>"
        "Ejemplo: <tt>=A1+B2*3</tt><br><br>"
        "Complejidad de inserción/eliminación: <b>O(k)</b> donde k es<br>"
        "el número de elementos en la fila o columna.");
}
