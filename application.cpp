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

    setApplicationVersion(APP_VERSION_STRING);

    // Initialize all components via ComponentManager, passing 'this' as the parent
    ComponentManager::instance().initializeComponents(this);

    // Apply the theme after ThemeManager is initialized
    ThemeManager* tm = ComponentManager::instance().themeManager();
    if (tm) {
        tm->applyDarkTheme(this); // 'this' is the QApplication instance
    } else {
        nucare::logE() << "Application::initialize: ThemeManager instance is null after component initialization!";
    }

    ComponentManager::instance().initializeNavigationComponent();
    ComponentManager::instance().platformController()->executeShellCommandAsync("/etc/init.d/S01splash stop");
}

} // namespace nucare
