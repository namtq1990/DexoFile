#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QFont>

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    explicit ThemeManager(QObject *parent = nullptr);

    static void applyDarkTheme(QApplication *app);
    static void applyLightTheme(QApplication *app); // Example for a light theme
    static void applyBaseSettings(QApplication *app);

signals:

};

#endif // THEMEMANAGER_H
