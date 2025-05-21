#include "settingmanager.h"
#include "model/basesettingitem.h"
using namespace std;
using namespace setting;

SubSettingItem* setting::buildSettingTree() {
    SubSettingItem* ret = new SubSettingItem();
    ret->setSettings({

        (new HeaderSettingItem(ret))->setName("Setting"),
        (new SubSettingItem(ret))
            ->setSettings({
                (new ChoiceSettingItem(QMap<QString, QVariant>({
                                           {"60", 60},
                                           {"120", 120},
                                           {"180", 180},
                                           {"300", 300},
                                           {"600", 600},
                                       }),
                                       ret))
                    ->setName("Measure Time"),
                (new InfoSettingItem(ret))->setName("Measure Background")->setClickAction("openBackground"),
            })
            ->setName("Background"),
        (new SubSettingItem(ret))
            ->setSettings({
                (new ChoiceSettingItem(
                     {
                         {"100000", 100000},
                         {"200000", 200000},
                         {"300000", 300000},
                         {"400000", 400000},
                         {"500000", 500000},
                     },
                     ret))
                    ->setName("Calibration Count"),
                (new InfoSettingItem(ret))->setName("Energy Calibration"),
                (new ChoiceSettingItem(
                     {
                         {"Ba", "Ba"},
                         {"Cs137", "Cs137"},
                     },
                     ret))
                    ->setName("Selec Source"),
            })
            ->setName("Calibration"),
        (new SubSettingItem(ret))
            ->setSettings({
                (new ChoiceSettingItem({}, ret))->setName("Material"),
                (new ChoiceSettingItem({}, ret))->setName("Density"),
                (new ChoiceSettingItem({}, ret))->setName("Thickness"),
                (new ChoiceSettingItem({}, ret))->setName("Diameter"),
            })
            ->setName("Pipe Information"),
        (new SubSettingItem(ret))
            ->setSettings({
                (new ChoiceSettingItem({}, ret))->setName("Measurement Time"),
                (new ChoiceSettingItem({}, ret))->setName("Measurement Interval Time"),
            })
            ->setName("Measurement Setup"),
        (new InfoSettingItem(ret))
            ->setName("Wireless Connection")
            ->setIcon(":/icons/wifi.png")
            ->setClickAction("handleWifiSetting"),
        (new InfoSettingItem(ret))
            ->setName("About S/W Version")
            // ->setIcon(":/icons/wifi.png")
            ->setClickAction("openSwVersion"),
    });

    return ret;
 }
