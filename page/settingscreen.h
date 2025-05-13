#ifndef SETTINGSCREEN_H
#define SETTINGSCREEN_H

#include "widget/basescreen.h" // Inherit from BaseScreen

// Forward declarations
class BaseListWidget;
namespace Ui { class SettingScreen; } // If using a .ui file for layout

class SettingScreen : public BaseScreen
{
    Q_OBJECT

public:
    explicit SettingScreen(QWidget *parent = nullptr);
    ~SettingScreen();

private:
    // If not using a .ui file for the layout of SettingScreen itself:
    BaseListWidget *m_listWidget; 

    // If using a .ui file that contains the BaseListWidget:
    // Ui::SettingScreen *ui; 
    // And m_listWidget would be ui->yourBaseListWidgetName

    void setupListItems();
    void setupViewActions();

    void handleSoundSetting();
    void handleWifiSetting();
};

#endif // SETTINGSCREEN_H
