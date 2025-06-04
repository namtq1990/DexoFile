#ifndef NCLIBRARY_H
#define NCLIBRARY_H

#define HALF_K40_WND 0.2

#define NO_MAX_K40 100

#include "model/Types.h"
#include "model/DetectorProp.h"
#include "model/Spectrum.h"
#include "PeakSearch.h"
#include "util/util.h"
#include <array>

namespace nucare {

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
    template<class Spectrum_t>
    static void AdaptFilter(const Spectrum_t* in, Spectrum_t* out, const FHM& fhm, const Coeffcients* coeffs = nullptr, const nucare::uint repeat = 3) {
        using Channel = typename Spectrum_t::Channel;
        bool byChannel = coeffs == nullptr;
        auto n = in->getSize();
        if (in == nullptr || out == nullptr)
            NC_THROW_ARG_ERROR("Invalid parameter for AdaptFilter_FWHM_In_Ch");

        double Ratio = 1; //=1: meaning: Peak-FWHM:Peak+FWHM
        double temp_wind0 = byChannel ? energyToFWHM(n, fhm)
                                      : channelToFWHM(n, fhm, *coeffs);
        double temp_wind = round(temp_wind0 * Ratio);
        if (temp_wind <= 0) temp_wind = 1;

        Channel data[n];

        auto dataIn = in->dataConst();
        auto dataOut = out->data();

        memcpy(data, dataIn, n * sizeof(Channel));
        memset(dataOut, 0, n * sizeof(Channel));

        for (nucare::uint nosmoo = 1; nosmoo <= repeat; nosmoo++)
        {
            if (nosmoo > 1)
            {
                memcpy(data, dataOut, n * sizeof(Channel));
            }

            double sum1 = 0;
            double wnd0 = 0;
            int wnd_half = 0;
            int wnd = 0;

            for (nucare::uint j = 3; j < n - temp_wind; j++)
            {
                sum1 = 0;
                wnd0 = byChannel ? energyToFWHM(j + 1, fhm)
                                 : channelToFWHM(j + 1, fhm, *coeffs);

                wnd = (int)(wnd0 * Ratio);

                //wnd = (int)(floor(aval * (j + 1) + bval));

                if (wnd < 0)
                {
                    dataOut[j] = data[j];
                }
                if (wnd % 2 == 0)
                {
                    wnd = wnd + 1;
                }
                wnd_half = (int)(floor(wnd / 2.0));
                nucare::uint start = std::max(0, (int) j - wnd_half);
                nucare::uint end = std::min(j + wnd_half + 1, byChannel ? n : CHSIZE);

                for (nucare::uint k = start; k < end; k++)
                {
                    sum1 += data[k];
                }

                dataOut[j] = sum1 / (double)wnd;
            }
        }
    }

    template <class Spectrum_t>
    static std::array<double, 2> FindPeak(Spectrum_t* spc, const std::array<int, 2>& peakInfo) {
        using Channel = typename Spectrum_t::Channel;
        std::array<double, 2> ret;

        try {
            auto N = spc->getSize();

            double xmax = 0, x2max = 0;
            double ymax = 0, y2max = 0;

            auto data = spc->data();
            //        double x[N];
            Channel y[N];

            double temp_ymax = 0;
            double temp_y2max = 0;

            memcpy(y, data, spc->getSize() * sizeof(Channel));

            for (int n = 5; n < 50; n++) // HH300: 32 keV:3 32 CHN
            {
                if (y[n] > ymax && y[n] > 50) // 5ch 50counts �씠�긽�씠硫댁꽌 �겙媛�
                {
                    if (y[n - 3] < y[n] && y[n + 5] < y[n]) {
                        if (n > 50) {
                            if (y[n - 3] < 0.85 * y[n] && y[n + 3] < 0.85 * y[n])
                            {

                                {
                                    // y2max=ymax;x2max=xmax;
                                    ymax = (int) y[n];
                                    xmax = n;
                                    // break;
                                }
                            }
                        } else {
                            if (y[n + 5] < 0.9 * y[n])
                            {
                                if (y[n] > temp_ymax) {
                                    // y2max=ymax;x2max=xmax;
                                    ymax = (int) y[n];
                                    xmax = n;
                                    temp_ymax = y[n];
                                    // break;
                                }
                            }
                        }
                    }
                }
            }

            //	for (int n = 480; n < 700; n++) {
            //        for (int n = peakInfo[0]; n < peakInfo[1]; n++) { TODO Move to using peakInfo
            for (int n = 200; n < 600; n++) {
                if (n > 50 && y[n] > y2max && y[n] > 50) {
                    if (y[n - 10] < y[n] && y[n + 10] < y[n]) {
                        if (y[n - 30] < 0.8 * y[n] && y[n + 30] < 0.8 * y[n])

                        {
                            if (abs(xmax - n) > 100) // 50ch
                            {
                                if (y[n] > temp_y2max) {
                                    y2max = (long) y[n];
                                    x2max = n;
                                    temp_y2max = y[n];
                                }
                            }
                        }
                    }
                }
            }

            ret[0] = (int) xmax;
            ret[1] = (int) x2max;

            return ret;
        } catch (const NcException& e) {
            nucare::logE() << "Error in FindPeak: " << e.what() << ", use empty peak instead";
            memset(ret.data(), 0, ret.size() * sizeof(Channel));
            return ret;
        }

    }

