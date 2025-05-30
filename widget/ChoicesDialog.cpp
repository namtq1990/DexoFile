#include "ChoicesDialog.h"
// #include "ui_BaseDialog.h"
#include "widget/basemodel.h"
#include "config.h"
#include "ui_ChoicesDialog.h"

#include <QPushButton>

void navigation::toChoiceDlg(NavigationComponent* navController, NavigationEntry* entry, ChoicesDialogArgs* args,
                             const QString& tag) {
    if (entry->view == nullptr) {
        if (auto w = dynamic_cast<QWidget*>(entry->host)) {
            entry->view = new ChoicesDialog(tag, w);
            entry->type = DIALOG;
        }
    }

    entry->setArguments<ChoicesDialogArgs>(args);
    navController->enter(entry);
}

navigation::NavigationEntry* navigation::toChoiceDlg(BaseView* parent, ChoicesDialogArgs* args) {
    if (auto widget = dynamic_cast<QWidget*>(parent)) {
        auto entry = new NavigationEntry(DIALOG, new ChoicesDialog(tag::CHOICE_TAG, widget), nullptr, widget);
        entry->setArguments(args);
        parent->getNavigation()->enter(entry);

        return entry;
    }

    return nullptr;
}

ChoicesDialog::ChoicesDialog(const QString& tag, QWidget* parent) : BaseDialog(tag, parent), ui(new Ui::ChoicesDialog) {
    QWidget* content = new QWidget(this);
    ui->setupUi(content);
    auto sizePolicy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->choiceList->setModel(new SimpleModel(this));
    setContentView(content);

    NC_UTIL_CLEAR_BGR(content);
    NC_UTIL_CLEAR_BGR(ui->choiceList);

    auto leftAct = new ViewAction(
        "UP",
        [this]() {
            ui->choiceList->moveUp();
            return true;
        },
        ":/icons/menu_up");
    auto centerAct = new ViewAction(
        "OK",
        [this]() {
            ui->choiceList->performClick();
            onOkClick();
            return true;
        },
        ":/icons/menu_ok");
    auto rightAct = new ViewAction(
        "DOWN",
        [this]() {
            ui->choiceList->moveDown();
            return true;
        },
        ":/icons/menu_down");
    setLeftAction(leftAct);
    setCenterAction(centerAct);
    setRightAction(rightAct);
}

ChoicesDialog::~ChoicesDialog() { delete ui; }

void ChoicesDialog::onCreate(navigation::NavigationArgs* entry) {
    BaseDialog::onCreate(entry);
    if (auto args = dynamic_cast<ChoicesDialogArgs*>(entry)) {
        setData(args->choiceData);
        setTitle(args->title);
    }
}

void ChoicesDialog::setData(Choices& choices) {
    if (auto model = dynamic_cast<BaseModel<QString>*>(ui->choiceList->model())) {
        mData     = choices;
        auto list = new QVector<QString>(mData.keys().toVector());
        std::sort(list->begin(), list->end(), [](QString& s1, QString& s2) -> int {
            auto cstr1 = s1.toUtf8();
            auto cstr2 = s2.toUtf8();
            if (std::isdigit(cstr1[0]) && std::isdigit(cstr2[0])) {
                auto f1 = std::stof(cstr1.data());
                auto f2 = std::stof(cstr2.data());
                return f1 < f2;
            }

            return s1.compare(s2) < 0;
        });
        model->setData(list);
    }
}

QVariant ChoicesDialog::curChoice() {
    auto slIndex = ui->choiceList->getSelectedIndex();
    if (slIndex >= 0) {
        if (auto model = dynamic_cast<SimpleModel*>(ui->choiceList->model())) {
            auto key = model->rawData(slIndex);
            return mData[key];
        }
    }

    return QVariant();
}
