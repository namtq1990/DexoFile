#ifndef CALIBRATIONSCREEN_H
#define CALIBRATIONSCREEN_H

#include "base/basescreen.h"
#include "config.h"

namespace Ui {
class CalibrationScreen;
}

class SpectrumAccumulator;

class CalibrationScreen : public BaseScreen {
    Q_OBJECT
private:
    Ui::CalibrationScreen* ui;
    SpectrumAccumulator* m_counter;
public:
    explicit CalibrationScreen(const QString& tag = tag::BACKGROUND_TAG, QWidget *parent = nullptr);
    ~CalibrationScreen();
    void setupLayout();

    void onCreate(navigation::NavigationArgs* args) override;

//    virtual void bindData(app::uc::meter::model::Data& data);
    void reloadLocal() override;

public slots:
    void onRecvSpectrum();
    void onRecvResult();
    void onAdjustCount(int count);

private:
    Calibration::Mode m_mode;
    bool m_updateStdPeak;

    friend navigation::NavigationEntry* navigation::toCalibration(BaseView*, Calibration::Mode,
                                     bool updateStdPeak);
    friend void navigation::toCalibration(navigation::NavigationComponent* navController, navigation::NavigationEntry* entry,
                                          Calibration::Mode mode, bool updateStdPeak, const QString& tag);
};

#endif // CALIBRATIONSCREEN_H
