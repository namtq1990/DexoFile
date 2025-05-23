#include "basedialog.h"
#include "ui_basedialog.h"
#include "component/navigationcomponent.h"
#include "component/componentmanager.h"
#include <QLabel>
#include <QVBoxLayout>

BaseDialog::BaseDialog(const QString& tag, QWidget* parent)
    : QDialog(parent),  // Explicitly call QDialog constructor
      BaseView(tag),    // Provide a tag for BaseView
      ui(new Ui::BaseDialog) {
    setWindowFlag(Qt::FramelessWindowHint);
//    setAttribute(Qt::WA_NoSystemBackground);
    setModal(false);
    installEventFilter(this);
    ui->setupUi(this);
    // Ensure contentWidget has a layout
    if (!ui->contentWidget->layout()) {
        ui->contentWidget->setLayout(new QVBoxLayout());
        ui->contentWidget->layout()->setContentsMargins(0, 0, 0, 0);
        ui->contentWidget->layout()->setSpacing(0);
    }

    auto actionLeft   = new ViewAction("BACK",
                                       [this]() {
                                         onCancelClick();
                                         return true;
                                     }
                                       //    ":/images/common/menu_back.png"
      );
    auto actionCenter = new ViewAction("OK",
                                       [this]() {
                                           onOkClick();
                                           return true;
                                       }
                                       //    ":/images/common/menu_ok.png"
    );

    setLeftAction(actionLeft);
    setCenterAction(actionCenter);
}

BaseDialog::~BaseDialog() {
    removeEventFilter(this);
    delete ui;
}

void BaseDialog::reloadLocal() {
    // Implementation for reloadLocal (pure virtual in BaseView)
}

void BaseDialog::onCreate(navigation::NavigationArgs* entry) {
    // Implementation for onCreate
    BaseView::onCreate(entry);
}

void BaseDialog::onEnter() {
    // Implementation for onEnter
    BaseView::onEnter();
}

void BaseDialog::onExit() {
    // Implementation for onExit
    BaseView::onExit();
}

void BaseDialog::onDestroy() {
    // Implementation for onDestroy
    BaseView::onDestroy();
}

void BaseDialog::setContentText(const QString& text) {
    // Clear existing content
    QLayoutItem* item;
    while ((item = ui->contentWidget->layout()->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    QLabel* label = new QLabel(text);
    label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);  // Align text to top-left
    label->setWordWrap(true);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);  // Ensure label expands
    ui->contentWidget->layout()->addWidget(label);
}

void BaseDialog::setTitle(const QString& title) { ui->titleLabel->setText(title); }

void BaseDialog::closeEvent(QCloseEvent* e) {
    QDialog::closeEvent(e);
    if (isFlagSet(navigation::FLAG_CREATED)) {
        // It is other close action from Navigation controller, so it has to be popped
        getNavigation()->pop(this);
    }
}

void BaseDialog::resizeEvent(QResizeEvent* ev) {
    QDialog::resizeEvent(ev);

    if (auto p = parentWidget()) {
        QPoint dialogCenter = mapToGlobal(rect().center());

        QPoint position     = pos();
        auto   pWindow      = p->window();
        QPoint parentCenter = pWindow->mapToGlobal(pWindow->rect().center());
        move(position + parentCenter - dialogCenter);
    }
}

bool BaseDialog::eventFilter(QObject* obj, QEvent* event) {
    if (obj->objectName() == "btnLeft" || obj->objectName() == "btnCenter" || obj->objectName() == "btnRight") {
        return false;
    }

    return QObject::eventFilter(obj, event);
}

void BaseDialog::setContentView(QWidget* view) {
    // Clear existing content
    QLayoutItem* item;
    while ((item = ui->contentWidget->layout()->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (view) {
        view->setParent(ui->contentWidget);                                   // Set parent to contentWidget
        view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);  // Ensure view expands
        ui->contentWidget->layout()->addWidget(view);
    }
}

void BaseDialog::onOkClick() {
    accept();
    close();
}

void BaseDialog::onCancelClick() {
    reject();
    close();
}

void BaseDialog::onNeutralClick() {
    done(-1);
    close();
}

void navigation::showWarning(NavigationComponent* navController, NavigationEntry* entry, const QString& text,
                             const QString& tag) {
    if (entry->view == nullptr) {
        if (auto w = dynamic_cast<QWidget*>(entry->host)) {
            BaseDialog* dialog = new BaseDialog(tag, w);
            dialog->setContentText(text);  // Use the new method
            entry->view = dialog;
        }
    }

    auto dlg    = dynamic_cast<BaseDialog*>(entry->view);
    entry->type = DIALOG;

    if (dlg) navController->enter(entry);
}

void navigation::showWarning(const QString&& text, const QString&& tag) {
    if (auto navControlelr = ComponentManager::instance().navigationComponent()) {
        auto w = (*navControlelr->findViewWithTag(tag::WINDOW_TAG))->view;
        if (auto window = dynamic_cast<QWidget*>(w)) {
            auto entry = new NavigationEntry(DIALOG, nullptr, nullptr, window);
            showWarning(navControlelr, entry, text, tag);
        }
    }
}

navigation::NavigationEntry* navigation::showWarning(BaseView* parent, const QString&& text, const QString& tag) {
    auto host = dynamic_cast<QWidget*>(parent);
    if (!host) return nullptr;

    auto view = new BaseDialog(tag, host);
    view->setContentText(text);
    auto ret = new NavigationEntry(DIALOG, view, nullptr, host);
    parent->getNavigation()->enter(ret);

    return ret;
}

void navigation::showSuccess(NavigationComponent* navController, NavigationEntry* entry, const QString& text,
                             const QString& tag) {
    showWarning(navController, entry, text, tag);
}

void navigation::showSuccess(const QString&& text, const QString&& tag) {
    showWarning(std::move(text), std::move(tag));
}

navigation::NavigationEntry* navigation::showSuccess(BaseView* parent, const QString&& text, const QString& tag) {
    auto host = dynamic_cast<QWidget*>(parent);
    if (!host) return nullptr;

    auto view = new BaseDialog(tag, host);
    view->setContentText(text);
    auto ret = new NavigationEntry(DIALOG, view, nullptr, host);
    parent->getNavigation()->enter(ret);

    return ret;
}
