/*
 * CalibrationScreen.cpp
 *
 *  Created on: Nov 11, 2021
 *      Author: quangnam
 */

#include "page/CalibrationScreen.h"
#include "config.h"
#include "base/basedialog.h"
#include "component/componentmanager.h"
#include "component/detectorcomponent.h"
#include "component/navigationcomponent.h"
#include "component/SpectrumAccumulator.h"
#include "model/DetectorProp.h"
#include "ui_CalibrationScreen.h"

#include <QDialog>

void navigation::toCalibration(NavigationComponent* navController, NavigationEntry* entry, const QString& tag) {
    if (auto w = dynamic_cast<QWidget*>(entry->host)) {
        entry->view = new CalibrationScreen(tag, w);
        entry->type = CHILD_IN_WINDOW;
        entry->childNav = new NavigationComponent(navController);
        navController->enter(entry);
    }
}

navigation::NavigationEntry* navigation::toCalibration(BaseView* parent) {
    auto nav = parent->getNavigation();
    auto widget = dynamic_cast<QWidget*>(parent);
    auto ret = new NavigationEntry(CHILD_IN_WINDOW,
                                   new CalibrationScreen(tag::BACKGROUND_TAG, widget),
                                   new NavigationComponent(nav),
                                   widget->parent());
    nav->enter(ret);

    return ret;
}

CalibrationScreen::CalibrationScreen(const QString &tag, QWidget *parent)
    : BaseScreen(tag, parent),
    ui(new Ui::CalibrationScreen()),
    m_counter(nullptr)
{
    ui->setupUi(this);
//    auto centerAct = new ViewAction {
//        .name = translate("TIME"),
//        .action = [this]() {
//            getPresenter<CalibrationPresenter>()->toAcqDialog(getChildNavigation(), this);
//            return true;
//        },
//        .icon = ":/images/common/menu_acq_time.png"
//    };
//    setCenterAction(centerAct);

    setupLayout();
}

CalibrationScreen::~CalibrationScreen()
{
    delete ui;
}

void CalibrationScreen::setupLayout() {
    if (auto detector = ComponentManager::instance().detectorComponent()) {
        ui->chart->setCoefficient(const_cast<Coeffcients*>(&detector->properties()->getCoeffcients()));
    }
}

void CalibrationScreen::onCreate(navigation::NavigationArgs *args)
{
    if (!m_counter) {
        auto builder = SpectrumAccumulator::Builder()
                .setParent(this)
                .setTimeoutSeconds(120)
                .setMode(SpectrumAccumulator::AccumulationMode::ByCount); // Changed mode to ByCount
        m_counter = builder.build();
        connect(m_counter, &SpectrumAccumulator::accumulationUpdated, this, &CalibrationScreen::onRecvSpectrum);
        connect(m_counter, &SpectrumAccumulator::stateChanged, this, &CalibrationScreen::onRecvBacground);
        m_counter->start();
    }
}

void CalibrationScreen::reloadLocal()
{
    ui->retranslateUi(this);
}

//void CalibrationScreen::bindData(app::uc::meter::model::Data &data)
//{
//    ui->acqCounter->setText(QString("%1 / %2")
//                             .arg(data.acqTime, 2, 10, QChar('0'))
//                             .arg(data.goalTime));
//    ui->count->setText(QString("%1K").arg(data.spc->getTotalCount() / 1000, 3, 'f', 1));
//    ui->cps->setText(QString::number((int) data.cps));
//    ui->chart->setData(data.spc);
//}

void CalibrationScreen::showConfirmDlg()
{
    auto entry = BaseScreen::showInfo("Background Saved.");
    auto action = [&]() { getNavigation()->pop(this, false); };

    connect(dynamic_cast<QDialog*>(entry->view), &QDialog::accepted, this, action);
    connect(dynamic_cast<QDialog*>(entry->view), &QDialog::rejected, this, action);
}

void CalibrationScreen::showGammaWarning()
{
//    auto entry = ""
//    auto action = [&]() { getNavigation()->pop(this, false); };
//    connect(view, &QDialog::accepted, this, action);
    //    connect(view, &QDialog::rejected, this, action);
}

void CalibrationScreen::onRecvSpectrum()
{
    if (!m_counter) return;
    if (m_counter->getCurrentState() == AccumulatorState::Measuring) {
        auto& ret = m_counter->getCurrentAccumulationResult();
        ui->acqCounter->setText(QString("%1 / %2")
                                .arg(ret.count, 2, 10, QChar('0'))
                                .arg(m_counter->getAcqTime()));
        ui->count->setText(QString("%1K").arg(ret.spectrum->getTotalCount() / 1000, 3, 'f', 1));
        ui->cps->setText(QString::number((int) ret.cps));
        ui->chart->setData(ret.spectrum); // Assuming HwSpectrumView has a setData method similar to SpectrumView
    }
}

void CalibrationScreen::onRecvBacground()
{
    if (!m_counter || m_counter->getCurrentState() != AccumulatorState::Completed) {
        return;
    }

    nucare::logD() << "Background Saved." << m_counter->getCurrentAccumulationResult().cps;
}
