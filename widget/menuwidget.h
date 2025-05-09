#ifndef MENUWIDGET_H
#define MENUWIDGET_H

#include <QWidget>

// Forward declaration of the UI class
namespace Ui {
class MenuWidget;
}

class MenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MenuWidget(QWidget *parent = nullptr);
    ~MenuWidget();

    // Example: If you need to access buttons to connect signals externally
    // class NcButton* getTargetButton() const;
    // class NcButton* getIdScanButton() const;
    // class NcButton* getSettingsButton() const;


private:
    Ui::MenuWidget *ui;
};

#endif // MENUWIDGET_H
