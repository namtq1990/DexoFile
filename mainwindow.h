#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <base/basewindow.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public BaseWindow
{
    Q_OBJECT

public:
    MainWindow(const QString& tag = "", QWidget *parent = nullptr);
    ~MainWindow();

    void reloadLocal() override;

    void onCreate(navigation::NavigationArgs* args) override;
    void updateMenu() override;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
