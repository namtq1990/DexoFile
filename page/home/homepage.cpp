#include "homepage.h"
#include "ui_homepage.h"  // Generated UI header
#include "component/settingmanager.h"
#include "component/SpectrumAccumulator.h"
#include "util/util.h"
#include "model/Event.h"
#include "component/databasemanager.h"
#include "component/componentmanager.h"
#include "component/ncmanager.h"
#include "model/DetectorInfo.h"
#include "model/Background.h"
#include "model/Calibration.h"
using namespace navigation;
using namespace setting;

void navigation::toHome(NavigationComponent* navController, NavigationEntry* entry, const QString& tag) {
    auto pWidget = dynamic_cast<QWidget*>(entry->host);
    entry->type  = TAB_IN_WINDOW;
    entry->view  = new HomePage(pWidget);
    navController->enter(entry);
}

HomePage::HomePage(QWidget* parent) : BaseScreen(tag::HOME_TAG, parent), ui(new Ui::HomePage) {
    ui->setupUi(this);

    setLeftAction(new ViewAction(
        "Events",
        [this]() {
            nucare::logD() << "Event action triggered";
            return true;
        },
        nullptr));
    setRightAction(new ViewAction(
        "Settings",
        [this]() {
            navigation::toSetting(
                getNavigation(),
                new navigation::NavigationEntry(navigation::CHILD_IN_WINDOW, nullptr, nullptr, this->parent()),
                setting::buildSettingTree());
            return true;
        },
        nullptr));
    setLongLeftAction(new ViewAction("Spectrum", [this]() {
        navigation::toSpectrumViewer(this, m_accumulator);
        return true;
    }));

    auto settingMgr = ComponentManager::instance().settingManager();
    auto builder = SpectrumAccumulator::Builder()
            .setParent(this)
            .setMode(AccumulationMode::ContinuousByTime)
            .setContinuousInterval(settingMgr->getMeasureInterval())
            .setTimeoutSeconds(settingMgr->getAcqTimeId());
    m_accumulator = std::shared_ptr<SpectrumAccumulator>(builder.build());
    connect(m_accumulator.get(), &SpectrumAccumulator::stateChanged, this, &HomePage::stateChanged);
    connect(m_accumulator.get(), &SpectrumAccumulator::accumulationUpdated, this, &HomePage::updateEvent);

    settingMgr->subscribeKey(SettingManager::KEY_ACQTIME_ID, (QObject*)this, [this](setting::SettingManager* mgr, auto) {
        setMeasureTime(mgr->getAcqTimeId());
    });
    settingMgr->subscribeKey(SettingManager::KEY_MEASURE_INVERVAL, (QObject*)this, [this](SettingManager* mgr, auto) {
        setIntervalTime(mgr->getMeasureInterval());
    });

    stateChanged(getState());
}

HomePage::~HomePage() { delete ui; }

void HomePage::reloadLocal() { ui->retranslateUi(this); }

void HomePage::start() {
    if (m_accumulator->getCurrentState() == AccumulatorState::Idle) {
        m_accumulator->start();
    }
}

void HomePage::stop() {
    if (m_accumulator) {
        m_accumulator->stop();
    }
}

AccumulatorState HomePage::getState()
{
    if (m_accumulator) return m_accumulator->getCurrentState();
    return AccumulatorState::Idle;
}

void HomePage::setIntervalTime(int sec)
{
    if (m_accumulator) {
        m_accumulator->setIntervalTime(sec);
        ui->intervalTimeLabel->setText(QString("Int.time: %1")
                                       .arg(datetime::formatDuration(m_accumulator->getIntervalTime())));
    }
}

void HomePage::setMeasureTime(int sec)
{
    if (m_accumulator) {
        m_accumulator->setTargetTime(sec);
        ui->stopTimeValueLabel->setText(datetime::formatDate_yyyyMMdd_HHmm(m_accumulator->getCurrentResult().finishTime));
    }
}

