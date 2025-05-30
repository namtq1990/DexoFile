#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "component/componentmanager.h" // Use ComponentManager
#include "component/navigationcomponent.h"
#include "widget/menuwidget.h"

using namespace navigation;

void navigation::toMainWindow(NavigationComponent* navController, const QString& tag) {
    auto window = new MainWindow(tag);
    auto childNavigation = new NavigationComponent(navController);
    auto entry = new navigation::NavigationEntry(navigation::WINDOW, window, childNavigation);

    navController->enter(entry);
}

MainWindow::MainWindow(const QString &tag, QWidget *parent)
    : BaseWindow(tag, parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    // m_navigationComponent is no longer owned by MainWindow
    delete ui;
}

void MainWindow::reloadLocal() {
    ui->retranslateUi(this);
}

void MainWindow::onCreate(navigation::NavigationArgs *args) {
    BaseWindow::onCreate(args);
    navigation::toHome(getChildNavigation(), new NavigationEntry(CHILD_IN_WINDOW, nullptr, nullptr, ui->mainStackedWidget));
}
