#ifndef MENUWIDGET_H
#define MENUWIDGET_H

#include <QWidget>
#include "navigationcomponent.h"

// Forward declarations
namespace Ui {
class MenuWidget;
}
class NcButton;
class ViewAction;

class MenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MenuWidget(QWidget *parent = nullptr);
    ~MenuWidget();

    navigation::NavigationComponent* navigation();
    void updateMenu();

    enum ActionType {
        LEFT,
        CENTER,
        RIGHT,
        LONG_LEFT,
        LONG_CENTER,
        LONG_RIGHT
    };

    std::shared_ptr<ViewAction> getActionFor(BaseView* view, ActionType type);
    std::shared_ptr<ViewAction> getActionFor(ActionType type);

    /**
     * @brief onMenuSelected    Handle when menu is selected.
     * @return  Return true then block custom action. Otherwise open view action
     */
    bool onMenuSelected();

private slots:
void onGlobalViewPushed(navigation::NavigationComponent* source, BaseView* newView);
void onGlobalViewPopped(navigation::NavigationComponent* source, BaseView* oldView);

void simulateButtonPress(NcButton* button);
void simulateButtonRelease(NcButton* button);

    // Slots for NcButton clicks
    void onLeftButtonClicked();
    void onCenterButtonClicked();
    void onRightButtonClicked();

    // Slots for NcButton long clicks
    void onLeftButtonLongClicked(long msec);
    void onCenterButtonLongClicked(long msec);
    void onRightButtonLongClicked(long msec);

private:
    Ui::MenuWidget *ui;

    void updateButtonWithAction(NcButton* button, const ViewAction& shortAction, const ViewAction& longAction);
};

#endif // MENUWIDGET_H
