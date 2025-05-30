#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "Spectrum.h"

#include <time.h>
#include <array>
#include "Types.h"

class Calibration {
public:
    enum Mode {
        SPRD_CS_137,
        SPRD_CO_60,
        HH300_CS_137,
    };
private:
    Coeffcients mCoefficients;
    Coeffcients mStdPeaks;
    Coeffcients mChCoefficients;
    std::shared_ptr<Spectrum> mSpc = nullptr;
    nucare::Timestamp mDate;
    double mTemperature;
    double mRatio;
    int mId = -1;
    int mGC;
    int detectorId;
public:
    inline const Coeffcients& coefficients() const { return mCoefficients; } // Made const
    void setCoefficients(const Coeffcients& coeff);

    const Coeffcients &chCoefficients() const; // Made const
    void setChCoefficients(const Coeffcients &newChCoefficients);

    const Coeffcients& getStdPeaks() const; // Made const
    void setStdPeaks(Coeffcients& peaks);

    inline double getRatio() const { return mRatio; } // Made const
    int getGC() const { return mGC; } // Made const, explicit return type
    inline void setRatio(const double ratio) { mRatio = ratio; }
    nucare::Timestamp getDate() const { return mDate; } // Made const, explicit return type
    const std::shared_ptr<Spectrum> &spc() const;
    void setSpc(const std::shared_ptr<Spectrum> &newSpc);
    double temperature() const;
    void setTemperature(double newTemperature);
    void setGC(int newGC);
    void setDate(nucare::Timestamp newDate);
    int getDetectorId() const;
    void setDetectorId(int newDetectorId);
    int getId() const;
    void setId(int newId);

};

#endif // CALIBRATION_H
