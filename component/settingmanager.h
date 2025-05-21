#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include "component.h"

class SubSettingItem;

namespace setting {

SubSettingItem* buildSettingTree();

class SettingManager : public Component
{
   public:
    SettingManager();
    virtual ~SettingManager();

    // Add your methods here
};

}  // namespace setting

#endif