/*
 * BaseView.cpp
 *
 *  Created on: Oct 18, 2021
 *      Author: quangnam
 */

#include <QEvent>
#include "baseview.h"
#include "basewindow.h"
#include "config.h"
#include "util/util.h"
#include "navigationcomponent.h"
#include "componentmanager.h"
#include <QWidget>
#include <QLayout>
#include <QStackedWidget>
#include <QDialog>
#include <QToolTip>
#include <QTimer>
#include <QApplication>
using namespace navigation;

void navigation::showShortMessage(const QString&& text) {
    if (auto navController = ComponentManager::instance().navigationComponent()) {
        navController->curScreen(true)->showShortMessage(text.toLatin1());
    }
}

BaseView::BaseView(const QString& tag) : mTag(tag), mNavigation(nullptr) {
    mLeftAction       = std::shared_ptr<ViewAction>(new ViewAction(
        "BACK",
        [this]() {
            NavigationComponent *top = getNavigation();
            while (true) {
                if (top->getParentNavigation()) {
                    top = top->getParentNavigation();
                } else {
                    break;
                }
            }

            top->pop(nullptr, true);
            return true;
        },
        ":/icons/menu_back.png"));
    mCenterAction = std::make_shared<ViewAction>();
    mRightAction = std::make_shared<ViewAction>();
    mLeftLongAction = std::make_shared<ViewAction>();
    mCenterLongAction = std::make_shared<ViewAction>();
    mRightLongAction = std::make_shared<ViewAction>();
}

BaseView::~BaseView() {
    nucare::logD() << "Screen " << mTag << '@' << this << " is Destroyed";
    if (mChildNavigation) {
        delete mChildNavigation;
    }
}

void BaseView::onCreate(NavigationArgs* entry) {
    Q_UNUSED(entry)
    setFlag(FLAG_CREATED);
    nucare::logD() << "Screen " << mTag << '@' << this << " is onCreate";
}

void BaseView::onEnter() {
    nucare::logD() << "Screen " << mTag << '@' << this << " is Enter";
}

void BaseView::onExit() {
    nucare::logD() << "Screen " << mTag << '@' << this << " is unBinding";
}

void BaseView::onDestroy() {
    nucare::logD() << "Screen " << mTag << '@' << this << " is onDestroy";
}

void BaseView::changeEvent(QEvent *ev) {
    if (ev->type() == QEvent::LanguageChange) {
        reloadLocal();
    }
}

void BaseView::performCreate(navigation::NavigationEntry *entry) {
    setFlag(navigation::FLAG_CREATED);
    switch (entry->type) {
    case DIALOG:
        break;
    case CHILD_IN_WINDOW:
        showChildWidget(*entry);
        break;
    case TAB_IN_WINDOW:
        break;
    case WINDOW:
        break;
    }

    onCreate(entry->getArguments());
    // TODO Remove argements from entry. I don't wanna data to be leaked
    entry->resetArguments();
}

void BaseView::performShow(navigation::NavigationEntry *entry) {
    if (isFlagSet(FLAG_HAS_SHOWN)) {
        nucare::logD() << mTag <<  ": View already show.";
        return;
    } else if (!isFlagSet(FLAG_CREATED)) {
        nucare::logD() << mTag << ": View hasn't been created";
        return;
    }

    setFlag(FLAG_HAS_SHOWN);
    switch (entry->type) {
    case DIALOG:
        showDialog(*entry);
        break;
    case CHILD_IN_WINDOW:
        showChildWidget(*entry);
        break;
    case TAB_IN_WINDOW:
        showTabWidget(*entry);
        break;
    case WINDOW:
        showWindow(*entry);
        break;
    }

    if (mShowLoading) {
        performShowLoading();
    } else {
        performHideLoading();
    }

    onEnter();
}

void BaseView::performHide(navigation::NavigationEntry *entry) {
    Q_UNUSED(entry)

    if (isFlagSet(navigation::FLAG_HAS_SHOWN)) {
        removeFlag(navigation::FLAG_HAS_SHOWN);
        onExit();

        if (auto childNav = getChildNavigation()) {
            auto entry = childNav->curEntry(false);
            if (entry) {
                entry->view->performHide(entry);
            }
        }
    }
}

void BaseView::performDestroy(navigation::NavigationEntry *entry) {
    removeFlag(navigation::FLAG_CREATED);
    if (mChildNavigation != nullptr) {
        auto childBackstack = mChildNavigation->getBackstack();
        for (auto entry : childBackstack) {
            entry->view->performDestroy(entry);
        }
    }

    switch (entry->type) {
    case DIALOG:
        popDialog(*entry);
        break;
    case CHILD_IN_WINDOW:
        popChildWidget(*entry);
        break;
    case TAB_IN_WINDOW:
        popTabWidget(*entry);
        break;
    case WINDOW:
        popWindow(*entry);
        break;
    }

    onDestroy();
}

