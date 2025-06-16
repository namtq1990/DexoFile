#include "NcLibrary.h"
#include "PeakSearch.h"

#include <cstring>
#include <math.h>
#include <algorithm>
using namespace nucare;
using namespace std;
using namespace nucare::math;

const QString TAG = "NcLibrary";

double nucare::NcLibrary::channelToEnergy(const double channel, const double *params)
{
    if (params == nullptr) {
        NC_THROW_ARG_ERROR("Coeff params is invalid. It's NULL");
    }

    return math::quadratic(channel, params);
}

double NcLibrary::channelToEnergy(const double channel, const Coeffcients &params)
{
    return channelToEnergy(channel, params.data());
}

double NcLibrary::energyToChannel(const double energy, const double *params)
{
    if (params == nullptr) {
        NC_THROW_ARG_ERROR("Coeff params is invalid. It's NULL");
    }

    try {
        if (params[0] == 0)
            return -(params[2] - energy) / params[1];

        return math::quadraticEquation(params[0], params[1], params[2] - energy);
    }  catch (NcException& e) {
        nucare::logE() << TAG << "Energy for channel " << energy << " couldn't be resolved";
        return 0;
    }
}

double NcLibrary::energyToChannel(const double energy, const Coeffcients &params)
{
    return energyToChannel(energy, params.data());
}

/**
 * @brief NcLibrary::computeGEFactor        factor0 + log10(energy) * factor1 + log10(energy) ^ 2 * factor2;
 * @param energy
 * @param geCoef
 * @param length
 * @return
 */
double NcLibrary::computeGEFactor(double energy, const double *geCoef, const int& )
{
    energy = max(energy, 1.0);
       double sum = 0;
      // for (int i = 0; i < length; i++) {
         //  sum += geCoef[i] * pow(log10(energy), i);
      // }
       double temp1=log10(energy);
        sum = geCoef[0] * pow(temp1, 0)
                + geCoef[1] * pow(temp1, 1)
                + geCoef[2] * pow(temp1, 2)
                + geCoef[3] * pow(temp1, 3)
                + geCoef[4] * pow(temp1, 4)
                + geCoef[5] * pow(temp1, 5);
       return max(sum, 0.0);
}

//double NcLibrary::computeDoserate(std::shared_ptr<Spectrum> spc, const Coeffcients &coeffs, const GeCoefficients &geCoeffs)
//{
//    if (spc == nullptr) return 0;

//    if (coeffs.size() < 3) {
//        NC_THROW_ARG_ERROR("Coeff params size is < 3");
//    }
//    if (geCoeffs.size() < 3) {
//        NC_THROW_ARG_ERROR("GeCoeff params size is < 3");
//    }

//    double dose = 0;
//    auto size = spc->getSize();
//    auto p = spc->data();
//    for (nucare::uint i = 0; i < size; i++) {
//        auto count = *p;
//        auto energy = NcLibrary::channelToEnergy(i, coeffs.data());
//        auto geFactor = NcLibrary::computeGEFactor(energy, geCoeffs.data(), geCoeffs.size());
//        dose += count * geFactor;
//        p++;
//    }

//    return to_nSv(dose, USV);
//}

//double NcLibrary::GM_to_nSV(const double GM_count)
//{
//    /* Hung- 23.10.16
//        Convert Pin Diode count to Dose rate
//        Equation: Dose (nSv/h)= 10^[ a * x + b ];
//        a & b: parameter
//        x = log10(CPS)

//    Dose return unit: nSv/h
//                           */

//                       // Coeficient from Pin diode to Dose rate
//                       //    double a = 1.031453179;
//                       //    double b = 4.9967076279;
//                       //  double a = 0.931184944;
//                       //   double b = 4.933084946;

//    double dose = 0;

//    if (GM_count > 0)
//    {

//#ifdef APP_HH300
//        double x = GM_count;
//        double temp = nucare::math::quartic(x, nucare::config::GM_PI_PARAM_LOW);
//        dose = temp * 1000; //convet unit uSv/h  to nSv/h
//        if(dose > nucare::config::DOSE_PIN_THRSHLD) {
//            temp = nucare::math::quartic(x, nucare::config::GM_PI_PARAM_HIGH);
//            dose = temp * 1000; //convet unit uSv/h  to nSv/h
//        }

//#else

//        double x = GM_count;
//        double temp = nucare::math::quadratic(x, nucare::config::GM_PI_PARAM);
//        dose = temp * 1000; //convet unit uSv/h  to nSv/h
//#endif
////        dose = pow(10, temp);
//    }

//    return dose;
//}

//double NcLibrary::DoseCSI_Factor_nSV(const double csi_dose_nsv)
//{
//    /* Dose CSI with Factor
//     */
////    double a = 0.0000041913;
////    double b = 0.000521958;
////    double c = 1.0293607193;
//    double a = nucare::config::DOSE_CSI_PARAM[0];
//    double b = nucare::config::DOSE_CSI_PARAM[1];
//    double c = nucare::config::DOSE_CSI_PARAM[2];

//    double uSv_dose = csi_dose_nsv / 1000;

//    double factor= a * uSv_dose * uSv_dose + b * uSv_dose + c;
//    if (factor < 1) factor = 1;

//    double dose = csi_dose_nsv*factor;
    

//    return dose;
//}

void NcLibrary::Nomalization(Spectrum& spc, Spectrum* bgr, Spectrum::SPC_DATA& out) {
    const auto size = spc.getSize();

    if (!bgr || bgr->getAcqTime() <= 0) {
        memcpy(out, spc.data(), size * sizeof(double));
        return;
    }

    auto acqTime = spc.getAcqTime();
    auto bgrAcqTime = bgr->getAcqTime();

    for (nucare::uint i = 0; i < size; i++) {
        out[i] = std::max(spc[i] - (((*bgr)[i] / bgrAcqTime) * acqTime), 0.0);
    }
}

//double NcLibrary::getIsotopeDoserate(Spectrum& spc, Spectrum* BG_Data,
//                                      IsotopeSummary* Target,
//                                       shared_ptr<repository::DetectorProperty> prop,
//                                       std::shared_ptr<Spectrum> normSPC) {
//    if (normSPC == nullptr) {
//        normSPC = make_shared<Spectrum>();
//        NcLibrary::Nomalization(spc, BG_Data, normSPC->data());
//    }
//    shared_ptr<Spectrum> tempSPC = make_shared<Spectrum>(spc.getSize());

//    for (auto peak : *Target->foundPeaks) {
//        int Vally_Start = max(peak.ROI_Left, 0);
//        int Vally_End = peak.ROI_Right;

//        for (int i = Vally_Start; i <= Vally_End; i++) {
//            if (Vally_Start == 0 && Vally_End == 0)
//                break;

