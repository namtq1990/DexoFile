#ifndef CHOICESDIALOG_H
#define CHOICESDIALOG_H

#include "base/basedialog.h"
#include <QMap>

namespace Ui {
class ChoicesDialog;
}

class ChoicesDialog : public BaseDialog
{
    Q_OBJECT


public:
    typedef QMap<QString, QVariant> Choices;

    explicit ChoicesDialog(const QString& tag, QWidget *parent = nullptr);
    ~ChoicesDialog();

    void onCreate(navigation::NavigationArgs *entry) override;

    void setData(Choices& choices);
    QVariant curChoice();

private:
    Choices mData;

    Ui::ChoicesDialog *ui;
};

struct ChoicesDialogArgs : public navigation::NavigationArgs {
    ChoicesDialog::Choices choiceData;
    QString title;

    ChoicesDialogArgs(const ChoicesDialog::Choices& choices, const QString& title) : choiceData(choices), title(title){}
};

#endif // CHOICESDIALOG_H
