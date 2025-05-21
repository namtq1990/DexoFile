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

    // If using a .ui file that contains the BaseListWidget:
    // Ui::SettingScreen *ui;
    // And m_listWidget would be ui->yourBaseListWidgetName

    void setupListItems();
    void setupViewActions();

    void handleSoundSetting();
    void handleWifiSetting();
};

#endif // SETTINGSCREEN_H
