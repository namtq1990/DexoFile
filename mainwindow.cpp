#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "component/componentmanager.h" // Use ComponentManager
#include "component/navigationcomponent.h"
#include "widget/menuwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize components through ComponentManager
    // ThemeManager is now initialized in Application::initialize()
    ComponentManager::instance().initializeNavigationComponent(ui->mainStackedWidget, this);

    NavigationComponent* navComp = ComponentManager::instance().navigationComponent();

    // Connect the MenuWidget (from ui) to the NavigationComponent

    if (ui->menuWidget && navComp) {
        ui->menuWidget->setNavigationComponent(navComp);
        navComp->navigateHomePage(this);
    } else {
        nucare::logW() << "MainWindow: menuWidget or navigationComponent (from ComponentManager) is null, cannot connect.";
    }
}

MainWindow::~MainWindow()
{
    // m_navigationComponent is no longer owned by MainWindow
    delete ui;
}
