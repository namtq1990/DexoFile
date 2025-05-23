#ifndef BASEDIALOG_H
#define BASEDIALOG_H

#include "base/baseview.h"
#include <QDialog> // Keep QDialog include for now, might be needed for some QDialog specific features or signals/slots

namespace Ui {
class BaseDialog;
}

class BaseDialog : public QDialog, public BaseView
{
    Q_OBJECT

public:
    explicit BaseDialog(const QString& tag = "", QWidget *parent = nullptr);
    ~BaseDialog() override;

    // BaseView overrides
    void onCreate(navigation::NavigationArgs* entry) override;
    void onEnter() override;
    void onExit() override;
    void onDestroy() override;

    // Pure virtual from BaseView
    void reloadLocal() override;

    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *ev) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

    void setContentText(const QString& text);
    void setTitle(const QString& title);
    void setContentView(QWidget* view);



public slots:
    void onNeutralClick();
    void onCancelClick();
    void onOkClick();

protected:
    Ui::BaseDialog* ui;
};

#endif // BASEDIALOG_H
