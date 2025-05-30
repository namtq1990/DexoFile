#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "component.h" // Base class
#include <QPointer>    // For QObject-derived components like NavigationComponent
#include <QScopedPointer> // For managing owned non-QObject components
#include "controller/wifi_service.h" // Include WiFiService header
#include "detectorcomponent.h" // Include DetectorComponent header
#include "databasemanager.h" // Include DatabaseManager header

// Forward declarations
class ThemeManager;
namespace navigation { class NavigationComponent; }
namespace nucare { class InputComponent; }
namespace nucare { class DetectorComponent; } // Forward declaration for DetectorComponent
namespace nucare { class DatabaseManager; } // Forward declaration for DatabaseManager
class NcManager; // Forward declaration for NcManager
class QStackedWidget;
class QObject;
class PlatformController;

class ComponentManager : virtual public Component
{
public:
    static ComponentManager& instance();

    ~ComponentManager();

    // Public method to initialize all components
    void initializeComponents(QObject* parent = nullptr);

    // Initialization methods - must be called after QApplication instance exists if components need it
    void initializeNavigationComponent();
    void initializeThemeManager(QObject* parent = nullptr); // Assuming ThemeManager might need a parent
    void initializeInputComponent(QObject* parent = nullptr); // For InputComponent
    void initializePlatformController(QObject* parent = nullptr);
    void initializeWiFiService(QObject* parent = nullptr);
    void initializeDetectorComponent(QObject* parent = nullptr); // Declare initializeDetectorComponent
    void initializeNcManager(QObject* parent = nullptr); // Declare initializeNcManager
    void initializeDatabaseManager(QObject* parent = nullptr); // Declare initializeDatabaseManager

    navigation::NavigationComponent* navigationComponent() const;
    ThemeManager* themeManager() const;
    nucare::InputComponent* inputComponent() const; // Getter for nucare::InputComponent
    nucare::DetectorComponent* detectorComponent() const; // Getter for DetectorComponent
    QSharedPointer<NcManager> ncManager() const; // Getter for NcManager
    PlatformController* platformController() const;
    WiFiService* wifiService() const;
    QPointer<nucare::DatabaseManager> databaseManager() const; // Getter for DatabaseManager
    QString dataDir() const;

    // Delete copy constructor and assignment operator
    ComponentManager(const ComponentManager&) = delete;
    ComponentManager& operator=(const ComponentManager&) = delete;

private:
    ComponentManager(); // Private constructor for singleton

    QScopedPointer<navigation::NavigationComponent> m_navigationComponent;
    ThemeManager* m_themeManager = nullptr;
    QScopedPointer<nucare::InputComponent> m_inputComponent; // Member for nucare::InputComponent
    QPointer<nucare::DetectorComponent> m_detectorComponent; // Member for DetectorComponent
    QPointer<nucare::DatabaseManager> m_databaseManager; // Member for DatabaseManager
    QSharedPointer<NcManager> m_ncManager; // Member for NcManager
    PlatformController* m_platformController = nullptr;
    WiFiService* m_wifiService = nullptr;
    QString mDataDir;
    // If ThemeManager becomes a QObject and needs QObject parenting for lifetime,
    // QPointer might be used if it's parented to something else,
    // or it can be a raw pointer if ComponentManager is its QObject parent.
    // For now, QScopedPointer assumes ComponentManager owns it.
};

#endif // COMPONENTMANAGER_H
