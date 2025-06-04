#ifndef DETECTORCALIBCONFIG_H
#define DETECTORCALIBCONFIG_H

#include "Types.h"
#include "Spectrum.h"

struct DetectorCalibConfig {
    std::shared_ptr<HwSpectrum> spc = nullptr;
    nucare::Timestamp time;
    Coeffcients calib;
    int detectorId = -1;
};

#endif // DETECTORCALIBCONFIG_H
