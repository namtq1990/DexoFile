#include "mainwindow.h"
#include "thememanager.h" // Include the ThemeManager header

#include <QApplication>
#include <QStyleFactory> // Required for QStyleFactory

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion")); // Set Fusion style

    // Apply the dark theme
    ThemeManager::applyDarkTheme(&a);

    MainWindow w;
    w.show();
    return a.exec();
}
