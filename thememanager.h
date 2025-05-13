#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QFont>
#include "component/component.h" // Include the Component base class

class ThemeManager : public QObject, virtual public Component
{
    Q_OBJECT
public:
    explicit ThemeManager(QObject *parent = nullptr);

    // Theme application methods are now instance methods
    void applyDarkTheme(QApplication *app);
    void applyLightTheme(QApplication *app); // Example for a light theme
    void applyBaseSettings(QApplication *app);

    // Getters for theme-specific properties
    QColor listHeaderTextColor() const;
    QFont listHeaderFont() const;

    // Setters (to be called by static applyTheme methods)
    void setListHeaderTextColor(const QColor& color);
    void setListHeaderFont(const QFont& font);

signals:

private:
    QColor m_listHeaderTextColor;
    QFont m_listHeaderFont;
};

#endif // THEMEMANAGER_H