void BaseView::performShowLoading() {
    if (auto window = dynamic_cast<BaseWindow*>(this->topWindow())) {
        window->showLoading();
    }
}

void BaseView::performHideLoading() {
    if (auto window = dynamic_cast<BaseWindow*>(this->topWindow())) {
        window->hideLoading();
    }
}

void BaseView::showChildWidget(const NavigationEntry &entry) {
    auto content = entry.host;
    QLayout* l = dynamic_cast<QLayout*>(content);
    if (!l) {
        l = dynamic_cast<QWidget*>(content)->layout();
    }
    QWidget* widget = dynamic_cast<QWidget*>(entry.view);
    if (l && widget) {
        if (auto gridLayout = dynamic_cast<QGridLayout*>(l)) {
            gridLayout->addWidget(widget, 0, 0, 1, 1);
        } else if (auto stackedView = dynamic_cast<QStackedWidget*>(content)) {
            stackedView->addWidget(widget);
            stackedView->setCurrentWidget(widget);
        } else {
            l->addWidget(widget);
        }
        widget->show();
    } else {
        throw std::runtime_error("Couldn't show widget, or Host doesnt have layout or view isn't widget");
    }
}

void BaseView::showTabWidget(const NavigationEntry &entry) {
    auto tab = dynamic_cast<QStackedWidget*>(entry.host);
    auto widget = dynamic_cast<QWidget*>(entry.view);
    if (tab && widget) {
        auto index = tab->indexOf(widget);
        if (index >= 0) {
            tab->setCurrentIndex(index);
        } else {
            tab->addWidget(widget);
            tab->setCurrentWidget(widget);
        }
    }
}

void BaseView::showDialog(const NavigationEntry &entry) {
    if (auto dialog = dynamic_cast<QDialog*>(entry.view)) {
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    }
}

void BaseView::showWindow(const NavigationEntry &entry) {
    if (auto window = dynamic_cast<QMainWindow*>(entry.view)) {
        window->show();
    } else {
        throw std::runtime_error("View isn't a window");
    }
}

void BaseView::popChildWidget(const NavigationEntry &entry) {
    QLayout* l = dynamic_cast<QLayout*>(entry.host);
    if (!l) {
        l = dynamic_cast<QWidget*>(entry.host)->layout();
    }
    QWidget* widget = dynamic_cast<QWidget*>(entry.view);
    if (l && widget) {
        if (auto stackedWidget = dynamic_cast<QStackedWidget*>(entry.host)) {
            stackedWidget->removeWidget(widget);
            widget->deleteLater();
        } else {
            l->removeWidget(widget);
            widget->deleteLater();
        }
    } else {
        throw std::runtime_error("Couldn't show widget, or Host doesnt have layout or view isn't widget");
    }
}

void BaseView::popTabWidget(const NavigationEntry &entry) {
    auto tab = dynamic_cast<QStackedWidget*>(entry.host);
    auto widget = dynamic_cast<QWidget*>(getNavigation()->curScreen());
    if (tab && widget) {
        auto index = tab->indexOf(widget);
        if (index >= 0) {
            tab->setCurrentIndex(index);
        }

        // TODO consider about remove widget from Tab
    }
}

void BaseView::popDialog(const NavigationEntry &entry) {
    if (auto dialog = dynamic_cast<QDialog*>(entry.view)) {
        dialog->close();
    }
}

void BaseView::popWindow(const NavigationEntry &entry) {
    if (auto window = dynamic_cast<QMainWindow*>(entry.view)) {
        window->close();
        window->deleteLater();
    } else {
        throw std::runtime_error("View isn't a window");
    }
}

BaseWindow* BaseView::topWindow() {
    return dynamic_cast<BaseWindow*>(ComponentManager::instance().navigationComponent()->curScreen());
}

navigation::NavigationEntry* BaseView::showError(const char* msg)
{
    auto entry = new navigation::NavigationEntry(DIALOG, nullptr, nullptr, dynamic_cast<QObject*>(this));
    navigation::showWarning(getNavigation(), entry, msg);
    return entry;
}

navigation::NavigationEntry* BaseView::showInfo(const char* msg)
{
    auto entry = new navigation::NavigationEntry(DIALOG, nullptr, nullptr, dynamic_cast<QObject*>(this));
    navigation::showWarning(getNavigation(), entry, msg);
    return entry;
}

