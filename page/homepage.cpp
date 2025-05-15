#include "page/homepage.h"
#include "ui_homepage.h"  // Generated UI header
using namespace navigation;

void navigation::toHome(NavigationComponent* navController, NavigationEntry* entry, const QString& tag) {
    auto pWidget = dynamic_cast<QWidget*>(entry->host);
    entry->type  = TAB_IN_WINDOW;
    entry->view  = new HomePage(pWidget);
    navController->enter(entry);
}

HomePage::HomePage(QWidget* parent) : BaseScreen(tag::HOME_TAG, parent), ui(new Ui::HomePage) {
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

    setCenterAction(new ViewAction(
        "Scan ID",
        [this]() {
            nucare::logD() << "Scan ID action triggered from HomePage";
            // TODO: if(m_navComponent) m_navComponent->navigateTo(new IdScanPage(m_navComponent, this));
            return true;
        },
        nullptr));

    setRightAction(new ViewAction(
        "Events",
        [this]() {
            nucare::logD() << "Event action triggered";
            return true;
        },
        nullptr));

    setRightAction(new ViewAction(
        "Settings",
        [this]() {
            navigation::toSetting(getNavigation(), new navigation::NavigationEntry(navigation::CHILD_IN_WINDOW, nullptr,
                                                                                   nullptr, this->parent()));
            return true;
        },
        nullptr));
}

HomePage::~HomePage() { delete ui; }

void HomePage::reloadLocal() { ui->retranslateUi(this); }
