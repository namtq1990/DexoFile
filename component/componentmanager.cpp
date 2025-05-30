#include "componentmanager.h"
#include "navigationcomponent.h"
#include "component/inputcomponent.h" // Include InputComponent (full path for clarity)
#include "component/detectorcomponent.h" // Include DetectorComponent
#include "component/ncmanager.h" // Include NcManager
#include "component/databasemanager.h" // Include DatabaseManager
#include "controller/platform_controller.h"
#include "../thememanager.h"
#include "util/nc_exception.h"
#include <QApplication>
#include <QStackedWidget>
#include <QObject>
#include <QFileInfo>
#include <QStandardPaths>

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

void ComponentManager::initializeComponents(QObject* parent)
{
    if (mDataDir.isEmpty()) {
        QStringList paths = {"/root"};
        for (auto& p : paths) {
            QFileInfo info(p);
            if (info.isDir() && info.isWritable()) {
                mDataDir = p;
                goto final;
            }
        }

        paths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
        paths.erase(std::remove_if(paths.begin(), paths.end(),
                                   [](const QString& str) {
                                       return str.startsWith("/usr/local");
                                   }),
                    paths.end());
        if (paths.empty()) NC_THROW(nucare::ErrorCode::FileNotFound, "Can't find any data directory");

        for (auto it = paths.rbegin(); it != paths.rend(); it++) {
            // Prefer data path in /usr
            auto p = std::forward<QString>(*it);
            logD() << "Candidate data path: " << p;

            if (p.contains("/usr/share")) {
                QFileInfo finfo("/usr/share");

                if (finfo.isDir() && finfo.isWritable()) {
                    mDataDir = p;
                    break;
                }
            } else {
                mDataDir = p;
            }
        }

    final:
        mDataDir += "/NDT";
        logD() << "Selected data directory at: " << mDataDir;
    }

    initializeThemeManager(parent);
    initializeInputComponent(parent);
    initializePlatformController(parent);
    initializeDatabaseManager(parent);
    initializeWiFiService(parent);
    initializeDetectorComponent(parent); // Initialize DetectorComponent with parent
    initializeNcManager(parent); // Initialize NcManager
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
        m_themeManager = new ThemeManager(parent); // Adjust if ThemeManager constructor is different
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
        m_inputComponent.reset(new InputComponent()); // Uses default tag "InputComponent"
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
        m_wifiService = new WiFiService(parent);
        if (m_wifiService) {
            logI() << "WiFiService initialized.";
        } else {
            logE() << "WiFiService allocation failed.";
        }
    } else {
        logE() << "WiFiService already initialized.";
    }
}

void ComponentManager::initializeDetectorComponent(QObject *parent)
{
    if (!m_detectorComponent) {
        m_detectorComponent = QPointer<nucare::DetectorComponent>(new nucare::DetectorComponent(parent));
        m_detectorComponent->open("/dev/ttyS2");
        m_detectorComponent->start();
    }
}

void ComponentManager::initializeNcManager(QObject *parent)
{
    if (!m_ncManager) {
        m_ncManager = QSharedPointer<NcManager>(new NcManager("NcManager"));
        // Connect DetectorComponent's data ready signal to NcManager's slot
        QObject::connect(m_detectorComponent.data(), &nucare::DetectorComponent::packageReceived,
                         m_ncManager.data(), &NcManager::onRecvPackage);
        QObject::connect(m_detectorComponent.data(), &nucare::DetectorComponent::detectorInfoReceived,
                         m_ncManager.data(), &NcManager::onRecvGC);
        logI() << "NcManager initialized.";
    } else {
        logE() << "NcManager already initialized.";
    }
}

void ComponentManager::initializeDatabaseManager(QObject* parent)
{
    if (!m_databaseManager) {
        m_databaseManager = QPointer<nucare::DatabaseManager>(new nucare::DatabaseManager(parent));
        m_databaseManager->initialize(mDataDir);
        logI() << "DatabaseManager initialized.";
    } else {
        logE() << "DatabaseManager already initialized.";
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
    return m_themeManager;
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
    return m_wifiService;
}

QPointer<nucare::DatabaseManager> ComponentManager::databaseManager() const
{
    if (!m_databaseManager) {
        logE() << "DatabaseManager accessed before initialization!";
    }
    return m_databaseManager;
}

QString ComponentManager::dataDir() const { return mDataDir; }

QSharedPointer<NcManager> ComponentManager::ncManager() const
{
    if (!m_ncManager) {
        logE() << "NcManager accessed before initialization!";
    }
    return m_ncManager;
}

PlatformController* ComponentManager::platformController() const
{
    if (!m_platformController) {
        logE() << "PlatformController accessed before initialization!";
    }
    return m_platformController;
}

void ComponentManager::initializePlatformController(QObject* parent)
{
     if (!m_platformController) {
         m_platformController = new PlatformController(parent);
         logI() << "PlatformController initialized.";
     } else {
         logE() << "PlatformController already initialized.";
     }
}