void BaseView::showShortMessage(const char* msg, bool delayed)
{
    auto action = [transTxt = QCoreApplication::translate(nullptr, msg)]() {
        constexpr int padding = 6 * 2 + 2;
        auto window = QApplication::activeWindow();
        auto fMetrics = QFontMetrics(QToolTip::font());
        auto rec = window->contentsRect();
        auto textWidth = fMetrics.horizontalAdvance(transTxt);
        auto pos = rec.center();

        pos.setX(pos.x() - textWidth / 2 - padding);
        pos.setY(rec.bottom() - 30 - 40 - fMetrics.height());
        QToolTip::showText(window->mapToGlobal(pos), transTxt, window);
    };

    if (delayed) {
        QTimer::singleShot(0, action);
    } else {
        action();
    }
}

void BaseView::showLoading() {
    if (!mShowLoading) {
        mShowLoading = true;
        performShowLoading();
    }
}

void BaseView::hideLoading() {
    if (mShowLoading) {
        mShowLoading = false;
        performHideLoading();
    }
}

bool BaseView::isShowLoading()
{
    return mShowLoading;
}

void BaseView::setLeftAction(std::shared_ptr<ViewAction> action)
{
    mLeftAction = action;

    if (auto nav = getNavigation()) {
        if (nav->curScreen() == this) {
            updateMenu();
        }
    }
}

void BaseView::setCenterAction(std::shared_ptr<ViewAction> action)
{
    mCenterAction = action;

    if (auto nav = getNavigation()) {
        if (nav->curScreen() == this) {
            updateMenu();
        }
    }
}

void BaseView::setRightAction(std::shared_ptr<ViewAction> action)
{
    mRightAction = action;
    if (auto nav = getNavigation()) {
        if (nav->curScreen() == this) {
            updateMenu();
        }
    }
}

void BaseView::setLongLeftAction(std::shared_ptr<ViewAction> action)
{
    mLeftLongAction = action;
    if (auto nav = getNavigation()) {
        if (nav->curScreen() == this) {
            updateMenu();
        }
    }
}

void BaseView::setLongCenterAction(std::shared_ptr<ViewAction> action)
{
    mCenterLongAction = action;
    if (auto nav = getNavigation()) {
        if (nav->curScreen() == this) {
            updateMenu();
        }
    }
}

void BaseView::setLongRightAction(std::shared_ptr<ViewAction> action)
{
    mRightLongAction = action;
    if (auto nav = getNavigation()) {
        if (nav->curScreen() == this) {
            updateMenu();
        }
    }
}

void BaseView::setLeftAction(ViewAction *action)
{
    setLeftAction(std::shared_ptr<ViewAction>(action));
}

void BaseView::setCenterAction(ViewAction *action)
{
    setCenterAction(std::shared_ptr<ViewAction>(action));
}

void BaseView::setRightAction(ViewAction *action)
{
    setRightAction(std::shared_ptr<ViewAction>(action));
}

void BaseView::setLongLeftAction(ViewAction *action)
{
    setLongLeftAction(std::shared_ptr<ViewAction>(action));
}

void BaseView::setLongCenterAction(ViewAction *action)
{
    setLongCenterAction(std::shared_ptr<ViewAction>(action));
}

void BaseView::setLongRightAction(ViewAction *action)
{
    setLongRightAction(std::shared_ptr<ViewAction>(action));
}

std::shared_ptr<ViewAction> BaseView::getLeftAction()
{
    return mLeftAction;
}

std::shared_ptr<ViewAction> BaseView::getCenterAction()
{
    return mCenterAction;
}

std::shared_ptr<ViewAction> BaseView::getRightAction()
{
    return mRightAction;
}

std::shared_ptr<ViewAction> BaseView::getLongLeftAction()
{
    return mLeftLongAction;
}

std::shared_ptr<ViewAction> BaseView::getLongCenterAction()
{
    return mCenterLongAction;
}

std::shared_ptr<ViewAction> BaseView::getLongRightAction()
{
    return mRightLongAction;
}

std::string BaseView::translate(const char *text)
{
    return QCoreApplication::translate(nullptr, text).toStdString();
}

QString BaseView::translateQ(const char *text)
{
    return QCoreApplication::translate(nullptr, text);
}

std::string BaseView::translate(std::string &&text)
{
    return translate(text.c_str());
}

void BaseView::updateMenu()
{
    if (auto window = topWindow()) {
        window->updateMenu();
    }
}
