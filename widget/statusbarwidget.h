#ifndef STATUSBARWIDGET_H
#define STATUSBARWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QDateTime>

// Forward declaration of the UI class
namespace Ui {
class StatusBarWidget;
}

class StatusBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatusBarWidget(QWidget *parent = nullptr);
    ~StatusBarWidget();

private slots:
    void updateTime();
    void updateWifiStatus();

private:
    Ui::StatusBarWidget *ui;
    QTimer *m_timer;
};

#endif // STATUSBARWIDGET_H
