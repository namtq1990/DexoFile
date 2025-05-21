#include "wifi_service.h"
#include <QSocketNotifier>
#include <QTimer>
#include <QFuture>
#include <QFutureWatcher>
#include "util/util.h"
#include "platform_controller.h"
#include "componentmanager.h"
#include "util/dualsocketnotifier.h"

WiFiService::WiFiService(QObject *parent, const QString& tag) 
    : QObject(parent), Component(tag), m_ctrl(nullptr), m_monitor(nullptr), 
      m_isConnected(false), m_retryTimer(new QTimer(this)), m_socketNotifier(nullptr),
      m_platformController(ComponentManager::instance().platformController())
{
    m_retryTimer->setInterval(3000);
    m_retryTimer->setSingleShot(false);
    
    QObject::connect(m_retryTimer, &QTimer::timeout, this, [this]() {
        if (!m_ctrl || !m_monitor) {
            if (initWpaSupplicant()) {
                m_retryTimer->stop();
            }
        }
    });

    if (!initWpaSupplicant()) {
        logE() << "Failed to initialize wpa_supplicant connection, will retry every 3 seconds";
        m_retryTimer->start();
    }
}

//WiFiService::~WiFiService()
//{
//    cleanupWpaSupplicant();
//}

void WiFiService::scanNetworks()
{
    scan();
}

void WiFiService::connectToNetwork(const QString &ssid, const QString &password)
{
    
    // Check if already connected to this network
    QString connectedNetwork = getConnectedNetwork();
    if (connectedNetwork == ssid) {
        emit connected(true);
        return;
    }

    // Find existing network or create new one
    int networkId = findNetworkId(ssid);
    if (networkId == -1) {
        QString result = executeWpaCommand("ADD_NETWORK");
        if (result.isEmpty() || result.contains("FAIL")) {
            emit connectionError("Failed to add network");
            return;
        }
        networkId = result.toInt();
        
        // Set SSID for new network
        QString setSsid = executeWpaCommand(QString("SET_NETWORK %1 ssid \"%2\"").arg(networkId).arg(ssid));
        if (setSsid.contains("FAIL")) {
            emit connectionError("Failed to set network SSID");
            return;
        }
    }

    // Update password and enable network
    updateNetworkConfig(networkId, password);
    m_currentSsid = ssid;
    startEventMonitor();
    
    logI() << "Connecting to network:" << ssid;
}

void WiFiService::disconnectFromNetwork()
{
    if (m_isConnected) {
        executeWpaCommand("DISCONNECT");
        m_isConnected = false;
        m_currentSsid.clear();
        emit disconnected();
    }
}

bool WiFiService::initWpaSupplicant()
{
    cleanupWpaSupplicant();
    
    const char *ctrl_path = "/var/run/wpa_supplicant/wlan0";
    m_ctrl = wpa_ctrl_open(ctrl_path);
    if (!m_ctrl) {
        logE() << "Failed to open wpa_supplicant control interface";
        return false;
    }

    m_monitor = wpa_ctrl_open(ctrl_path);
    if (!m_monitor) {
        logE() << "Failed to open wpa_supplicant monitor interface";
        wpa_ctrl_close(m_ctrl);
        m_ctrl = nullptr;
        return false;
    }

    if (wpa_ctrl_attach(m_monitor) != 0) {
        logE() << "Failed to attach to wpa_supplicant monitor";
        cleanupWpaSupplicant();
        return false;
    }

    int sock = wpa_ctrl_get_fd(m_monitor);
    if (sock >= 0) {
        if (m_dualSocketNotifier) {
            delete m_dualSocketNotifier;
            m_dualSocketNotifier = nullptr;
        }
        m_dualSocketNotifier = new DualSocketNotifier(sock, this);
        QObject::connect(m_dualSocketNotifier, &DualSocketNotifier::newData, this, [this]() {
            char buf[256];
            size_t len = sizeof(buf) - 1;
            if (wpa_ctrl_recv(m_monitor, buf, &len) != 0) {
                if (!m_retryTimer->isActive()) {
                    m_retryTimer->start();
                }
                return;
            }
            buf[len] = '\0';
            handleConnectionEvent(QString::fromLocal8Bit(buf));
        });
        QObject::connect(m_dualSocketNotifier, &DualSocketNotifier::disconnect, this, [this]() {
            logE() << "WiFi monitor socket disconnected";
            if (!m_retryTimer->isActive()) {
                m_retryTimer->start();
            }
        });
    }


    nucare::handleFuture(getConnectedNetwork(), std::function<void(QString)>([this](QString result) {
                             logD() << "Connected status: " << result;
                         }), this);

    return true;
}

void WiFiService::cleanupWpaSupplicant()
{
    stopEventMonitor();
    
    if (m_monitor) {
        wpa_ctrl_detach(m_monitor);
        wpa_ctrl_close(m_monitor);
        m_monitor = nullptr;
    }

    if (m_ctrl) {
        wpa_ctrl_close(m_ctrl);
        m_ctrl = nullptr;
    }
}

