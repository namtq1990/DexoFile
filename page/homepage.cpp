#include "page/homepage.h"
#include "ui_homepage.h" // Generated UI header
#include "component/componentmanager.h"
#include "component/navigationcomponent.h"
#include "util/util.h"

// Placeholder forward declarations for pages to be created
// class IdScanPage;
// class SettingsPage;

HomePage::HomePage(QWidget *parent)
    : BaseScreen(parent),
      ui(new Ui::HomePage)
{
    ui->setupUi(this);

    // Define ViewActions for HomePage
    // Left action: Typically "Back", but HomePage is root, so maybe empty or context-specific.
    // For now, let's make it an "Info" button as an example, or leave it empty.
    // If it were a back button for a non-root page:
    // setLeftViewAction("Back", [this]() {
    //     if(m_navComponent) m_navComponent->navigateBack();
    //     return true;
    // }, ":/icons/menu_back.png"); // Assuming you have a back icon

    // For HomePage as root, left action is empty/disabled by default ViewAction constructor
    // Or, set an alternative action if desired. For this example, let's leave it default (invalid).
    // You can access UI elements via ui->... e.g. ui->titleLabel->setText("New Title");

    setCenterViewAction("Scan ID", [this]() {
        nucare::logD() << "Scan ID action triggered from HomePage";
        // TODO: if(m_navComponent) m_navComponent->navigateTo(new IdScanPage(m_navComponent, this));
        return true;
    }, nullptr);

    setRightViewAction("Events", [this]() {
        nucare::logD() << "Event action triggered";
        return true;
    }, nullptr);

    setLongClickRightViewAction(
        "Settings",
        [this]() {
            ComponentManager::instance().navigationComponent()->navigateSettingPage(parentWidget());
            return true;
        },
        nullptr);
}

HomePage::~HomePage()
{
    delete ui;
}

void HomePage::onScreenShown()
{
    BaseScreen::onScreenShown(); // Call base implementation (emits signal)
    nucare::logD() << "HomePage shown";
}

void HomePage::onScreenHidden()
{
    BaseScreen::onScreenHidden(); // Call base implementation (emits signal)
    nucare::logD() << "HomePage hidden";
}
