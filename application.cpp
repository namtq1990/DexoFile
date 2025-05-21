#include "application.h"
#include "thememanager.h" // For ThemeManager::applyDarkTheme
#include "component/componentmanager.h" // For ComponentManager
#include "controller/platform_controller.h"
#include <QStyleFactory>  // For QStyleFactory
#include <QFile>
#include <QTextStream>
#include <iostream>

namespace nucare {

void customLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QTextStream cerrStream((type == QtMsgType::QtDebugMsg || type == QtMsgType::QtInfoMsg) ? stdout : stderr);
    cerrStream << msg << Qt::endl;

    // Log to file
    QFile logFile("/root/ndt.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream logStream(&logFile);
        logStream << msg << Qt::endl;
        logFile.close();
    }
}

// Application* Application::m_instance = nullptr; // No longer needed

Application* Application::instance()
{
    // QApplication itself is a singleton. We cast its instance.
    return static_cast<Application*>(QApplication::instance());
}

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
    // QApplication constructor handles singleton logic.
    // No need to manage m_instance here.
}

Application::~Application()
{
    // No m_instance to manage.
}

void Application::initialize()
{
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    qInstallMessageHandler(customLogHandler);

    // Initialize ThemeManager via ComponentManager first
    // Pass nullptr as parent, as ThemeManager instance is managed by ComponentManager
    // and global theme application is done by ThemeManager::applyDarkTheme
    ComponentManager::instance().initializeThemeManager(nullptr);

    // Then apply the theme using the now-initialized ThemeManager
    // This assumes ThemeManager::applyDarkTheme is a static method or
    // can be called on a globally accessible ThemeManager instance if needed.
    // ThemeManager methods are now non-static.
    ThemeManager* tm = ComponentManager::instance().themeManager();
    if (tm) {
        tm->applyDarkTheme(this); // 'this' is the QApplication instance
    } else {
        // Log error: ThemeManager instance not available.
        // This implies an issue with ComponentManager initialization order or ThemeManager init.
        // For now, we assume tm will be valid if initializeThemeManager was successful.
        nucare::logE() << "Application::initialize: ThemeManager instance is null after initialization!";
    }

    ComponentManager::instance().initializeInputComponent(this);
    ComponentManager::instance().initializePlatformController(this);
    ComponentManager::instance().initializeWiFiService(this);
    ComponentManager::instance().initializeNavigationComponent();
}

} // namespace nucare
