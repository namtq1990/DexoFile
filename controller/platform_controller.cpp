#include "controller/platform_controller.h"
#include <QProcess>
#include <QtConcurrent/QtConcurrent>
#include "util/util.h"
#include <algorithm>

PlatformController::PlatformController(QObject *parent, const QString& tag) 
    : Component(tag), QObject(parent)
{
}

int PlatformController::executeShellCommand(const QString &command, StringCallback callback)
{
    QProcess* process = new QProcess(this);
    int commandId = ++m_commandId;
    
    m_processes[commandId] = {process, true};
    m_stringCallbacks[commandId] = callback;

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &PlatformController::onProcessFinished);
    connect(process, &QProcess::errorOccurred,
            this, &PlatformController::onProcessErrorOccurred);

    nucare::logI() << "Executing command: " << command;
    process->start(command);
    process->waitForStarted();

    return commandId;
}

int PlatformController::executeShellCommand(const QString &command, IntCallback callback)
{
    QProcess* process = new QProcess(this);
    int commandId = ++m_commandId;
    
    m_processes[commandId] = {process, false};
    m_intCallbacks[commandId] = callback;

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &PlatformController::onProcessFinished);
    connect(process, &QProcess::errorOccurred,
            this, &PlatformController::onProcessErrorOccurred);

    nucare::logI() << "Executing command: " << command;
    process->start(command);
    process->waitForStarted();

    return commandId;
}

bool PlatformController::isCommandFinished(int commandId)
{
    return !m_processes.contains(commandId);
}

void PlatformController::cancelCommand(int commandId)
{
    if (m_processes.contains(commandId)) {
        m_processes[commandId].process->kill();
        m_processes[commandId].process->deleteLater();
        m_processes.remove(commandId);
        m_stringCallbacks.remove(commandId);
        m_intCallbacks.remove(commandId);
    }
}

void PlatformController::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    auto it = m_processes.begin();
    while (it != m_processes.end()) {
        if (it.value().process == process) {
            if (it.value().isStringCallback && m_stringCallbacks.contains(it.key())) {
                QString output = process->readAllStandardOutput();
                m_stringCallbacks[it.key()](output, exitCode);
            } else if (m_intCallbacks.contains(it.key())) {
                m_intCallbacks[it.key()](exitCode, exitCode);
            }
            
            process->deleteLater();
            m_processes.erase(it);
            m_stringCallbacks.remove(it.key());
            m_intCallbacks.remove(it.key());
            break;
        }
        ++it;
    }
}

void PlatformController::onProcessErrorOccurred(QProcess::ProcessError error)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    nucare::logE() << "Command failed: " << process->errorString();

    auto it = m_processes.begin();
    while (it != m_processes.end()) {
        if (it.value().process == process) {
            if (it.value().isStringCallback && m_stringCallbacks.contains(it.key())) {
                QString output = process->readAllStandardOutput();
                m_stringCallbacks[it.key()](output, -1);
            } else if (m_intCallbacks.contains(it.key())) {
                m_intCallbacks[it.key()](-1, -1);
            }
            
            process->deleteLater();
            m_processes.erase(it);
            m_stringCallbacks.remove(it.key());
            m_intCallbacks.remove(it.key());
            break;
        }
        ++it;
    }
}

QFuture<QString> PlatformController::executeShellCommandAsync(const QString &command)
{
    QFutureInterface<QString> interface;
    interface.reportStarted();

    QProcess *process = new QProcess;

    QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus) mutable {
            if (exitCode == 0) {
                QString output = QString::fromUtf8(process->readAllStandardOutput());
                interface.reportResult(output);
            } else {
                interface.reportException(CommandException("Command failed with exit code " + QString::number(exitCode)));
            }
            interface.reportFinished();
            process->deleteLater();
        });

    QObject::connect(process, &QProcess::errorOccurred,
        [=](QProcess::ProcessError error) mutable {
            interface.reportException(CommandException("Process error occurred: " + QString::number(error)));
            interface.reportFinished();
            process->deleteLater();
        });

    logD() << "Running command: " << command;

    process->start("sh", QStringList() << "-c" << command);
    if (process->error() != QProcess::UnknownError) {
        interface.reportException(CommandException("Failed to start process"));
        interface.reportFinished();
        process->deleteLater();
    }

    return interface.future();
}

QFuture<int> PlatformController::executeShellCommandIntAsync(const QString &command)
{
    QFutureInterface<int> interface;
    interface.reportStarted();

    QProcess *process = new QProcess;

    QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus) mutable {
            interface.reportResult(exitCode);
            interface.reportFinished();
            process->deleteLater();
        });

    QObject::connect(process, &QProcess::errorOccurred,
        [=](QProcess::ProcessError error) mutable {
            interface.reportException(CommandException(QString("Process error: ") + error));
            interface.reportFinished();
            process->deleteLater();
        });

    process->start("sh", QStringList() << "-c" << command);
    if (process->error() != QProcess::UnknownError) {
        interface.reportException(CommandException("Failed to start process"));
        interface.reportFinished();
        process->deleteLater();
    }

    return interface.future();
}
