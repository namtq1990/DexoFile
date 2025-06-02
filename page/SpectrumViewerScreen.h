#ifndef SPECTRUMVIEWERSCREEN_H
#define SPECTRUMVIEWERSCREEN_H

#include "base/basescreen.h"
#include "config.h"

namespace Ui {
class SpectrumViewerScreen;
}

class SpectrumAccumulator;

class SpectrumViewerScreen : public BaseScreen {
    Q_OBJECT
private:
    Ui::SpectrumViewerScreen* ui;
    SpectrumAccumulator* m_counter;
public:
    explicit SpectrumViewerScreen(const QString& tag = tag::BACKGROUND_TAG, QWidget *parent = nullptr);
    ~SpectrumViewerScreen();
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

#endif // SPECTRUMVIEWERSCREEN_H
