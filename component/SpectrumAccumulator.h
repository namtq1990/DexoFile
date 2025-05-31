#ifndef SPECTRUMACCUMULATOR_H
#define SPECTRUMACCUMULATOR_H

#include "model/AccumulationDataTypes.h" // For AccumulatorState, AccumulationResult
#include "model/Spectrum.h"           // For Spectrum
#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <memory>         // For std::shared_ptr
// #include <variant>     // REMOVE THIS LINE (if it was there)

// Forward declare DetectorComponent and DetectorPackage if needed by slots
namespace nucare {
    class DetectorComponent; // Forward declaration
}

class SpectrumAccumulator : public QObject { // Removed Component
    Q_OBJECT

public:
    enum class AccumulationMode { // New enum class
        NotSet,
        ByCount,
        ByTime,
        ContinuousByCount,
        ContinuousByTime
    };
    friend class Builder;

    class Builder {
    public:
        Builder() :
            m_mode(SpectrumAccumulator::AccumulationMode::NotSet),
            m_targetCountValue(0),
            m_timeoutValueSeconds(0),
            m_continuousIntervalSeconds(0),
            m_parent(nullptr) {}

        Builder& setMode(SpectrumAccumulator::AccumulationMode mode) {
            m_mode = mode;
            return *this;
        }

        Builder& setTargetCount(int count) {
            m_targetCountValue = count;
            return *this;
        }

        Builder& setTimeoutSeconds(int seconds) {
            m_timeoutValueSeconds = seconds;
            return *this;
        }

        Builder& setContinuousInterval(int intervalSeconds) {
            m_continuousIntervalSeconds = intervalSeconds;
            return *this;
        }

        Builder& setParent(QObject* parent) {
            m_parent = parent;
            return *this;
        }

        SpectrumAccumulator* build() {
            // Validation can be added here
            return new SpectrumAccumulator(*this, m_parent);
        }

    friend class SpectrumAccumulator;

    private:
        SpectrumAccumulator::AccumulationMode m_mode;
        int m_targetCountValue;
        int m_timeoutValueSeconds;
        int m_continuousIntervalSeconds;
        QObject* m_parent;
    };

    ~SpectrumAccumulator(); // Public Destructor

public slots:
    // Dynamic adjustments + public start/stop
    void adjustTargetTime(int secondsDelta);
    void adjustTargetCount(int countDelta);
    void start();
    void stop();

signals:
    void stateChanged(AccumulatorState newState); // Uses AccumulatorState from AccumulationDataTypes.h
    void accumulationComplete(const AccumulationResult& result); // Uses AccumulationResult from AccumulationDataTypes.h

private slots:
    void onNcManagerSpectrumReceived(std::shared_ptr<Spectrum> spcFromSignal); // ADD THIS LINE
    void onAccumulationTimeout();
    void onContinuousIntervalTimeout();

private:
    explicit SpectrumAccumulator(const Builder& builder, QObject* parent = nullptr); // Private constructor for Builder

    // Core state and configuration
    AccumulationMode m_mode; // New enum type
    AccumulatorState m_currentState; // From AccumulationDataTypes.h
    ActiveSpectrumType m_activeAccumulationType; // ADD THIS

    int m_targetCountValue;
    int m_timeoutValueSeconds;

    int m_continuousIntervalSeconds;
    QTimer* m_continuousIntervalTimer;

    // std::variant<...> m_accumulatedSpectrumVariant; // REMOVE THIS LINE
    std::shared_ptr<Spectrum> m_accumulatedSpectrum_Spectrum;   // ADD THIS
    std::shared_ptr<HwSpectrum> m_accumulatedSpectrum_HwSpectrum; // ADD THIS

    QTimer* m_accumulationTimer;

    QDateTime m_startTime;
    QDateTime m_finishTime;

    // Internal methods
    void internalStartAccumulation();
    void internalStopAccumulation(bool conditionMet);
    void transitionToState(AccumulatorState newState);

    // DetectorComponent access via ComponentManager::instance() in .cpp
};

#endif // SPECTRUMACCUMULATOR_H
