#ifndef ACCUMULATIONRESULT_H
#define ACCUMULATIONRESULT_H

#include "Spectrum.h" // Assuming Spectrum.h is in model/ or globally accessible
#include "SpectrumAccumulator.h" // For SpectrumAccumulator::AccumulationMode
#include <QDateTime>

struct AccumulationResult {
    Spectrum accumulatedSpectrum;
    QDateTime startTime;
    QDateTime finishTime;
    double totalCPS;
    double executionRealtimeSeconds;
    SpectrumAccumulator::AccumulationMode mode;
    bool primaryConditionMet;

    qlonglong detectorId;
    qlonglong backgroundId;
    qlonglong calibrationId;

    AccumulationResult()
        : startTime(),
          finishTime(),
          totalCPS(0.0),
          executionRealtimeSeconds(0.0),
          mode(SpectrumAccumulator::AccumulationMode::ContinuousByTime), // Assuming this is a valid default
          primaryConditionMet(false),
          detectorId(-1),
          backgroundId(-1),
          calibrationId(-1) {
        // accumulatedSpectrum is default constructed
    }
};

#endif // ACCUMULATIONRESULT_H
