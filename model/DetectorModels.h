#ifndef DETECTORMODELS_H
#define DETECTORMODELS_H

#include <cstdint> // For uint16_t, uint64_t, uint8_t
#include <memory>  // For std::shared_ptr
#include "model/Spectrum.h" // For Spectrum_t

// Domain package structure for GC Response (from Get Info command)
struct GcResponse {
    int gc = 0;
    int k40Ch = 0;
    int cs137Ch1 = 0; // Placeholder, assuming this might be derived or present
    int cs137Ch2 = 0; // Placeholder
    int detType = 0; // Corresponds to detectorCode
    int temperature = 0;
    bool hasTemperature = false;
    QString serial;
};

// Domain package structure for continuous data (from Start command)
struct DetectorPackage {
    std::shared_ptr<HwSpectrum> spc = nullptr;
    int neutron = 0;
    double gm = 0;
    double realtime = -1;
    int battery = 0;
    int batteryCharge = 0;
    bool hasNeutron = false;
    bool hasGM = false;
    int pileup = 0;
    int hvDac = -1;
    int risingCount = 0;
    int gc = 0;
    double temperature = 0.0;
    int temperatureRaw = 0;
    int detectorInfo = 0;
    // Add other relevant fields from Package::Payload as needed
};

#endif // DETECTORMODELS_H