#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include "component.h"
#include "platform_controller.h"
#include "util/dualsocketnotifier.h"

#include <QObject>
#include <QString>
#include <QList>
#include <QTimer>
#include <QFuture>
#include <QFutureWatcher>

// Include wpa_supplicant library
#include <wpa_ctrl.h>

class QSocketNotifier;

class WiFiService : public QObject, public Component
{
    Q_OBJECT

public:
    explicit WiFiService(QObject *parent = nullptr, const QString& tag = "WiFiService");

    QString currentSsid() const { return m_currentSsid; }
    bool isConnected() const { return m_isConnected; }

public slots:
    void scanNetworks();
    void connectToNetwork(const QString &ssid, const QString &password);
    void disconnectFromNetwork();

signals:
    void scanResultsAvailable(const QList<QString> &networks);
    void connected(bool success);
    void disconnected();
    void connectionError(const QString &error);
    void scanError(const QString &error);

private slots:
    void handleConnectionEvent(const QString &event);
    // void retryConnection();
    // void retryInit();
    // void handleWifiError(const QString &error);
    void setConnected(bool isConnected);

private:
    // wpa_supplicant control interface
    struct wpa_ctrl *m_ctrl;
    struct wpa_ctrl *m_monitor;
    QString m_currentSsid;
    bool m_isConnected;
    QTimer* m_retryTimer;
    QSocketNotifier* m_socketNotifier;
    DualSocketNotifier* m_dualSocketNotifier = nullptr;
    PlatformController* m_platformController;

    // Function to initialize the wpa_supplicant connection
    bool initWpaSupplicant();
    void cleanupWpaSupplicant();

    // Network management functions
    void scan();
    QFuture<QString> getNetworkStatus();
    QFuture<QString> getConnectedNetwork();
    QFuture<QStringList> getConfiguredNetworks();
    int findNetworkId(const QString &ssid);
    void updateNetworkConfig(int networkId, const QString &password);
    void saveConfig();

    // Event handling
    void startEventMonitor();
    void stopEventMonitor();
    void handleWifiEvent(const QString &event);

    // Helper functions
    QString executeWpaCommand(const QString &command);
    QFuture<QString> executeShellCommandAsync(const QString &command);
    bool parseScanResults(const QString &results, QList<QString> &networks);
};

#endif // WIFI_SERVICE_H