void HomePage::stateChanged(AccumulatorState state)
{
    if (state == AccumulatorState::Idle) {
        setCenterAction(new ViewAction("Start", [this]() {
            start();
            return true;
        }));
        updateMenu();
    } else {
        setCenterAction(new ViewAction("Stop", [this]() {
            stop();
            return true;
        }));
        updateMenu();
    }

    switch (state) {
    case AccumulatorState::Idle: {
        ui->stateLabel->setText("State: Idle");
        ui->startTimeLabel->hide();
        ui->startTimeValueLabel->hide();
        ui->stopTimeLabel->hide();
        ui->stopTimeValueLabel->hide();
        ui->etTitleLabel->hide();
        ui->etValueLabel->hide();

        ui->measureTimeValueLabel->setText(QString("%1 min").arg(m_accumulator->getAcqTime()));

        break;
    }
    case AccumulatorState::Measuring: {
        ui->stateLabel->setText("State: Measuring");
        ui->startTimeLabel->show();
        ui->startTimeValueLabel->show();
        ui->stopTimeLabel->show();
        ui->stopTimeValueLabel->show();
        ui->etTitleLabel->show();
        ui->etValueLabel->show();
        ui->stopTimeValueLabel->setText(datetime::formatDate_yyyyMMdd_HHmm(m_accumulator->getCurrentResult().finishTime));
        ui->startTimeValueLabel->setText(datetime::formatDate_yyyyMMdd_HHmm(m_accumulator->getCurrentResult().startTime));
        break;
    }
    case AccumulatorState::Waiting: {
        ui->stateLabel->setText("State: Waiting");
        break;
    }
    case AccumulatorState::Completed: {
        ui->stateLabel->setText("State: Waiting");

        auto ret = m_accumulator->getCurrentResult();

        auto dbManager = ComponentManager::instance().databaseManager();
        Event event;

        // Populate Event object
        event.setSoftwareVersion(QApplication::applicationVersion()); // Or a more appropriate version
        event.setDateBegin(datetime::formatIsoDate(ret.startTime));
        event.setDateFinish(datetime::formatIsoDate(ret.finishTime));
        event.setLiveTime(ret.finishTime.secsTo(ret.startTime));
        event.setAvgCps(ret.avgCPS);
        event.setMaxCps(ret.maxCPS);
        event.setMinCps(ret.minCPS);
        event.setAvgGamma_nSv(0); // Default value
        event.setMaxGamma_nSv(0); // Default value
        event.setMinGamma_nSv(0); // Default value
        event.setAvgFillCps(0);   // Default value

        // Use IDs from AccumulationResult
        event.setDetectorId(ret.detectorId);
        event.setBackgroundId(ret.backgroundId);
        event.setCalibrationId(ret.calibrationId);

        if (ret.detectorId == -1) {
            nucare::logW() << "Detector ID is -1 in accumulation result. Event might not be correctly associated.";
        }
        if (ret.backgroundId == -1) {
            nucare::logW() << "Background ID is -1 in accumulation result.";
        }
        if (ret.calibrationId == -1) {
            nucare::logW() << "Calibration ID is -1 in accumulation result.";
        }

        // Placeholder for detailId, will be updated later
        event.setDetailId(0);

        QString spectrumStringData;

        if (ret.spectrum) {
            spectrumStringData = ret.spectrum->toString();
            event.setRealTime(ret.spectrum->getRealTime());
            event.setAvgFillCps(ret.spectrum->getFillCps() / ret.spectrum->getAcqTime());
        }

        event.setE1Energy(0);     // Default value
        event.setE1Branching(0);  // Default value
        event.setE1Netcount(0);   // Default value
        event.setE2Energy(0);     // Default value
        event.setE2Branching(0);  // Default value
        event.setE2Netcount(0);   // Default value


        // Insert event data
        if (dbManager) {
            qlonglong eventId = dbManager->insertEvent(&event);
            nucare::logI() << "Event inserted with ID:" << eventId;

            if (eventId > 0 && !spectrumStringData.isEmpty()) {
                dbManager->insertEventDetail(eventId, spectrumStringData);
            }
        } else {
            nucare::logW() << "DatabaseManager not found!";
        }

        break;
    }
    }
}

void HomePage::updateEvent()
{
    if (m_accumulator) {
        auto& ret = m_accumulator->getCurrentResult();

        auto etTime = ret.finishTime.toSecsSinceEpoch() - QDateTime::currentSecsSinceEpoch();
        ui->etValueLabel->show();
        ui->etValueLabel->setText(datetime::formatDuration(etTime));
    }
}
