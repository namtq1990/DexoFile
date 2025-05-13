#include "thememanager.h"
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QStyle>

// Helper function to create the standard list header font
namespace { // Anonymous namespace for helper
    QFont createListHeaderFont() {
        QFont font("Pretendard", 6);
        font.setBold(true);
        return font;
    }
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , Component("ThemeManager") // Initialize Component base class
    , m_listHeaderTextColor(Qt::black) // Default for light theme or un-themed
    , m_listHeaderFont(createListHeaderFont()) // Initialize with helper
{
    // Further specific initializations if needed
}

// Getters
QColor ThemeManager::listHeaderTextColor() const
{
    return m_listHeaderTextColor;
}

QFont ThemeManager::listHeaderFont() const
{
    return m_listHeaderFont;
}

// Setters
void ThemeManager::setListHeaderTextColor(const QColor& color)
{
    m_listHeaderTextColor = color;
}

void ThemeManager::setListHeaderFont(const QFont& font)
{
    m_listHeaderFont = font;
}

// Instance method
void ThemeManager::applyBaseSettings(QApplication *app)
{
    if (!app)
        return;

    QFont defaultFont("Pretendard", 6);
    app->setFont(defaultFont);
}

void ThemeManager::applyDarkTheme(QApplication *app)
{
    if (!app)
        return;

    applyBaseSettings(app); // Call base settings first

    QPalette darkPalette;

    // Set background color to black
    darkPalette.setColor(QPalette::Window, QColor(0, 0, 0));

    // Set text color to white
    darkPalette.setColor(QPalette::WindowText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Text, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::BrightText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::PlaceholderText, QColor(128, 128, 128)); // Lighter gray for placeholder

    // Set base color (used for text entry widgets background)
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));

    // Set other colors for a more complete dark theme
    darkPalette.setColor(QPalette::ToolTipBase, QColor(0, 0, 0));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));

    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(40, 40, 40));
    darkPalette.setColor(QPalette::Disabled, QPalette::Window, QColor(30,30,30));


    app->setPalette(darkPalette);

    // Set instance properties for list headers for the dark theme
    this->setListHeaderTextColor(QColor(200, 200, 200)); // Light gray
    this->setListHeaderFont(createListHeaderFont());
}

// Instance method
void ThemeManager::applyLightTheme(QApplication *app)
{
    if (!app)
        return;

    applyBaseSettings(app); // Call base settings first

    // Reset to default system palette or define a light theme
    app->setPalette(QApplication::style()->standardPalette());

    // Or define a custom light theme:
    /*
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    // ... set other colors for light theme
    app->setPalette(lightPalette);
    */

    // Set instance properties for list headers for the light theme
    this->setListHeaderTextColor(QColor(0, 0, 128)); // Dark blue
    this->setListHeaderFont(createListHeaderFont());
}
