#include "ShutdownDialog.h"
#include "component/componentmanager.h"
#include "component/detectorcomponent.h"
#include "controller/platform_controller.h"

navigation::NavigationEntry* navigation::toShutdownDlg(const QString& tag) {
    auto topNav = ComponentManager::instance().navigationComponent();
        auto top = topNav->curEntry(true);
        if (dynamic_cast<ShutdownDialog*>(top->view)) {
            return top;
        } else if (auto curScreen = dynamic_cast<QWidget*>(top->view)) {
            auto entry = new NavigationEntry(DIALOG,
                                             new ShutdownDialog(curScreen, tag),
                                             nullptr,
                                             curScreen);
            topNav->enter(entry);

            return entry;
        }

        return nullptr;
}

ShutdownDialog::ShutdownDialog(QWidget *parent, const QString &tag)
    : BaseDialog(tag, parent)
{
    setContentText("Do you want to power off?");
    setTitle("Power");

    if (auto cAction = getCenterAction()) {
        cAction->action = [this]() {
            if (auto detector = ComponentManager::instance().detectorComponent()) {
                detector->sendPowerOff();
            }

            onOkClick();

            return true;
        };
    }
    if (auto rAction = getRightAction()) {
        rAction->name = "Restart";
        rAction->action = [this]() {
            ComponentManager::instance().platformController()->executeShellCommandAsync("reboot");
            onOkClick();
            return true;
        };
    }
}
