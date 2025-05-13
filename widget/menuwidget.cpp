#include "widget/menuwidget.h"
#include "ui_menuwidget.h"
#include "component/navigationcomponent.h"
#include "component/componentmanager.h" // Added for ComponentManager
#include "component/inputcomponent.h"   // Added for InputComponent
#include "util/util.h"                // Added for LOG_INFO, LOG_ERROR
#include "widget/NcButton.h" // Assuming NcButton is the class for ui->button1 etc.

#include <QMouseEvent>

MenuWidget::MenuWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MenuWidget),
    m_navigationComponent(nullptr)
{
    ui->setupUi(this);

    // Connect button clicks to their respective handlers
    connect(ui->btnLeft, &NcButton::clicked, this, &MenuWidget::onLeftButtonClicked);
    connect(ui->btnCenter, &NcButton::clicked, this, &MenuWidget::onCenterButtonClicked);
    connect(ui->btnRight, &NcButton::clicked, this, &MenuWidget::onRightButtonClicked);

    connect(ui->btnLeft, &NcButton::longClicked, this, &MenuWidget::onLeftButtonLongClicked);
    connect(ui->btnCenter, &NcButton::longClicked, this, &MenuWidget::onCenterButtonLongClicked);
    connect(ui->btnRight, &NcButton::longClicked, this, &MenuWidget::onRightButtonLongClicked);


    // Connect to InputComponent signals
    nucare::InputComponent* inputComp = ComponentManager::instance().inputComponent();
    if (inputComp) {
        connect(inputComp, &nucare::InputComponent::btnLeftPress, this, [this]{ simulateButtonPress(ui->btnLeft); });
        connect(inputComp, &nucare::InputComponent::btnLeftRelease, this, [this]{ simulateButtonRelease(ui->btnLeft); });
        connect(inputComp, &nucare::InputComponent::btnCenterPress, this, [this]{ simulateButtonPress(ui->btnCenter); });
        connect(inputComp, &nucare::InputComponent::btnCenterRelease, this, [this]{ simulateButtonRelease(ui->btnCenter); });
        connect(inputComp, &nucare::InputComponent::btnRightPress, this, [this]{ simulateButtonPress(ui->btnRight); });
        connect(inputComp, &nucare::InputComponent::btnRightRelease, this, [this]{ simulateButtonRelease(ui->btnRight); });
        nucare::logI() << "MenuWidget: Connected to InputComponent signals.";
    } else {
        nucare::logE() << "MenuWidget: InputComponent is null, cannot connect input signals.";
    }
}

MenuWidget::~MenuWidget()
{
    delete ui;
}

void MenuWidget::setNavigationComponent(NavigationComponent* navComp)
{
    if (m_navigationComponent) {
        // Disconnect old signals
        disconnect(m_navigationComponent, &NavigationComponent::currentScreenChanged, this, &MenuWidget::onCurrentScreenChanged);
        disconnect(m_navigationComponent, &NavigationComponent::canNavigateBackChanged, this, &MenuWidget::onCanNavigateBackChanged);
    }

    m_navigationComponent = navComp;

    if (m_navigationComponent) {
        connect(m_navigationComponent, &NavigationComponent::currentScreenChanged, this, &MenuWidget::onCurrentScreenChanged);
        connect(m_navigationComponent, &NavigationComponent::canNavigateBackChanged, this, &MenuWidget::onCanNavigateBackChanged);

        // Initial state update
        onCurrentScreenChanged(m_navigationComponent->currentScreen(), nullptr);
        onCanNavigateBackChanged(m_navigationComponent->canNavigateBack());
    } else {
        // No navigation component, perhaps clear/disable buttons
        onCurrentScreenChanged(nullptr, nullptr);
        onCanNavigateBackChanged(false);
    }
}

void MenuWidget::onCurrentScreenChanged(BaseScreen *newScreen, BaseScreen *oldScreen)
{
    Q_UNUSED(oldScreen);

    ViewAction leftAction, centerAction, rightAction;
    ViewAction longLeftAction, longCenterAction, longRightAction;

    if (newScreen) {
        leftAction = newScreen->leftViewAction;
        centerAction = newScreen->centerViewAction;
        rightAction = newScreen->rightViewAction;
        longLeftAction = newScreen->longClickLeftViewAction;
        longCenterAction = newScreen->longClickCenterViewAction;
        longRightAction = newScreen->longClickRightViewAction;
    }
    // If newScreen is null, default ViewAction() will be used for all.

    updateButtonWithAction(ui->btnLeft, leftAction, longLeftAction);
    updateButtonWithAction(ui->btnCenter, centerAction, longCenterAction);
    updateButtonWithAction(ui->btnRight, rightAction, longRightAction);

    // Special handling for the "Back" button (ui->btnLeft)
    // Its enabled state depends on its action's validity AND canNavigateBack.
    // This is now handled within updateButtonWithAction.
    // We might need to call onCanNavigateBackChanged explicitly if newScreen is null
    // or if the screen changes, to ensure back button state is correct.
    if (m_navigationComponent) {
        onCanNavigateBackChanged(m_navigationComponent->canNavigateBack());
    } else {
        onCanNavigateBackChanged(false);
    }
}

