#ifndef SPECTRUMVIEWERSCREEN_H
#define SPECTRUMVIEWERSCREEN_H

#include "base/basescreen.h"
#include "model/Spectrum.h"
#include "config.h"

namespace Ui {
class SpectrumViewerScreen;
}

class SpectrumAccumulator;

class SpectrumViewerScreen : public BaseScreen {
    Q_OBJECT
private:
    Ui::SpectrumViewerScreen* ui;
    std::shared_ptr<SpectrumAccumulator> m_counter;
    QMetaObject::Connection m_frameConn;
public:
    explicit SpectrumViewerScreen(const QString& tag = tag::BACKGROUND_TAG, QWidget *parent = nullptr);
    ~SpectrumViewerScreen();
    void setupLayout();

    void onCreate(navigation::NavigationArgs* args) override;

//    virtual void bindData(app::uc::meter::model::Data& data);
    void reloadLocal() override;

    void setAccumulator(std::shared_ptr<SpectrumAccumulator> accumulator);
    void setSpectrum(std::shared_ptr<Spectrum> spc);

public slots:
    void onRecvSpectrum();
    void updateState();
};

#endif // SPECTRUMVIEWERSCREEN_H
