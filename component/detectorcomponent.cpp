#include "detectorcomponent.h"
#include "util/util.h" // For logging
#include <QSerialPortInfo> // For dynamic port finding (optional, but good to have)

namespace nucare {

// Define the Payload struct for detector info
struct Payload {
    char header[2];     // GD
    uint16_t gain;
    uint16_t k40Ch;
    uint16_t detectorCode;
    uint16_t ch32Kev;
    uint16_t ch662Kev;
    char padding1[4];
    char temperatureFlag;
    uint16_t temperature;
    char serial[6];
    char padding2;
    char tail[2];
} __attribute__((__packed__));

static_assert (sizeof(Payload) == 28, "Invalid size for payload");


// Define the Package union for continuous data
union Package {
    char Bytes[4166]; // 4(header)+4096(spectrum)+2(offset1)+2(neutron)+6(realtime)+1(neutronGmFlag)+1(padding)+2(pileup)+2(temperatureChip)+8(padding3)+2(timestamp)+2(padding4)+2(detectorCode)+2(hvDac)+2(gc)+2(hvAdc)+2(ch32kev)+2(ch662kev)+2(ch1460kev)+2(tempOutside)+2(gm)+2(tail) = 4166
    struct Payload {
        char header[4]; // UUD0
        uint16_t spectrum[2048];
        char offset1[2]; // 33
        uint16_t neutron;
        uint64_t realtime : 48;
        uint8_t neutronGmFlag;
        uint8_t padding;
        uint16_t pileup;
        uint16_t temperatureChip;
        uint16_t padding3[4];
        uint16_t timestamp;
        uint16_t padding4;
        uint16_t detectorCode;
        uint16_t hvDac;
        uint16_t gc;
        uint16_t hvAdc;
        uint16_t ch32kev;
        uint16_t ch662kev;
        uint16_t ch1460kev;
        uint16_t tempOutside;
        uint16_t gm;
        char tail[20]; // tail is 20 bytes, last 2 bytes must be FF FF
    } __attribute__((__packed__));
};

static_assert (sizeof(Package::Payload) == sizeof (Package::Bytes), "Invalid size for package");

constexpr char PACKAGE_HEADER[4] = {'U', 'U', 'D', '0'};
constexpr char PACKAGE_TAIL[2] = {'6', '6'};

DetectorComponent::DetectorComponent(QObject *parent)
    : Component("DETECTOR"), m_serialPort(new QSerialPort(this)),
      m_retryCount(0), m_commandState(Idle), m_packageBytes(0)
{
    memset(m_packageBuffer, 0, sizeof(m_packageBuffer));
    // Connect serial port signals
    connect(m_serialPort, &QSerialPort::readyRead, this, &DetectorComponent::readData);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &DetectorComponent::handleError);

    // Connect timer signal
    connect(&m_responseTimer, &QTimer::timeout, this, &DetectorComponent::handleTimeout);
    m_responseTimer.setSingleShot(true); // Timer for response timeouts
}

DetectorComponent::~DetectorComponent()
{
    closeSerialPort();
}

void DetectorComponent::initialize(const QString& portName)
{
    logI() << "Initializing DetectorComponent.";
    int baudRate = 230400; // TODO: Load from settings

    if (openSerialPort(portName, baudRate)) {
        logI() << "Serial port" << portName << "opened successfully.";
    } else {
        logE() << "Failed to open serial port" << portName << ":" << m_serialPort->errorString();
        emit errorOccurred(m_serialPort->errorString());
    }
}

void DetectorComponent::start()
{
    logI() << "Starting DetectorComponent command flow.";
    startCommandFlow();
}

void DetectorComponent::stop()
{
    logI() << "Stopping DetectorComponent.";
    closeSerialPort();
    m_responseTimer.stop();
    m_commandState = Idle;
    m_readBuffer.clear();
    m_retryCount = 0;
}

bool DetectorComponent::openSerialPort(const QString &portName, int baudRate)
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    return m_serialPort->open(QIODevice::ReadWrite);
}

void DetectorComponent::closeSerialPort()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        logI() << "Serial port closed.";
    }
}

