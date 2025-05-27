#ifndef DETECTORCOMPONENT_H
#define DETECTORCOMPONENT_H

#include "component/component.h"
#include <QSerialPort>
#include <QByteArray>
#include <QTimer>

namespace nucare {

class DetectorComponent : public QObject, public Component
{
    Q_OBJECT
public:
    explicit DetectorComponent(QObject *parent = nullptr);
    ~DetectorComponent();

    void initialize(const QString &portName);
    void start();
    void stop();

    bool openSerialPort(const QString &portName, int baudRate);
    void closeSerialPort();

    void sendCommand(const QByteArray &command);

signals:
    void detectorInfoReceived(const QByteArray &info);
    void packageReceived(const QByteArray &packageData);
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
