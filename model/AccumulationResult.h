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
};

#endif // ACCUMULATIONRESULT_H
