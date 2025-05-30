#ifndef MYSYSTEMEXCEPTION_H
#define MYSYSTEMEXCEPTION_H

#include <QException>
#include <QString>
#include <QMetaEnum>

namespace nucare {
Q_NAMESPACE

    enum class ErrorCode {
        NoError = 0,
        UnknownError,
        InvalidArgument,
        FileNotFound,
        NetworkError,
        DatabaseError,
        OperationFailed,
        InvalidState
    };
Q_ENUM_NS(ErrorCode)

class NcException : public QException
{
   public:
    explicit NcException(ErrorCode      code    = ErrorCode::UnknownError,
                         const QString &message = "An unknown system error occurred.", const QString &context = "",
                         const QException *cause = nullptr);

    // Constructor để copy (cần thiết cho QException)
    NcException(const NcException &other);

    // Phương thức copy (cần thiết cho QException)
    [[nodiscard]] NcException *clone() const override;

    void raise() const override;

    ErrorCode         code() const { return m_code; }
    QString           message() const { return m_message; }
    QString           context() const { return m_context; }
    const QException *cause() const { return m_cause; }

    QString toString() const;

    // Giúp dễ dàng tạo ngoại lệ với thông tin file/line
    // Sử dụng macro để tự động điền __FILE__ và __LINE__
    static NcException create(ErrorCode code, const QString &message, const char *file, int line);

   private:
    ErrorCode         m_code;
    QString           m_message;
    QString           m_context;
    const QException *m_cause;  // Con trỏ tới ngoại lệ gây ra (không quản lý bộ nhớ)
};

class InvalidArgumentException : public NcException
{
   public:
    explicit InvalidArgumentException(const QString &message = "Invalid argument provided.",
                                      const QString &context = "", const QException *cause = nullptr);

    [[nodiscard]] InvalidArgumentException *clone() const override { return new InvalidArgumentException(*this); }
    void                                    raise() const override { throw *this; }
};

class NetworkException : public NcException
{
   public:
    explicit NetworkException(const QString &message = "Network operation failed.", const QString &context = "",
                              const QException *cause = nullptr);

    [[nodiscard]] NetworkException *clone() const override { return new NetworkException(*this); }
    void                            raise() const override { throw *this; }
};

}  // namespace nucare

#define NC_THROW(code, message) throw nucare::NcException::create(code, message, __FILE__, __LINE__);
#define NC_THROW_ARG_ERROR(message) throw nucare::InvalidArgumentException(message, QString("%1:%2").arg(__FILE__).arg(__LINE__));
#define NC_THROW_NETWORK_ERROR(message) throw nucare::NetworkException(message, QString("%1:%2").arg(__FILE__).arg(__LINE__));


#endif // MYSYSTEMEXCEPTION_H