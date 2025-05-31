#ifndef ACCUMULATIONDATATYPES_H
#define ACCUMULATIONDATATYPES_H

#include "model/Spectrum.h" // For Spectrum and HwSpectrum
#include <QDateTime>        // For QDateTime
#include <memory>           // For std::shared_ptr
// #include <variant>       // REMOVE THIS LINE

// Define ActiveSpectrumType enum (can be class enum if preferred, but regular enum is fine too)
// Let's use enum class for better scoping, as it was in the plan.
enum class ActiveSpectrumType {
    None,
    TypeSpectrum,    // Indicates Spectrum (float, 1024) is active
    TypeHwSpectrum   // Indicates HwSpectrum (int, 2048) is active
};

// AccumulatorState enum remains the same (simplified version)
enum class AccumulatorState {
    Idle,
    Waiting,    // Configured, ready to start (manual/continuous trigger)
    Measuring,
    Completed   // Cycle finished, final result emitted, before transitioning to Idle or Waiting (continuous)
};

struct AccumulationResult {
    ActiveSpectrumType activeType;
    std::shared_ptr<Spectrum> spectrum;     // Valid if activeType is TypeSpectrum
    std::shared_ptr<HwSpectrum> hwSpectrum; // Valid if activeType is TypeHwSpectrum

    QDateTime startTime;
    QDateTime finishTime;
    double cps;
    double executionRealtimeSeconds;
    uint count;

    // Constructor to initialize type and clear pointers
    AccumulationResult() : activeType(ActiveSpectrumType::None), spectrum(nullptr), hwSpectrum(nullptr), count(0) {}
};

#endif // ACCUMULATIONDATATYPES_H
