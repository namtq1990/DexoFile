#ifndef DUALSOCKETNOTIFIER_H
#define DUALSOCKETNOTIFIER_H

#include <QObject>
#include <QSocketNotifier>

class DualSocketNotifier : public QObject {
    Q_OBJECT
public:
    explicit DualSocketNotifier(int socketFd, QObject* parent = nullptr);
    ~DualSocketNotifier();

signals:
    void newData();
    void disconnect();

private slots:
    void handleReadActivated();
    void handleExceptionActivated();

private:
    int m_fd;
    QSocketNotifier* m_readNotifier;
    QSocketNotifier* m_exceptionNotifier;
};

#endif // DUALSOCKETNOTIFIER_H
