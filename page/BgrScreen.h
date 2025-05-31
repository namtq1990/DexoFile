#ifndef BGRSCREEN_H
#define BGRSCREEN_H

#include "base/basescreen.h"
#include "config.h"

namespace Ui {
class BackgroundScreen;
}

class BackgroundScreen : public BaseScreen {
    Q_OBJECT
private:
    Ui::BackgroundScreen* ui;
public:
    explicit BackgroundScreen(const QString& tag = tag::BACKGROUND_TAG, QWidget *parent = nullptr);
    ~BackgroundScreen();
    void setupLayout();

//    virtual void bindData(app::uc::meter::model::Data& data);
    void reloadLocal() override;

    void showConfirmDlg();
    void showGammaWarning();
};

#endif // BGRSCREEN_H
