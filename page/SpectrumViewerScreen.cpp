/*
 * SpectrumViewerScreen.cpp
 *
 *  Created on: Nov 11, 2021
 *      Author: quangnam
 */

#include "page/SpectrumViewerScreen.h"
#include "config.h"
#include "base/basedialog.h"
#include "component/componentmanager.h"
#include "component/detectorcomponent.h"
#include "component/navigationcomponent.h"
#include "component/SpectrumAccumulator.h"
#include "model/DetectorProp.h"
#include "ui_SpectrumViewerScreen.h"

#include <QDialog>

void navigation::toSpectrumViewer(NavigationComponent* navController, NavigationEntry* entry, const QString& tag) {
    if (auto w = dynamic_cast<QWidget*>(entry->host)) {
        entry->view = new SpectrumViewerScreen(tag, w);
        entry->type = CHILD_IN_WINDOW;
        entry->childNav = new NavigationComponent(navController);
        navController->enter(entry);
    }
}

navigation::NavigationEntry* navigation::toSpectrumViewer(BaseView* parent) {
    auto nav = parent->getNavigation();
    auto widget = dynamic_cast<QWidget*>(parent);
    auto ret = new NavigationEntry(CHILD_IN_WINDOW,
                                   new SpectrumViewerScreen(tag::BACKGROUND_TAG, widget),
                                   new NavigationComponent(nav),
                                   widget->parent());
    nav->enter(ret);

    return ret;
}

SpectrumViewerScreen::SpectrumViewerScreen(const QString &tag, QWidget *parent)
    : BaseScreen(tag, parent),
    ui(new Ui::SpectrumViewerScreen()),
    m_counter(nullptr)
{
    ui->setupUi(this);
//    auto centerAct = new ViewAction {
//        .name = translate("TIME"),
//        .action = [this]() {
//            getPresenter<SpectrumViewerPresenter>()->toAcqDialog(getChildNavigation(), this);
//            return true;
//        },
//        .icon = ":/images/common/menu_acq_time.png"
//    };
//    setCenterAction(centerAct);

    setupLayout();
}

SpectrumViewerScreen::~SpectrumViewerScreen()
{
    delete ui;
}

void SpectrumViewerScreen::setupLayout() {
    if (auto detector = ComponentManager::instance().detectorComponent()) {
        ui->chart->setCoefficient(const_cast<Coeffcients*>(&detector->properties()->getCoeffcients()));
    }
}

void SpectrumViewerScreen::onCreate(navigation::NavigationArgs *args)
{
    if (!m_counter) {
        auto builder = SpectrumAccumulator::Builder()
                .setParent(this)
                .setTimeoutSeconds(120)
                .setMode(SpectrumAccumulator::AccumulationMode::ByTime);
        m_counter = builder.build();
        connect(m_counter, &SpectrumAccumulator::accumulationUpdated, this, &SpectrumViewerScreen::onRecvSpectrum);
        connect(m_counter, &SpectrumAccumulator::stateChanged, this, &SpectrumViewerScreen::onRecvBacground);
        m_counter->start();
    }
}

void SpectrumViewerScreen::reloadLocal()
{
    ui->retranslateUi(this);
}

//void SpectrumViewerScreen::bindData(app::uc::meter::model::Data &data)
//{
//    ui->acqCounter->setText(QString("%1 / %2")
//                             .arg(data.acqTime, 2, 10, QChar('0'))
//                             .arg(data.goalTime));
//    ui->count->setText(QString("%1K").arg(data.spc->getTotalCount() / 1000, 3, 'f', 1));
//    ui->cps->setText(QString::number((int) data.cps));
//    ui->chart->setData(data.spc);
//}

void SpectrumViewerScreen::showConfirmDlg()
{
    auto entry = BaseScreen::showInfo("Background Saved.");
    auto action = [&]() { getNavigation()->pop(this, false); };

    connect(dynamic_cast<QDialog*>(entry->view), &QDialog::accepted, this, action);
    connect(dynamic_cast<QDialog*>(entry->view), &QDialog::rejected, this, action);
}

void SpectrumViewerScreen::showGammaWarning()
{
//    auto entry = ""
//    auto action = [&]() { getNavigation()->pop(this, false); };
//    connect(view, &QDialog::accepted, this, action);
    //    connect(view, &QDialog::rejected, this, action);
}

void SpectrumViewerScreen::onRecvSpectrum()
{
    if (!m_counter) return;
    if (m_counter->getCurrentState() == AccumulatorState::Measuring) {
        auto& ret = m_counter->getCurrentAccumulationResult();
        ui->acqCounter->setText(QString("%1 / %2")
                                .arg(ret.count, 2, 10, QChar('0'))
                                .arg(m_counter->getAcqTime()));
        ui->count->setText(QString("%1K").arg(ret.spectrum->getTotalCount() / 1000, 3, 'f', 1));
        ui->cps->setText(QString::number((int) ret.cps));
        ui->chart->setData(ret.spectrum);
    }
}

void SpectrumViewerScreen::onRecvBacground()
{
    if (!m_counter || m_counter->getCurrentState() != AccumulatorState::Completed) {
        return;
    }

    nucare::logD() << "Background Saved." << m_counter->getCurrentAccumulationResult().cps;
}
