/**
 * @file main.cpp
 * @brief GUI entry point for LSL Template application
 */

#include "MainWindow.hpp"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("LSL Template");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("LabStreamingLayer");
    app.setOrganizationDomain("labstreaminglayer.org");

    // Get config file from command line if provided
    QString config_file;
    if (argc > 1) {
        config_file = QString::fromLocal8Bit(argv[1]);
    }

    MainWindow window(config_file);
    window.show();

    return app.exec();
}
