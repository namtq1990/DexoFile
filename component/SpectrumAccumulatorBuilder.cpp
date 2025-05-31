#include "SpectrumAccumulatorBuilder.h"
#include "SpectrumAccumulator.h" // Needed for applyTo to call SpectrumAccumulator methods

SpectrumAccumulatorBuilder::SpectrumAccumulatorBuilder()
    : m_mode(SpectrumAccumulator::AccumulationMode::NotSet),
      m_targetCount(0),
      m_timeoutSeconds(0),
      m_adjustmentStep(0),
      m_continuousModeEnabled(false),
      m_continuousModeDelaySeconds(0) {
    // Initialize with sensible defaults
}

SpectrumAccumulatorBuilder& SpectrumAccumulatorBuilder::setMode(SpectrumAccumulator::AccumulationMode mode) {
    m_mode = mode;
    return *this;
}

SpectrumAccumulatorBuilder& SpectrumAccumulatorBuilder::setTargetCount(int count) {
    m_targetCount = count;
    return *this;
}

SpectrumAccumulatorBuilder& SpectrumAccumulatorBuilder::setTimeoutSeconds(int seconds) {
    m_timeoutSeconds = seconds;
    return *this;
}

SpectrumAccumulatorBuilder& SpectrumAccumulatorBuilder::setAdjustmentStep(int step) {
    m_adjustmentStep = step;
    return *this;
}

SpectrumAccumulatorBuilder& SpectrumAccumulatorBuilder::setContinuousMode(bool enabled) {
    m_continuousModeEnabled = enabled;
    return *this;
}

SpectrumAccumulatorBuilder& SpectrumAccumulatorBuilder::setContinuousModeDelaySeconds(int seconds) {
    m_continuousModeDelaySeconds = seconds;
    return *this;
}

void SpectrumAccumulatorBuilder::applyTo(SpectrumAccumulator* accumulator) {
    if (!accumulator) {
        // Handle error: log or throw
        return;
    }

    // Call public setters on SpectrumAccumulator.
    // These setters will need to be created in SpectrumAccumulator in the next steps.
    // For now, this is a conceptual layout.
    // If SpectrumAccumulator does not have these setters, this will need to change
    // or SpectrumAccumulator must friend this builder.
    // Plan step 4 mentions "Friend class SpectrumAccumulatorBuilder or use public setters".
    // Let's assume we will add public setters to SpectrumAccumulator.

    accumulator->setAccumulationMode(m_mode); // Assumes public setter
    if (m_mode == SpectrumAccumulator::AccumulationMode::ByCount) {
        accumulator->setTargetCount(m_targetCount); // Assumes public setter
        if (m_timeoutSeconds > 0) { // Optional timeout for ByCount
             accumulator->setAccumulationTimeout(m_timeoutSeconds); // Assumes public setter
        } else {
             accumulator->setAccumulationTimeout(0); // Or some default to disable timeout
        }
    } else if (m_mode == SpectrumAccumulator::AccumulationMode::ByTime) {
        accumulator->setAccumulationTimeout(m_timeoutSeconds); // Assumes public setter
        accumulator->setTargetCount(0); // Reset/clear target count
    }

    accumulator->setAdjustmentStep(m_adjustmentStep); // Assumes public setter
    accumulator->setContinuousMode(m_continuousModeEnabled); // Assumes public setter
    accumulator->setContinuousModeDelay(m_continuousModeDelaySeconds); // Assumes public setter

    // After applying settings, SpectrumAccumulator should transition to 'Configured' state
    accumulator->finalizeConfiguration(); // Assumes such a method to transition state
}
