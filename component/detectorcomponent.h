#ifndef DETECTORCOMPONENT_H
#define DETECTORCOMPONENT_H

#include "component/component.h"
#include "model/Spectrum.h" // Include Spectrum_t definition
#include "model/DetectorModels.h" // Include DetectorModels.h for GcResponse and DetectorPackage
#include <QSerialPort>
#include <QByteArray>
#include <QTimer>
#include <memory> // For std::shared_ptr

namespace nucare {

class DetectorProperty;

class DetectorComponent : public QObject, public Component
{
    Q_OBJECT
private:
    std::shared_ptr<DetectorProperty> m_properties;
public:
    explicit DetectorComponent(QObject *parent = nullptr);
    ~DetectorComponent();

    void open(const QString &portName);
    void start();
    void stop();
    auto properties() const { return m_properties; }

    bool openSerialPort(const QString &portName, int baudRate);
    void closeSerialPort();
    void clearSerialBuffer();

    void sendCommand(const QByteArray &command);

    void initialize();

signals:
    void detectorInfoReceived(DetectorComponent* dev, std::shared_ptr<GcResponse> info);
    void packageReceived(DetectorComponent* dev, std::shared_ptr<DetectorPackage> packageData);
    void detectorInitialized();
    void errorOccurred(const QString &errorMessage);

private slots:
    void readData();
    void handleTimeout();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort;
    QByteArray m_readBuffer;
    QTimer m_responseTimer;
    QByteArray m_currentCommand;
    int m_retryCount;

    enum CommandState {
        Idle,
        SendingStop,
        WaitingForStopResponse,
        SendingGetInfo,
        WaitingForInfoResponse,
        SendingStart,
        WaitingForStartResponse,
        ReceivingPackage
    };

    CommandState m_commandState;

    // Optimized package buffer for accumulation
    char m_packageBuffer[4166];
    int m_packageBytes = 0;

    void processReceivedData();
    void processDetectorInfo(const QByteArray &data);
    void processPackageData(const QByteArray &data);
    void startCommandFlow();
    void sendStopCommand();
    void sendGetInfoCommand();
    void sendStartCommand();

};

} // namespace nucare

#endif // DETECTORCOMPONENT_H
