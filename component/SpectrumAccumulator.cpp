#include "SpectrumAccumulator.h"
#include "component/componentmanager.h" // For DetectorComponent, NcManager
#include "component/ncmanager.h"        // For NcManager signals
#include "model/DetectorProp.h"       // For dev->properties()->getOriginSpc()
#include "model/Spectrum.h"           // For Spectrum_t and Spectrum aliases (includes HwSpectrum)
#include "util/util.h"                // For nucare::logX
#include "util/nc_exception.h"        // For NC_THROW_X

SpectrumAccumulator::SpectrumAccumulator(const Builder& builder, QObject* parent)
    : QObject(parent),
      m_mode(builder.m_mode),
      m_currentState(AccumulatorState::Idle),
      m_targetCountValue(builder.m_targetCountValue),
      m_timeoutValueSeconds(builder.m_timeoutValueSeconds),
      m_continuousIntervalSeconds(builder.m_continuousIntervalSeconds),
      m_continuousIntervalTimer(new QTimer(this)),
      m_accumulationTimer(new QTimer(this)) {

    if (m_mode == AccumulationMode::ByTime || m_mode == AccumulationMode::ContinuousByTime) {
        m_activeAccumulationType = ActiveSpectrumType::TypeSpectrum;
        m_accumulatedSpectrum_Spectrum = std::make_shared<Spectrum>();
        m_accumulatedSpectrum_HwSpectrum = nullptr;
    } else if (m_mode == AccumulationMode::ByCount || m_mode == AccumulationMode::ContinuousByCount) {
        m_activeAccumulationType = ActiveSpectrumType::TypeHwSpectrum;
        m_accumulatedSpectrum_HwSpectrum = std::make_shared<HwSpectrum>();
        m_accumulatedSpectrum_Spectrum = nullptr;
    } else {
        m_activeAccumulationType = ActiveSpectrumType::None;
        m_accumulatedSpectrum_Spectrum = nullptr;
        m_accumulatedSpectrum_HwSpectrum = nullptr;
        nucare::logE() << "SpectrumAccumulator: Mode not set at construction or invalid mode! Mode:" << static_cast<int>(m_mode);
    }

    connect(m_accumulationTimer, &QTimer::timeout, this, &SpectrumAccumulator::onAccumulationTimeout);
    connect(m_continuousIntervalTimer, &QTimer::timeout, this, &SpectrumAccumulator::onContinuousIntervalTimeout);

    NcManager* ncMgr = ComponentManager::instance().ncManager();
    if (ncMgr) {
        connect(ncMgr, &NcManager::spectrumReceived, this, &SpectrumAccumulator::onNcManagerSpectrumReceived);
    } else {
        nucare::logE() << "SpectrumAccumulator: NcManager instance is null. SpectrumAccumulator will not receive spectrum data.";
        // NC_THROW_FATAL("NcManager instance is null, SpectrumAccumulator cannot operate.");
    }
    nucare::logI() << "SpectrumAccumulator: Instance created. Mode: " << static_cast<int>(m_mode) << ", ActiveType: " << static_cast<int>(m_activeAccumulationType);
}

SpectrumAccumulator::~SpectrumAccumulator() {
    nucare::logI() << "SpectrumAccumulator: Destructor called.";
}

void SpectrumAccumulator::transitionToState(AccumulatorState newState) {
    if (m_currentState == newState) {
        return;
    }
    AccumulatorState oldState = m_currentState;
    m_currentState = newState;
    nucare::logI() << "SpectrumAccumulator: State changed from " << static_cast<int>(oldState) << " to " << static_cast<int>(m_currentState);
    emit stateChanged(m_currentState);
}

