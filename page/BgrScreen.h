#ifndef BGRSCREEN_H
#define BGRSCREEN_H

#include "Model/MeterState.h"
#include "Base/BaseMVP.h"
#include "Common/Constants.h"
#include <QWidget>

namespace Ui {
class BackgroundScreen;
}

class BackgroundScreen;

namespace app::uc::meter { class SpectrumMeterUsecase; }
namespace nucare { class NcManager; }

class BackgroundPresenter : public BasePresenter {
private:
    nucare::NcManager* mNcManager;
    std::shared_ptr<app::uc::meter::SpectrumMeterUsecase> mSpcRepository;
public:
    BackgroundPresenter();
    ~BackgroundPresenter();
    virtual void bind(BaseScreen* view) override;

    void checkResult();
    void saveBackground();

    void toAcqDialog(app_nav::NavigationComponent* nav, QObject* curScreen);

    friend class BackgroundScreen;
};

class BackgroundScreen : public QWidget, public BaseScreen {
    Q_OBJECT
private:
    Ui::BackgroundScreen* ui;
public:
    explicit BackgroundScreen(const std::string& tag = app_const::BACKGROUND_TAG, QWidget *parent = nullptr);
    ~BackgroundScreen();
    void createPresenter(app_nav::NavigationArgs* args) override;
    void setupLayout();

    virtual void bindData(app::uc::meter::model::Data& data);
    virtual void onHandleAlarm(Alarm& alarm) override;
    bool enableGainStab() override { return false; }


    void showConfirmDlg();
    void showGammaWarning();

    BASEVIEW
};

#endif // BGRSCREEN_H