void DetectorComponent::sendCommand(const QByteArray &command)
{
    if (m_serialPort->isOpen()) {
        m_serialPort->write(command);
        logD() << "Sent command:" << command;
        m_currentCommand = command;
        m_readBuffer.clear(); // Clear buffer for new response
        m_responseTimer.start(1000); // Start timeout timer (1 second)
    } else {
        logE() << "Cannot send command, serial port not open.";
        emit errorOccurred("Cannot send command, serial port not open.");
    }
}

void DetectorComponent::readData()
{
    m_readBuffer.append(m_serialPort->readAll());
    logD() << "Received data. Current buffer size:" << m_readBuffer.size();
    processReceivedData();
}

void DetectorComponent::handleTimeout()
{
    logW() << "Response timeout for command:" << m_currentCommand;
    m_retryCount++;
    if (m_commandState == WaitingForStopResponse) {
        // For Stop, timeout means success (no data expected)
        logI() << "Stop command acknowledged by timeout (no data expected).";
        m_retryCount = 0;
        sendGetInfoCommand();
        return;
    }
    if (m_retryCount < 3) {
        logI() << "Retrying command:" << m_currentCommand << "(Attempt" << m_retryCount + 1 << "of 3)";
        sendCommand(m_currentCommand); // Retry the command
    } else {
        logE() << "Command failed after 3 retries:" << m_currentCommand;
        emit errorOccurred("Command failed after 3 retries.");
        m_commandState = Idle; // Reset state
        m_retryCount = 0;
    }
}

void DetectorComponent::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        logE() << "Serial port error:" << m_serialPort->errorString();
        emit errorOccurred(m_serialPort->errorString());
    }
}

void DetectorComponent::processReceivedData()
{
    // Improved buffer processing: handle partial, multiple, and misaligned messages
    while (true) {
        switch (m_commandState) {
            case WaitingForInfoResponse: {
                int headerIndex = m_readBuffer.indexOf("GD");
                if (headerIndex < 0) break;
                if (m_readBuffer.size() < headerIndex + sizeof(Payload)) break;
                QByteArray payloadData = m_readBuffer.mid(headerIndex, sizeof(Payload));
                processDetectorInfo(payloadData);
                m_readBuffer.remove(0, headerIndex + sizeof(Payload));
                m_responseTimer.stop();
                m_retryCount = 0;
                sendStartCommand();
                continue;
            }
            case ReceivingPackage: {
                while (!m_readBuffer.isEmpty()) {
                    if (m_packageBytes == 0) {
                        // Not currently accumulating, search for header
                        int headerIndex = m_readBuffer.indexOf(QByteArray(PACKAGE_HEADER, 4));
                        if (headerIndex < 0) {
                            if (m_readBuffer.size() > 3)
                                m_readBuffer = m_readBuffer.right(3);
                            break;
                        }
                        if (headerIndex > 0) {
                            // There is data before the header: treat as tail of previous package if enough bytes
                            int tailBytes = headerIndex;
                            if (m_packageBytes > 0 && (m_packageBytes + tailBytes) >= sizeof(Package)) {
                                int bytesNeeded = sizeof(Package) - m_packageBytes;
                                int bytesToCopy = std::min(bytesNeeded, tailBytes);
                                memcpy(m_packageBuffer + m_packageBytes, m_readBuffer.constData(), bytesToCopy);
                                m_packageBytes += bytesToCopy;
                                if (m_packageBytes == sizeof(Package)) {
                                    processPackageData(QByteArray(m_packageBuffer, sizeof(Package)));
                                    m_packageBytes = 0;
                                }
                            }
                            // Remove up to the header for new package
                            m_readBuffer.remove(0, headerIndex);
                        }
                        int bytesToCopy = std::min((int)m_readBuffer.size(), (int)sizeof(Package));
                        memcpy(m_packageBuffer, m_readBuffer.constData(), bytesToCopy);
                        m_packageBytes = bytesToCopy;
                        m_readBuffer.remove(0, bytesToCopy);
                        if (m_packageBytes == sizeof(Package)) {
                            processPackageData(QByteArray(m_packageBuffer, sizeof(Package)));
                            m_packageBytes = 0;
                            continue;
                        }
                        break;
                    } else {
                        int bytesNeeded = sizeof(Package) - m_packageBytes;
                        int bytesToCopy = std::min(bytesNeeded, m_readBuffer.size());
                        memcpy(m_packageBuffer + m_packageBytes, m_readBuffer.constData(), bytesToCopy);
                        m_packageBytes += bytesToCopy;
                        m_readBuffer.remove(0, bytesToCopy);
                        if (m_packageBytes == sizeof(Package)) {
                            processPackageData(QByteArray(m_packageBuffer, sizeof(Package)));
                            m_packageBytes = 0;
                            continue;
                        }
                        break;
                    }
                }
                break;
            }
            case WaitingForStopResponse: {
                // After sending Stop, we expect NO data. Timeout means success.
                // If any data is received, log a warning and clear it, but do not treat as success.
                if (!m_readBuffer.isEmpty()) {
                    logW() << "Unexpected data received after sending Stop command. Clearing buffer.";
                    m_readBuffer.clear();
                }
                // Do not trigger sendGetInfoCommand here; let the timeout handler advance the state.
                break;
            }
            case WaitingForStartResponse: {
                if (!m_readBuffer.isEmpty()) {
                    logI() << "Response received after sending Start command. Entering ReceivingPackage state.";
                    m_responseTimer.stop();
                    m_retryCount = 0;
                    m_commandState = ReceivingPackage;
//                    m_readBuffer.clear();
                    processReceivedData();  // If received data in start response, forward this buffer for next step
                }
                break;
            }
            case Idle: {
                if (!m_readBuffer.isEmpty()) {
                    logW() << "Received unexpected data in Idle state." << m_readBuffer.toHex();
                    m_readBuffer.clear();
                }
                break;
            }
            case SendingStop:
            case SendingGetInfo:
            case SendingStart:
                // Should not process data while just sending, wait for response state.
                break;
        }
        // If we didn't continue, break the loop
        break;
    }
}

