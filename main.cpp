#include "mainwindow.h"
#include "application.h" // Include the nucare::Application header
#include "thememanager.h" // Include the ThemeManager header

#include <QStyleFactory> // Required for QStyleFactory

int main(int argc, char *argv[])
{
    nucare::Application app(argc, argv);
    app.initialize(); // Initialize application settings

    return app.exec();
}