void SpectrumAccumulator::start() {
    nucare::logI() << "SpectrumAccumulator: Start requested. Current state: " << static_cast<int>(m_currentState);
    if (m_mode == AccumulationMode::NotSet || m_activeAccumulationType == ActiveSpectrumType::None) {
        nucare::logW() << "SpectrumAccumulator: Mode not set or active spectrum type is None. Configure via Builder first. Mode:" << static_cast<int>(m_mode) << ", ActiveType: " << static_cast<int>(m_activeAccumulationType);
        return;
    }

    if (m_currentState == AccumulatorState::Idle ||
       (m_currentState == AccumulatorState::Waiting &&
        (m_mode == AccumulationMode::ContinuousByCount || m_mode == AccumulationMode::ContinuousByTime))) {

        if (m_currentState == AccumulatorState::Waiting) {
            m_continuousIntervalTimer->stop();
        }
        transitionToState(AccumulatorState::Waiting);
        internalStartAccumulation();
    } else {
        nucare::logW() << "SpectrumAccumulator: Cannot start from current state " << static_cast<int>(m_currentState);
    }
}

void SpectrumAccumulator::stop() {
    nucare::logI() << "SpectrumAccumulator: Stop requested. Current state: " << static_cast<int>(m_currentState);
    if (m_currentState == AccumulatorState::Measuring) {
        internalStopAccumulation(false);
    } else if (m_currentState == AccumulatorState::Waiting &&
               (m_mode == AccumulationMode::ContinuousByCount || m_mode == AccumulationMode::ContinuousByTime)) {
        m_continuousIntervalTimer->stop();
        nucare::logI() << "SpectrumAccumulator: Stopped continuous interval timer.";
    }
    transitionToState(AccumulatorState::Idle);
}

void SpectrumAccumulator::internalStartAccumulation() {
    nucare::logI() << "SpectrumAccumulator: Internal accumulation starting. Mode: " << static_cast<int>(m_mode) << ", ActiveType: " << static_cast<int>(m_activeAccumulationType);
    m_startTime = QDateTime::currentDateTime();

    if (m_activeAccumulationType == ActiveSpectrumType::TypeSpectrum) {
        m_accumulatedSpectrum_Spectrum = std::make_shared<Spectrum>();
    } else if (m_activeAccumulationType == ActiveSpectrumType::TypeHwSpectrum) {
        m_accumulatedSpectrum_HwSpectrum = std::make_shared<HwSpectrum>();
    } else {
        nucare::logE() << "SpectrumAccumulator: Cannot start, active spectrum type is None or invalid for mode " << static_cast<int>(m_mode);
        transitionToState(AccumulatorState::Idle);
        return;
    }
    transitionToState(AccumulatorState::Measuring);

    bool needsTimeout = false;
    if (m_mode == AccumulationMode::ByTime || m_mode == AccumulationMode::ContinuousByTime) {
        needsTimeout = true;
    } else if ((m_mode == AccumulationMode::ByCount || m_mode == AccumulationMode::ContinuousByCount) && m_timeoutValueSeconds > 0) {
        needsTimeout = true;
    }

    if (needsTimeout) {
        if (m_timeoutValueSeconds > 0) {
            m_accumulationTimer->start(m_timeoutValueSeconds * 1000);
            nucare::logI() << "SpectrumAccumulator: Accumulation timer started for " << m_timeoutValueSeconds << " seconds.";
        } else {
            nucare::logW() << "SpectrumAccumulator: Time-dependent mode selected but timeout is zero or negative. Accumulation may not stop as expected.";
        }
    }
}

