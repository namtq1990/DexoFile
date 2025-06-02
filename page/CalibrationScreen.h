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

    void showConfirmDlg();
    void showGammaWarning();

public slots:
    void onRecvSpectrum();
    void onRecvBacground();
};

#endif // CALIBRATIONSCREEN_H
