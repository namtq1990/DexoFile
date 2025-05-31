#ifndef ACCUMULATIONDATATYPES_H
#define ACCUMULATIONDATATYPES_H

#include "model/Spectrum.h" // For Spectrum and HwSpectrum (typedef Spectrum_t<int, HW_CHSIZE>)
#include <QDateTime>        // For QDateTime
#include <variant>          // For std::variant
#include <memory>           // For std::shared_ptr

// Simplified AccumulatorState enum
enum class AccumulatorState {
    Idle,
    Waiting,    // Configured, ready to start (manual/continuous trigger)
    Measuring
};

struct AccumulationResult {
    std::variant<std::shared_ptr<Spectrum>, std::shared_ptr<HwSpectrum>> accumulatedSpectrumVariant;
    QDateTime startTime;
    QDateTime finishTime;
    double totalCPS;
    double executionRealtimeSeconds;
};

#endif // ACCUMULATIONDATATYPES_H
