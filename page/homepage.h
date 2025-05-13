#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include "widget/basescreen.h" // Inherit from BaseScreen

// Forward declaration for the UI class
namespace Ui {
class HomePage;
}

// Forward declare other pages if actions navigate to them directly
// class IdScanPage;
// class SettingsPage;

// NavigationComponent is forward-declared in basescreen.h which is included above.
// No need for a redundant forward declaration here.

class HomePage : public BaseScreen
{
    Q_OBJECT
public:
    explicit HomePage(QWidget *parent = nullptr); // navComp parameter removed
    ~HomePage() override;

    // BaseScreen interface
    void onScreenShown() override;
    void onScreenHidden() override;

private:
    Ui::HomePage *ui;
    // Other UI elements for HomePage can be accessed via ui->...
};

#endif // HOMEPAGE_H
