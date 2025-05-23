#ifndef SETTINGSCREEN_H
#define SETTINGSCREEN_H

#include "base/basescreen.h"
#include "model/settingmodel.h"

#include <memory>
#include <vector>

// Forward declarations
class BaseList;
namespace Ui { class SettingScreen; } // If using a .ui file for layout

class SettingScreen : public BaseScreen
{
    Q_OBJECT

public:
    explicit SettingScreen(QWidget *parent = nullptr);
    ~SettingScreen();

    void reloadLocal() override {};

private:
    // If not using a .ui file for the layout of SettingScreen itself:
    BaseList *m_listWidget;
    SettingModel* m_settingModel;
    SubSettingItem* m_subSetting = nullptr;

    void onCreate(navigation::NavigationArgs* arg) override;
    void onDestroy() override;

    void setupListItems();
    void setupViewActions();
    

    friend void navigation::toSetting(navigation::NavigationComponent* navController, navigation::NavigationEntry* entry,
                           SubSettingItem* item,
                           const QString& tag);

private slots:
 void openSubSetting(BaseSettingItem* item);
 void openChoice(BaseSettingItem* item);
 void openSwVersion(BaseSettingItem* item);
 void handleSoundSetting(BaseSettingItem* item);
 void handleWifiSetting(BaseSettingItem* item);
};

#endif // SETTINGSCREEN_H
