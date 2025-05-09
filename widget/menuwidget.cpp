#include "widget/menuwidget.h"
#include "ui_menuwidget.h" // Generated UI header for MenuWidget
// You might need to include NcButton.h if you add getter methods for NcButton
// #include "widget/NcButton.h"

MenuWidget::MenuWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MenuWidget)
{
    ui->setupUi(this);

    // Example of how you might connect signals if needed:
    // connect(ui->button1, &NcButton::clicked, this, &YourSlotForButton1);
    // connect(ui->button2, &NcButton::clicked, this, &YourSlotForButton2);
    // connect(ui->button3, &NcButton::clicked, this, &YourSlotForButton3);
}

MenuWidget::~MenuWidget()
{
    delete ui;
}

/* Example getter implementations if you uncomment them in the header
NcButton* MenuWidget::getTargetButton() const
{
    return ui->button1;
}

NcButton* MenuWidget::getIdScanButton() const
{
    return ui->button2;
}

NcButton* MenuWidget::getSettingsButton() const
{
    return ui->button3;
}
*/
