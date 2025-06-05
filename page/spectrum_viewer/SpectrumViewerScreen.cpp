/*
 * SpectrumViewerScreen.cpp
 *
 *  Created on: Nov 11, 2021
 *      Author: quangnam
 */

#include "SpectrumViewerScreen.h"
#include "config.h"
#include "base/basedialog.h"
#include "component/componentmanager.h"
#include "component/detectorcomponent.h"
#include "component/navigationcomponent.h"
#include "component/SpectrumAccumulator.h"
#include "component/ncmanager.h"
#include "model/DetectorProp.h"
#include "ui_SpectrumViewerScreen.h"

#include <QDialog>

void navigation::toSpectrumViewer(NavigationComponent* navController, NavigationEntry* entry,
                                  std::shared_ptr<SpectrumAccumulator> accumulator,
                                  const QString& tag) {
    if (auto w = dynamic_cast<QWidget*>(entry->host)) {
        auto view = new SpectrumViewerScreen(tag, w);
        entry->view = view;
        entry->type = CHILD_IN_WINDOW;
        entry->childNav = new NavigationComponent(navController);
        view->setAccumulator(accumulator);
        navController->enter(entry);
    }
}

navigation::NavigationEntry* navigation::toSpectrumViewer(BaseView* parent, std::shared_ptr<SpectrumAccumulator> accumulator) {
    auto nav = parent->getNavigation();
    auto widget = dynamic_cast<QWidget*>(parent);
    auto view = new SpectrumViewerScreen(tag::SPECTRUMVIEW_TAG, widget);
    auto ret = new NavigationEntry(CHILD_IN_WINDOW,
                                   view,
                                   new NavigationComponent(nav),
                                   widget->parent());

    view->setAccumulator(accumulator);
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
    BaseScreen::onCreate(args);
    if (m_counter) {
        connect(m_counter.get(), &SpectrumAccumulator::accumulationUpdated, this, &SpectrumViewerScreen::onRecvSpectrum);
        connect(m_counter.get(), &SpectrumAccumulator::stateChanged, this, &SpectrumViewerScreen::updateState);
//        m_counter->start();
        updateState();
    } else {
        NC_THROW_ARG_ERROR("SpectrumViewer: missing argument, Accumulator is invalid");
    }
}

void SpectrumViewerScreen::reloadLocal()
{
    ui->retranslateUi(this);
}

void SpectrumViewerScreen::setAccumulator(std::shared_ptr<SpectrumAccumulator> accumulator)
{
    m_counter = accumulator;
}

void SpectrumViewerScreen::setSpectrum(std::shared_ptr<Spectrum> spc)
{
    if (spc)
        ui->chart->setData(spc);
}

void SpectrumViewerScreen::onRecvSpectrum()
{
    if (!m_counter) return;

    int cps = 0;
    int totalCount = 0;
    std::shared_ptr<Spectrum> spc;

    if (m_counter->getCurrentState() == AccumulatorState::Measuring) {
        auto& ret = m_counter->getCurrentResult();
        cps = ret.cps;
        spc = ret.spectrum;
        totalCount = ret.spectrum->getTotalCount();

        ui->acqCounter->setText(QString("%1 / %2")
                                .arg(ret.count, 2, 10, QChar('0'))
                                .arg(m_counter->getAcqTime()));

        ui->timeContainer->show();
    } else {
        auto prop = ComponentManager::instance().detectorComponent()->properties();
        cps = prop->getCps();
        spc = prop->getCurrentSpectrum();
        totalCount = spc ? spc->getTotalCount() : 0;
        ui->timeContainer->hide();
    }

    ui->count->setText(QString("%1K").arg(totalCount / 1000.0, 3, 'f', 1));
    ui->cps->setText(QString::number(cps));
    setSpectrum(spc);
}

void SpectrumViewerScreen::updateState()
{
    auto state = m_counter->getCurrentState();
    if (state == AccumulatorState::Measuring) {
        if (m_frameConn) {
            disconnect(m_frameConn);
        }
    } else if (!m_frameConn) {
        auto ncManager = ComponentManager::instance().ncManager();
        m_frameConn = connect(ncManager.get(), &NcManager::spectrumReceived, this, &SpectrumViewerScreen::onRecvSpectrum);
    }
}
