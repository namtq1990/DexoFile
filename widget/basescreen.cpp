#include "widget/basescreen.h"
#include "component/componentmanager.h"    // For ComponentManager
#include "component/navigationcomponent.h" // For NavigationComponent

BaseScreen::BaseScreen(QWidget *parent) : QWidget(parent) // m_navComponent removed
{
    // Default actions can be initialized to be "empty" or placeholder
    // The ViewAction constructor already handles default initialization.

    // Set default leftViewAction to navigate back
    leftViewAction.name = ""; // Typically, a back button uses an icon, not text.
    leftViewAction.iconPath = ":/icons/menu_back.png"; // Common back icon
    leftViewAction.action = []() -> bool {
        NavigationComponent* navComp = ComponentManager::instance().navigationComponent();
        if (navComp && navComp->canNavigateBack()) {
            navComp->navigateBack();
            return true; // Action attempted
        }
        return false; // Action not attempted or not possible
    };
}


void BaseScreen::setLeftViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath)
{
    updateViewAction(leftViewAction, name, act, iconPath);
}

void BaseScreen::setCenterViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath)
{
    updateViewAction(centerViewAction, name, act, iconPath);
}

void BaseScreen::setRightViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath)
{
    updateViewAction(rightViewAction, name, act, iconPath);
}

void BaseScreen::setLeftViewAction(ViewAction action)
{
    updateViewAction(leftViewAction, action);
}

void BaseScreen::setCenterViewAction(ViewAction action)
{
    updateViewAction(centerViewAction, action);
}

void BaseScreen::setRightViewAction(ViewAction action)
{
    updateViewAction(rightViewAction, action);
}

// Setters for long click view actions
void BaseScreen::setLongClickLeftViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath)
{
    updateViewAction(longClickLeftViewAction, name, act, iconPath);
}

void BaseScreen::setLongClickCenterViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath)
{
    updateViewAction(longClickCenterViewAction, name, act, iconPath);
}

void BaseScreen::setLongClickRightViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath)
{
    updateViewAction(longClickRightViewAction, name, act, iconPath);
}

void BaseScreen::setLongClickLeftViewAction(ViewAction action)
{
    updateViewAction(longClickLeftViewAction, action);
}

void BaseScreen::setLongClickCenterViewAction(ViewAction action)
{
    updateViewAction(longClickCenterViewAction, action);
}

void BaseScreen::setLongClickRightViewAction(ViewAction action)
{
    updateViewAction(longClickRightViewAction, action);
}

void BaseScreen::onScreenShown()
{
    // Default implementation can be empty or emit the signal
    emit screenShown();
}

void BaseScreen::onScreenHidden()
{
    // Default implementation can be empty or emit the signal
    emit screenHidden();
}

void BaseScreen::updateViewAction(ViewAction& currentAction, const QString& name, std::function<bool(void)> act, const char* iconPath)
{
    currentAction.name = name;
    if (act) {
        currentAction.action = act;
    } else {
        currentAction.action = [](){ return false; }; // Default if nullptr
    }
    currentAction.iconPath = iconPath;
    emit viewActionsChanged();
}

void BaseScreen::updateViewAction(ViewAction& currentAction, ViewAction newAction)
{
    currentAction = newAction;
    // Ensure action is callable if it was default-constructed by newAction
    if (!newAction.action) {
         currentAction.action = [](){ return false; };
    }
    emit viewActionsChanged();
}
