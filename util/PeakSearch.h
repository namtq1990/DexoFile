#ifndef PEAKSEARCH_H
#define PEAKSEARCH_H

#include "model/Types.h"
#include "model/Spectrum.h"
//#include "Model/Isotopes.h"
#include <list>

//class NcPeak;


namespace nucare {

class DetectorProperty;

class PeakSearch
{
public:
    static double CalStdDevSpc(const Spectrum* doseSpc, const Threshold& thshold);

    /**
     * @brief PeakSearch_V1 Searching for K40 list peak, mostly used for calibration function with newer method
     */
    static std::list<int> PeakSearch_V1(const Spectrum* in, const FWHM& fwhm, const Coeffcients& coeff,
                                        const double thshold, const Threshold& thsholdK40,
                                        const Threshold& theshold1,
                                        const int MaxDist2Peaks = 1);

    static std::list<int> PeakSearch_Co60(const Spectrum* in, const FWHM& fwhm, const Coeffcients& coeff,
                                                       const double thshold, const Threshold& thsholdK40,
                                                       const Threshold& theshold1,
                                                       const int MaxDist2Peaks);
    /**
     * @brief PeakSearch_Hight  Searching for K40 list peak, call to @ref PeakSearch_V1 3 times, with different
     * sigma threshold to get value. => Hight performance, but more precise
     */
    static std::list<int> PeakSearch_Hight(const Spectrum* in, const FWHM& fwhm, const Coeffcients& coeff,
                                         const double thshold, const Threshold& k40Thshold);

//    static std::list<NcPeak> FindPeak_Beta(Spectrum& PPSpec, Spectrum& DChSpec, repository::DetectorProperty* prop);

    static std::array<double, 4> findVally(const Spectrum* in, int firstlocation , int secondlocation , int peakvalue);

//    static std::list<NcPeak> GetPPSpectrum_H(std::shared_ptr<Spectrum> spc,
//                                             std::shared_ptr<nucare::repository::DetectorProperty> prop);
//    static std::list<NcPeak> findPeak(NcSP<Spectrum> spc, std::shared_ptr<nucare::repository::DetectorProperty> prop);

//    static std::list<NcPeak> findPeak_Stab(Spectrum* spc, repository::DetectorProperty* prop, double ROIWND);

//    static std::list<IsotopeSummary> PeakMatchIsotope_H(std::list<NcPeak>& mFoundPeak_data, repository::DetectorProperty* prop);

//    static void IndexFillter_H(std::list<IsotopeSummary>& result2, repository::DetectorProperty* prop,
//                        const double Confiden_Index1, const	double Confiden_Index2);
//    static void IndexFillter_H_Apply(std::list<IsotopeSummary>& result2, repository::DetectorProperty* prop,
//                               const double Confiden_Index1, const	double Confiden_Index2);

    static void TransferFunct(double* out, const int len, const FWHM& fwhm, const Coeffcients& coeff);

    /**
     * @brief BGErosion
     * @param MSBinSpec Array len @BINSIZE
     * @param IterCoeff Array len 8 bytes
     * @param bgBinOut  Array len BINSIZE
     * @param TF        Array len BINSIZE
     * @param coeff     Coefficient of detector
     */
    static void BGErosion(double* MSBinSpec, const double* IterCoeff, double* bgBinOut, double* TF,
                                     const Coeffcients& coeff);

    /**
     * @brief ReturnReBinning
     * @param BinSpec   array len BINSIZE
     * @param TF        array len BINSIZE
     * @param ChOut     Output array len CHSIZE
     */
    static void ReturnReBinning(double* BinSpec, double* TF, double* ChOut);

    static void BGSubtration(double* MSChSpec, double* ReBinChSpec, Spectrum* PPChSpecOut, const SmoothP& smooth );

    static void GenDSpecrum(Spectrum& PPChSpec, Spectrum& reBinChSpec, double* DChSpecOut, DetectorProperty* prop,
                                       bool isMeasurement);

//    static void SearchROI_N(Spectrum& Spc, std::list<NcPeak>& PeakInfo, repository::DetectorProperty* prop);

//    static void NetCount_N(SPC_DATA& Spc, std::list<NcPeak>& PeakInfo, repository::DetectorProperty* prop);

//    static void BGNetCount(SPC_DATA& BGEroChSpec, std::list<NcPeak>& PeakInfo);

//    static void Calculate_LC(std::list<NcPeak>& PeakInfo);

//    static void LC_Filter(std::list<NcPeak>& PeakInfo);

//    static void NetBGSubtract_N(std::list<NcPeak>& PeakInfo, std::list<NcPeak>& PeakInfo_bg,
//                         double mstime,double bgtime, repository::DetectorProperty* prop);

//    static void LogicComptonPeakCs_Co60(std::list<IsotopeSummary>& Result2,
//                                        std::list<NcPeak>& PeakInfo,
//                                        const double wndROI,
//                                        repository::DetectorProperty* prop);

//    static void AddCondition_Cs_U233HE_U235HE(std::list<IsotopeSummary>& result);
//    static void AddCondition_Cs_U233_U235(std::list<IsotopeSummary>& result);
//    static void LogicHighEnricUranium(std::list<IsotopeSummary>& ret, std::list<NcPeak>& PeakInfo,
//                               repository::DetectorProperty* prop);
//    static void AddCondition_WGPu_RGPU(std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo,
//                                repository::DetectorProperty* prop);
//    static void Logic_Lu177_Sm153(std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo,
//                           repository::DetectorProperty* prop);
//    static void Logic_Cs137_Co67(std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo,
//                          repository::DetectorProperty* prop);

//    static void AddCondition_Exception_Isopte(const std::string&& ExpectedIso, const std::string&& UnExpectIsotope,
//                                       std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo, repository::DetectorProperty* prop,
//                                       double Acqtime, double Thrshld_Index2, double ActThshld);

//    static void CValue_Filter_H(Spectrum& smoothSpc, std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo,
//                         repository::DetectorProperty* prop, double Act_Thshld);
//    static std::list<NcPeak> CValue_Return_UnclaimedEn(Spectrum& smoothSpc, std::list<IsotopeSummary>& Result2,
//                                                std::list<NcPeak>& PeakInfo,
//                                                       double mstime, repository::DetectorProperty* prop);
//    static void IsotopeID_UnClaimedLine(Spectrum &Spc, std::list<IsotopeSummary> &Result2, std::list<NcPeak> &PeakInfo,
//                                        repository::DetectorProperty *prop);

//    static Vec1D ReListIndMax(Vec1D ListMax, Vec1D Data, double thsld);
//    static std::list<IsotopeSummary> UpdateUnclaimedResult( std::list<IsotopeSummary> result2,std::list<IsotopeSummary> Unclaimed_Result);

};

}

#endif // PEAKSEARCH_H