    template <class Spectrum_t>
    static int FindK40CalibCo60(Spectrum_t *spc, Coeffcients& CurPeak, DetectorProperty* prop)
    {
         //const nucare::uint N = spc->getSize();
         //Spectrum ChSpecSmoo(N);

       // nucare::log::d("Spc time: %s", spc->toString().data());

         double PeakThslod=0.1; //

         Coeffcients coeffs;
         coeffs[0] = 0;
         coeffs[1] = ((std::max(nucare::CS137_PEAK1, nucare::CS137_PEAK2) - std::min(nucare::CS137_PEAK1, nucare::CS137_PEAK2))
                      / (std::max(CurPeak.data()[0], CurPeak.data()[1]) - std::min(CurPeak.data()[0], CurPeak.data()[1])));
         coeffs[2] = nucare::CS137_PEAK1 - CurPeak.data()[0] * coeffs[1];

         //NcLibrary::AdaptFilter(spc, &ChSpecSmoo, prop->mFHM, &coeffs, 3);


         Threshold roi(NcLibrary::energyToChannel(nucare::Co60_PEAK * (1.0-nucare::Co60_WND), coeffs),
                       NcLibrary::energyToChannel(nucare::Co60_PEAK * (1.0+nucare::Co60_WND), coeffs));

         auto ListPeak = PeakSearch_Co60(spc, prop->getFWHM(), coeffs, PeakThslod, roi, roi, 10); // if correct, then 2 peaks

         if(ListPeak.size() <2) {
            NC_THROW_ALG_ERROR("Can't find 2 peak Co60 in Calibration ");
            return -1;
         }

         //convert to energy
         std::vector<double> ListEn;
         std::vector<double> ListCh;
         for (auto it = ListPeak.begin(); it != ListPeak.end(); it++)
         {
             double temp= NcLibrary::channelToEnergy(*it, coeffs);
             ListEn.push_back(temp);
             ListCh.push_back(*it);
         }

         // validate Co-60 Peak
         double Co60PeakGap=1332-1172;
         double WND=0.3;

         int count=0;
         auto it = ListEn.begin();
         double prev = *it;  // Initialize prev with the first element
         ++it;  // Move to the second element

         for (; it != ListEn.end(); ++it) {
             double curr = *it;
             double distance = std::abs(curr - prev);  // Calculate the Gap energy between 2 peaks
             count++;
             if(distance > Co60PeakGap*(1-WND) && distance< Co60PeakGap*(1+WND))
             {
                 CurPeak.data()[2]= ListCh[count];//NcLibrary::energyToChannel(*it, coeffs);
                 break;
             }
             prev = curr;  // Update prev to the current element
         }

         if(CurPeak.data()[2]==0)
         {
             NC_THROW_ALG_ERROR("Found Co60 peak but 2 peaks are not correct in Calibration ");
             return -1;
         }

         // Calclate K40 Peak
         Coeffcients stdPeaks = {nucare::CS137_PEAK1, nucare::CS137_PEAK2, nucare::Co60_PEAK};
         Coeffcients fitParam = NcLibrary::computeCalib(CurPeak, stdPeaks);

         int K40_Ch = NcLibrary::energyToChannel(nucare::K40_PEAK, fitParam);
         return K40_Ch;
    }

