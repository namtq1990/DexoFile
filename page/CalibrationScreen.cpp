/*
 * CalibrationScreen.cpp
 *
 *  Created on: Nov 11, 2021
 *      Author: quangnam
 */

#include "page/CalibrationScreen.h"
#include "page/CalibAcqDialog.h"
#include "config.h"
#include "base/basedialog.h"
#include "component/componentmanager.h"
#include "component/detectorcomponent.h"
#include "component/navigationcomponent.h"
#include "component/SpectrumAccumulator.h"
#include "component/ncmanager.h"
#include "component/settingmanager.h"
#include "model/DetectorProp.h"
#include "ui_CalibrationScreen.h"

#include <QDialog>

void navigation::toCalibration(NavigationComponent* navController, NavigationEntry* entry, Calibration::Mode mode, bool updateStdPeak, const QString& tag) {
    if (auto w = dynamic_cast<QWidget*>(entry->host)) {
        auto view = new CalibrationScreen(tag, w);
        view->m_mode = mode;
        view->m_updateStdPeak = updateStdPeak;

        entry->view = view;
        entry->type = CHILD_IN_WINDOW;
        entry->childNav = new NavigationComponent(navController);
        navController->enter(entry);
    }
}

navigation::NavigationEntry* navigation::toCalibration(BaseView* parent, Calibration::Mode mode, bool updateStdPeak) {
    auto nav = parent->getNavigation();
    auto widget = dynamic_cast<QWidget*>(parent);
    auto view = new CalibrationScreen(tag::CALIBRATION_TAG, widget);
    auto ret = new NavigationEntry(CHILD_IN_WINDOW,
                                   view,
                                   new NavigationComponent(nav),
                                   widget->parent());
    view->m_mode = mode;
    view->m_updateStdPeak = updateStdPeak;

    nav->enter(ret);

    return ret;
}

CalibrationScreen::CalibrationScreen(const QString &tag, QWidget *parent)
    : BaseScreen(tag, parent),
    ui(new Ui::CalibrationScreen()),
    m_counter(nullptr)
{
    ui->setupUi(this);
    auto centerAct = new ViewAction (
                "TIME", [this]() {
        auto entry = navigation::toCalibCountDlg(this);
        auto dlg = dynamic_cast<CalibAcqDialog*>(entry->view);
        connect(dlg, &ChoicesDialog::accepted, this, [this, dlg]() {
            auto acqCount = dlg->curChoice().toInt();
            onAdjustCount(acqCount);
        });
//            getPresenter<CalibrationPresenter>()->toAcqDialog(getChildNavigation(), this);
            return true;
        }, ":/icons/menu_acq_time"
    );
    setCenterAction(centerAct);

    setupLayout();
}

CalibrationScreen::~CalibrationScreen()
{
    delete ui;
}

void CalibrationScreen::setupLayout() {
//    if (auto detector = ComponentManager::instance().detectorComponent()) {
//        ui->chart->setCoefficient(const_cast<Coeffcients*>(&detector->properties()->getCoeffcients()));
//    }
}

void CalibrationScreen::onCreate(navigation::NavigationArgs *args)
{
    BaseScreen::onCreate(args);
    if (!m_counter) {
        auto settingMgr = ComponentManager::instance().settingManager();

        auto builder = SpectrumAccumulator::Builder()
                .setParent(this)
                .setTargetCount(settingMgr->getCalibCount())
                .setMode(SpectrumAccumulator::AccumulationMode::ByCount); // Changed mode to ByCount
        m_counter = builder.build();
        connect(m_counter, &SpectrumAccumulator::accumulationUpdated, this, &CalibrationScreen::onRecvSpectrum);
        connect(m_counter, &SpectrumAccumulator::stateChanged, this, &CalibrationScreen::onRecvResult);
        m_counter->start();
    }
}

void CalibrationScreen::reloadLocal()
{
    ui->retranslateUi(this);
}

navigation::NavigationEntry *CalibrationScreen::showInfo(const char *msg)
{
    auto entry = BaseScreen::showInfo(msg);
    if (auto dlg = dynamic_cast<QDialog*>(entry->view)) {

        auto action = [&]() { getNavigation()->pop(this, false); };
        connect(dlg, &QDialog::accepted, this, action);
        connect(dlg, &QDialog::rejected, this, action);
    }

    return entry;
}

navigation::NavigationEntry *CalibrationScreen::showError(const char *msg)
{
    return showInfo(msg);
}

void CalibrationScreen::onRecvSpectrum()
{
    if (!m_counter) return;
    if (m_counter->getCurrentState() == AccumulatorState::Measuring) {
        auto& ret = m_counter->getCurrentAccumulationResult();
        ui->acqCounter->setText(QString("%1 / %2")
                                .arg(ret.count, 2, 10, QChar('0'))
                                .arg(m_counter->getAcqTime()));
        ui->count->setText(QString("%1K").arg(ret.hwSpectrum->getTotalCount() / 1000, 3, 'f', 1));
        ui->cps->setText(QString::number((int) ret.cps));
        ui->chart->setData(ret.hwSpectrum); // Assuming HwSpectrumView has a setData method similar to SpectrumView
    }
}

void CalibrationScreen::onRecvResult()
{
    if (!m_counter || m_counter->getCurrentState() != AccumulatorState::Completed) {
        return;
    }

    if (auto ncManager = ComponentManager::instance().ncManager()) {
        try {
            auto& ret = m_counter->getCurrentAccumulationResult();

            ncManager->computeCalibration(ncManager->getCurrentDetector(), ret.spectrum, ret.hwSpectrum,
                                          m_mode, m_updateStdPeak);
            showInfo("Calibration Saved");
        } catch (const nucare::NcException& e) {
            nucare::logE() << "Failed to calibration: " << e.toString();
            showError(e.message().toLatin1());
        }
    }
}

void CalibrationScreen::onAdjustCount(int count)
{
    if (m_counter) {
        m_counter->setTargetCount(count);
    }
}
