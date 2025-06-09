/*
 * BackgroundScreen.cpp
 *
 *  Created on: Nov 11, 2021
 *      Author: quangnam
 */

#include "BgrScreen.h"
#include "config.h"
#include "base/basedialog.h"
#include "component/componentmanager.h"
#include "component/detectorcomponent.h"
#include "component/navigationcomponent.h"
#include "component/SpectrumAccumulator.h"
#include "component/settingmanager.h"
#include "model/DetectorProp.h"
#include "ui_BgrScreen.h"

#include <QDialog>

void navigation::toBackground(NavigationComponent* navController, NavigationEntry* entry, const QString& tag) {
    if (auto w = dynamic_cast<QWidget*>(entry->host)) {
        entry->view = new BackgroundScreen(tag, w);
        entry->type = CHILD_IN_WINDOW;
        entry->childNav = new NavigationComponent(navController);
        navController->enter(entry);
    }
}

navigation::NavigationEntry* navigation::toBackground(BaseView* parent) {
    auto nav = parent->getNavigation();
    auto widget = dynamic_cast<QWidget*>(parent);
    auto ret = new NavigationEntry(CHILD_IN_WINDOW,
                                   new BackgroundScreen(tag::BACKGROUND_TAG, widget),
                                   new NavigationComponent(nav),
                                   widget->parent());
    nav->enter(ret);

    return ret;
}

BackgroundScreen::BackgroundScreen(const QString &tag, QWidget *parent)
    : BaseScreen(tag, parent),
    ui(new Ui::BackgroundScreen()),
    m_counter(nullptr)
{
    ui->setupUi(this);
//    auto centerAct = new ViewAction {
//        .name = translate("TIME"),
//        .action = [this]() {
//            getPresenter<BackgroundPresenter>()->toAcqDialog(getChildNavigation(), this);
//            return true;
//        },
//        .icon = ":/images/common/menu_acq_time.png"
//    };
//    setCenterAction(centerAct);

    setupLayout();
}

BackgroundScreen::~BackgroundScreen()
{
    delete ui;
}

void BackgroundScreen::setupLayout() {
    if (auto detector = ComponentManager::instance().detectorComponent()) {
        ui->chart->setCoefficient(const_cast<Coeffcients*>(&detector->properties()->getCoeffcients()));
    }
}

void BackgroundScreen::onCreate(navigation::NavigationArgs *args)
{
    BaseScreen::onCreate(args);
    if (!m_counter) {
        auto setting = ComponentManager::instance().settingManager();
        auto builder = SpectrumAccumulator::Builder()
                .setParent(this)
                .setTimeoutSeconds(setting->getAcqTimeBgr())
                .setMode(AccumulationMode::ByTime);
        m_counter = builder.build();
        connect(m_counter, &SpectrumAccumulator::accumulationUpdated, this, &BackgroundScreen::onRecvSpectrum);
        connect(m_counter, &SpectrumAccumulator::stateChanged, this, &BackgroundScreen::onRecvBacground);
        m_counter->start();
    }
}

void BackgroundScreen::reloadLocal()
{
    ui->retranslateUi(this);
}

//void BackgroundScreen::bindData(app::uc::meter::model::Data &data)
//{
//    ui->acqCounter->setText(QString("%1 / %2")
//                             .arg(data.acqTime, 2, 10, QChar('0'))
//                             .arg(data.goalTime));
//    ui->count->setText(QString("%1K").arg(data.spc->getTotalCount() / 1000, 3, 'f', 1));
//    ui->cps->setText(QString::number((int) data.cps));
//    ui->chart->setData(data.spc);
//}

void BackgroundScreen::showConfirmDlg()
{
    auto entry = BaseScreen::showInfo("Background Saved.");
    auto action = [&]() { getNavigation()->pop(this, false); };

    connect(dynamic_cast<QDialog*>(entry->view), &QDialog::accepted, this, action);
    connect(dynamic_cast<QDialog*>(entry->view), &QDialog::rejected, this, action);
}

void BackgroundScreen::showGammaWarning()
{
//    auto entry = ""
//    auto action = [&]() { getNavigation()->pop(this, false); };
//    connect(view, &QDialog::accepted, this, action);
    //    connect(view, &QDialog::rejected, this, action);
}

void BackgroundScreen::onRecvSpectrum()
{
    if (!m_counter) return;
    if (m_counter->getCurrentState() == AccumulatorState::Measuring) {
        auto& ret = m_counter->getCurrentResult();
        ui->acqCounter->setText(QString("%1 / %2")
                                .arg(ret.count, 2, 10, QChar('0'))
                                .arg(m_counter->getAcqTime()));
        ui->count->setText(QString("%1K").arg(ret.spectrum->getTotalCount() / 1000, 3, 'f', 1));
        ui->cps->setText(QString::number((int) ret.cps));
        ui->chart->setData(ret.spectrum);
    }
}

void BackgroundScreen::onRecvBacground()
{
    if (!m_counter || m_counter->getCurrentState() != AccumulatorState::Completed) {
        return;
    }

    auto dbMgr = ComponentManager::instance().databaseManager();
    auto bgr = std::make_shared<Background>();
    auto date = QDateTime::currentDateTime();
    bgr->date = datetime::formatIsoDate(date);
    bgr->spc = m_counter->getCurrentResult().spectrum;
    bgr->id = dbMgr->insertBackground(bgr.get());

    if (bgr->id > 0) {
        showConfirmDlg();
    }
}