//            auto norm = normSPC->data()[i];
//            double a = (float) ((spc[Vally_Start] - spc[Vally_End]) / (Vally_Start - Vally_End));
//            double b = (float) (spc[Vally_Start] - (a * Vally_Start));
//            auto en = NcLibrary::channelToEnergy(i, {0, a, b});
//            tempSPC->data()[i] = max(floor((norm - en)), 0.0);
//        }
//    }

//    return NcLibrary::computeDoserate(tempSPC, prop->getCoeffcients(), prop->getGeCoeffcients());
//}

//void NcLibrary::quantitativeAnalysis(Spectrum&& spc, Spectrum* bg,
//                                     list<std::shared_ptr<IsotopeSummary>>& result,
//                           shared_ptr<nucare::repository::DetectorProperty> prop) {
//    if (result.size() == 1) {
//        if (result.back()->clazz == Class::UNKNOWN
//            || result.back()->clazz == Class::NOT_ID)
//            return;
//    }
//    if (!result.empty()) {
//        auto normSPC = make_shared<Spectrum>(spc.getSize());
//        Nomalization(spc, bg, normSPC->data());

//        for (auto& isotope : result) {
//            isotope->doseRate = getIsotopeDoserate(spc, bg, isotope.get(), prop, normSPC);
//            isotope->doseRate /= spc.getAcqTime();
//        }
//    }
//}

Coeffcients NcLibrary::computeCalib(const Coeffcients &chPeaks,
                                    const Coeffcients &enPeaks)
{
    auto size = chPeaks.size();

    // Find fit param to match with CS137 Peak and K40 peak
    auto matrix = Matrix(new double[size * size] {
        chPeaks[0] * chPeaks[0], chPeaks[0], 1,
        chPeaks[1] * chPeaks[1], chPeaks[1], 1,
        chPeaks[2] * chPeaks[2], chPeaks[2], 1
    });
    Matrix rev;
    matrix.revert(rev);

    Coeffcients coef;
    auto tmp = rev.multiply(enPeaks.data(), size);
    memcpy(coef.data(), tmp, sizeof(double) * size);

    delete[] tmp;

    return coef;
}

double NcLibrary::calibConvert(const double *in, double *out, const int& iChSize, const int& oChSize)
{
    double ratio = 0;
    auto energyRange = 3000.0;
    auto curRange = channelToEnergy(iChSize, in);
    if (curRange <= energyRange) {
        ratio = iChSize / oChSize;
    } else {
        double ch_3mev = energyToChannel(energyRange, in);
        ratio = ch_3mev / oChSize;
    }

    ratio = max(1.0, ratio);

    out[0] = in[0] * ratio * ratio;
    out[1] = in[1] * ratio;
    out[2] = in[2];

    return ratio;
}

void NcLibrary::convertSpectrum(const double *in, const int inSize, double *out, const int outSize, const double ratio)
{
    fill(out, out + outSize, 0);
    if(ratio > 1) {
        bool First = true;
        double sum1 = 0, sum2 = 0, z_flt = 0, z_flt_pre = 0, ztmp = 0;
        int ind_chn = 0;

        int z_int = 0;
        for (int i = 0; i < outSize; i++) {
            out[i] = 0;

            if (ratio < 1) {
                sum1 = sum1 + ratio;

                if (sum1 > 1) {
                    ind_chn = ind_chn + 1;
                    sum2 = sum1 - 1;
                    out[i] = (ratio - sum2) * in[ind_chn - 1] + sum2 * in[ind_chn];
                    sum1 = sum2;
                } else {
                    sum2 = ratio;
                    out[i] = sum2 * in[ind_chn];
                }

            } else if (ratio > 1) {
                if (First == true) {
                    First = false;
                    z_int = (int) floor(ratio);
                    z_flt = ratio - z_int;

                    if (ind_chn + z_int + 1 > inSize - 1) {
                        break;
                    }

                    sum1 = 0;
                    for (int j = 1; j <= z_int; j++) {
                        sum1 = sum1 + in[ind_chn + j];
                    }

                    sum1 = sum1 + z_flt * in[ind_chn + z_int + 1];

                    ind_chn = ind_chn + z_int + 1;
                    z_flt_pre = 1 - z_flt;
                } else {
                    ztmp = ratio - z_flt_pre;

                    z_int = (int) floor(ztmp);

                    if (z_int >= 1) {
                        z_flt = ztmp - z_int;

                        if (ind_chn + z_int + 1 > inSize - 1) {
                            break;
                        }

                        sum1 = z_flt_pre * in[ind_chn];

                        for (int j = 1; j <= z_int; j++) {
                            sum1 = sum1 + in[ind_chn + j];
                        }

                        sum1 = sum1 + z_flt * in[ind_chn + z_int + 1];

                        ind_chn = ind_chn + z_int + 1;
                        z_flt_pre = 1 - z_flt;

                    } else {
                        z_flt = ratio - z_flt_pre;

                        if (ind_chn + z_int + 1 > inSize - 1) {
                            break;
                        }

                        sum1 = z_flt_pre * in[ind_chn] + z_flt * in[ind_chn + 1];
                        z_flt_pre = 1 - z_flt;
                        ind_chn = ind_chn + 1;
                    }
                }
                out[i] = sum1;
            } else if (ratio == 1) {
                if (ind_chn < inSize) {
                    out[i] = in[ind_chn];
                    ind_chn = ind_chn + 1;
                }

            }
        }
    }
    else
    {
        if(outSize < inSize)
        {
            for (int i = 0; i < outSize;i++)
            {
                out[i] = in[i];
            }
        }

    }
}

std::vector<int> NcLibrary::findNoPeak(Spectrum& spc, Threshold& roi, double value) {
    std::vector<int> ylist;
    int wnd = 30;

    // Find the maximum value in the specified range
    double max1 = 0;
    for (int i = roi.first; i <= roi.second; ++i) {
        if (spc[i] > max1) {
            max1 = spc[i];
        }
    }

    // Iterate through the range and find peaks
    for (int i = roi.first; i <= roi.second; ++i) {
        if (spc[i] > max1 * value) {
            bool isPeak = true;

            // Check the neighborhood of the current index
            for (int j = -wnd; j <= wnd; ++j) {
                int neighborIndex = i + j;
                if (neighborIndex >= 0 && neighborIndex < static_cast<int>(spc.getSize())) {
                    if (spc[i] < spc[neighborIndex]) {
                        isPeak = false;
                        break;
                    }
                }
            }

            if (isPeak) {
                ylist.push_back(i);
            }
        }
    }

    return ylist;
}

int NcLibrary::findClosestToK40Est(const std::vector<int>& peaks, int reference) {
    // Initialize the closest peak to the first element
    int closest = peaks[0];
    int minDifference = std::abs(peaks[0] - reference);

    // Iterate through the peaks to find the closest one
    for (int peak : peaks) {
        int difference = std::abs(peak - reference);
        if (difference < minDifference) {
            closest = peak;
            minDifference = difference;
        }
    }

    return closest;
}

