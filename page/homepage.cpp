#include "page/homepage.h"
#include "ui_homepage.h"  // Generated UI header
#include "component/settingmanager.h"
#include "component/SpectrumAccumulator.h"
#include "util/util.h"
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
