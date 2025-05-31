#include "widget/statusbarwidget.h"
#include "ui_statusbarwidget.h"
#include "componentmanager.h"
#include "controller/wifi_service.h"
#include "component/detectorcomponent.h"
#include "model/DetectorProp.h"
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

    // Connect to WiFiService signals for status updates
    auto wifiService = ComponentManager::instance().wifiService();
    if (wifiService) {
        connect(wifiService, &WiFiService::connected, this, [this](bool success) {
            updateWifiStatus();
        });
        connect(wifiService, &WiFiService::disconnected, this, [this]() {
            updateWifiStatus();
        });
    }

    if (auto detector = ComponentManager::instance().detectorComponent()) {
        connect(detector, &nucare::DetectorComponent::detectorInitialized, this, [this, detector]() {
            ui->serialLabel->setText(detector->properties()->getSerial());
        });
    }

    updateWifiStatus();
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

void StatusBarWidget::updateWifiStatus()
{
    auto wifiService = ComponentManager::instance().wifiService();
    if (!wifiService) {
        ui->wifiIconLabel->setVisible(false);
        return;
    }
    bool connected = wifiService->isConnected();
    ui->wifiIconLabel->setVisible(connected);

    nucare::logD() << "Updated to " << connected;
}