void NcLibrary::doseSpcConvert(const Spectrum *ChSpec, Spectrum *out, const Coeffcients &coeff, const GeCoefficients &geCoef)
{
    if (ChSpec == nullptr || out == nullptr || ChSpec->getSize() != out->getSize()) {
        NC_THROW_ARG_ERROR("doseSpcConvert Invalid parameter");
    }

    const int N = ChSpec->getSize();
    auto in = ChSpec->dataConst();
    auto ret = out->data();

    double en;
    double GEtemp = 0; //GE factor
    for (int i = 0; i < N ; i++)
    {
        en = channelToEnergy(i, coeff);
        GEtemp = computeGEFactor(en, geCoef.data(), geCoef.size());
        ret[i] = in[i] * GEtemp;
    }

}

void NcLibrary::doseSpcConvert(const Spectrum *ChSpec, Spectrum *out, DetectorProperty *prop)
{
    return doseSpcConvert(ChSpec, out, prop->getCoeffcients(), prop->getGeCoeffcients());
}

void NcLibrary::weightSpc(const Spectrum *in, Spectrum *out, int wf)
{
    if (!in || !out || in->getSize() != out->getSize()) NC_THROW_ARG_ERROR("weightSpc: Input invalid");

    nucare::uint size = in->getSize();
    auto dataIn = in->dataConst();
    auto dataOut = out->data();
    for (nucare::uint i = 0; i < size; i++) {
        dataOut[i] = dataIn[i] * pow(i, wf);
    }
}

double NcLibrary::fwhm_eff(Spectrum *smoothSpc, int Peak_Channel, const Coeffcients &coeffs, bool Energy)
{
    try{
        auto Smoothed_ChArray = smoothSpc->data();
        int LeftMax = Peak_Channel;
        int RightMax = Peak_Channel;

        int OriMax = Peak_Channel;
        int HV = (int) (Smoothed_ChArray[Peak_Channel] / 2);

        int count=0;

        int tempy0=0;

        int tempy1=0;

        double tempdev1=0.0;
        double tempdev2=0.0;

        while(Smoothed_ChArray[LeftMax] >= HV) {
            if(LeftMax==0) return 0;
            LeftMax--;count++;
            tempy0=(int) Smoothed_ChArray[LeftMax];
            tempy1=(int) Smoothed_ChArray[LeftMax+1];

        }
        if ((tempy0-tempy1) != 0)
            tempdev1= LeftMax + (double)(abs(HV-tempy0)/(double)abs(tempy0-tempy1));
        else
            tempdev1=LeftMax;



        while(Smoothed_ChArray[RightMax] >= HV)
        {
            if(RightMax == smoothSpc->getSize()) return 0;
            RightMax++;count++;
            tempy0=(int) Smoothed_ChArray[RightMax];
            tempy1=(int) Smoothed_ChArray[RightMax-1];


        }
        if((tempy0-tempy1) !=0)
            tempdev2=(double)(RightMax-1)+(double)abs(HV-tempy1)/(double)abs(tempy0-tempy1);
        else
            tempdev2=RightMax;

        double a1 = channelToEnergy(tempdev2, coeffs);
        double a2 = channelToEnergy(tempdev1, coeffs);
        double a3 = channelToEnergy(OriMax, coeffs);

        if(Energy == true)
            return ((a1 - a2) / a3) * 100.0;
        else return ((tempdev2 - tempdev1) / OriMax) * 100;
    } catch(const NcException& e){
        nucare::logE() << "NcLibrary exception in FWHM " << e.what();
        return 0;
    }

}

void convertSpectrum(const double *in, const int inSize, double *out, const int outSize, double(*ratioFor)(int))
{
    fill(out, out + outSize, 0);
    int start = 0;

    for (int i = 0; i < outSize; i++) {
        // Find the mapped range of this index in IN spectrum
        auto ratio = ratioFor(i);
        auto range = pair<double, double>(start,
                                          min(start + ratio, (double) inSize));
        int start = ceil(range.first);

        if (start >= inSize) {
            break;
        }

        int end;
        for (end = start + 1; end < range.second; end++) {
            out[i] += in[end];
        }

        // Compute linear value of Range.start and Range.end
        out[i] += (start - range.first) * in[start];
        out[i] += (range.second - end + 1) * in[end];
    }
}

double NcLibrary::channelToFWHM(const double channel, const FWHM& fwhm, const Coeffcients& coeff)
{
    double energy = max(channelToEnergy(channel, coeff.data()), 1.0);
    return energyToFWHM(energy, fwhm);
}

double NcLibrary::energyToFWHM(const double energy, const FWHM &fwhm)
{
    return fwhm[0] * sqrt(energy) + fwhm[1];
}

double NcLibrary::Get_Roi_window_by_energy(double energy) {

    if (energy > 2200) {
        return 6.5;
    } else
        return (72 * pow(energy, -0.43));// *1.7;
}

pair<double, double> NcLibrary::Get_Roi_window_by_energy_used_FWHM(double en, const FWHM& FWHMCoeff,
                                                                   const Coeffcients& coeff, const double Ratio)
{
    //double Ratio=0.6;


    double Ch = energyToChannel(en, coeff.data());

    //double FWHM=FWHMCoeff[0]*Math.sqrt(Ch)+FWHMCoeff[1];

    double FWHM= channelToFWHM(Ch, FWHMCoeff, coeff);

    double ROI_L = Ch - Ratio * FWHM;
    double ROI_R = Ch + Ratio * FWHM;

    double ROI_L_En = channelToEnergy(ROI_L, coeff.data());
    double ROI_R_En = channelToEnergy(ROI_R, coeff.data());

    pair<double, double> Thshold;
    Thshold.first = ROI_L_En;
    Thshold.second = ROI_R_En;

    //Window for enegy thrshold Th232 at 2615 keV
    if(en == 2615) {
        double Wnd=0.05; // +/- 4%
        Thshold.first = en * (1.0 - Wnd);
        Thshold.second = en * (1.0 + Wnd);
    } else if(en == 60) {
        ROI_L = ROI_L - 3;
        ROI_R = ROI_R + 3;

        ROI_L_En = channelToEnergy(ROI_L, coeff.data());
        ROI_R_En = channelToEnergy(ROI_R, coeff.data());

       // nucare::log::d("Hung Am-241:Left: %f, Right: %f\n",ROI_L_En, ROI_R_En);

        Thshold.first = ROI_L_En;
        Thshold.second = ROI_R_En;
    } else if (en == 123) {
        double Wnd = 0.075; // +/- 15%
        Thshold.first = en * (1.0 - Wnd);
        Thshold.second = en * (1.0 + Wnd);
    }

    return Thshold;
}

std::pair<double, double> NcLibrary::Get_Roi_window_by_energy_used_FWHM(double en, DetectorProperty *prop)
{
    return Get_Roi_window_by_energy_used_FWHM(en, prop->getFWHM(), prop->getCoeffcients(), prop->getWndROI());
}

