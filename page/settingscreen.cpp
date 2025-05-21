#include "page/settingscreen.h"
#include "widget/baselist.h"
#include "util/util.h"                      // For nucare::logI
#include "component/componentmanager.h"     // For ComponentManager
#include "component/navigationcomponent.h"  // For NavigationComponent
#include "model/settingmodel.h"
#include "widget/settingitemdelegate.h"
#include "model/basesettingitem.h"
#include <QVBoxLayout>
#include <QList>
#include <QStandardItemModel>
using namespace std;

void navigation::toSetting(navigation::NavigationComponent* navController, navigation::NavigationEntry* entry,
                           //                   SettingArgs* args,
                           const QString& tag) {
    if (!entry->view) {
        entry->view = new SettingScreen(static_cast<QWidget*>(entry->host));
    }
    navController->enter(entry);
}

SettingScreen::SettingScreen(QWidget* parent) : BaseScreen(tag::SETTING_TAG, parent), m_listWidget(nullptr), m_settingModel(new SettingModel(this)) {
    m_listWidget = new BaseList(this);
    m_listWidget->setModel(m_settingModel);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_listWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);  // Use full space
    setLayout(mainLayout);
    m_listWidget->setItemDelegate(new SettingItemDelegate(this));

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

    m_listWidget->setClickListener([this](auto index) {
        auto item = m_settingModel->rawData(index.row());
        item->onClicked();
    });

    auto soundItem = new InfoSettingItem(this);
    soundItem->setName("Sound")->setIcon(":/icons/sound_on.png");
    connect(soundItem, &BaseSettingItem::clicked, this, &SettingScreen::handleSoundSetting);

    auto wifiItem = new InfoSettingItem(this);
    wifiItem->setName("WiFi")->setIcon(":/icons/wifi.png");
    connect(wifiItem, &BaseSettingItem::clicked, this, &SettingScreen::handleWifiSetting);

    auto backgroundItem = new InfoSettingItem(this);
    backgroundItem->setName("Measure Background");

    auto settings = std::shared_ptr<QVector<BaseSettingItem*>>(new QVector<BaseSettingItem*>{
        (new HeaderSettingItem(this))->setName("Setting"),
        (new SubSettingItem(this))->setSettings(QVector<BaseSettingItem*>({
            backgroundItem
        })),
        soundItem,
        wifiItem,
    });
    m_settingModel->setData(settings);
}

void SettingScreen::setupViewActions() {
    // leftViewAction is "Back" by default from BaseScreen constructor.
    // We can customize its name or icon if needed:
    // leftViewAction.name = "Exit"; // Example

    setCenterAction(new ViewAction("OK", [this]() -> bool {
        if (m_listWidget) {
            m_listWidget->performClick();
        }
        return false;
    }));

    setRightAction(new ViewAction("Down", [this]() {
        if (m_listWidget) {
            m_listWidget->moveDown();
            return true;
        }
        return false;
    }));

    setLongRightAction(new ViewAction("Up", [this]() -> bool {
        if (m_listWidget) {
            m_listWidget->moveUp();
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
