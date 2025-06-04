#ifndef ACQTIMEDIALOG_H
#define ACQTIMEDIALOG_H

#include "base/BaseDialog.h"
#include "model/MeterState.h"
#include "config.h"

namespace Ui {
class AcqTimeDialog;
}

namespace app::uc::meter {
class SpectrumMeterUsecase;
}

struct AcqTimeArgs : public app_nav::NavigationArgs {
    std::shared_ptr<app::uc::meter::SpectrumMeterUsecase> usecase;
};

class AcqTimePresenter : public BasePresenter {
private:
    int mUpdatingTime;      // The updating time for dialog
    std::shared_ptr<app::uc::meter::SpectrumMeterUsecase> mSpcRepository;
public:
    AcqTimePresenter(std::shared_ptr<app::uc::meter::SpectrumMeterUsecase> spcRepository,
                     std::string tag = app_const::ACQ_TIME_TAG);
    virtual void bind(BaseScreen* v) override;
    void increaseTime();
    void decreaseTime();
    void changeAcqTime();
};

class AcqTimeDialog : public BaseDialog
{
    Q_OBJECT

public:
    explicit AcqTimeDialog(QWidget *parent = nullptr, const QString& tag = tag::ACQ_TIME_TAG);
    ~AcqTimeDialog();
    virtual void createPresenter(app_nav::NavigationArgs* args) override;
    void bindData(app::uc::meter::model::Data& data, int newTime);
    void closeEvent(QCloseEvent* ev) override;
private:
    Ui::AcqTimeDialog *ui;
};

#endif // ACQTIMEDIALOG_H