int NcLibrary::ROIAnalysis(Spectrum *spc, const Threshold& roiThshold)
{
    ///////////
    try {
        nucare::uint N = spc->getSize();
        auto data = spc->data();
        double MAX = 0;
        double AVG = 0;
        double SUM = 0;
        double Base_Pyuncha = 0;
        list<double> roiSPC;
        list<double> Pyuncha;

        for (int i = roiThshold.first; i <= roiThshold.second; i++) {
            roiSPC.push_back(data[i]);
            SUM += data[i];
            if (data[i] > MAX) {
                MAX = data[i];
            }
        }
        AVG = SUM / roiSPC.size();

        for (int i = roiThshold.first; i <= roiThshold.second; i++) {
            double bunsan_per = ((AVG - data[i]) / MAX) * 100;
            Pyuncha.push_back(bunsan_per * bunsan_per);

            Base_Pyuncha += bunsan_per * bunsan_per;
        }
        AVG = Base_Pyuncha / roiSPC.size();
        Base_Pyuncha = sqrt(AVG);

        if (Base_Pyuncha < 17)
            return 0;
        /////////
        double sst[N];
        memcpy(sst, data, sizeof(sst));

        double y2max = 0;
        double xmax = 0;
        double temp_y2max = 0;
        double x2max = 0;

        for (int n = roiThshold.first; n < roiThshold.second; n++) {
            if (sst[n] > y2max)// && Smoothed_ChArray[n]>50)
            {
                if (sst[n - 10] < sst[n] && sst[n + 10] < sst[n]) {
                    if (sst[n - 30] < 0.8 * sst[n] && sst[n + 30] < 0.8 * sst[n]) // 洹쇱쿂�쓽
                        // 媛믩뱾�씠
                        // �뵾�겕濡�
                        // 遺��꽣
                        // 湲됯꺽�엳
                        // 媛먯냼�븯�뒗吏�
                        // �뙋�떒
                    {
                        if (abs(xmax - n) > 100) // 50ch 洹쇱쿂�쓽 �뵾�겕�뱾��
                            // �씤�젙�븯吏� �븡�쓬
                        {
                            if (sst[n] > temp_y2max) {
                                y2max = sst[n];
                                x2max = n;
                                temp_y2max = sst[n];
                            }
                        }
                    }
                }
            }
        }

        return round(x2max);

    } catch (const NcException& e) {
        nucare::logE() << e.what();
        auto ncEx = NcException(ErrorCode::UnknownError, "Exception while execute ROIAnalysis", "", &e);
        throw ncEx;
    }
}

double NcLibrary::CalcROIK40(const Spectrum* spc, FWHM& fwhm, Coeffcients& coeffs) {
    if (spc) {
        if (spc->getSize() != CHSIZE) {
            NC_THROW_ARG_ERROR("SPC .length!=CHSIZE:CalcROIK40");
        } else {
            const int N = spc->getSize();
            Spectrum ChSpecSmoo;

            double En_theshold1 = (double) K40_PEAK * (1.0 - HALF_K40_WND);
            double En_thshold2 = (double) K40_PEAK * (1.0 + HALF_K40_WND);
            double theshold1 = (int) (energyToChannel(En_theshold1, coeffs));
            double theshold2 = (int) energyToChannel(En_thshold2, coeffs);

            AdaptFilter(spc, &ChSpecSmoo, fwhm, &coeffs, 3);
            double SumCnt = 0.0;
            if (theshold2 > (double)(N - 1)) {
                theshold2 = (double)(N - 1);
            }

            if (theshold1 > (double)(N - 2)) {
                theshold1 = (double)(N - 2);
            }
//            theshold1 = max(0.0, theshold1);

            for(int i = (int)theshold1; i <= (int)theshold2; ++i) {
                SumCnt += ChSpecSmoo[i];
            }

            return SumCnt;
        }
    } else {
        NC_THROW_ARG_ERROR("CalcROIK40 SPC is NULL");
    }
}

int NcLibrary::FindK40DoseSpc_CALIB(Spectrum *spc, const Coeffcients &coeff,
                                    const FWHM &fwhm, const GeCoefficients &GECoef,
                                    const double roi1, const double roi2, const double RsqrThsld)
{
    int peakK40 = -1;

    //Default parameter for K40 finding Peak of HH300
    //Author: Hung
    double	SignalMin = 0.25;  //minimum signal
    double	ThesholdPeak = 0.5;//minimum thsold for signal

    //Step 0: Get Spectrum information
    const nucare::uint N = spc->getSize();
    Spectrum ChSpecSmoo;
    Spectrum WChSpec;
    Spectrum WChSpecSmoo;

    // Define ROI for K40
    double En_theshold1 = K40_PEAK * (1.0 - roi1);
    double En_thshold2 = K40_PEAK * (1.0 + roi1);

    Threshold theshold1 = Threshold(round(energyToChannel(En_theshold1, coeff)),
                                    round(energyToChannel(En_thshold2, coeff)));
    theshold1.first = max(min(theshold1.first, (double) N - 1), 0.0);
    theshold1.second = max(min(theshold1.second, (double) N - 1), theshold1.first);


    // Define ROI for 10%
    double En_low10 = K40_PEAK * (1.0 - roi2);
    double En_high10= K40_PEAK * (1.0 + roi2);

    Threshold thresholdLow10(round(energyToChannel(En_low10, coeff)),
                             round(energyToChannel(En_high10, coeff)));

    // Step 1: Convert to Dose Spectrum
    //ChSpecSmoo = DoseSpcConvert(ChSpec, ChSpecSmoo, Coeff, GE);
    //ChSpecSmoo = ADapt(ChSpec, ChSpecSmoo, Coeff, GE);
    AdaptFilter(spc, &ChSpecSmoo, fwhm, &coeff, 3);

    int indexSpc = indexOfMax(ChSpecSmoo.dataConst(), (nucare::uint) theshold1.first, (nucare::uint) theshold1.second);

    double MaxSpec = ChSpecSmoo.data()[indexSpc];

    if (MaxSpec > SignalMin)
    {
        // Step 1: Dose Spectrum Convert USING ORIGINAL SPECTRUM
        //WChSpec = WeightSpc(ChSpecSmoo, WChSpec, 1);
        doseSpcConvert(spc, &WChSpec, coeff, GECoef);

        //Step 3: Regular Filteringfor Weighted Spectrum
        //WChSpecSmoo = RegularFlt(WChSpec, WChSpecSmoo, FixWndSmoo, 3);
        AdaptFilter(&WChSpec, &WChSpecSmoo, fwhm, &coeff, 3);
        //Step 4: Find Peak

        auto K40FindResult = GauFitFunct_Dose(&WChSpecSmoo, fwhm, coeff, ThesholdPeak, theshold1, thresholdLow10);

        for (int i = 0; i < 2; i++)
        {
            if (K40FindResult[0] > 0
                    && K40FindResult[1] > RsqrThsld)
            {
                peakK40 = (int) K40FindResult[0];
            }
        }
    }

    if(peakK40 < theshold1.first || peakK40> theshold1.second)
        peakK40 = -1;

    return peakK40;
}

