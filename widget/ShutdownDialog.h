#ifndef SHUTDOWNDIALOG_H
#define SHUTDOWNDIALOG_H

#include "base/basedialog.h"

class ShutdownDialog : public BaseDialog
{
public:
    ShutdownDialog(QWidget* parent = nullptr, const QString& tag = tag::SHUTDOWN_DLG);
};

#endif // SHUTDOWNDIALOG_H
