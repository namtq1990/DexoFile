/*
 * BackgroundScreen.cpp
 *
 *  Created on: Nov 11, 2021
 *      Author: quangnam
 */

#include "Feature/Background/BgrScreen.h"
#include "config.h"
#include "Nucare/NcManager.h"
#include "Base/BaseDialog.h"
#include "Base/ResourceManager.h"
#include "Base/Navigation.h"
#include "ui_BgrScreen.h"

#include <QPushButton>

BackgroundScreen::BackgroundScreen(const std::string &tag, QWidget *parent)
    : QWidget(parent),
    BaseScreen(std::move(tag)),
    ui(new Ui::BackgroundScreen())
{
    ui->setupUi(this);
    auto centerAct = new ViewAction {
        .name = translate("TIME"),
        .action = [this]() {
            getPresenter<BackgroundPresenter>()->toAcqDialog(getChildNavigation(), this);
            return true;
        },
        .icon = ":/images/common/menu_acq_time.png"
    };
    setCenterAction(centerAct);
}

BackgroundScreen::~BackgroundScreen()
{
    delete ui;
}

void BackgroundScreen::setupLayout() {
    if (auto detector = sdi::getComponent<nucare::NcManager>()->getCurDetector()) {
        ui->chart->setCoefficient(&detector->mProp->getCoeffcients());
    }

    auto res = getResouce();
    auto theme = res->themeMgr()->getTheme();
    ui->chart->setFont(theme->small());

#ifdef PLATFORM_EMBEDDED
    QFont font = theme->normal();
#elif defined PLATFORM_DESKTOP
    QFont font = theme->large();
#endif
    ui->cps->setFont(font);
    ui->tvCps->setFont(font);
    ui->tvCount->setFont(font);
    ui->count->setFont(font);
    ui->acqTime->setFont(font);
    ui->acqSec->setFont(font);
    ui->acqCounter->setFont(font);
}

void BackgroundScreen::createPresenter(app_nav::NavigationArgs *)
{
    mPresenter = new BackgroundPresenter();
}

void BackgroundScreen::onCreate(app_nav::NavigationArgs *entry) {
    BaseScreen::onCreate(entry);
    setupLayout();
}

void BackgroundScreen::bindData(app::uc::meter::model::Data &data)
{
    ui->acqCounter->setText(QString("%1 / %2")
                             .arg(data.acqTime, 2, 10, QChar('0'))
                             .arg(data.goalTime));
    ui->count->setText(QString("%1K").arg(data.spc->getTotalCount() / 1000, 3, 'f', 1));
    ui->cps->setText(QString::number((int) data.cps));
    ui->chart->setData(data.spc);
}

void BackgroundScreen::onHandleAlarm(Alarm &alarm)
{
    alarm.sound = nullptr;
    alarm.vibration = false;
}

void BackgroundScreen::showConfirmDlg()
{
    auto view = new BaseDialog(app_const::WARNING_DLG_TAG, this);
    auto entry = new app_nav::NavigationEntry();
    entry->setHost(this);
    entry->view = view;
    app_nav::showWarning(getNavigation(), entry, "BGR_Saved");


    auto action = [&]() { getNavigation()->pop(this, false); };
    connect(view, &QDialog::accepted, this, action);
    connect(view, &QDialog::rejected, this, action);
}

void BackgroundScreen::showGammaWarning()
{
    auto view = new BaseDialog(app_const::WARNING_DLG_TAG, this);
    auto entry = new app_nav::NavigationEntry();
    entry->setHost(this);
    entry->view = view;
    app_nav::showWarning(getNavigation(), entry, "BGR_Gamma_Warning");
    auto action = [&]() { getNavigation()->pop(this, false); };
    connect(view, &QDialog::accepted, this, action);
    connect(view, &QDialog::rejected, this, action);
}

TRANSLATION(BackgroundScreen, ui)