int NcLibrary::FindK40DoseSpc_Hight(Spectrum *spc, const Coeffcients &coeff, const FWHM &fwhm, const double roi1)
{

    if (spc == nullptr) {
        NC_THROW_ARG_ERROR("FindK40DoseSpc_LOW: Invalid parameter");
    }

    int peakK40 = -1;

    //Default parameter for K40 finding Peak of HH300
    //Author: Hung
    double	SignalMin = 0.25;  //minimum signal
    double	ThesholdPeak = 0.5;//minimum thsold for signal

    //Step 0: Get Spectrum information
    const nucare::uint N = spc->getSize();
    Spectrum ChSpecSmoo;
    Spectrum WChSpec;
    Spectrum WChSpecSmoo;

    // Define ROI for K40
    double En_theshold1 = K40_PEAK * (1.0 - roi1);
    double En_thshold2 = K40_PEAK * (1.0 + roi1);

    Threshold theshold1 = Threshold(round(energyToChannel(En_theshold1, coeff)),
                                    round(energyToChannel(En_thshold2, coeff)));
    theshold1.first = max(min(theshold1.first, (double) N - 1), 0.0);
    theshold1.second = max(min(theshold1.second, (double) N - 1), theshold1.first);

    // Step 1: Adaptive Filtering for orignal Spectrum
    AdaptFilter(spc, &ChSpecSmoo, fwhm, &coeff, 3);

    int indexSpc = indexOfMax(ChSpecSmoo.dataConst(), (nucare::uint) theshold1.first, (nucare::uint) theshold1.second);

    double MaxSpec = ChSpecSmoo[indexSpc];

    if (MaxSpec > SignalMin)
    {
        // Step 2: Weighted Spectrum
        weightSpc(&ChSpecSmoo, &WChSpec, 1);


        //Step 3: Adapvtive Filteringfor Weighted Spectrum
        AdaptFilter(&WChSpec, &WChSpecSmoo, fwhm, &coeff, 3);

        //Step 4: Find Peak


        //Step 4.2: Get Max valude of  Weighted signal inside K40 ROI
//        int indexWSpc= util::indexOfMax(WChSpecSmoo.dataConst(), theshold1.first, theshold1.second);

//        double MaxWSpc = WChSpecSmoo[indexWSpc];

        // Step 4.3: Condition for finding K40 peak

        //Step 4.3.1: 1st Condition
        auto listK40 = PeakSearch::PeakSearch_Hight(&WChSpecSmoo, fwhm, coeff, ThesholdPeak, theshold1);

        //Step 4.3.2: 2nd Condition, finding only 1 peak

        if (!listK40.empty())
        {
            peakK40 = listK40.front();
        }
    }

    return peakK40;
}

int NcLibrary::K40FinderByGaussFitting_Dose(Spectrum *spc, DetectorProperty *prop)
{
    double ROIWnd = 0.2; // Standard Peak In 20%
    double ROIWnd2 = 0.1; //
    int K40_Ch = FindK40DoseSpc_CALIB(spc, prop->getCoeffcients(), prop->getFWHM(), prop->getGeCoeffcients(),
                                      ROIWnd, ROIWnd2);

    if(K40_Ch == -1) {
        /*
        //try again'
        ROIWnd = 0.2; //Searching in 10%
        ROIWnd2 = 0.2;
        int K40_Ch = FindK40DoseSpc_CALIB(spc, prop->getCoeffcients(), prop->getFWHM(), prop->getGeCoeffcients(),
                                          ROIWnd, ROIWnd2);
        if (K40_Ch == -1) {
            K40_Ch = FindK40DoseSpc_Hight(spc, prop->getCoeffcients(), prop->getFWHM());
        }
*/
    }

    if(K40_Ch <= 0 || K40_Ch > (int) spc->getSize()) K40_Ch = -1;

    return  K40_Ch;
}

std::array<double, 2> NcLibrary::GauFitFunct_Dose(Spectrum *doseSpc, const FWHM &fwhm, const Coeffcients &coeff,
                                                  const double PeakThslod,
                                                  const Threshold& chDis20,
                                                  const Threshold& roi)
{
    array<double, 2> ret;
    auto in = doseSpc->data();


    //Step #1: Finding K40 Peak
    // Maximum between 2 continues peak is 10
    auto ListK40 = PeakSearch::PeakSearch_V1(doseSpc, fwhm, coeff, PeakThslod, chDis20, roi, 10);
    const int CntPeak = ListK40.size();

    double RSqr = 0, Peak = 0;

    if (ListK40.size() > 0)
    {
        double PeakG[CntPeak];
        double RSqrG[CntPeak];

        memset(PeakG, 0, sizeof(PeakG));
        memset(RSqrG, 0, sizeof(RSqrG));

        double H = 0, MIU=0, Sig = 0; // SIngle Gaussian Peak
        double A = 0, B = 0; // BG Estimate
        constexpr int NoPara = 5 + 1; // Number para estimate
        int PeakTemp1 = 0;

        array<double, NoPara> ParaFit;
        int i = 0;

        for (auto it = ListK40.begin(); it != ListK40.end(); it++)
        {
            MIU = *it;

            PeakTemp1 = (int) MIU;

            H = in[PeakTemp1];

            //Get Data from Thsold1 & Thsld2
            int thslod1 = (int)(MIU * 0.8);
            int thslod2 = (int)(MIU * 1.2);

            if(thslod1 <= 10) thslod1 = (int)roi.first;
            if(thslod2 > (int) CHSIZE - 1) thslod2 = roi.second;
            if(thslod1 >= thslod2)
            {
                thslod1 = (int) roi.first;
                thslod2 = (int) roi.second;
            }

            int NoPtFit = (int)( thslod2 - thslod1) + 1;
            double X[NoPtFit];
            double Y[NoPtFit];

            for (int i1 = (int)thslod1; i1 <= (int)thslod2; i1++)
            {
                X[i1- thslod1] = i1;
                Y[i1 - thslod1] = in[i1];
            }


            //Sig = (FWHM[0] * Math.sqrt(MIU) + FWHM[1]) / 2.355;
            Sig = channelToFWHM(MIU, fwhm, coeff) / 2.355;

            A = (in[thslod1] - in[thslod2]) /(double)(thslod1 - thslod2);

            B = in[thslod1] - A * thslod1;

            GaussFit_H_G1(X, Y, NoPtFit, H, MIU, Sig, A, B, ParaFit);

            //
            //				PeakG[i] = ParaFit[1];
            PeakG[i] = MIU;// 2021/08.30: Get true peak because get peak from gauss shifting 1-2 channel
            RSqrG[i] = ParaFit[5];
            i++;
        }


        //finding Maximum
        int indexMax = 0;
        double RSqrMax = 0;
        for (int i = 0; i < CntPeak; i++)
        {
            if (RSqrG[i] > RSqrMax)
            {
                RSqrMax = RSqrG[i];
                indexMax = i;
            }
        }
        Peak = PeakG[indexMax];
        RSqr = RSqrG[indexMax];
    }

    ret[0] = Peak;
    ret[1] = RSqr;

    return ret;
}

