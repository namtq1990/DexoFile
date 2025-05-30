#ifndef NCLIBRARY_H
#define NCLIBRARY_H

#include "model/Types.h"
#include "model/Spectrum.h"
#include "PeakSearch.h"
#include <array>

namespace nucare {

#define HALF_K40_WND 0.2

static constexpr int NO_MAX_K40 = 100;

class DetectorProperty;

class NcLibrary
{
public:
    static inline double channelToEnergy(const double channel, const double* params);

    static double channelToEnergy(const double channel, const Coeffcients& params);

    static double energyToChannel(const double energy, const double* params);

    static double energyToChannel(const double energy, const Coeffcients& params);

    static double computeGEFactor(double energy, const double* geCoef, const int& length);

//    static double computeDoserate(std::shared_ptr<Spectrum> spc, const Coeffcients &coeffs, const GeCoefficients &geCoeffs);

//    static double GM_to_nSV(const double gmCount);

//    static double DoseCSI_Factor_nSV(const double csi_dose);

    static void Nomalization(Spectrum& spc, Spectrum *bgr, Spectrum::SPC_DATA& out);

//    static double getIsotopeDoserate(Spectrum &spc, Spectrum *BG_Data,
//                                     IsotopeSummary* target,
//                                     std::shared_ptr<repository::DetectorProperty> prop,
//                                     std::shared_ptr<Spectrum> normSPC = nullptr);

//    static void quantitativeAnalysis(Spectrum &&spc, Spectrum *bg,
//                                     std::list<std::shared_ptr<IsotopeSummary>> &result,
//                                     NcSP<repository::DetectorProperty> prop);

    static Coeffcients computeCalib(const std::array<double, 3>& chPeaks, const std::array<double, 3>& enPeaks);

    /**
     * @brief calibConvert Convert calibration param to 3MeV
     * @param in Array of input Calib parameter
     * @param out Array of output Calib parameter
     * @param oChannel Output channel target, default is 1024
     * @return Ratio of this parameter
     */
    static double calibConvert(const double* in, double* out, const int& iChSize = 2048, const int& oChSize = 1024);

    /**
     * @deprecated
     * @brief convertSpectrum   Convert a spectrum data from any size into library size, normally 2048->1024 channel
     * @param in        Data of input spectrum
     * @param inSize    Size of input spectrum
     * @param out       Data of output spectrum
     * @param outSize   Size of output spectrum
     * @param ratio     Ratio saved to convert
     */
    static void convertSpectrum(const double* in, const int inSize, double* out, const int outSize, const double ratio);

    static std::vector<int> findNoPeak(Spectrum& spc, Threshold& roi, double value);
    static int findClosestToK40Est(const std::vector<int>& peaks, int reference);

    static void doseSpcConvert(const Spectrum* ChSpec, Spectrum* out, const Coeffcients& coeff, const GeCoefficients& geCoef);
    static void doseSpcConvert(const Spectrum* ChSpec, Spectrum* out, DetectorProperty* prop);

    static void weightSpc(const Spectrum* in, Spectrum* out, int wf);

    static std::array<double, BINSIZE> computeFWHM(const FWHM& _FWHMCoeff, const Coeffcients& coeff);

    static double fwhm_eff(Spectrum* smoothSpc, int Peak_Channel, const Coeffcients& coeffs, bool Energy);

    /**
     * @brief smoothSpectrum Smooth spectrum before find its peaks.
     * @param spc           Input Spectrum to be smooth
     * @param smoothParam   Smooth paramter
     * @param out           Output result
     */
    static void smoothSpectrum(Spectrum& spc, int wdSize, const int repeat, double* out);

    static void smoothSpectrum(Spectrum& spc, Spectrum& out, const std::pair<double, double>& smoothPar);

    static double channelToFWHM(const double channel, const FWHM& fwhm, const Coeffcients& coeff);
    static double energyToFWHM(const double energy, const FWHM& fwhm);

    static double Get_Roi_window_by_energy(double energy);

