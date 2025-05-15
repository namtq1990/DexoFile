#include "page/settingscreen.h"
#include "widget/baselist.h"
#include "util/util.h"                      // For nucare::logI
#include "component/componentmanager.h"     // For ComponentManager
#include "component/navigationcomponent.h"  // For NavigationComponent
#include <QVBoxLayout>
#include <QListWidgetItem>
#include "settingscreen.h"

void navigation::toSetting(navigation::NavigationComponent* navController, navigation::NavigationEntry* entry,
                           //                   SettingArgs* args,
                           const QString& tag) {
    if (!entry->view) {
        entry->view = new SettingScreen(static_cast<QWidget*>(entry->host));
    }
    navController->enter(entry);
}

SettingScreen::SettingScreen(QWidget* parent) : BaseScreen(tag::SETTING_TAG, parent), m_listWidget(nullptr) {
    m_listWidget = new BaseListWidget(this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_listWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);  // Use full space
    setLayout(mainLayout);

    setupListItems();
    setupViewActions();

    // Ensure the list widget has focus to receive potential keyboard events
    // if not solely relying on ViewActions from menu.
    // m_listWidget->setFocus();
}

SettingScreen::~SettingScreen() {
    // m_listWidget is a child of this widget and will be deleted automatically
    // when SettingScreen is deleted, especially if added to a layout.
    // No explicit delete ui; if not using a .ui file for SettingScreen itself.
}

void SettingScreen::setupListItems() {
    if (!m_listWidget) return;

    m_listWidget->addHeaderItem("SETTING");
    m_listWidget->addSelectableItem("Sound");
    m_listWidget->addSelectableItem("WiFi");
    // Potentially select the first selectable item
    // m_listWidget->selectNextSelectableItem(); // BaseListWidget constructor already does this.
}

void SettingScreen::setupViewActions() {
    // leftViewAction is "Back" by default from BaseScreen constructor.
    // We can customize its name or icon if needed:
    // leftViewAction.name = "Exit"; // Example

    setCenterAction(new ViewAction("OK", [this]() -> bool {
        if (m_listWidget) {
            QListWidgetItem* activatedItem = m_listWidget->activateCurrentItem();
            if (activatedItem) {
                nucare::logI() << "SettingScreen: Activated item -" << activatedItem->text();
                if (activatedItem->text() == "Sound") {
                    handleSoundSetting();
                } else if (activatedItem->text() == "WiFi") {
                    handleWifiSetting();
                }
                return true;
            }
        }
        return false;
    }));

    setRightAction(new ViewAction("Right", [this]() {
        if (m_listWidget) {
            m_listWidget->selectNextSelectableItem();
            return true;
        }
        return false;
    }));

    setLongRightAction(new ViewAction("Up", [this]() -> bool {
        if (m_listWidget) {
            m_listWidget->selectPreviousSelectableItem();
            return true;
        }
        return false;
    }));
}

void SettingScreen::handleSoundSetting() {
    nucare::logI() << "Handling Sound Setting (Navigation Not Implemented)";
    // NavigationComponent* navComp = ComponentManager::instance().navigationComponent();
    // if (navComp) navComp->navigateTo(new SoundPage(this)); // Assuming SoundPage exists
}

void SettingScreen::handleWifiSetting() {
    nucare::logI() << "Handling WiFi Setting (Navigation Not Implemented)";
    // NavigationComponent* navComp = ComponentManager::instance().navigationComponent();
    // if (navComp) navComp->navigateTo(new WifiPage(this)); // Assuming WifiPage exists
}
