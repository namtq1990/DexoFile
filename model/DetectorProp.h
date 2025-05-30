#ifndef DETECTORPROPERTY_H
#define DETECTORPROPERTY_H

#include "Types.h"
#include "Calibration.h"
#include "Background.h"
#include "DetectorInfo.h"
#include "util/util.h"

#define DETECTOR_PROBE_DEFAULT "None"
#define DETECTOR_TYPE_GAMMA_PRO "GP"

class Isotopes;
class NcManager;

namespace nucare {


#define NC_GAIN_REST_TIME_HIGH 120      // stabilization rest time if K40diff <= 1%:150
#define NC_GAIN_REST_TIME_MEDIUM 60     // stabilization rest time if K40diff <= 2%:60
#define NC_GAIN_REST_TIME_LOW 10        // stabilization rest time if K40diff > 2%:10

struct DetectorProperty
{

private:
    std::shared_ptr<Calibration> mCalib;
    GeCoefficients mGeCoeffcients;
    StdEff mStdEff;
    FWHM mFWHM;
    FHM mFHM;
    PeakCoefficients mPeakCoefficients;
    double mWndROI;
    std::pair<double, double> mSmoothParams;
    std::array<int, 2> mPeakInfo;
    double mGCFactor;
    int mGC = 0;
    int mTemperature = 0;
    int mRawTemperature = 0;
    std::array<double, 2> mTemperatureFactor;
    double mRsqrGaugeFit;

    std::vector<std::shared_ptr<Isotopes>> mIsotopes;
    std::shared_ptr<Background> mBackground;
    std::shared_ptr<Spectrum> mSpc;
    std::shared_ptr<Spectrum> mOriginSpc;
    double mCPS = 0;
    double mCPS0 = 0;
    double mDoserate = 0;
    double mRealtime = 0;
    double mNeutron = 0;

    nucare::Average<float, 60> mAvgGM;

    bool mHasNeutron = false;
    bool m_initialized = false;

public:

    DetectorInfo info;

    struct Debug {
        std::shared_ptr<Spectrum> mGrubSpc;
        std::shared_ptr<Spectrum> hwSpc;
        double mGrubGM = 0;
        double spcDoserate0 = 0;
        double spcDoserate1 = 0;
        double gmDoserate = 0;
    } debugInfo;

    struct GainProp {
        bool isEnabled = true;
        int gainRestTime = NC_GAIN_REST_TIME_LOW;
        int gainElapsedTime = 0;
        int gainThrhold = 75;
        std::shared_ptr<Spectrum> gainSpc = nullptr;

        int cntGCNeg = 0;
        int cntGCPos = 0;

#define GAIN_GC_FREQ_DISC 2
    } gainProperty;

    DetectorProperty();

    auto getId() const { return info.id; }
    auto isInitialized() const { return m_initialized; }
    auto getCalibration() { return mCalib; }
    auto& getCoeffcients() { return mCalib->coefficients(); }
    auto& getGeCoeffcients() { return mGeCoeffcients; }
    auto& getStdEffecients() { return mStdEff; }
    auto& getFWHM() {return mFWHM; }
    auto& getPeakCoefficients() { return mPeakCoefficients; }
    auto& getWndROI() { return mWndROI; }
    auto& getSmoothParams() { return mSmoothParams; }
    auto getGCFactor() { return mGCFactor; }
    auto& getTemperatureFact() { return mTemperatureFactor; }
    auto& getIsotopes() { return mIsotopes; }
    double getWndRatio_Co57_Tc99m() { return 0.45; }
    double getNeutronCps() { return mNeutron; }
    auto getOriginSpc() { return mOriginSpc; }
    int getGC() { return mGC; }
    int getTemperature() { return mTemperature; }
    int getRawTemperature() { return mRawTemperature; }
    bool hasNeutron() { return mHasNeutron; }
    double getK40Ch();

    void setCoeffcients(const Coeffcients &newCoeffcients) { mCalib->setCoefficients(newCoeffcients); }
    void setGeCoefficient(const GeCoefficients &geCoeffs) { mGeCoeffcients = geCoeffs; }
    void setFWHM(const FWHM& fwhm) { mFWHM = fwhm; }
    void setFHM(const FHM& fhm) { mFHM = fhm; }
    void setROI(const double& roi) { mWndROI = roi; }
    void setSmooths(const double smoothA, const double smoothB) { mSmoothParams = std::pair<double, double>(smoothA, smoothB); }
    void setPeakInfo(const int& middleCalib, const int& endCalib);
    void setPeakCoefficient(const PeakCoefficients& peakCoeffs) { mPeakCoefficients = peakCoeffs; }
    void setStdEffecients(const StdEff& efficients) { mStdEff = efficients; }
    void setGCFactor(const double& gcFactor) { mGCFactor = gcFactor; }
    void setGC(const int gc) { mGC = gc; }
    void setTemperatureFactor(const decltype(mTemperatureFactor) &factor );
    void setGaugeFit(const double gaugeFit) { mRsqrGaugeFit = gaugeFit; }
    void setGainThreshold(const int &threshold);
    void setIsotopes(std::vector<std::shared_ptr<Isotopes>>& isotopes) { mIsotopes = isotopes; }
    void setNeutron(const double&& neutron) { mNeutron = neutron; }
    void setTemperature(const int temperature) { mTemperature = temperature; }
    void setRawTemperature(const int tmp) { mRawTemperature = tmp; }
    void setGM(double gm);
    void setHasNeutron(bool hasNeutron) { mHasNeutron = hasNeutron; }
    void setCps(double cps) { mCPS = cps; }
    void setCps0(double cps) { mCPS0 = cps; } //Origanl CPS

    void setDetectorCode(const DetectorCode_E code);
    void setSerial(const QString& serial) { info.serialNumber = serial; }
    void setDetectorType(const QString& type) { info.detectorType = type; }
    void setCalibration(std::shared_ptr<Calibration> calib) { mCalib = calib; }
    void setBackgroundSpc(std::shared_ptr<Background> background) { mBackground = background; }
    auto getBackgroundSpc() { return mBackground ? mBackground->spc : nullptr; }
    auto getBackground() { return mBackground; }
    std::shared_ptr<Spectrum> getCurrentSpectrum() { return mSpc; }
    auto getDetectorCode() { return info.detectorCode; }
    auto getSerial() { return info.serialNumber; }

    double getCps() const { return mCPS; }
    double getCps0() const { return mCPS0; }
    double getDoserate() const { return mDoserate; }
    double getGmCount() const;

    bool isEnableGainStab();
    void setEnableGainStab(bool isEnable);


    static const std::array<double, 8> InterCoeff;
//    = { -0.0000000001, 0.0000005531, -0.0008610261, 0.5684236932,
//            -53.5185548731, 0.0002779219, -0.0100275772, 5.8129370431 };
    static constexpr double THRSHLD = 0.3;



    friend class ::NcManager;
    friend class DetectorComponent;
};

}

#endif // DETECTORPROPERTY_H
