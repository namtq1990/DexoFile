#include "basedialog.h"
#include "component/navigationcomponent.h"
#include "component/componentmanager.h"

void navigation::showWarning(NavigationComponent* navController, NavigationEntry* entry, const QString& text,
                             const QString& tag) {
    if (entry->view == nullptr) {
        if (auto w = dynamic_cast<QWidget*>(entry->host)) {
//            entry->view = new BaseDialog(tag, w);
        }
    }

    auto dlg    = dynamic_cast<BaseDialog*>(entry->view);
    entry->type = DIALOG;

    //    auto img = new QPushButton(dlg);
    //    img->setStyleSheet("border: none;");
    //    app_util::fitWidth(":/images/common/alert_yellow.png", img);
//    dlg->setTitle("Information")
        //            .setLayout(img)
//        .setMessage(text)
//        .show();

    navController->enter(entry);
}

void navigation::showWarning(const QString&& text,
                 const QString&& tag) {
    if (auto navControlelr = ComponentManager::instance().navigationComponent()) {

        auto w = (*navControlelr->findViewWithTag(tag::WINDOW_TAG))->view;
        if (auto window = dynamic_cast<QWidget*>(w)) {
            auto entry = new NavigationEntry(DIALOG,
                                             nullptr,
                                             nullptr,
                                             window);
            showWarning(navControlelr, entry, text, tag);
        }
    }
}

void navigation::showSuccess(NavigationComponent* navController,
                 NavigationEntry* entry,
                 const QString& text,
                 const QString& tag) {

}

void navigation::showSuccess(const QString&& text,
                 const QString&& tag) {

}

BaseDialog::BaseDialog()
{

}
