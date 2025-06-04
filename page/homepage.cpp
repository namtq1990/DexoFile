#include "page/homepage.h"
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


    auto builder = SpectrumAccumulator::Builder()
            .setParent(this)
            .setMode(SpectrumAccumulator::AccumulationMode::ContinuousByTime)
            .setContinuousInterval(7200)
            .setTimeoutSeconds(3600);
    m_accumulator = std::shared_ptr<SpectrumAccumulator>(builder.build());
    connect(m_accumulator.get(), &SpectrumAccumulator::stateChanged, this, &HomePage::stateChanged);
    connect(m_accumulator.get(), &SpectrumAccumulator::accumulationUpdated, this, &HomePage::updateEvent);

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
        ui->intervalTimeLabel->setText(QString("Int.time: %1")
                                       .arg(datetime::formatDuration(m_accumulator->getIntervalTime())));

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
        break;
    }
    case AccumulatorState::Waiting: {
        ui->stateLabel->setText("State: Waiting");
        break;
    }
    case AccumulatorState::Completed: {
        ui->stateLabel->setText("State: Waiting");

        // Retrieve accumulation result
        auto ret = m_accumulator->getCurrentAccumulationResult();

        // Create Event object
        model::Event event;

        // Populate Event object
        event.setSoftwareVersion("1.0.0"); // Or a more appropriate version
        event.setDateBegin(datetime::formatDate_yyyyMMdd_HHmmss(ret.startTime));
        event.setDateFinish(datetime::formatDate_yyyyMMdd_HHmmss(ret.finishTime));
        event.setLiveTime(ret.liveTime);
        event.setRealTime(ret.realTime);
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
        event.setDetail_id(0);

        // Spectrum data from ret.accumulatedSpectrum
        // Assuming AccumulationResult has 'Spectrum accumulatedSpectrum;' and Spectrum has 'toString()'
        QString spectrumStringData = ret.accumulatedSpectrum.toString();
        if (spectrumStringData.isEmpty()) { // Adjust this check if toString() on empty spectrum is not empty string
            nucare::logW() << "Accumulation result has empty spectrum data string.";
        }

        event.setE1Energy(0);     // Default value
        event.setE1Branching(0);  // Default value
        event.setE1Netcount(0);   // Default value
        event.setE2Energy(0);     // Default value
        event.setE2Branching(0);  // Default value
        event.setE2Netcount(0);   // Default value

        // Get DatabaseManager instance
        auto dbManager = ComponentManager::instance()->getComponent<DatabaseManager>("DATABASE");

        // Insert event data
        if (dbManager) {
            qlonglong eventId = dbManager->insertEvent(&event);
            nucare::logI() << "Event inserted with ID:" << eventId;

            if (eventId > 0 && !spectrumStringData.isEmpty()) {
                qlonglong detailId = dbManager->insertEventDetail(eventId, spectrumStringData);
                nucare::logI() << "Event detail inserted with ID:" << detailId;

                if (detailId > 0) {
                    bool updateSuccess = dbManager->updateEventDetailId(eventId, detailId);
                    if (updateSuccess) {
                        nucare::logI() << "Successfully updated event" << eventId << "with detail_id" << detailId;
                    } else {
                        nucare::logW() << "Failed to update event" << eventId << "with detail_id" << detailId;
                    }
                } else {
                    nucare::logW() << "Failed to insert event_detail for event ID:" << eventId << "(detailId was" << detailId << ")";
                }
            } else {
                nucare::logW() << "Skipping event_detail insertion due to invalid eventId or empty spectrum data for event ID:" << eventId;
                if (spectrumStringData.isEmpty()) {
                    nucare::logW() << "Spectrum string data was empty.";
                }
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
        auto& ret = m_accumulator->getCurrentAccumulationResult();
        ui->startTimeValueLabel->setText(datetime::formatDate_yyyyMMdd_HHmm(ret.startTime));
        ui->stopTimeValueLabel->setText(datetime::formatDate_yyyyMMdd_HHmm(ret.finishTime));

        auto etTime = ret.finishTime.toSecsSinceEpoch() - QDateTime::currentSecsSinceEpoch();
        ui->etValueLabel->show();
        ui->etValueLabel->setText(datetime::formatDuration(etTime));
    }
}
