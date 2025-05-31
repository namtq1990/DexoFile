#ifndef SPECTRUMACCUMULATORBUILDER_H
#define SPECTRUMACCUMULATORBUILDER_H

#include "SpectrumAccumulator.h" // For SpectrumAccumulator::AccumulationMode and to configure SpectrumAccumulator

// Forward declare SpectrumAccumulator to avoid full include if it were heavy,
// but since we need its nested types and will call its methods, full include is better.
// class SpectrumAccumulator;

class SpectrumAccumulatorBuilder {
public:
    SpectrumAccumulatorBuilder();

    SpectrumAccumulatorBuilder& setMode(SpectrumAccumulator::AccumulationMode mode);
    SpectrumAccumulatorBuilder& setTargetCount(int count);
    SpectrumAccumulatorBuilder& setTimeoutSeconds(int seconds);
    SpectrumAccumulatorBuilder& setAdjustmentStep(int step); // For count or time adjustment
    SpectrumAccumulatorBuilder& setContinuousMode(bool enabled);
    SpectrumAccumulatorBuilder& setContinuousModeDelaySeconds(int seconds);

    void applyTo(SpectrumAccumulator* accumulator);

private:
    SpectrumAccumulator::AccumulationMode m_mode;
    int m_targetCount;
    int m_timeoutSeconds;
    int m_adjustmentStep;
    bool m_continuousModeEnabled;
    int m_continuousModeDelaySeconds;

    // Keep track of which fields were set to apply only those,
    // or apply all with defaults. For simplicity, apply all with defaults.
};

#endif // SPECTRUMACCUMULATORBUILDER_H
