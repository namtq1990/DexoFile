#ifndef NDT_UTIL_H
#define NDT_UTIL_H

#include <QDebug>
#include <QString>
#include <QFuture>
#include <QFutureWatcher>
#include <functional>

namespace nucare {

template<typename T>
void handleFuture(QFuture<T> future, std::function<void(T)> successHandler, QObject* host = nullptr,
                 std::function<void(const QString&)> errorHandler = nullptr)
{
    auto* watcher = new QFutureWatcher<T>(host);
    QObject::connect(watcher, &QFutureWatcher<T>::finished, [=]() {
        try {
            successHandler(future.result());
        } catch (const std::exception& e) {
            if (errorHandler) {
                errorHandler(e.what());
            }
        }
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}

template<typename T, typename R>
QFuture<R> chainFuture(QFuture<T> future, std::function<R(T)> transform, QObject* host = nullptr)
{
    QFutureInterface<R> promise;
    auto* watcher = new QFutureWatcher<T>(host);
    QObject::connect(watcher, &QFutureWatcher<T>::finished, [=]() mutable {
        try {
            promise.reportResult(transform(future.result()));
        } catch (const std::exception& e) {
            promise.reportException(QException());
        }
        promise.reportFinished();
        watcher->deleteLater();
    });
    watcher->setFuture(future);
    return promise.future();
}

}
#include <QDateTime>
#include <QThread>
#include <QTextStream>

// Helper function to get current thread name or ID if name is not set
inline QString getCurrentThreadNameOrId() {
    QThread *currentThread = QThread::currentThread();
    QString threadName = currentThread->objectName();
    if (!threadName.isEmpty()) {
        return threadName;
    } else {
        // Fallback to thread ID if name is not set
        Qt::HANDLE threadId = currentThread->thread(); // QThread::currentThreadId() is also an option
        QString threadIdStr;
        QTextStream stream(&threadIdStr);
        stream << "Thread(0x" << Qt::hex << reinterpret_cast<quintptr>(threadId) << ")";
        return threadIdStr;
    }
}

namespace nucare {

// Base function to create a prefixed QDebug stream
inline QDebug createLogStream(QtMsgType type, const char* levelTag) {
    QDebug stream = QDebug(type);
    stream.nospace().noquote()
           << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "]"
           << " [" << getCurrentThreadNameOrId() << "]"
           << " [" << levelTag << "] "; // Add a space after the tag
    return stream;
}

inline QDebug logI() {
    return createLogStream(QtInfoMsg, "INFO");
}

inline QDebug logD() {
    return createLogStream(QtDebugMsg, "DEBUG");
}

inline QDebug logE() {
    return createLogStream(QtCriticalMsg, "ERROR");
}

inline QDebug logW() {
    return createLogStream(QtWarningMsg, "WARN");
}

} // namespace nucare

#endif // NDT_UTIL_H