void SpectrumAccumulator::internalStopAccumulation(bool conditionMet) {
    if (m_currentState != AccumulatorState::Measuring) {
        nucare::logW() << "SpectrumAccumulator: InternalStop called but not in Measuring state. Current state: " << static_cast<int>(m_currentState);
        m_accumulationTimer->stop();
        return;
    }

    m_accumulationTimer->stop();
    m_finishTime = QDateTime::currentDateTime();
    nucare::logI() << "SpectrumAccumulator: Internal accumulation stopped. Condition met: " << conditionMet;

    AccumulationResult result;
    result.activeType = m_activeAccumulationType;
    result.startTime = m_startTime;
    result.finishTime = m_finishTime;
    result.executionRealtimeSeconds = static_cast<double>(m_startTime.msecsTo(m_finishTime)) / 1000.0;

    double totalCount = 0;
    if (m_activeAccumulationType == ActiveSpectrumType::TypeSpectrum && m_accumulatedSpectrum_Spectrum) {
        result.spectrum = m_accumulatedSpectrum_Spectrum;
        totalCount = static_cast<double>(m_accumulatedSpectrum_Spectrum->getTotalCount());
    } else if (m_activeAccumulationType == ActiveSpectrumType::TypeHwSpectrum && m_accumulatedSpectrum_HwSpectrum) {
        result.hwSpectrum = m_accumulatedSpectrum_HwSpectrum;
        totalCount = static_cast<double>(m_accumulatedSpectrum_HwSpectrum->getTotalCount());
    } else {
        nucare::logW() << "SpectrumAccumulator: No accumulated spectrum data available or type mismatch on stop. ActiveType: " << static_cast<int>(m_activeAccumulationType);
    }

    result.totalCPS = (result.executionRealtimeSeconds > 0.00001) ? (totalCount / result.executionRealtimeSeconds) : 0.0;

    emit accumulationComplete(result);

    if (m_mode == AccumulationMode::ContinuousByCount || m_mode == AccumulationMode::ContinuousByTime) {
        transitionToState(AccumulatorState::Waiting);
        if (m_continuousIntervalSeconds > 0) {
            m_continuousIntervalTimer->start(m_continuousIntervalSeconds * 1000);
            nucare::logI() << "SpectrumAccumulator: Continuous mode: interval timer started for " << m_continuousIntervalSeconds << " seconds.";
        } else {
            nucare::logI() << "SpectrumAccumulator: Continuous mode with 0s interval, re-triggering immediately.";
            internalStartAccumulation();
        }
    } else {
        transitionToState(AccumulatorState::Idle);
    }
}

void SpectrumAccumulator::onNcManagerSpectrumReceived(std::shared_ptr<Spectrum> spcFromSignal) {
    if (m_currentState != AccumulatorState::Measuring) {
        return;
    }

    if (m_activeAccumulationType == ActiveSpectrumType::TypeSpectrum) {
        if (spcFromSignal && m_accumulatedSpectrum_Spectrum) {
            m_accumulatedSpectrum_Spectrum->accumulate(*spcFromSignal);
        } else if (!spcFromSignal) {
            nucare::logW() << "SpectrumAccumulator: Received null Spectrum in ByTime mode from NcManager.";
        } else { // m_accumulatedSpectrum_Spectrum is null
            nucare::logE() << "SpectrumAccumulator: m_accumulatedSpectrum_Spectrum is null in ByTime mode.";
        }
    } else if (m_activeAccumulationType == ActiveSpectrumType::TypeHwSpectrum) {
        nucare::DetectorComponent* detComp = ComponentManager::instance().detectorComponent();
        if (detComp && detComp->properties()) {
            std::shared_ptr<HwSpectrum> originHwSpc = detComp->properties()->getOriginSpc();

            if (originHwSpc && m_accumulatedSpectrum_HwSpectrum) {
                m_accumulatedSpectrum_HwSpectrum->accumulate(*originHwSpc);
                double currentTotalCount = m_accumulatedSpectrum_HwSpectrum->getTotalCount();
                if (m_targetCountValue > 0 && currentTotalCount >= m_targetCountValue) {
                    nucare::logI() << "SpectrumAccumulator: Target count " << m_targetCountValue << " reached.";
                    internalStopAccumulation(true);
                }
            } else if (!originHwSpc) {
                nucare::logW() << "SpectrumAccumulator: Failed to get HwSpectrum from DetectorProperty::getOriginSpc() in ByCount mode (nullptr received).";
            } else { // m_accumulatedSpectrum_HwSpectrum is null
                nucare::logE() << "SpectrumAccumulator: m_accumulatedSpectrum_HwSpectrum is null in ByCount mode.";
            }
        } else {
            nucare::logW() << "SpectrumAccumulator: DetectorComponent or its properties not available for ByCount mode.";
        }
    } else {
        nucare::logW() << "SpectrumAccumulator: Data received but active spectrum type is None. Mode: " << static_cast<int>(m_mode);
    }
}

