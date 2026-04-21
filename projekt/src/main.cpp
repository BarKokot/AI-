#include "MainWindow.h"
#include <QApplication>
#include <QFont>
#include <QIcon>

/**
 * @file main.cpp
 * @brief Punkt wejścia aplikacji FilterDesigner.
 *
 * Inicjalizuje aplikację Qt, ustawia czcionkę i uruchamia główne okno.
 */

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("FilterDesigner");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("JPO2026");

    QFont font("Segoe UI", 10);
    app.setFont(font);

    MainWindow window;
    window.show();

    return app.exec();
}
