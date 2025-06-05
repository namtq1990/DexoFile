#ifndef BGRSCREEN_H
#define BGRSCREEN_H

#include "base/basescreen.h"
#include "config.h"

namespace Ui {
class BackgroundScreen;
}

class SpectrumAccumulator;

class BackgroundScreen : public BaseScreen {
    Q_OBJECT
private:
    Ui::BackgroundScreen* ui;
    SpectrumAccumulator* m_counter;
public:
    explicit BackgroundScreen(const QString& tag = tag::BACKGROUND_TAG, QWidget *parent = nullptr);
    ~BackgroundScreen();
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

#endif // BGRSCREEN_H
