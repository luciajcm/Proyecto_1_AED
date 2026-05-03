#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Hoja de Cálculo - Matrices Dispersas");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Proyecto #1 — EDA");

    // Estilo moderno
    app.setStyle("Fusion");

    MainWindow window;
    window.show();

    return app.exec();
}