    static std::pair<double, double> Get_Roi_window_by_energy_used_FWHM(double en, const FWHM& FWHMCoeff, const Coeffcients& coeff, const double Ratio);
    static std::pair<double, double> Get_Roi_window_by_energy_used_FWHM(double en, DetectorProperty* prop);
    /**
     * @brief AdaptFilter Adapt Filter spectrum. If coeffs is NULL, perform fhm as FWHM for channel,
     * otherwise perform FWHM as for energy
     * @param in
     * @param out
     * @param fhm
     * @param coeffs if NULL, perform fhm as FWHM for channel, otherwise perform FWHM as for energy
     * @param repeat
     */
    static void AdaptFilter(const Spectrum* in, Spectrum* out, const FHM& fhm, const Coeffcients* coeffs = nullptr, const nucare::uint repeat = 3);

    static std::array<double, 2> FindPeak(Spectrum* spc, const std::array<int, 2>& peakInfo);

    /**
     * @brief ROIAnalysis Find K40_Channel in this ROI
     */
    static int ROIAnalysis(Spectrum* spc, const Threshold& threshold);

    /**
     * @brief CalcROIK40 Get CPS for K40's ROI
     */
    static double CalcROIK40(const Spectrum* spc, FWHM& fwhm, Coeffcients& coeffs);

    /**
     * @brief FindK40DoseSpc_CALIB Find K40 in Calibration function
     * @param fgThrhold     Theshold for Fitting Gaussian
     */
    static int FindK40DoseSpc_CALIB(Spectrum* spc, const Coeffcients& coeff,
                                    const FWHM& fwhm, const GeCoefficients& GECoef,
                                    const double roi1 = 0.2, const double roi2 = 0.2,
                                    const double fgThrshold = 0.7);

    /**
     * @brief FindK40DoseSpc_Hight    Find K40 but with larger threshold and condition to try search K40
     */
    static int FindK40DoseSpc_Hight(Spectrum* spc, const Coeffcients& coeff,
                                    const FWHM& fwhm,
                                    const double roi1 = 0.2);

    /**
     * @brief K40FinderByGaussFitting_Dose  Find K40 using FindK40DoseSpc, but repeat for it's ROI
     */
    static int K40FinderByGaussFitting_Dose(Spectrum* spc, DetectorProperty* prop);

    static std::array<double, 2> GauFitFunct_Dose(Spectrum* doseSpc, const FWHM& fwhm, const Coeffcients& coeff,
                                                  const double PeakThslod,
                                                  const Threshold& chDis20,
                                                  const Threshold& roi);
    static void GaussFit_H_G1(double* X, double* Y, int NoPtFit, double a1, double b1, double c1, double A, double B,
                              std::array<double, 6>& out);

    static void ReBinning(double* ChSpec, const int&& chSize,
                          double* transferSpc, const int&& tfSize,
                          double* binSpecOut);

    static void BintoCh(double* TF, double* BinOut);

    /**
     * @brief Find_Vally
     * @param arrfloat
     * @param firstlocation     ROI left index of array
     * @param secondlocation    ROI right index of array
     * @param peakvalue
     * @param out               Return array len 2 with range of ROI
     */
    static void Find_Vally(double* arrfloat, int firstlocation, int secondlocation, int peakvalue, double* out);

    static double confidenceCal(double Found_Peak_Energy, double Iso_Peak_Energy);

    static double Calculate_PeakUncertainty(Spectrum& Spc, double BGSum, int ROI_L, int ROI_R);

    static double Calculate_EffUncertainty(double en, DetectorProperty* prop);
};

//namespace iso {

///**
// * @brief shouldStopStab if this isotope have con fidence level bigger than threshold,. then exit
// */
//bool shouldStopStab(std::list<NcSP<IsotopeSummary>> isotopes);

///**
// * @brief findIsoStab Find isotopes for auto calib or gain stabilization function
// */
//std::list<NcSP<IsotopeSummary>> findIsoStab(Spectrum* SPC, repository::DetectorProperty* prop);

//}

}

#endif // NCLIBRARY_H
