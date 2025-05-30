#include "nc_exception.h"

namespace nucare {

NcException::NcException(ErrorCode code, const QString &message,
                                     const QString &context, const QException *cause)
    : m_code(code),
      m_message(message),
      m_context(context),
      m_cause(cause) // Không quản lý bộ nhớ của 'cause'
{}

// Copy constructor
NcException::NcException(const NcException &other)
    : QException(other), // Gọi copy constructor của lớp cơ sở
      m_code(other.m_code),
      m_message(other.m_message),
      m_context(other.m_context),
      m_cause(other.m_cause) // Con trỏ 'cause' cũng chỉ sao chép
{}

// Phương thức clone()
NcException *NcException::clone() const {
    return new NcException(*this);
}

// Phương thức raise()
void NcException::raise() const {
    throw *this;
}

// Phương thức toString()
QString NcException::toString() const {
    QMetaEnum metaEnum = QMetaEnum::fromType<ErrorCode>();
    QString errorCodeName = metaEnum.valueToKey(static_cast<int>(m_code));

    QString str = QString("MySystemException: %1 (Code: %2)").arg(m_message).arg(errorCodeName);

    if (!m_context.isEmpty()) {
        str += QString(" Context: %1").arg(m_context);
    }
    if (m_cause) {
        const NcException *sysCause = dynamic_cast<const NcException*>(m_cause);
        if (sysCause) {
            str += QString("\n  Caused by: %1").arg(sysCause->toString().replace("\n", "\n    "));
        } else {
            str += QString("\n  Caused by: %1").arg(m_cause->what());
        }
    }
    return str;
}

// Hàm static để tạo ngoại lệ với thông tin file/line
NcException NcException::create(ErrorCode code,
                                            const QString &message,
                                            const char *file, int line) {
    return NcException(code, message, QString("%1:%2").arg(file).arg(line));
}

// Implementations cho các lớp con
InvalidArgumentException::InvalidArgumentException(const QString &message,
                                                   const QString &context,
                                                   const QException *cause)
    : NcException(ErrorCode::InvalidArgument, message, context, cause) {}

NetworkException::NetworkException(const QString &message,
                                   const QString &context,
                                   const QException *cause)
    : NcException(ErrorCode::NetworkError, message, context, cause) {}

} // namespace MySystem