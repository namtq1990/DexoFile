#ifndef ACQTIMEDIALOG_H
#define ACQTIMEDIALOG_H

#include "base/basedialog.h"
#include "config.h"
#include <QPointer>

namespace Ui {
class AcqTimeDialog;
}

namespace setting { class SettingManager; }
class SpectrumAccumulator;

struct AcqTimeArgs : public navigation::NavigationArgs {
    SpectrumAccumulator* accumulator;
};

class AcqTimeDialog : public BaseDialog
{
    Q_OBJECT

public:
    explicit AcqTimeDialog(QWidget *parent = nullptr, const QString& tag = tag::ACQ_TIME_TAG);
    ~AcqTimeDialog();
    void onCreate(navigation::NavigationArgs* args) override;
    void reloadLocal() override;


public slots:
    void increaseTime();
    void decreaseTime();
    void updateTime();
    void dataChanged();
private:
    Ui::AcqTimeDialog *ui;
    int mUpdatingTime;      // The updating time for dialog
    SpectrumAccumulator* mAccumulator;
    QPointer<setting::SettingManager> mSetting;
};

#endif // ACQTIMEDIALOG_H