void NcLibrary::GaussFit_H_G1(double *X, double *Y, int NoPtFit, double a1, double b1, double c1,
                              double A, double B,
                              std::array<double, 6>& out)
{
    double dfda1[NoPtFit];
    double dfdb1[NoPtFit];
    double dfdc1[NoPtFit];
    double dfdA[NoPtFit];
    double dfdB[NoPtFit];
    double f0[NoPtFit];
    double Yminusf0[NoPtFit];// Y-f0

    const int NoPara = 5;
    const int total = NoPtFit * NoPara;
    double F[NoPtFit][NoPara];

    double P0[NoPara] = {0};

    double FT[NoPara][NoPtFit];
    double F3[NoPara] [NoPtFit];
    fill(F[0], F[0] + total, 0);
    fill(FT[0], FT[0] + total, 0);
    fill(F3[0], F3[0] + total, 0);

    double F2[NoPara][NoPtFit];
    double invF2[NoPara][NoPtFit];
    fill(F2[0], F2[0] + total, 0);
    fill(invF2[0], invF2[0] + total, 0);

    double F4[NoPara] = {0};
    double Pud[NoPara] = {0};

    // set max iteration
    int Nomax = 10;

    for (int iter = 0; iter < Nomax; iter++)
    {
        for (int i = 0; i < NoPtFit; i++)
        {
            dfda1[i] = exp(-0.5*(X[i] - b1)*(X[i] - b1) / (c1*c1));

            dfdb1[i] = a1 * exp(-0.5*(X[i] - b1)*(X[i] - b1) / (c1*c1));
            dfdb1[i] = dfdb1[i] * (X[i] - b1) / (c1*c1);

            dfdc1[i] = exp(-0.5*(X[i] - b1)*(X[i] - b1) / (c1*c1));
            dfdc1[i]= dfdc1[i]* (X[i] - b1)*(X[i] - b1) / (c1*c1*c1);

            dfdA[i] = X[i];
            dfdB[i] = 1;

            //Get
            F[i][0] = dfda1[i];
            F[i][1] = dfdb1[i];
            F[i][2] = dfdc1[i];
            F[i][3] = dfdA[i];
            F[i][4] = dfdB[i];

            f0[i] = a1 * exp(-0.5*(X[i] - b1)*(X[i] - b1) / (c1*c1));
            f0[i] = f0[i] + A*X[i] + B;

            Yminusf0[i] = Y[i] - f0[i];
        }

        //Get Current parameter
        P0[0] = a1;
        P0[1] = b1;
        P0[2] = c1;
        P0[3] = A;
        P0[4] = B;

        // Solve equation
        // P = inverse(F'*F)*F'*(y-f0) + P0;
        TransposeMatrix(F[0], FT[0], NoPtFit, NoPara);

        Multi2Matrix(FT[0], NoPara, NoPtFit, F[0], NoPtFit, NoPara, F2[0]);

        InverseMatrix(invF2[0], F2[0], NoPara); // NoPara x NoPara


        Multi2Matrix(invF2[0], NoPara, NoPara, FT[0], NoPara, NoPtFit, F3[0]); // NoPara x NoPtFit

        MultiMatrix2Dby1D(F3[0], NoPara, NoPtFit, Yminusf0, NoPtFit, F4); // F4: NoPara x 1

        //Update
        AddMatrix1D(F4, P0, NoPara, Pud);


        //update parameter

        a1 = Pud[0];
        b1 = Pud[1];
        c1 = Pud[2];
        A  = Pud[3];
        B  = Pud[4];

    }

    // process
    if (a1 <= 0|| b1<=0|| isnan(b1))
    {
        a1 = 0;
        b1 = 0;
        c1 = 0;
        A = 0;
        B = 0;
    }

    out[0] = a1;
    out[1] = b1;
    out[2] = c1;
    out[3] = A;
    out[4] = B;

    //double
    for (int i = 0; i < NoPtFit; i++)
    {
        f0[i] = a1 * exp(-0.5*(X[i] - b1)*(X[i] - b1) / (c1*c1));
        f0[i] = f0[i] + A*X[i] + B;
    }
    double R_sqr = Rsquare_Fit(Y, f0,NoPtFit);
    out[5] = R_sqr;
}

void NcLibrary::ReBinning(double *ChSpec, const int &&chSize,
                          double *transferSpc, const int &&tfSize,
                          double* binSpecOut) {
    bool first = true;
    double Z = 0, sum1 = 0, sum2 = 0, z_flt = 0, z_flt_pre = 0, ztmp = 0;
    int ind_chn = 0;

    int z_int = 0;
    for (int i = 0; i < BINSIZE; i++) {
        Z = transferSpc[i];

        if (Z < 1) {
            sum1 = sum1 + Z;
            sum2 = 0;

            if (sum1 > 1) {
                ind_chn = ind_chn + 1;
                sum2 = sum1 - 1;
                binSpecOut[i] = (Z - sum2) * ChSpec[ind_chn - 1] + sum2 * ChSpec[ind_chn];
                sum1 = sum2;
            } else {
                sum2 = Z;
                binSpecOut[i] = sum2 * ChSpec[ind_chn];
            }

        } else if (Z > 1) {
            if (first == true) {
                first = false;
                z_int = (int) floor(Z);
                z_flt = Z - z_int;

                if (ind_chn + z_int + 1 > CHSIZE - 1) {
                    break;
                }

                sum1 = 0;
                for (int j = 1; j <= z_int; j++) {
                    sum1 = sum1 + ChSpec[ind_chn + j];
                }

                sum1 = sum1 + z_flt * ChSpec[ind_chn + z_int + 1];

                ind_chn = ind_chn + z_int + 1;
                z_flt_pre = 1 - z_flt;
            }

            else {
                ztmp = Z - z_flt_pre;

                z_int = (int) floor(ztmp);
                sum1 = 0;

                if (z_int >= 1) {
                    z_flt = ztmp - z_int;

                    if (ind_chn + z_int + 1 > CHSIZE - 1) {
                        break;
                    }

                    sum1 = z_flt_pre * ChSpec[ind_chn];

                    for (int j = 1; j <= z_int; j++) {
                        sum1 = sum1 + ChSpec[ind_chn + j];
                    }

                    sum1 = sum1 + z_flt * ChSpec[ind_chn + z_int + 1];

                    ind_chn = ind_chn + z_int + 1;
                    z_flt_pre = 1 - z_flt;

                } else {
                    z_flt = Z - z_flt_pre;

                    if (ind_chn + z_int + 1 > CHSIZE - 1) {
                        break;
                    }

                    sum1 = z_flt_pre * ChSpec[ind_chn] + z_flt * ChSpec[ind_chn + 1];
                    z_flt_pre = 1 - z_flt;
                    ind_chn = ind_chn + 1;
                }
            }
            binSpecOut[i] = sum1;
        } else if (Z == 1) {
            if (ind_chn < CHSIZE) {
                binSpecOut[i] = ChSpec[ind_chn];
                ind_chn = ind_chn + 1;
            }

        }
    }
}

