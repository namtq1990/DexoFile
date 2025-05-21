#ifndef PLATFORM_CONTROLLER_H
#define PLATFORM_CONTROLLER_H

#include "component/component.h"
#include <QObject>
#include <QString>
#include <functional>
#include <QFuture>
#include <QFutureInterface>
#include <QProcess>
#include <QMap>
#include <QException>
#include <QString>

class CommandException : public QException
{
public:
    explicit CommandException(const QString& message) : m_message(message) {}
    CommandException(const CommandException& other) : QException(other), m_message(other.m_message) {}
    void raise() const override { throw *this; }
    CommandException *clone() const override { return new CommandException(*this); }
    QString message() const { return m_message; }
    
private:
    QString m_message;
};

class PlatformController : public QObject, public Component
{
    Q_OBJECT

public:
    explicit PlatformController(QObject *parent = nullptr, const QString& tag = "PlatformController");

    using StringCallback = std::function<void(const QString&, int)>;
    using IntCallback = std::function<void(int, int)>;

    // Existing callback-based methods
    int executeShellCommand(const QString &command, StringCallback callback);
    int executeShellCommand(const QString &command, IntCallback callback);
    
    // New QFuture-based async methods
    QFuture<QString> executeShellCommandAsync(const QString &command);
    QFuture<int> executeShellCommandIntAsync(const QString &command);
    bool isCommandFinished(int commandId);
    void cancelCommand(int commandId);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(QProcess::ProcessError error);

private:
    struct ProcessContext {
        QProcess* process;
        bool isStringCallback;
    };

    int executeCommandInternal(const QString &command, QProcess* process);

    QMap<int, ProcessContext> m_processes;
    QMap<int, StringCallback> m_stringCallbacks;
    QMap<int, IntCallback> m_intCallbacks;
    int m_commandId = 0;
};

#endif // PLATFORM_CONTROLLER_H
