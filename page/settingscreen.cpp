#include "page/settingscreen.h"
#include "widget/baselist.h"
#include "util/util.h"                      // For nucare::logI
#include "component/componentmanager.h"     // For ComponentManager
#include "component/navigationcomponent.h"  // For NavigationComponent
#include "model/settingmodel.h"
#include "widget/settingitemdelegate.h"
#include "widget/ChoicesDialog.h"
#include "model/basesettingitem.h"
#include <QVBoxLayout>
#include <QList>
#include <QStandardItemModel>
using namespace std;

void navigation::toSetting(navigation::NavigationComponent* navController, navigation::NavigationEntry* entry,
                           SubSettingItem* item, const QString& tag) {
    if (!entry->view) {
        auto view = new SettingScreen(static_cast<QWidget*>(entry->host));
        view->setTag(tag);
        view->m_subSetting = item;

        if (!item->parent()) {
            item->setParent(view);
        }

        entry->view = view;
    }
    navController->enter(entry);
}

SettingScreen::SettingScreen(QWidget* parent)
    : BaseScreen(tag::SETTING_TAG, parent), m_listWidget(nullptr), m_settingModel(new SettingModel(this)) {
    m_listWidget = new BaseList(this);
    m_listWidget->setModel(m_settingModel);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_listWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);  // Use full space
    setLayout(mainLayout);
    m_listWidget->setItemDelegate(new SettingItemDelegate(this));

    setupViewActions();

    // Ensure the list widget has focus to receive potential keyboard events
    // if not solely relying on ViewActions from menu.
    // m_listWidget->setFocus();
}

void SettingScreen::onCreate(navigation::NavigationArgs* args) {
    BaseScreen::onCreate(args);
    setupListItems();
}

void SettingScreen::onDestroy()
{
    BaseScreen::onDestroy();
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

    if (!m_subSetting) return;

    if (auto nodes = m_subSetting->getNodes()) {
        for (auto setting : *nodes) {
            if (auto action = setting->getAction()) {
                connect(setting, &BaseSettingItem::clicked, this, [this, action, setting]() {
                    QMetaObject::invokeMethod(this, action, Q_ARG(BaseSettingItem*, setting));
                });
            }
        }
        m_settingModel->setData(nodes);
    }
}

void SettingScreen::setupViewActions() {
    // leftViewAction is "Back" by default from BaseScreen constructor.
    // We can customize its name or icon if needed:
    // leftViewAction.name = "Exit"; // Example

    setCenterAction(new ViewAction("UP", [this]() -> bool {
        if (m_listWidget) {
            m_listWidget->moveUp();
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

    setLongCenterAction(new ViewAction("OK", [this]() -> bool {
        if (isShowLoading()) {
            return true;
        }

        if (m_listWidget) {
            m_listWidget->performClick();
            return true;
        }
        return false;
    }));
}

void SettingScreen::openSubSetting(BaseSettingItem* item) {
    if (auto subs = qobject_cast<SubSettingItem*>(item)) {
        auto entry = (new navigation::NavigationEntry(navigation::CHILD_IN_WINDOW))->setHost(this->parent());
        auto tag   = getTag() + "_" + subs->getName();
        navigation::toSetting(getNavigation(), entry, subs, tag);
    }
}

void SettingScreen::handleSoundSetting(BaseSettingItem* item) {
    nucare::logI() << "Handling Sound Setting (Navigation Not Implemented)";
    // NavigationComponent* navComp = ComponentManager::instance().navigationComponent();
    // if (navComp) navComp->navigateTo(new SoundPage(this)); // Assuming SoundPage exists
}

void SettingScreen::handleWifiSetting(BaseSettingItem* item) {
    nucare::logI() << "Handling WiFi Setting (Navigation Not Implemented)";
    // NavigationComponent* navComp = ComponentManager::instance().navigationComponent();
    // if (navComp) navComp->navigateTo(new WifiPage(this)); // Assuming WifiPage exists
}

void SettingScreen::openChoice(BaseSettingItem* item) {
    auto choices = qobject_cast<ChoiceSettingItem*>(item);
    navigation::toChoiceDlg(this, new ChoicesDialogArgs(choices->getChoices(), item->getName()));
}

void SettingScreen::openBackground(BaseSettingItem *)
{
    navigation::toBackground(this);
}

void SettingScreen::openSwVersion(BaseSettingItem *item)
{
    navigation::showSuccess(this, "SW version: 0.0.1");
}