void NcLibrary::ReBinning(const Spectrum &ChSpec, const BinSpectrum &transferSpc, BinSpectrum &binSpecOut)
{
    bool first = true;
    Energy Z = 0, sum1 = 0, sum2 = 0, z_flt = 0, z_flt_pre = 0, ztmp = 0;
    int ind_chn = 0;

    int z_int = 0;
    for (int i = 0; i < transferSpc.getSize(); i++) {
        Z = transferSpc[i];

        if (Z < 1) {
            sum1 = sum1 + Z;
            sum2 = 0;

            if (sum1 > 1) {
                ind_chn = ind_chn + 1;
                sum2 = sum1 - 1;
                binSpecOut[i] = (Z - sum2) * ChSpec[ind_chn - 1] + sum2 * ChSpec[ind_chn];
                sum1 = sum2;
            } else {
                sum2 = Z;
                binSpecOut[i] = sum2 * ChSpec[ind_chn];
            }

        } else if (Z > 1) {
            if (first == true) {
                first = false;
                z_int = (int) floor(Z);
                z_flt = Z - z_int;

                if (ind_chn + z_int + 1 > CHSIZE - 1) {
                    break;
                }

                sum1 = 0;
                for (int j = 1; j <= z_int; j++) {
                    sum1 = sum1 + ChSpec[ind_chn + j];
                }

                sum1 = sum1 + z_flt * ChSpec[ind_chn + z_int + 1];

                ind_chn = ind_chn + z_int + 1;
                z_flt_pre = 1 - z_flt;
            }

            else {
                ztmp = Z - z_flt_pre;

                z_int = (int) floor(ztmp);
                sum1 = 0;

                if (z_int >= 1) {
                    z_flt = ztmp - z_int;

                    if (ind_chn + z_int + 1 > CHSIZE - 1) {
                        break;
                    }

                    sum1 = z_flt_pre * ChSpec[ind_chn];

                    for (int j = 1; j <= z_int; j++) {
                        sum1 = sum1 + ChSpec[ind_chn + j];
                    }

                    sum1 = sum1 + z_flt * ChSpec[ind_chn + z_int + 1];

                    ind_chn = ind_chn + z_int + 1;
                    z_flt_pre = 1 - z_flt;

                } else {
                    z_flt = Z - z_flt_pre;

                    if (ind_chn + z_int + 1 > CHSIZE - 1) {
                        break;
                    }

                    sum1 = z_flt_pre * ChSpec[ind_chn] + z_flt * ChSpec[ind_chn + 1];
                    z_flt_pre = 1 - z_flt;
                    ind_chn = ind_chn + 1;
                }
            }
            binSpecOut[i] = sum1;
        } else if (Z == 1) {
            if (ind_chn < CHSIZE) {
                binSpecOut[i] = ChSpec[ind_chn];
                ind_chn = ind_chn + 1;
            }

        }
    }
}

void NcLibrary::BintoCh(double* TF, double* BinOut) {
    bool First = true;
    double Z = 0, sum1 = 0, sum2 = 0, z_flt = 0, z_flt_pre = 0, ztmp = 0;
    int ind_chn = 0;

    int z_int = 0;
    for (int i = 0; i < BINSIZE; i++) {
        Z = TF[i];

        if (Z < 1) {
            sum1 = sum1 + Z;
            sum2 = 0;

            if (sum1 > 1) {
                ind_chn = ind_chn + 1;
                sum2 = sum1 - 1;
                BinOut[i] = ind_chn;
                sum1 = sum2;
            } else {
                sum2 = Z;
                BinOut[i] = ind_chn;
            }

        } else if (Z > 1) {
            if (First == true) {
                First = false;
                z_int = (int) floor(Z);
                z_flt = Z - z_int;

                if (ind_chn + z_int + 1 > CHSIZE - 1) {
                    break;
                }

                sum1 = 0;
                // for (int j = 1; j <= z_int; j++)
                // {
                // sum1 = sum1 + ChSpec[ind_chn + j];
                // }

                // sum1 = sum1 + z_flt * ChSpec[ind_chn + z_int + 1];

                ind_chn = ind_chn + z_int + 1;
                z_flt_pre = 1 - z_flt;
            }

            else {
                ztmp = Z - z_flt_pre;

                z_int = (int) floor(ztmp);
                sum1 = 0;

                if (z_int >= 1) {
                    z_flt = ztmp - z_int;

                    if (ind_chn + z_int + 1 > CHSIZE - 1) {
                        break;
                    }

                    // sum1 = z_flt_pre * ChSpec[ind_chn];

                    // for (int j = 1; j <= z_int; j++)
                    // {
                    // sum1 = sum1 + ChSpec[ind_chn + j];
                    // }

                    // sum1 = sum1 + z_flt * ChSpec[ind_chn + z_int + 1];

                    ind_chn = ind_chn + z_int + 1;
                    z_flt_pre = 1 - z_flt;

                } else {
                    z_flt = Z - z_flt_pre;

                    if (ind_chn + z_int + 1 > CHSIZE - 1) {
                        break;
                    }

                    // sum1 = z_flt_pre * ChSpec[ind_chn] + z_flt *
                    // ChSpec[ind_chn + 1];
                    z_flt_pre = 1 - z_flt;
                    ind_chn = ind_chn + 1;
                }
            }
            BinOut[i] = ind_chn;
        } else if (Z == 1) {
            if (ind_chn < CHSIZE) {
                BinOut[i] = ind_chn;
                ind_chn = ind_chn + 1;
            }

        }
    }
}