void WiFiService::scan()
{
    QString results = executeWpaCommand("SCAN");
    if (results.isEmpty() || results.contains("FAIL")) {
        emit scanError("Failed to initiate scan");
        return;
    }

    // Wait a moment for scan results
    QTimer::singleShot(3000, [this]() {
        QString scanResults = executeWpaCommand("SCAN_RESULTS");
        QList<QString> networks;
        if (parseScanResults(scanResults, networks)) {
            emit scanResultsAvailable(networks);
        } else {
            emit scanError("Failed to parse scan results");
        }
    });
}

void WiFiService::startEventMonitor()
{
    // No-op: handled by DualSocketNotifier now
}

void WiFiService::stopEventMonitor()
{
    if (m_dualSocketNotifier) {
        delete m_dualSocketNotifier;
        m_dualSocketNotifier = nullptr;
    }
}

QFuture<QString> WiFiService::getNetworkStatus()
{
    return executeShellCommandAsync("wpa_cli -i wlan0 status");
}

QFuture<QString> WiFiService::getConnectedNetwork()
{
    return nucare::chainFuture<QString, QString>(
        getNetworkStatus(),
        [this](const QString& status) {
            QStringList lines = status.split('\n');
            for (const QString &line : lines) {
                if (line.startsWith("ssid=")) {
                    setConnected(true);
                    return line.mid(5);
                }
            }
            throw CommandException("No connected network found");
        }, this
    );
}

QFuture<QStringList> WiFiService::getConfiguredNetworks()
{
    return nucare::chainFuture<QString, QStringList>(
        executeShellCommandAsync("wpa_cli -i wlan0 list_networks"),
        [](const QString& result) {
            if (result.isEmpty()) {
                throw CommandException("Failed to list configured networks");
            }
            return result.split('\n');
        }, this
    );
}

int WiFiService::findNetworkId(const QString &ssid)
{
    QStringList networks = getConfiguredNetworks();
    if (networks.size() < 2) return -1; // Header + at least one network
    
    // Skip header line
    for (int i = 1; i < networks.size(); i++) {
        QStringList fields = networks[i].split('\t');
        if (fields.size() > 1 && fields[1] == ssid) {
            bool ok;
            int id = fields[0].toInt(&ok);
            return ok ? id : -1;
        }
    }
    return -1;
}

void WiFiService::updateNetworkConfig(int networkId, const QString &password)
{
    executeWpaCommand(QString("SET_NETWORK %1 psk \"%2\"").arg(networkId).arg(password));
    executeWpaCommand(QString("ENABLE_NETWORK %1").arg(networkId));
    saveConfig();
}

void WiFiService::saveConfig()
{
    nucare::handleFuture<QString>(
        executeShellCommandAsync("wpa_cli -i wlan0 save_config"),
        [this](const QString& result) {
            if (result.contains("FAIL")) {
                logE() << "Failed to save wpa_supplicant config";
            }
        }
    );
}

void WiFiService::handleConnectionEvent(const QString &event)
{
    if (event.contains("CTRL-EVENT-CONNECTED")) {
        m_isConnected = true;
        emit connected(true);
    } else if (event.contains("CTRL-EVENT-DISCONNECTED")) {
        m_isConnected = false;
        emit disconnected();
        // Start retry timer when disconnected
        if (!m_retryTimer->isActive()) {
            m_retryTimer->start();
        }
    } else if (event.contains("CTRL-EVENT-SSID-TEMP-DISABLED")) {
        emit connectionError("Authentication failed");
    }
}

void WiFiService::setConnected(bool isConnected)
{
    m_isConnected = isConnected;
    emit connected(m_isConnected);
}

QFuture<QString> WiFiService::executeShellCommandAsync(const QString &command)
{
    if (!m_platformController) {
        logE() << "PlatformController not available";
        return QFuture<QString>();
    }
    return m_platformController->executeShellCommandAsync(command);
}

QString WiFiService::executeWpaCommand(const QString &command)
{
    if (!m_ctrl) return QString();

    char buf[256];
    size_t len = sizeof(buf) - 1;
    if (wpa_ctrl_request(m_ctrl, command.toLocal8Bit().constData(), 
                        command.length(), buf, &len, nullptr)) {
        return QString();
    }
    buf[len] = '\0';
    return QString::fromLocal8Bit(buf);
}

bool WiFiService::parseScanResults(const QString &results, QList<QString> &networks)
{
    QStringList lines = results.split('\n');
    for (const QString &line : lines) {
        QStringList parts = line.split('\t');
        if (parts.size() >= 5) { // bssid, frequency, signal level, flags, ssid
            networks.append(parts[4]);
        }
    }
    return !networks.isEmpty();
}
