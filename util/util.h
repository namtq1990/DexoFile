#ifndef NDT_UTIL_H
#define NDT_UTIL_H

#include <QDebug>
#include <QString>
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

} // namespace nucare

#endif // NDT_UTIL_H
