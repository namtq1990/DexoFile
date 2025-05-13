#include "widget/statusbarwidget.h"
#include "ui_statusbarwidget.h"
#include <QDateTime>

StatusBarWidget::StatusBarWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusBarWidget)
{
    setAttribute(Qt::WA_StyledBackground, true);
    ui->setupUi(this);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &StatusBarWidget::updateTime);
    m_timer->start(1000); // Update every second

    updateTime(); // Initial time update
}

StatusBarWidget::~StatusBarWidget()
{
    delete ui;
}

void StatusBarWidget::updateTime()
{
    QString currentTime = QDateTime::currentDateTime().toString("hh:mm AP");
    ui->timeLabel->setText(currentTime);
}
