/*
 * BaseWindow.cpp
 *
 *  Created on: Oct 18, 2021
 *      Author: quangnam
 */

#include "basewindow.h"
#include "componentmanager.h"
#include <QLayout>
using namespace std;




BaseWindow::BaseWindow(const QString& tag, QWidget* parent) : QMainWindow(parent), BaseView(tag) {
}

BaseWindow::~BaseWindow() {}

void BaseWindow::closeEvent(QCloseEvent *ev)
{
    QMainWindow::closeEvent(ev);

    if (isFlagSet(navigation::FLAG_CREATED)) {
        // We only pop windows if this screen still in stack
        auto navController = ComponentManager::instance().navigationComponent();
        navController->pop(this, false);
    }
}

void BaseWindow::updateMenu()
{}

void BaseWindow::changeEvent(QEvent *ev) {
    QMainWindow::changeEvent(ev);
    BaseView::changeEvent(ev);
}

void BaseWindow::performShowLoading() {}

void BaseWindow::performHideLoading() {}