void NcLibrary::BinToCh(const BinSpectrum &TF, BinSpectrum &BinOut) {
    bool First = true;
    Energy Z = 0, sum1 = 0, sum2 = 0, z_flt = 0, z_flt_pre = 0, ztmp = 0;
    int ind_chn = 0;

    int z_int = 0;
    for (int i = 0; i < BINSIZE; i++) {
        Z = TF[i];

        if (Z < 1) {
            sum1 = sum1 + Z;
            sum2 = 0;

            if (sum1 > 1) {
                ind_chn = ind_chn + 1;
                sum2 = sum1 - 1;
                BinOut[i] = ind_chn;
                sum1 = sum2;
            } else {
                sum2 = Z;
                BinOut[i] = ind_chn;
            }

        } else if (Z > 1) {
            if (First == true) {
                First = false;
                z_int = (int) floor(Z);
                z_flt = Z - z_int;

                if (ind_chn + z_int + 1 > CHSIZE - 1) {
                    break;
                }

                sum1 = 0;
                // for (int j = 1; j <= z_int; j++)
                // {
                // sum1 = sum1 + ChSpec[ind_chn + j];
                // }

                // sum1 = sum1 + z_flt * ChSpec[ind_chn + z_int + 1];

                ind_chn = ind_chn + z_int + 1;
                z_flt_pre = 1 - z_flt;
            }

            else {
                ztmp = Z - z_flt_pre;

                z_int = (int) floor(ztmp);
                sum1 = 0;

                if (z_int >= 1) {
                    z_flt = ztmp - z_int;

                    if (ind_chn + z_int + 1 > CHSIZE - 1) {
                        break;
                    }

                    // sum1 = z_flt_pre * ChSpec[ind_chn];

                    // for (int j = 1; j <= z_int; j++)
                    // {
                    // sum1 = sum1 + ChSpec[ind_chn + j];
                    // }

                    // sum1 = sum1 + z_flt * ChSpec[ind_chn + z_int + 1];

                    ind_chn = ind_chn + z_int + 1;
                    z_flt_pre = 1 - z_flt;

                } else {
                    z_flt = Z - z_flt_pre;

                    if (ind_chn + z_int + 1 > CHSIZE - 1) {
                        break;
                    }

                    // sum1 = z_flt_pre * ChSpec[ind_chn] + z_flt *
                    // ChSpec[ind_chn + 1];
                    z_flt_pre = 1 - z_flt;
                    ind_chn = ind_chn + 1;
                }
            }
            BinOut[i] = ind_chn;
        } else if (Z == 1) {
            if (ind_chn < CHSIZE) {
                BinOut[i] = ind_chn;
                ind_chn = ind_chn + 1;
            }

        }
    }
}


/**
 * @brief Find_Vally
 * @param arrfloat
 * @param firstlocation     ROI left index of array
 * @param secondlocation    ROI right index of array
 * @param peakvalue
 * @param out               Return array len 2 with range of ROI
 */
void NcLibrary::Find_Vally(double* arrfloat, int firstlocation, int secondlocation, int peakvalue, double* out) {
    // start x
    int startpx = 0;
    int center = peakvalue;
    int contcnt = 0;
    int cntlimit = 2;

    double threshold = 0;

    int findflag = 0;

    for (int i = center - (int) (center * 0.02); i >= firstlocation; i--) {
        if ((arrfloat[i - 1] - arrfloat[i]) >= threshold) {
            if (findflag == 0) {
                findflag = 1;
                contcnt = 1;
            } else {
                contcnt++;
                if (contcnt >= cntlimit) {
                    startpx = i + (cntlimit - 1);
                    if (center == startpx) {

                    } else {
                        break;
                    }
                }
            }
        } else {
            findflag = 0;
            contcnt = 0;
        }
    }

    if (findflag == 0 || contcnt < cntlimit) {
        startpx = firstlocation;
        // startpy = arrfloat[firstlocation];
    }

    // search for py
    int endpx = 0;
    contcnt = 0;
    findflag = 0;
    for (int i = center + (int) (center * 0.01); i <= secondlocation; i++) {
        if ((arrfloat[i + 1] - arrfloat[i]) >= threshold) {
            if (findflag == 0) {
                findflag = 1;
                contcnt = 1;
            } else {
                contcnt++;
                if (contcnt >= cntlimit) {
                    endpx = i - (cntlimit - 1);
                    if (center == endpx) {

                    } else {
                        break;
                    }
                }
            }
        } else {
            findflag = 0;
            contcnt = 0;
        }
    }

    if (findflag == 0 || contcnt < cntlimit) {
        endpx = secondlocation;
        // endpy = arrfloat[secondlocation];
    }

    out[0] = startpx;
    out[1] = endpx;
}

double NcLibrary::confidenceCal(double Found_Peak_Energy, double Iso_Peak_Energy) {
    double Peak_Confidence_Value = 0;
    Peak_Confidence_Value = 100 - abs((Found_Peak_Energy - Iso_Peak_Energy) / Iso_Peak_Energy * 100);
    return Peak_Confidence_Value;
}

double NcLibrary::Calculate_PeakUncertainty(Spectrum& Spc, double BGSum, int ROI_L, int ROI_R) {
    double U = 0, G = 0;

    for (int i = ROI_L; i <= ROI_R; i++) {
        G = G + Spc[i];
    }

    double W = ROI_R - ROI_L + 1;
    W=2;
    double BG1 = W / 2 * BGSum;

    if (G + BG1 > 0) {
        U = sqrt(G + W / 2 * BGSum);
    }

    return U;
}

double NcLibrary::Calculate_EffUncertainty(double en, DetectorProperty* prop) {
    double std = 0;

    vector<double> En = { 59.54, 88.03, 122.06, 165.86, 391.7, 661.66, 898.04, 1173.23, 1332.49, 1836.05 };

    prop->getStdEffecients();

    auto& StdEff = prop->getStdEffecients();

    int N = En.size();

    int ind = 0;

    for (int i = 0; i < N - 1; i++) {
        if (en >= En[i] && en <= En[i + 1]) {
            ind = i;
            std = StdEff[i] + (StdEff[i + 1] - StdEff[i]) / (En[i + 1] - En[i]) * (en - En[i]);
        }
    }

    if (ind == 0) {
        if (en < En[0]) {
            std = StdEff[0];
        }

        if (en > En[N - 1]) {
            std = StdEff[N - 1];
        }
    }

    return std;
}



//std::list<shared_ptr<IsotopeSummary>> nucare::iso::findIsoStab(Spectrum* SPC,
//                                                               repository::DetectorProperty* prop)
//{
//    double ROIWND = 0.3; // 30% from 1022--1899 keV

//    auto peakInfo = PeakSearch::findPeak_Stab(SPC, prop, ROIWND);

//    list<IsotopeSummary> result2;

//    if(peakInfo.size() > 0)
//        result2 = PeakSearch::PeakMatchIsotope_H(peakInfo, prop);

//    //SaveText(" result2_H0, " + result2.size()  ,"result2_H");

//    if(result2.size() > 0)
//    {
//        double Thrshld_Index1 = 0;
//        double Thrshld_Index2 = 0;

//        PeakSearch::IndexFillter_H(result2, prop, Thrshld_Index1, Thrshld_Index2);
//    }

//    list<shared_ptr<IsotopeSummary>> ret;
//    for (auto& foundIso : result2) {
//        ret.emplace_back(std::make_shared<IsotopeSummary>(foundIso));
//    }

//    return ret;
//}

//bool iso::shouldStopStab(std::list<NcSP<IsotopeSummary> > isotopes)
//{
//    return std::find_if(isotopes.begin(), isotopes.end(), [](NcSP<IsotopeSummary> iso) {
//               return iso && iso->name != "K-40"
//                      && iso->Index2 > 0.7;     // 70% confidence
//           }) != isotopes.end();
//}
