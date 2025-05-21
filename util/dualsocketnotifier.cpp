#include "dualsocketnotifier.h"
#include <unistd.h>

DualSocketNotifier::DualSocketNotifier(int socketFd, QObject* parent)
    : QObject(parent), m_fd(socketFd)
{
    m_readNotifier = new QSocketNotifier(socketFd, QSocketNotifier::Read, this);
    m_exceptionNotifier = new QSocketNotifier(socketFd, QSocketNotifier::Exception, this);
    connect(m_readNotifier, &QSocketNotifier::activated, this, &DualSocketNotifier::handleReadActivated);
    connect(m_exceptionNotifier, &QSocketNotifier::activated, this, &DualSocketNotifier::handleExceptionActivated);
}

DualSocketNotifier::~DualSocketNotifier() {
    // QSocketNotifier objects are deleted automatically as children
}

void DualSocketNotifier::handleReadActivated() {
    qDebug("Reading data");
    emit newData();
}

void DualSocketNotifier::handleExceptionActivated() {
    qDebug("Exception got");
    emit disconnect();
}