void MenuWidget::onCanNavigateBackChanged(bool canGoBack)
{
    // This function now primarily updates the back button's enabled state
    // based on canGoBack and the current left action's validity.
    BaseScreen* currentScreen = m_navigationComponent ? m_navigationComponent->currentScreen() : nullptr;
    ViewAction leftAction = currentScreen ? currentScreen->leftViewAction : ViewAction();
    
    ui->btnLeft->setEnabled(leftAction.isValid() && canGoBack);
}

void MenuWidget::onLeftButtonClicked()
{
    if (!m_navigationComponent) return;
    BaseScreen* screen = m_navigationComponent->currentScreen();
    if (screen && screen->leftViewAction.isValid() && screen->leftViewAction.action) {
        screen->leftViewAction.action();
    }
}

void MenuWidget::onCenterButtonClicked()
{
    if (!m_navigationComponent) return;
    BaseScreen* screen = m_navigationComponent->currentScreen();
    if (screen && screen->centerViewAction.isValid() && screen->centerViewAction.action) {
        screen->centerViewAction.action();
    }
}

void MenuWidget::onRightButtonClicked()
{
    if (!m_navigationComponent) return;
    BaseScreen* screen = m_navigationComponent->currentScreen();
    if (screen && screen->rightViewAction.isValid() && screen->rightViewAction.action) {
        screen->rightViewAction.action();
    }
}

void MenuWidget::onLeftButtonLongClicked(long)
{
    if (!m_navigationComponent) return;
    BaseScreen* screen = m_navigationComponent->currentScreen();
    if (screen && screen->longClickLeftViewAction.isValid() && screen->longClickLeftViewAction.action) {
        screen->longClickLeftViewAction.action();
    }
}

void MenuWidget::onCenterButtonLongClicked(long)
{
    if (!m_navigationComponent) return;
    BaseScreen* screen = m_navigationComponent->currentScreen();
    if (screen && screen->longClickCenterViewAction.isValid() && screen->longClickCenterViewAction.action) {
        screen->longClickCenterViewAction.action();
    }
}

void MenuWidget::onRightButtonLongClicked(long)
{
    if (!m_navigationComponent) return;
    BaseScreen* screen = m_navigationComponent->currentScreen();
    if (screen && screen->longClickRightViewAction.isValid() && screen->longClickRightViewAction.action) {
        screen->longClickRightViewAction.action();
    }
}

void MenuWidget::updateButtonWithAction(NcButton* button, const ViewAction& shortAction, const ViewAction& longAction)
{
    if (!button) return;

    bool hasShortIcon = shortAction.iconPath && QString(shortAction.iconPath).startsWith(":/");
    bool hasLongIcon = longAction.iconPath && QString(longAction.iconPath).startsWith(":/");
    bool hasAnyValidIcon = hasShortIcon || hasLongIcon;

    if (hasAnyValidIcon) {
        button->setText(""); // Hide text if icon is present
        if (hasShortIcon) {
            button->setIcon(QIcon(shortAction.iconPath));
        } else { // Must be hasLongIcon
            button->setIcon(QIcon(longAction.iconPath));
        }
    } else {
        // No valid icon, set text based on short and long action names
        QString buttonText = shortAction.name;
        if (!longAction.name.isEmpty()) {
            buttonText += " / " + longAction.name;
        }
        button->setText(buttonText);
        button->setIcon(QIcon()); // Clear icon
    }

    // Set Enabled state (primarily based on shortAction)
    // A button is generally considered "active" if its primary (short click) action is valid.
    // Long click is often a secondary/alternative action.
    if (button == ui->btnLeft) {
        // For ui->btnLeft (Back), enabled only if shortAction is valid AND can navigate back.
        bool canGoBack = m_navigationComponent ? m_navigationComponent->canNavigateBack() : false;
        button->setEnabled(shortAction.isValid() && canGoBack);
    } else {
        // For other buttons, enabled state is based on shortAction's validity.
        button->setEnabled(shortAction.isValid());
    }
}

void MenuWidget::simulateButtonPress(NcButton* button)
{
    if (!button) return;
    QPoint center = button->rect().center();
    QMouseEvent pressEvent(QEvent::MouseButtonPress, center, button->mapToGlobal(center), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(button, &pressEvent);
}

void MenuWidget::simulateButtonRelease(NcButton* button)
{
    if (!button) return;
    QPoint center = button->rect().center();
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, center, button->mapToGlobal(center), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(button, &releaseEvent);
}
