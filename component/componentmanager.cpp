#include "componentmanager.h"
#include "navigationcomponent.h"
#include "component/inputcomponent.h" // Include InputComponent (full path for clarity)
#include "controller/platform_controller.h"
#include "../thememanager.h"
#include <QApplication>
#include <QStackedWidget>
#include <QObject>

using namespace navigation;

// Using declaration for InputComponent from the nucare namespace
// This allows using InputComponent without nucare:: prefix in this file.
namespace nucare { class InputComponent; } // Forward declaration for the using statement below
using nucare::InputComponent;


ComponentManager& ComponentManager::instance()
{
    static ComponentManager inst; // Meyers' Singleton
    return inst;
}

ComponentManager::ComponentManager()
    : Component("ComponentManager")
{
    // Constructor is private
}

ComponentManager::~ComponentManager()
{
    // QScopedPointers will automatically delete managed objects
}

void ComponentManager::initializeNavigationComponent()
{
    if (!m_navigationComponent) {
        m_navigationComponent.reset(new NavigationComponent());
        logI() << "NavigationComponent initialized.";

        navigation::toMainWindow(m_navigationComponent.get());
    } else {
        logE() << "NavigationComponent already initialized.";
    }
}

void ComponentManager::initializeThemeManager(QObject* parent)
{
    if (!m_themeManager) {
        // Assuming ThemeManager's constructor takes a QObject* parent
        // If ThemeManager is not a QObject, the parent parameter might be for a different purpose
        // or not needed. For now, let's assume it can take a parent.
        // If ThemeManager's constructor is just ThemeManager(), then remove 'parent'.
        m_themeManager.reset(new ThemeManager(parent)); // Adjust if ThemeManager constructor is different
        logI() << "ThemeManager initialized.";
    } else {
        logE() << "ThemeManager already initialized.";
    }
}

void ComponentManager::initializeInputComponent(QObject* parent)
{
    if (!m_inputComponent) {
        // InputComponent constructor is nucare::InputComponent(QObject *parent = nullptr, const QString& tag = "InputComponent")
        // With 'using nucare::InputComponent;', we can use 'InputComponent' directly.
        m_inputComponent.reset(new InputComponent(parent)); // Uses default tag "InputComponent"
        if (m_inputComponent && m_inputComponent->initialize()) { // Check m_inputComponent after new
            logI() << "InputComponent initialized.";
        } else {
            logE() << "InputComponent failed to initialize or allocation failed.";
            m_inputComponent.reset(); // Release if initialization failed or new failed
        }
    } else {
        logE() << "InputComponent already initialized.";
    }
}

void ComponentManager::initializeWiFiService(QObject* parent)
{
    if (!m_wifiService) {
        m_wifiService.reset(new WiFiService(parent));
        if (m_wifiService) {
            logI() << "WiFiService initialized.";
        } else {
            logE() << "WiFiService allocation failed.";
        }
    } else {
        logE() << "WiFiService already initialized.";
    }
}

NavigationComponent* ComponentManager::navigationComponent() const
{
    if (!m_navigationComponent) {
        logE() << "NavigationComponent accessed before initialization!";
    }
    return m_navigationComponent.data();
}

ThemeManager* ComponentManager::themeManager() const
{
    if (!m_themeManager) {
        logE() << "ThemeManager accessed before initialization!";
    }
    return m_themeManager.data();
}

// Return type matches header: nucare::InputComponent*
// With 'using nucare::InputComponent;', we can use 'InputComponent*' here.
InputComponent* ComponentManager::inputComponent() const
{
    if (!m_inputComponent) {
        logE() << "InputComponent accessed before initialization!";
    }
    return m_inputComponent.data();
}

WiFiService* ComponentManager::wifiService() const
{
    if (!m_wifiService) {
        logE() << "WiFiService accessed before initialization!";
    }
    return m_wifiService.data();
}

PlatformController* ComponentManager::platformController() const
{
    if (!m_platformController) {
        logE() << "PlatformController accessed before initialization!";
    }
    return m_platformController.data();
}

void ComponentManager::initializePlatformController(QObject* parent)
{
     if (!m_platformController) {
         m_platformController.reset(new PlatformController(parent));
         logI() << "PlatformController initialized.";
     } else {
         logE() << "PlatformController already initialized.";
     }
}
