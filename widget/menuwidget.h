#ifndef MENUWIDGET_H
#define MENUWIDGET_H

#include <QWidget>
#include "widget/basescreen.h" // Include full definition for BaseScreen and ViewAction

// Forward declarations
namespace Ui {
class MenuWidget;
}
class NavigationComponent;
class NcButton;
// BaseScreen and ViewAction are now included

class MenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MenuWidget(QWidget *parent = nullptr);
    ~MenuWidget();

    void setNavigationComponent(NavigationComponent* navComp);

private slots:
    void simulateButtonPress(NcButton* button);
    void simulateButtonRelease(NcButton* button);
    void onCurrentScreenChanged(BaseScreen *newScreen, BaseScreen *oldScreen);
    void onCanNavigateBackChanged(bool canGoBack); // If one button is dedicated to "Back"

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
    NavigationComponent *m_navigationComponent;


    void updateButtonWithAction(NcButton* button, const ViewAction& shortAction, const ViewAction& longAction);
};

#endif // MENUWIDGET_H
