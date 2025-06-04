#include "Calibration.h"
#include <cstring>

const std::shared_ptr<HwSpectrum> &Calibration::spc() const
{
    return mSpc;
}

void Calibration::setSpc(std::shared_ptr<HwSpectrum> newSpc)
{
    mSpc = newSpc;
}

const Coeffcients &Calibration::chCoefficients() const
{
    return mChCoefficients;
}

void Calibration::setChCoefficients(const Coeffcients &newChCoefficients)
{
    mChCoefficients = newChCoefficients;
}

const Coeffcients &Calibration::getStdPeaks() const
{
    return mStdPeaks;
}

void Calibration::setStdPeaks(Coeffcients &peaks)
{
    mStdPeaks = peaks;
}

double Calibration::temperature() const
{
    return mTemperature;
}

void Calibration::setTemperature(double newTemperature)
{
    mTemperature = newTemperature;
}

void Calibration::setGC(int newGC)
{
    mGC = newGC;
}

void Calibration::setDate(nucare::Timestamp newDate)
{
    mDate = newDate;
}

int Calibration::getDetectorId() const
{
    return detectorId;
}

void Calibration::setDetectorId(int newDetectorId)
{
    detectorId = newDetectorId;
}

int Calibration::getId() const
{
    return mId;
}

void Calibration::setId(int newId)
{
    mId = newId;
}

void Calibration::setCoefficients(const Coeffcients &coeff)
{
    std::memcpy(mCoefficients.data(), coeff.data(), coeff.size() * sizeof(double));
}
