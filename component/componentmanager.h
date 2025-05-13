#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "component.h" // Base class
#include <QPointer>    // For QObject-derived components like NavigationComponent
#include <QScopedPointer> // For managing owned non-QObject components

// Forward declarations
class NavigationComponent;
class ThemeManager;
namespace nucare { class InputComponent; } // Forward declare nucare::InputComponent
class QStackedWidget; // Needed for NavigationComponent initialization
class QObject;        // For parent parameter

class ComponentManager : virtual public Component
{
public:
    static ComponentManager& instance();

    ~ComponentManager();

    // Initialization methods - must be called after QApplication instance exists if components need it
    void initializeNavigationComponent(QStackedWidget* stackedWidget, QObject* parent = nullptr);
    void initializeThemeManager(QObject* parent = nullptr); // Assuming ThemeManager might need a parent
    void initializeInputComponent(QObject* parent = nullptr); // For InputComponent

    NavigationComponent* navigationComponent() const;
    ThemeManager* themeManager() const;
    nucare::InputComponent* inputComponent() const; // Getter for nucare::InputComponent

    // Delete copy constructor and assignment operator
    ComponentManager(const ComponentManager&) = delete;
    ComponentManager& operator=(const ComponentManager&) = delete;

private:
    ComponentManager(); // Private constructor for singleton

    QScopedPointer<NavigationComponent> m_navigationComponent;
    QScopedPointer<ThemeManager> m_themeManager;
    QScopedPointer<nucare::InputComponent> m_inputComponent; // Member for nucare::InputComponent
    // If ThemeManager becomes a QObject and needs QObject parenting for lifetime,
    // QPointer might be used if it's parented to something else,
    // or it can be a raw pointer if ComponentManager is its QObject parent.
    // For now, QScopedPointer assumes ComponentManager owns it.
};

#endif // COMPONENTMANAGER_H
