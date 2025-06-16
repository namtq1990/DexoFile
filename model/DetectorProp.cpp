 #include "DetectorProp.h"

using namespace std;
using namespace nucare;

Coeffcients origin = {13, 372, 861};
const int GC = 27000;

const std::array<double, 8> DetectorProperty::interCoeff = { -0.0000000001, 0.0000005531, -0.0008610261, 0.5684236932,
            -53.5185548731, 0.0002779219, -0.0100275772, 5.8129370431 };

DetectorProperty::DetectorProperty() : mCalib(), mOriginSpc(nullptr)
{
//    mCalib.coefficients = { -0.0005103568925150289, 3.443973937583906, 8.518676839918477};
    mGeCoeffcients = {-0.009219166, 0.025628772, -0.027106762, 0.014082004, -0.003681875, 0.000398323};
    mFWHM = {0.375246174 , -1.6613683291};
    mPeakCoefficients = { -0.027939138, 0.694026779, -6.627760069, 28.20796375, -48.74100729};
    mSmoothParams = pair<double, double>(0.0264599, 2.5608255);

    mFHM = {0.6714086, -1.2000947};
    mPeakInfo = { 150, 450 };
    setDetectorCode(CSI_SPRD);
//    setDetectorCode(LaBr_1_5x1_5);

    // TODO Hardcode for sprd csi
    setFWHM({0.737424355,-4.269930955});
    setPeakCoefficient({-0.027939138, 0.694026779,-6.627760069,28.20796375,-48.74100729});
    setStdEffecients({0.000426431, 0.000598509, 0.00066558,0.000749811,0.0009499,0.000402778,0.000531464,0.000291814,0.000239788,0.000218446});
    setGeCoefficient({-0.027991301, 0.088086519, -0.107955794, 0.066087146, -0.020434902, 0.002566901});
    setROI(0.6);
    setGCFactor(9.0);
    setTemperatureFactor({-0.134095617, 6.239816076});
    setSmooths(0.014096065, 1.65610479);
    setFHM({1.2964663, -3.633816});
    setGaugeFit(0.7);
    setGainThreshold(75);
    setPeakInfo(300, 550);
    // End
}

double DetectorProperty::getK40Ch()
{
    if (mCalib) {
        return mCalib->chCoefficients()[2];
    }

    return 0;
}

void DetectorProperty::setPeakInfo(const int &middleCalib, const int &endCalib)
{
    mPeakInfo[0] = middleCalib;
    mPeakInfo[1] = endCalib;
}

void DetectorProperty::setTemperatureFactor(const decltype(mTemperatureFactor) &factor )
{
    mTemperatureFactor = factor;
}

void DetectorProperty::setGainThreshold(const int &threshold)
{
    gainProperty.gainThrhold = threshold;
}

void DetectorProperty::setGM(double gm)
{
    mAvgGM.addValue(gm);
}

void DetectorProperty::setDetectorCode(const DetectorCode_E code)
{
    info.detectorCode = make_shared<DetectorCode>(code);
}

double DetectorProperty::getGmCount() const
{
    return mAvgGM.calculate();
}

bool DetectorProperty::isEnableGainStab()
{
    return false;
//    return gainProperty.isEnabled && nucare::config::ENABLE_GAIN_STAB;
}

void DetectorProperty::setEnableGainStab(bool isEnable)
{
    gainProperty.isEnabled = isEnable;
}