    static double K40EstRefEnergy(const Coeffcients& hwPeak, Coeffcients& CurPeak)
    {
        /*......Hung-2024/05/21.................................................................................
         * ................all parameter is Raw data at 2048 CHANNEL........................
         * HwPeak is 3 peaks calibration for Reference Data as selected when user select know peak for calibration  . example: hwcali=(8,380,800)
         * CurPeak is current peak from calibration. Format CurPeak=(8,350,0). K40 peak is unknown, so we need estimate
         */
        Coeffcients stdPeaks = {nucare::CS137_PEAK1, nucare::CS137_PEAK2, nucare::K40_PEAK};
        Coeffcients fitParam = NcLibrary::computeCalib(hwPeak, stdPeaks);

        double EnCs_hwcalib= NcLibrary::channelToEnergy(CurPeak.data()[1], fitParam);
        double ratio=EnCs_hwcalib/nucare::CS137_PEAK2;
        double En_K40_hwcalib=nucare::K40_PEAK*ratio;
        double K40Est= NcLibrary::energyToChannel(En_K40_hwcalib,fitParam);

        //update Calibration
        return K40Est;
    }

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

    template <class Spectrum_t>
    static std::list<int> PeakSearch_Co60(const Spectrum_t* in, const FWHM& fwhm, const Coeffcients& coeff,
                                          const double thshold, const Threshold& thsholdK40,
                                          const Threshold& theshold1,
                                          const int MaxDist2Peaks)
    {
        // TODO I don't like this idea when allocate too much this array size, change it pls
        int peaklist[NO_MAX_K40] = {0};
        int templist[NO_MAX_K40] = {0};
        auto data = in->dataConst();
        std::list<int> ret;

        // Finding Max value
        int IndexMax = nucare::indexOfMax(data, theshold1.first, theshold1.second);

        double MAX = data[IndexMax];

        double PeakAvg = (theshold1.first + theshold1.second) / 2.0;

        double FWHM_Peak= NcLibrary::channelToFWHM(PeakAvg, fwhm, coeff);

        //%% To >95 % density, then - 2sigma to 2 sigma.
        //Reference link: https://www.mathsisfun.com/definitions/standard-normal-distribution.html

        double PeakMinWidth = 4.0 / 2.355 * FWHM_Peak;  //%% To >95 % density, then - 2sigma to 2 sigma.

        //double PeakMinWidth = 40;

        double halfWidth = (PeakMinWidth / 2.0) * 0.8;
        double quarterWidth = halfWidth / 2.0;

        //        int ch1 = (int) round(halfWidth);
        int ch2 = (int) round(halfWidth - quarterWidth / 2.0);
        int ch3 = (int) round(quarterWidth);
        int ch4 = (int) round(quarterWidth - quarterWidth / 2.0);

        int peakCount = 0;

        double thshold_val = thshold * MAX;

        for (int  i = (int)theshold1.first; i <=(int) theshold1.second; i++)
        {
            if (data[i] > thshold_val)
            {
                if (data[i - ch2] < data[i - ch3] && data[i + ch2] < data[i + ch3])
                {
                    if (data[i - ch3] < data[i - ch4] && data[i + ch3] < data[i + ch4])
                    {
                        if (data[i - ch4] < data[i] && data[i + ch4] < data[i])
                        {
                            if (peakCount < NO_MAX_K40)
                            {
                                templist[peakCount] = i;
                                peakCount = peakCount + 1;
                            }
                        }
                    }
                }
            }
        }



        // Select For True Peak Energy
        //Modify function in here

        int realPeakCount=0;

        int ind = 0;

        double maxval = 0;
        int maxch = 0;
        int index0 = 0;

        for (int i = 0; i < peakCount ; i++)
        {
            //maxch = templist[i];
            //maxval = ChSpec[maxch];

            index0 = templist[i];
            maxval = data[index0];
            maxch = i;


            if(i>0 && i<=ind-1)
            {
                continue;
            }

            int j=0;

            for (j = i + 1; j < peakCount - 1; j++)
            {
                if (abs((long) (templist[j] - templist[j + 1])) <= MaxDist2Peaks)
                {
                    index0 = templist[j];

                    if (maxval < data[index0])
                    {
                        maxval = data[index0];
                        maxch = j;
                    }
                }
                else
                {
                    break;
                }
            }
            ind = j + 1;

            peaklist[realPeakCount] = templist[maxch];
            realPeakCount = realPeakCount + 1;

            if (ind == peakCount-1)
            {
                break;
            }
        }

        //
        if (realPeakCount > 0)
        {
            for (int i = 0; i < realPeakCount; i++)
            {
                ret.push_back(peaklist[i]);
            }

        }



        return ret;
    }
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
