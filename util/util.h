#ifndef UTIL_H
#define UTIL_H

#include <QDebug>
#include <QString>
#include <QFuture>
#include <QFutureWatcher>
#include <functional>
#include <QFontMetrics>
#include "model/Matrix.h"

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

QString toExponentUnicode(const char& c);
QString toExponentFormat(const int& base, const int& exponent);

template<class P>
QString toString(const P& p);
template <class P>
P fromString(const QString& s);

template <typename T, size_t N, typename O = float, typename Container = std::list<T>>
struct Average {
    static_assert(std::is_arithmetic<T>::value, "Template parameter T must be number");

    Container values;
    O sum = T(0);

    Average() {}
    size_t size() { return N; }

    O calculate() const {
        if (values.empty()) {
            return 0;
        }

        return sum / values.size();
    }

    void addValue(T value) {
        if (values.size() >= N) {
            sum -= values.front();
            values.pop_front();
        }

        values.push_back(value);
        sum += value;
    }

    O addedValue(T value) {
        addValue(value);
        return calculate();
    }

    void reset() {
        values.clear();
        sum = T(0);
    }
};

template <class T>
int indexOfMax(const T* list, const nucare::uint start, const nucare::uint end) {
    nucare::uint max = start;
    for (nucare::uint i = start; i <= end; i++) {
        if (list[i] > list[max]) {
            max = i;
        }
    }

    return max;
}

namespace math {

/**
 * @brief quadratic return value of ax^2 + bx + c
 * @param x
 * @param params    params array, param[0] -> a, param[1] -> b, param[2] -> c
 */
inline double quadratic(const double x, const double* params) {
    return params[0] * x * x + params[1] * x + params[2];
}

/**
 * @brief cubic return Cubic function value: ax^3 + bx^2 + cx + d
 * @param x
 * @param params    params array, param[0] -> a, param[1] -> b, param[2] -> c, param[3] -> d
 */
inline double cubic(const double x, const double* params) {
    return params[0] * x * x *x + params[1] * x * x + params[2] * x + params[3];
}

/**
 * @brief quartic return Quartic function value: aX^4 + bX^3 + cX^2 + dX + e
 * @param x
 * @param params    Array len 5, p[0] -> a, p[1] -> b, p[2] -> c, p[3] -> d, p[4] -> e
 */
inline double quartic(const double x, const double* params) {
    return params[0] * x * x * x * x
            + params[1] * x * x * x
            + params[2] * x * x
            + params[3] * x
            + params[4];
}

/**
 * @brief quadraticEquation Solve equation ax^2 + bx + c = 0
 * @return
 */
double quadraticEquation(const double a, const double b, const double c);

/**
 * @brief linear    Compute Linear value of position, in a Linear line of start and end
 */
double linear(const double xStart, const double xEnd, const double yStart, const double yEnd, double position);

/**
 * @brief return determine of matrix 3x3
 * @return det(m)
 */
inline double determine(const nucare::math::Matrix& m) {
    return m.value(0, 0) * m.value(1, 1) * m.value(2, 2)
            + m.value(2, 0) * m.value(0, 1) * m.value(1, 2)
            + m.value(1, 0) * m.value(2, 1) * m.value(0, 2)
            - m.value(0, 2) * m.value(1, 1) * m.value(2, 0)
            - m.value(0, 0) * m.value(1, 2) * m.value(2, 1)
            - m.value(1, 0) * m.value(0, 1) * m.value(2, 2);
}

}

} // namespace nucare

namespace datetime {

QString formatDuration(int totalSeconds);

QString formatDate_yyyyMMdd_HHmm(QDateTime& datetime);

QString formatIsoDate(nucare::Time& datetime);

}

namespace ui {

inline int textWidth(QFontMetrics& metric, QString&& text) {
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
    return metric.horizontalAdvance(text);
#else
    return metric.width(text);
#endif
}

inline int textWidth(QFontMetrics& metric, QString& text) {
    return textWidth(metric, std::move(text));
}

}

#define NC_UTIL_CLEAR_BGR(v) v->setAttribute(Qt::WA_NoSystemBackground)

#endif // UTIL_H
