#include "component/navigationcomponent.h"
#include "widget/basescreen.h" // Needs full definition for onScreenShown/Hidden
#include <QStackedWidget>
#include "navigationcomponent.h"
#include "page/homepage.h"
#include "page/settingscreen.h"

NavigationComponent::NavigationComponent(QStackedWidget *stackedWidget, QObject *parent)
    : QObject(parent)
    , Component("NavigationComponent") // Initialize Component base class
    , m_stackedWidget(stackedWidget)
{
    if (!m_stackedWidget) {
        logE() << "QStackedWidget is null upon construction.";
    }
}

void NavigationComponent::navigateTo(BaseScreen *screen)
{
    if (!m_stackedWidget) {
        logE() << "navigateTo: m_stackedWidget is null.";
        return;
    }
    if (!screen) {
        logE() << "navigateTo: screen is null.";
        return;
    }

    // Avoid navigating to the same screen if it's already current
    if (!m_navigationStack.isEmpty() && m_navigationStack.top() == screen) {
        return;
    }

    BaseScreen *oldScreen = nullptr;
    if (!m_navigationStack.isEmpty()) {
        oldScreen = m_navigationStack.top();
    }

    bool canGoBackBefore = canNavigateBack();

    // Add widget to QStackedWidget if it's not already there
    if (m_stackedWidget->indexOf(screen) == -1) {
        m_stackedWidget->addWidget(screen);
    }

    m_navigationStack.push(screen);
    m_stackedWidget->setCurrentWidget(screen);

    if (oldScreen) {
        oldScreen->onScreenHidden();
    }
    screen->onScreenShown();

    emit currentScreenChanged(screen, oldScreen);
    updateCanNavigateBackState(canGoBackBefore);
}

void NavigationComponent::navigateBack()
{
    if (!canNavigateBack() || !m_stackedWidget) {
        if (!m_stackedWidget) logE() << "navigateBack: m_stackedWidget is null.";
        return;
    }

    bool canGoBackBefore = canNavigateBack(); // Should be true here

    BaseScreen *oldScreen = m_navigationStack.pop();
    BaseScreen *newScreen = m_navigationStack.top(); // Should not be empty due to canNavigateBack check

    m_stackedWidget->setCurrentWidget(newScreen);

    if (oldScreen) { // Should always be true
        oldScreen->onScreenHidden();
    }
    if (newScreen) { // Should always be true
        newScreen->onScreenShown();
    }

    emit currentScreenChanged(newScreen, oldScreen);
    updateCanNavigateBackState(canGoBackBefore);

    // Note: Popped screens are not deleted here. They remain in the QStackedWidget.
    // Management of screen instances (creation/deletion) is outside NavComponent's scope for now.
}

BaseScreen* NavigationComponent::currentScreen() const
{
    if (m_navigationStack.isEmpty()) {
        return nullptr;
    }
    return m_navigationStack.top();
}

bool NavigationComponent::canNavigateBack() const
{
    return m_navigationStack.size() > 1;
}

void NavigationComponent::updateCanNavigateBackState(bool oldCanGoBackState) {
    bool newCanGoBackState = canNavigateBack();
    if (oldCanGoBackState != newCanGoBackState) {
        emit canNavigateBackChanged(newCanGoBackState);
    }
}

void NavigationComponent::navigateHomePage(QWidget* parent) {
    HomePage *homePage = new HomePage(parent);
    navigateTo(homePage);
}

void NavigationComponent::navigateSettingPage(QWidget *parent) {
    auto page = new SettingScreen(parent);
    navigateTo(page);
}