void DetectorComponent::processDetectorInfo(const QByteArray &data)
{
    // Process the received detector info (28 bytes)
    if (data.size() == sizeof(Payload)) {
        const Payload* payload = reinterpret_cast<const Payload*>(data.constData());
        // Validate header 'GD' - already done in processReceivedData, but double-check
        if (payload->header[0] == 'G' && payload->header[1] == 'D') {
            logI() << "Successfully processed detector info.";
            // You can access payload members here, e.g., payload->gain, payload->detectorCode
            // For now, just emit the raw data.
            emit detectorInfoReceived(data);
        } else {
            logE() << "Detector info validation failed: Incorrect header.";
            emit errorOccurred("Detector info validation failed: Incorrect header.");
        }
    } else {
        logE() << "Detector info validation failed: Incorrect size (" << data.size() << " bytes, expected" << sizeof(Payload) << ").";
        emit errorOccurred("Detector info validation failed: Incorrect size.");
    }
}


void DetectorComponent::processPackageData(const QByteArray &data)
{
    // Process the received package data (4166 bytes)
    if (data.size() == sizeof(Package::Bytes)) {
        auto package = reinterpret_cast<const Package::Payload*>(data.constData());
        // Use constexpr header/tail for fast check
        bool headerOk = (memcmp(package->header, PACKAGE_HEADER, 4) == 0);
        bool tailOk = (memcmp(package->tail + 18, PACKAGE_TAIL, 2) == 0);
        if (headerOk && tailOk) {
            logD() << "Successfully processed package data.";
            emit packageReceived(data);
        } else {
            logW() << "Package data validation failed: Incorrect header or tail.";
        }
    } else {
        logW() << "Package data validation failed: Incorrect size (" << data.size() << " bytes, expected" << sizeof(Package::Bytes) << ").";
    }
}

void DetectorComponent::startCommandFlow()
{
    if (m_commandState == Idle) {
        sendStopCommand();
    } else {
        logW() << "Command flow already in progress. Current state:" << m_commandState;
    }
}

void DetectorComponent::sendStopCommand()
{
    logI() << "Sending Stop command (A4).";
    m_commandState = SendingStop;
    sendCommand("A4");
    m_commandState = WaitingForStopResponse;
}

void DetectorComponent::sendGetInfoCommand()
{
    logI() << "Sending Get Info command (GS).";
    m_commandState = SendingGetInfo;
    sendCommand("GS");
    m_commandState = WaitingForInfoResponse;
}

void DetectorComponent::sendStartCommand()
{
    logI() << "Sending Start command (A2).";
    m_commandState = SendingStart;
    sendCommand("A2");
    m_commandState = WaitingForStartResponse;
}

} // namespace nucare