void SpectrumAccumulator::onAccumulationTimeout() {
    if (m_currentState != AccumulatorState::Measuring) {
        nucare::logW() << "SpectrumAccumulator: AccumulationTimeout received but not in Measuring state. State: " << static_cast<int>(m_currentState);
        return;
    }
    nucare::logI() << "SpectrumAccumulator: Accumulation timer timed out.";

    if (m_mode == AccumulationMode::ByTime || m_mode == AccumulationMode::ContinuousByTime) {
        internalStopAccumulation(true);
    } else if (m_mode == AccumulationMode::ByCount || m_mode == AccumulationMode::ContinuousByCount) {
        internalStopAccumulation(false);
    } else {
         nucare::logW() << "SpectrumAccumulator: Timeout in unhandled mode " << static_cast<int>(m_mode);
         internalStopAccumulation(false);
    }
}

void SpectrumAccumulator::onContinuousIntervalTimeout() {
    if (m_currentState == AccumulatorState::Waiting &&
        (m_mode == AccumulationMode::ContinuousByCount || m_mode == AccumulationMode::ContinuousByTime)) {
        nucare::logI() << "SpectrumAccumulator: Continuous interval timeout. Restarting accumulation.";
        internalStartAccumulation();
    } else {
        nucare::logW() << "SpectrumAccumulator: Continuous interval timeout received in unexpected state " << static_cast<int>(m_currentState) << " or mode " << static_cast<int>(m_mode);
    }
}

void SpectrumAccumulator::adjustTargetTime(int secondsDelta) {
    if (m_activeAccumulationType != ActiveSpectrumType::TypeSpectrum) {
        nucare::logW() << "SpectrumAccumulator: adjustTargetTime called but not in a time-based accumulation type (TypeSpectrum). ActiveType: " << static_cast<int>(m_activeAccumulationType);
        return;
    }

    int newTimeout = m_timeoutValueSeconds + secondsDelta;
    if (newTimeout < 1) {
        newTimeout = 1;
        nucare::logW() << "SpectrumAccumulator: Adjusted time resulted in less than 1s, setting to 1s.";
    }
    m_timeoutValueSeconds = newTimeout;
    nucare::logI() << "SpectrumAccumulator: Target time adjusted by " << secondsDelta << "s. New target: " << m_timeoutValueSeconds << "s.";

    if (m_currentState == AccumulatorState::Measuring && m_accumulationTimer->isActive()) {
        qint64 elapsedMsecs = m_startTime.msecsTo(QDateTime::currentDateTime());
        qint64 newTotalMsecs = static_cast<qint64>(m_timeoutValueSeconds) * 1000;
        qint64 remainingMsecs = newTotalMsecs - elapsedMsecs;

        m_accumulationTimer->stop();
        if (remainingMsecs <= 0) {
            nucare::logI() << "SpectrumAccumulator: Time adjustment resulted in timeout already being met or passed.";
            onAccumulationTimeout();
        } else {
            m_accumulationTimer->start(remainingMsecs);
            nucare::logI() << "SpectrumAccumulator: Accumulation timer restarted with remaining " << remainingMsecs << " ms.";
        }
    }
}

void SpectrumAccumulator::adjustTargetCount(int countDelta) {
    if (m_activeAccumulationType != ActiveSpectrumType::TypeHwSpectrum) {
        nucare::logW() << "SpectrumAccumulator: adjustTargetCount called but not in a count-based accumulation type (TypeHwSpectrum). ActiveType: " << static_cast<int>(m_activeAccumulationType);
        return;
    }

    int newTargetCount = m_targetCountValue + countDelta;
    if (newTargetCount < 1) {
        newTargetCount = 1;
        nucare::logW() << "SpectrumAccumulator: Adjusted count resulted in less than 1, setting to 1.";
    }
    m_targetCountValue = newTargetCount;
    nucare::logI() << "SpectrumAccumulator: Target count adjusted by " << countDelta << ". New target: " << m_targetCountValue;

    if (m_currentState == AccumulatorState::Measuring) {
        if (m_accumulatedSpectrum_HwSpectrum && m_accumulatedSpectrum_HwSpectrum->getTotalCount() >= m_targetCountValue) {
            nucare::logI() << "SpectrumAccumulator: Count adjustment resulted in target count already being met.";
            internalStopAccumulation(true);
        } else if (!m_accumulatedSpectrum_HwSpectrum) {
            nucare::logE() << "SpectrumAccumulator: adjustTargetCount: m_accumulatedSpectrum_HwSpectrum is null in Measuring state.";
        }
    }
}
