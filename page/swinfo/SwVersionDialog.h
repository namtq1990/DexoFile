#ifndef SWVERSIONDIALOG_H
#define SWVERSIONDIALOG_H

#include "base/basedialog.h"

namespace Ui { class SwVersionDialog; }

struct VersionInfo {
    QString version;
//    std::string user;
//    std::string location;
    QString device;
    QString detectorType;
};

class SwVersionDialog : public BaseDialog
{
    Q_OBJECT
private:
    Ui::SwVersionDialog* ui;
    VersionInfo m_info;
public:
    SwVersionDialog(const QString&& tag = tag::SW_VERSION, QWidget* parent = NULL);
    ~SwVersionDialog();

    void dataChanged();
};

#endif // SWVERSIONDIALOG_H
