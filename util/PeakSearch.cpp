#include "PeakSearch.h"
#include "util/util.h"
#include "NcLibrary.h"
#include "model/DetectorProp.h"
//#include "Model/NcPeak.h"

#include <math.h>
#include <algorithm>
using namespace nucare;
using namespace std;

double nucare::PeakSearch::CalStdDevSpc(const Spectrum *doseSpc, const Threshold &thshold)
{
    double MAX = 0;
    double AVG = 0;
    double SUM = 0;
    auto in = doseSpc->dataConst();
    double ROI_End = max(thshold.second, (double) doseSpc->getSize() - 2);
    int cnt = 0;

    for (int i = thshold.first; i <= (int)ROI_End; i++)
    {
        SUM = SUM + in[i];

        if (in[i] > MAX)
        {
            MAX = in[i];
        }

        cnt = cnt + 1;
    }

    if (cnt == 0) cnt = 1;


    AVG = SUM / (double)(cnt);

    //%%calculate varian
    double m_devia=0, std_dev=0;
    for (int i = (int)thshold.first; i <=(int) ROI_End; i++)
    {
        m_devia = (AVG - in[i]) / MAX * 100.0;

        std_dev = std_dev + m_devia * m_devia;
    }


    AVG = std_dev / (double)cnt;

    std_dev = sqrt(AVG);

    return std_dev;
}

std::list<int> nucare::PeakSearch::PeakSearch_V1(const Spectrum* in, const FWHM& fwhm, const Coeffcients& coeff,
                                                 const double thshold, const Threshold& thsholdK40,
                                                 const Threshold& theshold1,
                                                 const int MaxDist2Peaks)
{
    // TODO I don't like this idea when allocate too much this array size, change it pls
    int peaklist[NO_MAX_K40] = {0};
    int templist[NO_MAX_K40] = {0};
    auto data = in->dataConst();
    std::list<int> ret;

    //1st Step: Calculate Standard deviation
    double Std_dev = CalStdDevSpc(in, thsholdK40);


    if (Std_dev > 17)
    {

        // Finding Max value
        int IndexMax = nucare::indexOfMax(data, theshold1.first, theshold1.second);

        double MAX = data[IndexMax];

        double PeakAvg = (theshold1.first + theshold1.second) / 2.0;

        //double FWHM_Peak = FWHM[0]*Math.sqrt(PeakAvg) + FWHM[1]; //%% fwhm = 2.355 sigma

        // New Calculation FWHM (2020.01.29) FWHM (ch)=a x sqrt(En) +b
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

    }

    return ret;
}



std::list<int> PeakSearch::PeakSearch_Hight(const Spectrum *in, const FWHM &fwhm, const Coeffcients &coeff, const double thshold, const Threshold &k40Thshold)
{
    double PeakAvg = (k40Thshold.first + k40Thshold.second) / 2.0;

    double FWHM_K40 = fwhm[0] * sqrt(PeakAvg) + fwhm[1];

    double Width_2Sig = FWHM_K40 * 4.0 / 2.355;
    double Width_3Sig = FWHM_K40 * 6.0 / 2.355;

    Threshold sigThreshold1;
    Threshold sigThreshold2;

    sigThreshold1.first = round(PeakAvg - Width_2Sig / 2.0);
    sigThreshold1.second = round(PeakAvg + Width_2Sig / 2.0);

    sigThreshold2.first = round(PeakAvg - Width_3Sig / 2.0);
    sigThreshold2.second = round(PeakAvg + Width_3Sig / 2.0);

    auto ret = PeakSearch_V1(in, fwhm, coeff, thshold, k40Thshold, sigThreshold1);

    if (ret.empty()) {
        ret = PeakSearch_V1(in, fwhm, coeff, thshold, k40Thshold, sigThreshold2);

        if (ret.empty()) {
            ret = PeakSearch_V1(in, fwhm, coeff, thshold, k40Thshold, k40Thshold);
        }
    }

    return ret;
}

std::array<double, 4> PeakSearch::findVally(const Spectrum *in, int firstlocation, int secondlocation, int peakvalue) {
    std::array<double, 4> ret;

    auto chArray = in->dataConst();
    //peak value가 없으면 결과로 0을 return
    if (peakvalue == 0)
    {
        return ret;	//
    }

    double startpx=0;
    double startpy=0;
    double endpx=0;
    double endpy=0;

    //int center = firstlocation+ (int)((secondlocation - firstlocation)/2);
    int center = peakvalue;
    int contcnt=0;
    int cntlimit= 2;//NcLibrary.Auto_floor(NcLibrary.Get_Roi_window_by_energy(NcLibrary.Channel_to_Energy(peakvalue, calib_A2, calib_B2, calib_C2)));
    double threshold=0;

    int findflag=0;
    for(int i = center - (int)(center * 0.01); i > firstlocation; i--)
    {
        if((chArray[i-1] - chArray[i]) >= threshold) {
            if(findflag ==0) {
                findflag=1;
                contcnt=1;
            } else {
                contcnt++;
                if(contcnt>=cntlimit)
                {	startpx=i+(cntlimit-1);
                    startpy=chArray[i];
                    break;
                }
            }
        }else
        {	findflag = 0;
            contcnt=0;
        }
    }
    if (findflag==0 || contcnt<cntlimit)
    {	startpx=firstlocation;
        startpy=chArray[firstlocation];
    }

    contcnt=0;
    findflag=0;
    for (int i=center+(int)(center*0.01); i<secondlocation;i++)
    {	if((chArray[i + 1] - chArray[i]) >= threshold)
        {	if(findflag ==0)
            {	findflag=1;
                contcnt=1;
            } else {
                contcnt++;
                if(contcnt>=cntlimit) {
                    endpx = i-(cntlimit-1);
                    endpy = chArray[i];
                    break;
                }
            }
        }else
        {	findflag = 0;
            contcnt=0;
        }
    }
    if (findflag==0 || contcnt<cntlimit)
    {	endpx = secondlocation;
        endpy = chArray[secondlocation];
    }

    // final A, B 값계산
    //resulta=(float)((startpy-endpy)/(startpx-endpx));
    //resultb=(float)(startpy-(resulta*startpx));


    ///(startpy==endpy & startpx==endpx){result.A=0;result.B=0;}
    //else if(startpx==endpx){result.A=0;result.B=0;}
    //else {
    ret[2] = ((startpy - endpy) / (startpx - endpx));
    ret[3] = (startpy - (ret[2] * startpx));

    //}
    ret[0] = endpx;
    ret[1] = startpx;

    return ret;
}

//std::list<NcPeak> PeakSearch::findPeak(shared_ptr<Spectrum> spc, std::shared_ptr<nucare::repository::DetectorProperty> prop)
//{

//    // 1st: Define parameters
//    shared_ptr<Spectrum> BG = prop->getBackgroundSpc();
//    std::list<NcPeak> peakInfo_bg;

//    auto bg = prop->getBackground();
//    if (bg) {
//        if (bg->peaksInfo.empty())
//        {
//            bg->peaksInfo = GetPPSpectrum_H(BG, prop);     // TODO Add peak into background db
//        }

//        peakInfo_bg = bg->peaksInfo;
//    }

//    //

//    Spectrum smoothSpc;
//    double BinSpec[BINSIZE];
//    double BGEroBinSpec[BINSIZE];
//    Spectrum BGEroChSpec;
//    Spectrum PPChSpec;
//    Spectrum DChSpec;

//    double TF[BINSIZE] = {0};

//    // Vector<NcPeak> peakInfo = new Vector<NcPeak>();

//    // Processing
//    // Step 0: Generate tranfer function
//    TransferFunct(TF, BINSIZE, prop->getFWHM(), prop->getCoeffcients());

//    // Step 1: Smooth data to reduce noise
//    NcLibrary::smoothSpectrum(*spc, smoothSpc, prop->getSmoothParams());

//      // Step 2: ReBinning
//    NcLibrary::ReBinning(smoothSpc.data(), smoothSpc.getSize(), TF, move(BINSIZE), BinSpec);

//    // Step 3: BGErosion
//    BGErosion(BinSpec, prop->InterCoeff.data(), BGEroBinSpec, TF, prop->getCoeffcients());

//    // Step 4:ReturnReBinning
//    Spectrum reBincEmptySpc;
//    ReturnReBinning(BGEroBinSpec, TF, reBincEmptySpc.data());
//    NcLibrary::smoothSpectrum(reBincEmptySpc, BGEroChSpec, prop->getSmoothParams());

//    // Step 5:BGSubtration
//    BGSubtration(smoothSpc.data(), BGEroChSpec.data(), &PPChSpec, prop->getSmoothParams()); // Chek


//    // Step 6: GenDSpecrum
//    GenDSpecrum(PPChSpec, BGEroChSpec, DChSpec.data(), prop.get(), true);

//    // Step 7:Find Peak
//    //peakInfo = NewNcAnalsys.FindPeak(DChSpec, peakInfo); // C# 전용 함수로 변경
//    auto peakInfo = FindPeak_Beta(PPChSpec, DChSpec, prop.get()); // C# 전용 함수로 변경

//    // Step 8:Search ROI based on FWHM
//    SearchROI_N(PPChSpec, peakInfo, prop.get());

//    // Step 9: NetCount
//    NetCount_N(PPChSpec.data(), peakInfo, prop.get());

//    // Step 10: Calculate BG: This function: HoongJae Lee has error because he added
//    // Orignal BG to BG
//    BGNetCount(BGEroChSpec.data(), peakInfo);

//    // Step 11: BG Subtract
//    NetBGSubtract_N(peakInfo, peakInfo_bg, spc->getAcqTime(), BG ? BG->getAcqTime() : 1, prop.get());

//    // Step 12: Calculate Critical Level filter
//    Calculate_LC(peakInfo);

//    // Step 13: Applied to Critical Level filter
//    LC_Filter(peakInfo);

//    // WBCLog log = new WBCLog();

//    // step 14: PeakMatching Isotope

//    return peakInfo;

//}

//list<NcPeak> PeakSearch::findPeak_Stab(Spectrum* spc, repository::DetectorProperty* prop, double ROIWND) {

//    // .......................Start Procesisng.......................
//    Spectrum smoothSpc;
//    double BinSpec[BINSIZE];
//    double BGEroBinSpec[BINSIZE];
//    Spectrum BGEroChSpec;
//    Spectrum PPChSpec;
//    Spectrum DChSpec;

//    double TF[BINSIZE] = {0};

//    list<NcPeak> peakInfo;

//    // Processing
//    // Step 0: Generate tranfer function
//    TransferFunct(TF, BINSIZE, prop->getFWHM(), prop->getCoeffcients());

//    //Save Text
//    //SaveText(" TF, " + Arrays.toString(TF)  ,"TF");


//    // Step 1: Smooth data to reduce noise
//    NcLibrary::smoothSpectrum(*spc, smoothSpc, prop->getSmoothParams());

//    // Step 2: ReBinning
//    NcLibrary::ReBinning(smoothSpc.data(), smoothSpc.getSize(), TF, move(BINSIZE), BinSpec);

//    // Step 3: BGErosion
//    BGErosion(BinSpec, prop->InterCoeff.data(), BGEroBinSpec, TF, prop->getCoeffcients());

//    //Save Text
//    //SaveText(" BGEroBinSpec, " + Arrays.toString(BGEroBinSpec)  ,"BGEroBinSpec");


//    // Step 4:ReturnReBinning
//    Spectrum reBincEmptySpc;
//    ReturnReBinning(BGEroBinSpec, TF, reBincEmptySpc.data());
//    NcLibrary::smoothSpectrum(reBincEmptySpc, BGEroChSpec, prop->getSmoothParams());

//    // Step 5:BGSubtration
//    BGSubtration(smoothSpc.data(), BGEroChSpec.data(), &PPChSpec, prop->getSmoothParams()); // Chek

//    //Step 6: Finding Peak

//    double thshold = 0.5;
//    double K40Peak = 1461.0;
//    double En_theshold1 = K40Peak * (1.0 - ROIWND);
//    double En_thshold2 = K40Peak * (1.0 + ROIWND);

//    double theshold1 = round(NcLibrary::energyToChannel(En_theshold1, prop->getCoeffcients()));
//    double theshold2 = round(NcLibrary::energyToChannel(En_thshold2, prop->getCoeffcients()));
//    Threshold k40Thshld = Threshold(theshold1, theshold2);

//    auto listK40 = PeakSearch_V1(&PPChSpec, prop->getFWHM(), prop->getCoeffcients(), thshold, k40Thshld, k40Thshld, 10);

//    if (!listK40.empty()) {
//        // Step 7.1: Convert Peak channel to Peak Energy
//        for (auto chPeak : listK40) {
//            peakInfo.emplace_back();
//            peakInfo.back().channel = chPeak;
//            peakInfo.back().peakEnergy = NcLibrary::channelToEnergy(chPeak, prop->getCoeffcients());
//        }

//        // Step 8:Search ROI based on FWHM
//        SearchROI_N(PPChSpec, peakInfo, prop);

//        // Step 9: NetCount
//        NetCount_N(PPChSpec.data(), peakInfo, prop);

//        // Step 10: Calculate BG: This function: HoongJae Lee has error because he added
//        // Orignal BG to BG
//        BGNetCount(BGEroChSpec.data(), peakInfo);

//        // Step 11: BG Subtract

//        // Step 12: Calculate Critical Level filter
//        Calculate_LC(peakInfo);
//    }
//    // Step 13: Applied to Critical Level filter
//    //peakInfo = NewNcAnalsys.LC_Filter(peakInfo);
//    //Notice: We cannot apply LC filter because Stabilization Spectrum is just 10 sec for NaI 3"

//    //		String SumTxt1="";
//    //		if(NoPeak>0)
//    //		{
//    //			for(int j=0;j<NoPeak;j++)
//    //			{
//    //				SumTxt1=SumTxt1+peakInfo.get(j).Peak_Energy;
//    //				SumTxt1=SumTxt1+",";
//    //				SumTxt1=SumTxt1+",";
//    //			}
//    //		}
//    //SaveText(" NoPeak, " + NoPeak  +" Peaklist, " + SumTxt+" LisEn, " + SumTxt1 ,"NoPeak");

//    return peakInfo;
//}

//std::list<NcPeak> PeakSearch::FindPeak_Beta(Spectrum& PPSpec, Spectrum& DChSpec, repository::DetectorProperty* prop)
//{
//    //1st: Searching Peak in PP Spectrum
//    std::list<int> Peak_ch_PP;
//    std::list<int> Peak_ch_D;
//    std::list<NcPeak> ret;
//    double a1, a2, b1, b2;

//    for (nucare::uint i = 4; i < CHSIZE - 4; i++)
//    {
//        //1st: Searching Peak in PP Spectrum
//        if (PPSpec[i] > prop->THRSHLD)
//        {
//            a1 = PPSpec[i] - PPSpec[i + 1];
//            a2 = PPSpec[i] - PPSpec[i - 1];

//            b1 = PPSpec[i] - PPSpec[i + 2];
//            b2 = PPSpec[i] - PPSpec[i - 2];

//            if (a1 > 0 && a2 > 0 && b1 > 0 && b2 > 0)
//            {
//                Peak_ch_PP.push_back(i);
//            }
//        }

//        //2st: Searching Peak in D Spectum Spectrum
//        if (DChSpec[i] > prop->THRSHLD)
//        {
//            a1 = DChSpec[i] - DChSpec[i + 1];
//            a2 = DChSpec[i] - DChSpec[i - 1];

//            b1 = DChSpec[i] - DChSpec[i + 2];
//            b2 = DChSpec[i] - DChSpec[i - 2];

//            if (a1 > 0 && a2 > 0 && b1 > 0 && b2 > 0)
//            {
//                Peak_ch_D.push_back(i);
//            }
//        }
//    }

//    double fwhm;
//    double Thsld;
//    int PeakTemp;
//    int	Flg = 0;
//    double tmp;

//    for (auto& j : Peak_ch_D)
//    {
//        PeakTemp = j;
//        fwhm= NcLibrary::channelToFWHM(PeakTemp, prop->getFWHM(), prop->getCoeffcients());

//        Thsld = 0.3 * fwhm;
//        if(Thsld <= 2)
//        {
//            Thsld = 2;
//        }

//        int count = 0;
//        Flg = 0;
//        int PeakP = 0;
//        for (auto& i : Peak_ch_PP)
//        {
//            tmp = abs(PeakTemp - i);
//            if (tmp<Thsld)
//            {
//                Flg = 1;
//                count = count + 1;
//                PeakP = i;
//            }
//        }

//        int PeakD = PeakTemp;

//        //update
//        if(count == 1 && Flg == 1)
//        {
//            PeakD = PeakP;
//        }

//        //Before adding, Checking Peak is exist or not
//        //update: 2020.08.13

//        if(find_if(ret.begin(), ret.end(),
//                   [PeakD](const NcPeak& p) { return p.channel == PeakD; }) == ret.end()) {
//            NcPeak peakData;
//            peakData.channel = PeakD;//
//            peakData.peakEnergy = NcLibrary::channelToEnergy(peakData.channel, prop->getCoeffcients());
//            ret.push_back(peakData);
//        }
//    }

//    // 3rd: Adding Peak


//    for (int& i : Peak_ch_PP)
//    {
//        PeakTemp = i;
//        fwhm= NcLibrary::channelToFWHM(PeakTemp, prop->getFWHM(), prop->getCoeffcients());

//        Thsld = 0.3 * fwhm;

//        if(Thsld <= 2)
//        {
//            Thsld = 2;
//        }

//        Flg = 0;

//        for (int&  j : Peak_ch_D)
//        {
//            tmp = abs(PeakTemp - j);
//            if (tmp<Thsld)	Flg = 1;
//        }

//        if (DChSpec[PeakTemp] > prop->THRSHLD && Flg == 0)
//        {

//            //Before adding, Checking Peak is exist or not
//            //update: 2020.08.13
//            if(find_if(ret.begin(), ret.end(),
//                       [PeakTemp](const NcPeak& p) { return p.channel == PeakTemp; }) == ret.end()) {
//                NcPeak peakData;
//                peakData.channel = PeakTemp;//
//                peakData.peakEnergy = NcLibrary::channelToEnergy(peakData.channel, prop->getCoeffcients());
//                ret.push_back(peakData);
//            }
//        }
//    }

//    return ret;
//}

//std::list<NcPeak> PeakSearch::GetPPSpectrum_H(std::shared_ptr<Spectrum> spc,
//                                              std::shared_ptr<nucare::repository::DetectorProperty> prop) {

//    // 1st: Define parameters

//    auto FWHM_gen = prop->getFWHM();

//    auto EnCoeff_Cali = prop->getCoeffcients();// Energy Calibration


//    // .......................Start Procesisng.......................
//    Spectrum smoothSpc;
//    double BinSpec[BINSIZE];
//    double BGEroBinSpec[BINSIZE];
//    Spectrum BGEroChSpec;
//    Spectrum PPChSpec;
//    Spectrum DChSpec;

//    double TF[BINSIZE] = {0};

//    list<NcPeak> peakInfo;

//    // Processing
//    // Step 0: Generate tranfer function
//    TransferFunct(TF, BINSIZE, FWHM_gen, EnCoeff_Cali);

//    // Step 1: Smooth data to reduce noise
//    NcLibrary::smoothSpectrum(*spc, smoothSpc, prop->getSmoothParams());


//    // Step 2: ReBinning
//    NcLibrary::ReBinning(smoothSpc.data(), smoothSpc.getSize(),
//                                   TF, move(BINSIZE),
//                                   BinSpec);

//    // Step 3: BGErosion
//    BGErosion(BinSpec, prop->InterCoeff.data(), BGEroBinSpec, TF, EnCoeff_Cali);


//    // Step 4:ReturnReBinning
//    Spectrum reBincEmptySpc;
//    ReturnReBinning(BGEroBinSpec, TF, reBincEmptySpc.data());
//    NcLibrary::smoothSpectrum(reBincEmptySpc, BGEroChSpec, prop->getSmoothParams());


//    // Step 5:BGSubtration
//    BGSubtration(smoothSpc.data(), BGEroChSpec.data(), &PPChSpec, prop->getSmoothParams()); // Chek


//    // Step 6: GenDSpecrum
//    GenDSpecrum(PPChSpec, BGEroChSpec, DChSpec.data(), prop.get(), true);


//    // Step 7:Find Peak
//    //peakInfo = NewNcAnalsys.FindPeak(DChSpec, peakInfo); // C# 전용 함수로 변경

//    peakInfo = FindPeak_Beta(PPChSpec, DChSpec, prop.get()); // C# 전용 함수로 변경


//    // Step 7.1: Convert Peak channel to Peak Energy

////    peakInfo = NewNcAnalsys.PeakChannelToEnergy(peakInfo, peakInfo.size(), EnCoeff_Cali);

//    // Step 8:Search ROI based on FWHM
//    SearchROI_N(PPChSpec, peakInfo, prop.get());

//    // Step 9: NetCount
//    NetCount_N(PPChSpec.data(), peakInfo, prop.get());

//    // Step 10: Calculate BG: This function: HoongJae Lee has error because he added
//    // Orignal BG to BG
//    BGNetCount(BGEroChSpec.data(), peakInfo);

//    // Step 11: BG Subtract

//    // Step 12: Calculate Critical Level filter
//    Calculate_LC(peakInfo);

//    // Step 13: Applied to Critical Level filter
//    LC_Filter(peakInfo);

//    // String BGSmooth = "\n BGSmooth " + SpectrumToString(BG);

//    // BG_Erosion = BG.Get_Erosion_Spec();

//    return peakInfo;
//}


void PeakSearch::TransferFunct(double* out, const int len,
                               const FWHM& fwhm, const Coeffcients& coeff) // FWHMCoeff
// index
// 2
{

    double fit1[CHSIZE];

    double x = 0;

    for (int i = 0; i < (int) CHSIZE; i++)
    {
        x = i ;
        fit1[i] = NcLibrary::channelToFWHM(x, fwhm, coeff);
    }

    // resize fit1 from CHSIZE to BINSIZE
    double sc = (double) CHSIZE / (double) BINSIZE;

    double sc0 = sc / 2;

    int ind = 0;
    double tmp;
    double sum1 = 0;
    for (int i = 0; i < BINSIZE; i++) {
        x = i + 1;
        // reset TF function
        out[i] = 0;
        tmp = sc0 + (x - 1) * sc + 1;
        ind = (int) floor(tmp) - 1;
        if (ind >= 0 && ind < (int) CHSIZE) {
            out[i] = fit1[ind];
            sum1 = sum1 + out[i];
        }
    }

    // normalize
    for (int i = 0; i < BINSIZE; i++) {
        out[i] = out[i] / sum1 * (double)(CHSIZE);
    }
}

void PeakSearch::BGErosion(double* MSBinSpec, const double* IterCoeff, double* bgBinOut, double* TF,
                                 const Coeffcients& coeff) {
    double a4 = IterCoeff[0];
    double a3 = IterCoeff[1];
    double a2 = IterCoeff[2];
    double a1 = IterCoeff[3];
    double a0 = IterCoeff[4];

    double b0, b1, b2;
    b2 = IterCoeff[5];
    b1 = IterCoeff[6];
    b0 = IterCoeff[7];


    double x = 0;
    int noiter = 0;

    int MaxIter = 70; // Defined: MaxNoIter=70+1

    double DataEro[MaxIter][BINSIZE];
    for (int i = 0; i < MaxIter; i++)
    {
        memcpy(DataEro[i], MSBinSpec, BINSIZE * sizeof(double));
    }

    // 1st :Erosion and then save array
    double tmp;
    for (int iter = 0; iter < MaxIter - 1; iter++)
    {
        for (int i = 0; i < BINSIZE; i++)
        {
            if (i >= 4 && i <= BINSIZE - 5)
            {
                tmp = (DataEro[iter][i - 4] + DataEro[iter][i + 4]) / 2;
                if (DataEro[iter][i] > tmp)
                {
                    DataEro[iter][i] = tmp;
                }
                // DataEro[iter+1][i] = DataEro[iter][i];
            }
            DataEro[iter + 1][i] = DataEro[iter][i];
        }

    }

    // 2 step:

    array<double, BINSIZE> CHArray; // BinSpec[BINSIZE]

    NcLibrary::BintoCh(TF, CHArray.data());

    double noiter1;

    // iterosion
    double ch, en;
    for (int ind = 0; ind < BINSIZE; ind++)
    {
        ch = CHArray[ind];
        en = NcLibrary::channelToEnergy(ch, coeff);

        x = en;

        if (en <= 145) {
            noiter1 = (int) (b2 * x * x + b1 * x + b0);
        } else {
            noiter1 = a4 * x * x * x * x + a3 * x * x * x + a2 * x * x + a1 * x + a0;
        }

        noiter = (int) round(noiter1);

        if (noiter < 2)
            noiter = 2;

        if (noiter > MaxIter - 1)
            noiter = MaxIter - 1;

        bgBinOut[ind] = DataEro[noiter - 1][ind];
    }

}

void PeakSearch::ReturnReBinning(double* BinSpec, double* TF, double* ChOut) {
    double sumz1 = 0, sumz2 = 0, sumz3 = 0, z = 0, sumtmp = 0;
    int cnt = 0, ind_chn = 0, i = 0;
    bool First = true, First1 = true;

    int z_int = 0;
    double z_pre_w = 0, z_pre = 0, z_cur = 0, z_res = 0;

    while (true) {
        cnt = 0;
        if (TF[i] < 1) {
            while (true) {
                z = TF[i + cnt];
                sumz1 = sumz1 + z;

                if (sumz1 >= 1) {
                    break;
                }
                cnt = cnt + 1;
            }

            sumz2 = (z - (sumz1 - 1)) / z;

            if (First == true) {
                First = false;

                sumtmp = 0;
                for (int j = i; j <= (i + cnt - 1); j++) {
                    sumtmp = sumtmp + BinSpec[j];
                }
                sumtmp = sumtmp + sumz2 * BinSpec[i + cnt];

                ChOut[ind_chn] = sumtmp;

                sumz3 = (sumz1 - 1) / z;
            } else {
                sumtmp = 0;

                sumtmp = sumz3 * BinSpec[i - 1];

                for (int j = i; j <= (i + cnt - 1); j++) {
                    sumtmp = sumtmp + BinSpec[j];
                }
                sumtmp = sumtmp + sumz2 * BinSpec[i + cnt];

                ChOut[ind_chn] = sumtmp;

                sumz3 = (sumz1 - 1) / z;
            }

            sumz1 = sumz1 - 1;
            ind_chn = ind_chn + 1;
            i = i + cnt + 1;

            if (i > BINSIZE - 1) {
                break;
            }
        } else {

            z = TF[i];

            if (First1 == true) {
                First1 = false;
                z_int = (int) floor(z);
                z_pre_w = z - z_int;
                z_pre = z_pre_w / z;

                ChOut[ind_chn] = 1 / z * BinSpec[i];
                ind_chn = ind_chn + 1;
                i = i + 1;
            } else {
                z_cur = 1 - z_pre_w;

                ChOut[ind_chn] = z_pre * BinSpec[i - 1] + z_cur / z * BinSpec[i];
                z_res = z - z_cur;

                if (z_res > 1) {
                    z_int = (int) floor(z_res);

                    for (int j = 1; j <= z_int; j++) {
                        ind_chn = ind_chn + 1;

                        if (i > BINSIZE - 1) {
                            break;
                        }

                        ChOut[ind_chn] = 1 / z * BinSpec[i];
                    }

                    z_pre_w = z - z_cur - z_int;
                    z_pre = z_pre_w / z;
                } else {
                    z_pre_w = z_res;
                    z_pre = z_pre_w / z;
                }
                ind_chn = ind_chn + 1;
            }
            i = i + 1;
        }

        if (i > BINSIZE - 1) {
            break;
        }
    }
}

void PeakSearch::BGSubtration(double* MSChSpec, double* ReBinChSpec, Spectrum* PPChSpecOut, const SmoothP& smooth ) {
    Spectrum spc_sub_NB;
    auto data = spc_sub_NB.data();
    double tmp = 0;
    for (nucare::uint i = 0; i < CHSIZE; i++) {
        tmp = MSChSpec[i] - ReBinChSpec[i];
        if (tmp < 0)
            tmp = 0;
        data[i] = tmp;
    }

    // smooth function
    NcLibrary::smoothSpectrum(spc_sub_NB, *PPChSpecOut, smooth);
}

void PeakSearch::GenDSpecrum(Spectrum& PPChSpec, Spectrum& reBinChSpec, double* DChSpecOut, DetectorProperty* prop,
                                   bool isMeasurement) {

    isMeasurement = false;
    //int CHSIZE = 2048;
    Spectrum bg_est;
    Spectrum D;

    NcLibrary::smoothSpectrum(reBinChSpec, bg_est, prop->getSmoothParams());
    double fwhm, W, A, C, S;
    int W_half, lowchn, highchn;
    for (nucare::uint i = 0; i < CHSIZE - 2; i++) {
        if (reBinChSpec[i] > 0) {
            D[i] = 0;
            DChSpecOut[i] = 0;

            if (i >= 0 && i < CHSIZE - 2)
            {
                //	fwhm = FWHMCoeff[0] * Math.sqrt((double) (i + 1)) + FWHMCoeff[1];

                fwhm = NcLibrary::channelToFWHM(i + 1, prop->getFWHM(), prop->getCoeffcients());

                W = fwhm;
                W_half = (int) round(W / 2);

                if (W_half > 0) {
                    lowchn = i - W_half;
                    highchn = i + W_half;

                    if (lowchn > 0 && highchn < (int) CHSIZE) {
                        A = 0;
                        C = 0;
                        for (int j = lowchn; j < highchn + 1; j++) {
                            A = A + PPChSpec[j];
                            C = C + bg_est[j];

                        }

                        if (C > 1) {
                            S = sqrt(C / W) * W;
                            D[i] = (A - S) * (A - S) / ((C + S) * W);
                        }
                    }
                }
            }

            DChSpecOut[i] = D[i];
        }

    }
}

//void PeakSearch::SearchROI_N(Spectrum& Spc, std::list<NcPeak>& PeakInfo, DetectorProperty* prop) {
////    int NoPeak = PeakInfo.size();
//    int ROI_L, ROI_R;
//    double fwhm_ch = 0;
//    double Pt[2];
//    for (NcPeak& peak : PeakInfo) {

//        // search for left
//        //fwhm_ch = FWHMCoeff[0] * Math.sqrt(PeakInfo.get(i).Channel) + FWHMCoeff[1];

//        fwhm_ch = NcLibrary::channelToFWHM(peak.channel, prop->getFWHM(), prop->getCoeffcients());

//        ROI_L = (int) (peak.channel - fwhm_ch * 2);
//        ROI_R = (int) (peak.channel + fwhm_ch * 2);

//        NcLibrary::Find_Vally(Spc.data(), ROI_L, ROI_R, (int) peak.channel, Pt);

//        // if we can not find, we will set ROI as default?
//        // if(Pt[0]== ROI_L) Pt[0]= (int)(PeakInfo.get(i).Channel - fwhm_ch);
//        // if (Pt[1] == ROI_R) Pt[1] = (int)(PeakInfo.get(i).Channel + fwhm_ch);

//        peak.ROI_Left = (int) Pt[0];
//        peak.ROI_Right = (int) Pt[1];
//    }
//}

//void PeakSearch::NetCount_N(SPC_DATA& Spc,
//                            std::list<NcPeak>& PeakInfo,
//                            repository::DetectorProperty* prop) {
//    int ind_ch;

//    double x1, y1, x2, y2, a, b;

//    int ind_ch_max = 0;
//    double max_val = 0;
//    double Hest = 0;
//    double fit_fwhm;

//    for (auto& peak : PeakInfo)
//    {
//        ind_ch = (int) peak.channel;

//        x1 = peak.ROI_Left;
//        y1 = Spc[(int) x1];

//        x2 = peak.ROI_Right;
//        y2 = Spc[(int) x2];

//        if (x2 == x1)
//            x2 = x1 + 1;

//        a = (y2 - y1) / (x2 - x1);
//        b = y1 - a * x1;

//        ind_ch_max = 0;
//        max_val = 0;

//        for (int j = ind_ch - 2; j < ind_ch + 2; j++)
//        {
//            if (j > 0 && j < CHSIZE)
//            {
//                if (Spc[j] > max_val)
//                {
//                    max_val = Spc[j];
//                    ind_ch_max = j;
//                }
//            }
//        }

//        if (ind_ch_max == 0)
//            ind_ch_max = ind_ch;

//        Hest = Spc[ind_ch_max] - (a * ind_ch_max + b);

//        if (Hest < 0)
//            Hest = 0;

//        //fit_fwhm = FWHMCoeff[0] * Math.sqrt((double) ind_ch_max) + FWHMCoeff[1];
//        fit_fwhm = NcLibrary::channelToFWHM(ind_ch_max, prop->getFWHM(), prop->getCoeffcients());

//        peak.netCount = sqrt(M_PI) * Hest * sqrt(2) * (fit_fwhm / 2.355);

//        peak.BG_a = a;
//        peak.BG_b = b;
//    }
//}

//void PeakSearch::BGNetCount(SPC_DATA& BGEroChSpec,
//                            std::list<NcPeak>& PeakInfo) {
//    double ROI_L, ROI_R;
//    double bg_major;// major background=natural backgorund+ scatering
//    // Background
//    double bg_minor; // minor backgorud: residual backgroud=bg_a*x+b;

//    for (auto& peak : PeakInfo) {
//        ROI_L = peak.ROI_Left;
//        ROI_R = peak.ROI_Right;
//        bg_major = 0;
//        bg_minor = 0;

//        for (int j = (int) ROI_L; j <= ROI_R; j++) {
//            bg_major = bg_major + BGEroChSpec[j];
//            bg_minor = bg_minor + peak.BG_a * j + peak.BG_b;
//        }
//        peak.Background_Net_Count = bg_major + bg_minor;
//    }
//}

//void PeakSearch::Calculate_LC(std::list<NcPeak>& PeakInfo) {

//    double No_avg_bg_ch = 2;
//    double L, R, W, BGTemp, sigma;
//    double k = 1.645; // % 1.645: if 95 % confidence

//    for (auto& peak : PeakInfo) {
//        L = peak.ROI_Left;
//        R = peak.ROI_Right;

//        BGTemp = peak.Background_Net_Count;

//        W = R - L + 1;

//        sigma = W / (2 * No_avg_bg_ch);

//        peak.LC = k * sqrt(BGTemp * (1 + sigma));
//    }
//}

//void PeakSearch::LC_Filter(std::list<NcPeak>& PeakInfo) {
//    PeakInfo.remove_if([](NcPeak& p) {
//        return p.LC > p.netCount;
//    });
//}

//void PeakSearch::NetBGSubtract_N(std::list<NcPeak>& PeakInfo, std::list<NcPeak>& PeakInfo_bg,
//                     double mstime,double bgtime, repository::DetectorProperty* prop)
//{
//    double erg;
//    double erg_bg;
//    double lowen, highen;
//    for (auto& peak : PeakInfo)
//    {
//        erg = peak.peakEnergy;

//        if (erg > 0)
//        {
//            auto Thshold = NcLibrary::Get_Roi_window_by_energy_used_FWHM(erg, prop);

//            //roi = Get_Roi_window_by_energy(erg);// %%% percent
//            //roi = roi / 2d;

//            lowen = Thshold.first;

//            highen = Thshold.second;

//            for (auto& peakBg : PeakInfo_bg)
//            {
//                erg_bg = peakBg.peakEnergy;

//                if (erg_bg >= lowen && erg_bg <= highen)
//                {
//                    peak.netCount = peak.netCount - peakBg.netCount * mstime / bgtime;
//                    peak.Background_Net_Count = peak.Background_Net_Count + peakBg.netCount * mstime / bgtime;
//                }
//            }
//        }
//    }

//    double Thsld_BG = 10; // removinng noise peak
//    PeakInfo.remove_if([Thsld_BG](auto& peak) {
//        return peak.netCount < Thsld_BG;
//    });
//}

//std::list<IsotopeSummary> PeakSearch::PeakMatchIsotope_H(std::list<NcPeak>& mFoundPeak_data,
//                                                         repository::DetectorProperty* prop) {
//    list<IsotopeSummary> result2;
//    double Peak_Confidence_Value_sum = 0;

//    std::vector<double> FoundMSEn;
//    std::vector<double> FoundMSNet;
//    FoundMSEn.reserve(10);
//    FoundMSNet.reserve(10);

//    int index = 0;
//    double max1 = 0;

//    const auto isotopeLib = prop->getIsotopes();

//    for (shared_ptr<Isotopes> SourceInfo : isotopeLib)
//    {
//        auto sourcePeaks = SourceInfo->peaks();
//        shared_ptr<vector<double>> foundPeaksBR = make_shared<vector<double>>();
//        shared_ptr<vector<double>> isoPeaksEn = make_shared<vector<double>>();
//        shared_ptr<vector<NcPeak>> foundPeaks = make_shared<vector<NcPeak>>();

//        for (NcPeak& srcPeak : sourcePeaks) {
//            FoundMSEn.clear();
//            FoundMSNet.clear();


//            for (NcPeak& foundPeak : mFoundPeak_data) {

//                bool isIn = srcPeak.computeEnergyInWindowH(foundPeak.peakEnergy, prop->getFWHM(),
//                                                           prop->getCoeffcients(),
//                                                           prop->getWndROI());
//                if (isIn) {
//                    FoundMSEn.push_back(foundPeak.peakEnergy);
//                    FoundMSNet.push_back(foundPeak.netCount);
//                }
//            }

//            if (!FoundMSEn.empty()) {
//                max1 = 0.0;
//                for (nucare::uint j = 0; j < FoundMSEn.size(); j++) {
//                    if (FoundMSNet[j] > max1) {
//                        max1 = FoundMSNet[j];
//                        index = j;
//                    }
//                }

//                // adding to source infor

//                for (NcPeak& foundPeak : mFoundPeak_data) {

//                    bool isIn = srcPeak.computeEnergyInWindowH(foundPeak.peakEnergy, prop->getFWHM(),
//                                                               prop->getCoeffcients(),
//                                                               prop->getWndROI());

//                    if (isIn) {
//                        if (FoundMSEn[index] == foundPeak.peakEnergy)
//                        {
//                            Peak_Confidence_Value_sum += NcLibrary::confidenceCal(foundPeak.peakEnergy,
//                                                                              srcPeak.peakEnergy);

//                            foundPeaks->push_back(foundPeak);
//                            foundPeaksBR->emplace_back(srcPeak.isotopeGammaEnBR);
//                            isoPeaksEn->emplace_back(srcPeak.peakEnergy);

//                            break;
//                        }
//                    }
//                }

//            }

//        }
//        if (!foundPeaks->empty()) {
//            result2.emplace_back(SourceInfo.get());
//            IsotopeSummary& isoSummary = result2.back();
//            isoSummary.confidence = Peak_Confidence_Value_sum / SourceInfo->peaks().size();
//            isoSummary.foundPeaks = foundPeaks;
//            isoSummary.foundPeaksBR = foundPeaksBR;
//            isoSummary.isoPeaksEn = isoPeaksEn;
//        }
//        Peak_Confidence_Value_sum = 0;
//    }

//    return result2;
//}

//void Peak_Index_H(IsotopeSummary& isoSummary, repository::DetectorProperty* prop) {

//    // Hung Function
//    const int NoPeakEnTrue = isoSummary.isotope->peaks().size();
//    const int NoFoundPeakEn = isoSummary.foundPeaks->size();

//    // Step 1: Get information to calculation
//    double ListIsoEn[NoPeakEnTrue]= {0};
//    double ListIsoBr[NoPeakEnTrue]= {0};
//    vector<NcPeak>& listIsoPeak = isoSummary.isotope->peaks();
//    auto listFoundPeak = isoSummary.foundPeaks;

//    double ListFoundEn[NoPeakEnTrue] = {0};
//    double ListFoundBr[NoPeakEnTrue] = {0};

//    // TODO Optimize here, poor assignment

//    double sumListIsoBr = 0;
//    for (int i = 0; i < NoPeakEnTrue; i++) {
//        ListIsoEn[i] = listIsoPeak[i].peakEnergy;
//        ListIsoBr[i] = listIsoPeak[i].isotopeGammaEnBR;

//        sumListIsoBr = sumListIsoBr + ListIsoBr[i];

//        for (int j = 0; j < NoFoundPeakEn; j++) {
//            if (ListIsoEn[i] == isoSummary.isoPeaksEn->at(j)) {
//                ListFoundEn[i] = isoSummary.foundPeaks->at(j).peakEnergy;
//                ListFoundBr[i] = ListIsoBr[i];
//            }
//        }
//    }

//    // Step 2: For each observed peak,confidence of each peak
//    double Index1 = 1;

//    Threshold Thshld;
//    double ETOL, dev_en, tmp, tmp1;

//    for (int i = 0; i < NoPeakEnTrue; i++) {
//        if (ListFoundEn[i] > 0 && ListIsoBr[i] > 0) {
//            Thshld = NcLibrary::Get_Roi_window_by_energy_used_FWHM(ListIsoEn[i], prop);

//            ETOL = Thshld.second - Thshld.first;

//            dev_en = ListFoundEn[i] - ListIsoEn[i];

//            tmp = -0.16 / (ETOL * ETOL) * (dev_en * dev_en) * ListFoundBr[i];

//            tmp1 = tmp / sumListIsoBr;

//            Index1 = Index1 * exp(tmp1);
//        }
//    }

//    isoSummary.Index1 = Index1;

//    // Step 3: Caluate Index 2
//    double Index2 = 0;
//    double SumNotMatchObservePeak = 0;
//    double SumPeakLibrary = 0;

//    double mCalEfficiency = 0, X = 0, efftmp3;

//    for (int i = 0; i < NoPeakEnTrue; i++) {

//        X = std::log(ListIsoEn[i]);

//        efftmp3 = nucare::math::quartic(X, prop->getPeakCoefficients().data());
//        mCalEfficiency = exp(efftmp3);

//        if (ListFoundEn[i] == 0) {
//            SumNotMatchObservePeak = SumNotMatchObservePeak + ListIsoBr[i] * sqrt(mCalEfficiency);
//        }

//        SumPeakLibrary = SumPeakLibrary + ListIsoBr[i] * sqrt(mCalEfficiency);

//    }

//    if (SumPeakLibrary > 0) {
//        Index2 = Index1 - 1.6 * SumNotMatchObservePeak / SumPeakLibrary;
//    } else {
//        Index2 = 0;
//    }

//    isoSummary.Index2 = Index2;
//}

//void PeakSearch::IndexFillter_H(std::list<IsotopeSummary>& result2, repository::DetectorProperty* prop,
//                    const double Confiden_Index1, const	double Confiden_Index2 )
//{
//    nucare::util::erase_if(result2, [&](IsotopeSummary& isotope) {
//        Peak_Index_H(isotope, prop);

//        //Update information
//        if(isotope.IndexMax <=0) {
//            isotope.IndexMax = isotope.Index2;
//        }
//        isotope.confidence = isotope.IndexMax * 100;

//        if (isotope.Index1 <= Confiden_Index1
//            || isotope.IndexMax <= Confiden_Index2) {
//            return true;
//        }

//        return false;
//    });
//}

//void PeakSearch::IndexFillter_H_Apply(std::list<IsotopeSummary>& result2, repository::DetectorProperty* prop,
//                                     const double Confiden_Index1, const	double Confiden_Index2 )
//{
//    nucare::util::erase_if(result2, [&](IsotopeSummary& isotope) {

//        if (isotope.Index1 <= Confiden_Index1
//            || isotope.IndexMax <= Confiden_Index2) {
//            return true;
//        }

//        return false;
//    });
//}



//std::vector<double> FinCEPeak(IsotopeSummary& mIso, std::list<NcPeak>& PeakInfo,
//                              const double wndROI, repository::DetectorProperty* prop)
//{
//    std::vector<double> cePeaks;

//    Threshold Thshold1;
//    double Left_thsld1,High_thsld1;

//    int NoMaxEn = mIso.isotope->isoMinorPeakBR().size();
//    double EnergyTmp;
//    double  erg0,br0;

//    bool Flg=false;

//    for(auto& peak : PeakInfo) {
//        EnergyTmp = peak.peakEnergy;
//        Flg = false;

//        for(int j = 0;j < NoMaxEn; j++) {
//            erg0 = mIso.isotope->isoMinorPeakEn()[j];	//C.E peak from Idea
//            br0 = mIso.isotope->isoMinorPeakBR()[j];    //C.E peak from Idea

//            if(br0 == 0) {
//                Thshold1 = NcLibrary::Get_Roi_window_by_energy_used_FWHM(erg0, prop->getFWHM(),
//                                                                         prop->getCoeffcients(),
//                                                                         wndROI);

//                Left_thsld1 = Thshold1.first;
//                High_thsld1 = Thshold1.second;

//                if(EnergyTmp >= Left_thsld1
//                        && EnergyTmp <= High_thsld1) {
//                    Flg = true;
//                }
//            }
//        }

//        //Step 2: Select C.E peak
//        if(Flg == true) {
//            cePeaks.push_back(EnergyTmp);
//        }
//    }

//    return cePeaks;
//}

//void CELogicPeak(std::list<IsotopeSummary>& Result2, std::vector<double>& CEPeak, std::list<NcPeak>& PeakInfo,
//                 repository::DetectorProperty* prop)
//{
//    double ErgTemp;

//    for(auto it = Result2.begin(); it != Result2.end(); it++)
//    {
//        int NoFoundCE=0;
//        for(nucare::uint j = 0;j < it->foundPeaks->size();j++) {
//            ErgTemp = it->foundPeaks->at(j).peakEnergy;

//            for(nucare::uint k = 0; k < CEPeak.size(); k++) {
//                if(CEPeak[k] == ErgTemp) {
//                    NoFoundCE++;
//                }
//            }
//        }

//        //Remove source

//        if(NoFoundCE > 0) {
//            if(NoFoundCE == it->foundPeaks->size()) {
//                it = Result2.erase(it);
//            }
//        }
//    }
//}

//void LogicComptonPeak(const string& IsoName, std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo,
//                      const double wndROI, repository::DetectorProperty* prop)
//{
//    std::vector<double> cePeaks;
//    int NoCEPeak=0;

//    for (auto& isoSummary : Result2) {
//        if (isoSummary.name == IsoName) {
//            cePeaks = FinCEPeak(isoSummary, PeakInfo, wndROI, prop);
//            break;
//        }
//    }

//    NoCEPeak = cePeaks.size();

//    if(NoCEPeak>0)
//    {
//        CELogicPeak(Result2, cePeaks, PeakInfo, prop);
//    }
//}

//void PeakSearch::LogicComptonPeakCs_Co60(std::list<IsotopeSummary>& Result2,
//                             std::list<NcPeak>& PeakInfo, const double wndROI,
//                             repository::DetectorProperty* prop)
//{

//    if(Result2.size() > 0) {
//        LogicComptonPeak("Cs-137", Result2, PeakInfo, wndROI, prop);
//        LogicComptonPeak("Co-60", Result2, PeakInfo, wndROI, prop);
//    }
//}

//void PeakSearch::AddCondition_Cs_U233HE_U235HE(std::list<IsotopeSummary>& Result2)
//{
//    bool Cs_Flg = false, U233HE_Flg = false, U235HE_Flg = false;


//    for (auto& foundIso : Result2)
//    {
//        if (foundIso.name == "Cs-137") {
//            Cs_Flg = true;
//        }

//        if (foundIso.name == "U-233HE") {
//            U233HE_Flg = true;
//        }

//        if (foundIso.name == "U-235HE") {
//            U235HE_Flg = true;
//        }
//    }

//    Result2.remove_if([Cs_Flg, U233HE_Flg, U235HE_Flg](IsotopeSummary& isotope) {
//        return (Cs_Flg && U233HE_Flg && isotope.name == "U-233HE")
//                ||(Cs_Flg && U235HE_Flg && isotope.name == "U-235HE");
//    });
//}

//void PeakSearch::AddCondition_Cs_U233_U235(std::list<IsotopeSummary>& Result2)
//{
//    bool Cs_Flg=false,U233HE_Flg=false,U235HE_Flg=false;

//    for (auto& foundIso : Result2)
//    {
//        if (foundIso.name == "Cs-137") {
//            Cs_Flg = true;
//        }

//        if (foundIso.name == "U-233") {
//            U233HE_Flg=true;
//        }

//        if (foundIso.name == "U-235") {
//            U235HE_Flg=true;
//        }
//    }

//    Result2.remove_if([Cs_Flg, U233HE_Flg, U235HE_Flg](IsotopeSummary& isotope) {
//        return (Cs_Flg && U233HE_Flg && isotope.name == "U-233")
//                ||(Cs_Flg && U235HE_Flg && isotope.name == "U-235");
//    });
//}

//void LogicUranium_HE(const std::string& isoname, const std::string& isoname_he, std::list<IsotopeSummary>& Result2,
//                     std::list<NcPeak>& PeakInfo, repository::DetectorProperty* prop)
//{
//    bool Src_Flg1=false,Src_Flg2=false;

//    for (auto& isoSummary : Result2) {
//        if (isoSummary.name == isoname) {
//            Src_Flg1=true;
//        }
//        if (isoSummary.name == isoname_he) {
//            Src_Flg2=true;
//        }
//    }

//    // Set condition
//    //if there are 2615 keV then U-233HE
//    //if there are not, source ID: U-233
//    if(Src_Flg1 && Src_Flg2) {
//        //Find Peak: based on minor and major peak

//        //remove noise peak

//        Threshold Thshold;
//        double EnTmp;

//        Thshold = NcLibrary::Get_Roi_window_by_energy_used_FWHM(2615, prop);

//        bool HighEnrich_Flg=false;

//        for (auto& peak : PeakInfo) {
//            EnTmp= peak.peakEnergy;

//            if(util::isInThreshold(EnTmp, Thshold)) {
//                HighEnrich_Flg=true;
//            }
//        }

//        Result2.remove_if([&](IsotopeSummary& isotope) {
//            return (HighEnrich_Flg && isotope.name == isoname)
//                    || (!HighEnrich_Flg && isotope.name == isoname_he);
//        });
//    }
//}

//void PeakSearch::LogicHighEnricUranium(std::list<IsotopeSummary>& ret, std::list<NcPeak>& PeakInfo,
//                           repository::DetectorProperty* prop)
//{
//    // U233 vs U233-HE
//    LogicUranium_HE( "U-233","U-233HE", ret, PeakInfo, prop);

//    //U235 vs U-235HE
//    LogicUranium_HE( "U-235","U-235HE", ret, PeakInfo, prop);
//}

//void PeakSearch::AddCondition_WGPu_RGPU(std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo,
//                            repository::DetectorProperty* prop)
//{

//    bool WGPu_Flg=false,RGPu_Flg=false;

//    for (auto& isotope : Result2) {
//        if (isotope.name == "WGPu") {
//            WGPu_Flg=true;
//        }
//        if (isotope.name == "RGPu") {
//            RGPu_Flg=true;
//        }
//    }

//    //if two sources are exist
//    //if 662 and 1001
//    if(WGPu_Flg && RGPu_Flg) {
//        double EnTmp;
//        Threshold Thshold1;
//        Threshold Thshold2;
//        double Left_thsld1,High_thsld1,Left_thsld2,High_thsld2;
//        bool Flg_662=false, Flg_1001=false;


//        Thshold1 = NcLibrary::Get_Roi_window_by_energy_used_FWHM(662, prop);

//        Left_thsld1=Thshold1.first;
//        High_thsld1=Thshold1.second;

//        Thshold2 = NcLibrary::Get_Roi_window_by_energy_used_FWHM(1001, prop);

//        Left_thsld2=Thshold2.first;
//        High_thsld2=Thshold2.second;


//        for (auto& peak : PeakInfo) {
//            EnTmp = peak.peakEnergy;

//            if(EnTmp >= Left_thsld1 && EnTmp <= High_thsld1) {
//                Flg_662 = true;
//            }

//            if(EnTmp >= Left_thsld2 && EnTmp<=High_thsld2) {
//                Flg_1001=true;
//            }
//        }

//        Result2.remove_if([&](IsotopeSummary& isotope) {
//            if (Flg_662 && Flg_1001) {
//                return isotope.name == "WGPu";
//            } else {
//                return isotope.name == "RGPu";
//            }
//        });
//    }
//}

//void PeakSearch::Logic_Lu177_Sm153(std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo,
//                       repository::DetectorProperty* prop)
//{
//    bool Lu_Flg=false,Sm_Flg=false;
//    for (auto& isotope : Result2) {
//        if (isotope.name == "Lu-177") {
//            Lu_Flg = true;
//        }

//        if (isotope.name == "Sm-153") {
//            Sm_Flg = true;
//        }
//    }

//    //remove Sm-153 if peak 208keV is existing
//    if(Lu_Flg && Sm_Flg)
//    {
//        Threshold Thshold = NcLibrary::Get_Roi_window_by_energy_used_FWHM(208, prop);
//        double EnTmp;

//        bool Peak208keV_FLg = false;

//        for (auto& peak : PeakInfo) {
//            EnTmp= peak.peakEnergy;

//            if(util::isInThreshold(EnTmp, Thshold)) {
//                Peak208keV_FLg=true;
//            }
//        }

//        Result2.remove_if([&](IsotopeSummary& peak) {
//            return (Peak208keV_FLg && peak.name == "Lu-177")
//                    || (Peak208keV_FLg && peak.name == "Sm-153");
//        });
//    }
//}

//void PeakSearch::Logic_Cs137_Co67(std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo,
//                      repository::DetectorProperty* prop)
//{
//    bool Cs_Flg = false, Co67_Flg = false;


//    for (auto& isotope : Result2) {
//        if (isotope.name == "Cs-137") {
//            Cs_Flg=true;
//        }

//        if (isotope.name == "Co-67") {
//            Co67_Flg=true;
//        }
//    }

//    //remove Sm-153 if peak 208keV is existing
//    if(Cs_Flg && Co67_Flg)
//    {
//        Threshold Thshold = NcLibrary::Get_Roi_window_by_energy_used_FWHM(2155, prop);
//        double EnTmp;

//        bool Peak2155keV_FLg=false;

//        for (auto& peak : PeakInfo) {
//            EnTmp = peak.peakEnergy;

//            if(util::isInThreshold(EnTmp, Thshold)) {
//                Peak2155keV_FLg=true;
//            }
//        }

//        //Remove Sm-153 if 208keV is exist
//        if(Peak2155keV_FLg==false)
//        {
//            Result2.remove_if([](IsotopeSummary& is) {
//                return is.name == "Co-57";
//            });
//        }
//    }
//}

//std::array<double, 2> IndexFillter_Major_Minor_H(IsotopeSummary& result2, std::list<NcPeak>& PeakInfo_new, double MsTime,
//                                repository::DetectorProperty* prop)
//{
//    auto isoPeaksEn = result2.isotope->isoMinorPeakEn();
//    auto isoPeaksBr = result2.isotope->isoMinorPeakBR();

//    //Hung Modified: 17/11/22
//    vector<double> ListIsoEn(isoPeaksEn.begin(), isoPeaksEn.end());
//    vector<double> ListIsoBr(isoPeaksBr.begin(), isoPeaksBr.end());
//    vector<double> ListFoundEn;
//    vector<double> ListFoundBr;
//    ListFoundEn.reserve(PeakInfo_new.size());
//    ListFoundBr.reserve(PeakInfo_new.size());

////    double[][] PeakEnBr_Ref = new double[][] {};
//    // 0 is En
//    // 1 is BR

//    int NoMaxEn = result2.isotope->isoMinorPeakEn().size();
//    double EnTmp, BrTmp;
//    double sumListIsoBr=0;

//    //Except Ba-133 because Ba-133: 32keV 109% Branching, this peak is x-ray peak
//    if (result2.name == "Ba-133") {
//        auto enIt = find(ListIsoEn.begin(), ListIsoEn.end(), 32);
//        if (enIt != ListIsoEn.end()) {
//            auto index = std::distance(ListIsoEn.begin(), enIt);
//            ListIsoBr[index] = 0;
//        }
//    }

//    for (int j = 0; j < NoMaxEn; j++) {
//        sumListIsoBr = sumListIsoBr + ListIsoBr[j];
//    }

//    //Find Peak: based on minor and major peak

//    //remove noise peak
//    for(int i = 0; i < NoMaxEn; i++) {
//        Threshold Thshold = NcLibrary::Get_Roi_window_by_energy_used_FWHM(ListIsoEn[i], prop);

//        for (auto& peak : PeakInfo_new) {
//            EnTmp = peak.peakEnergy;

//            if(util::isInThreshold(EnTmp, Thshold)) {
//                ListFoundEn.push_back(EnTmp);
//                ListFoundBr.push_back(ListIsoBr[i]);
//            }
//        }
//    }


//    //Step 2: For each observed peak,confidence of each peak
//    double Index1 = 1;

//    double ETOL,dev_en, tmp, tmp1;

//    for (int i=0;i<NoMaxEn;i++)
//    {
//        if(ListFoundEn[i]>0&&ListIsoBr[i]>0)
//        {
//            Threshold Thshld = NcLibrary::Get_Roi_window_by_energy_used_FWHM(ListIsoEn[i], prop);

//            ETOL = Thshld.second - Thshld.first;

//            dev_en = ListFoundEn[i] - ListIsoEn[i];

//            tmp = -0.16/(ETOL*ETOL)*(dev_en*dev_en)*ListFoundBr[i];

//            tmp1 = tmp / sumListIsoBr;

//            Index1 = Index1 * exp(tmp1);
//        }
//    }


//    //Step 3: Caluate Index 2
//    double Index2=0;
//    double SumNotMatchObservePeak=0;
//    double SumPeakLibrary=0;

//    double mCalEfficiency=0,X=0,efftmp3;

//    for (int i = 0; i < NoMaxEn; i++)
//    {
//        X = std::log(ListIsoEn[i]);

//        efftmp3 = math::quartic(X, prop->getPeakCoefficients().data());
//        mCalEfficiency = exp(efftmp3);

//        if(ListFoundEn[i] == 0) {
//            SumNotMatchObservePeak = SumNotMatchObservePeak + ListIsoBr[i] * sqrt(mCalEfficiency);
//        }

//        SumPeakLibrary = SumPeakLibrary+ListIsoBr[i] * sqrt(mCalEfficiency);
//    }

//    if(SumPeakLibrary>0)
//    {
//        Index2=Index1-1.6*SumNotMatchObservePeak/SumPeakLibrary;
//    }
//    else
//    {
//        Index2=0;
//    }

//    return {Index1, Index2};

//}

//void PeakSearch::AddCondition_Exception_Isopte(const string&& ExpectedIso, const string&& UnExpectIsotope,
//                                   std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo, repository::DetectorProperty* prop,
//                                   double Acqtime, double Thrshld_Index2, double ActThshld)
//{
//    //Step 1: Using all major and minor by calculating index for all major and minor peak
//    bool Ra_Flg=false,UnExpectIsotope_Flg=false;


//    for (auto& isotope : Result2) {
//        if (isotope.name == ExpectedIso) {
//            Ra_Flg=true;
//        }

//        if (isotope.name == UnExpectIsotope) {
//            UnExpectIsotope_Flg=true;
//        }
//    }

//    if(Ra_Flg && UnExpectIsotope_Flg) {
//        for (auto it = Result2.begin(); it != Result2.end(); it++) {
//            if (it->name == UnExpectIsotope) {
//                auto ReIndex = IndexFillter_Major_Minor_H(*it, PeakInfo, Acqtime, prop);

//                if(ReIndex[1] < Thrshld_Index2) {
//                    it = Result2.erase(it);
//                }
//            }
//        }
//    }



//    //Using again activity for comfirmation
//    Ra_Flg=false;UnExpectIsotope_Flg=false;

//    double Act_UnExpectIsotope=0;
//    double Sum_Act=0;
//    for (auto& isotope : Result2) {
//        if (isotope.name == ExpectedIso) {
//            Ra_Flg = true;
//        }

//        if (isotope.name == UnExpectIsotope) {
//            UnExpectIsotope_Flg = true;
//            Act_UnExpectIsotope = isotope.Act;
//        }

//        Sum_Act = Sum_Act + isotope.Act;
//    }

//    if(Ra_Flg && UnExpectIsotope_Flg) {
//        double Ratio = Act_UnExpectIsotope/Sum_Act;

//        if(Ratio<ActThshld)
//        {
//            Result2.remove_if([&](auto isotope) { return isotope.name == UnExpectIsotope; });
//        }
//    }
//}

//void AddCondition_Co57_TC99m(std::list<IsotopeSummary>& Result2, repository::DetectorProperty* prop)
//{
//    bool Co57_Flg=false,Tc99m_Flg=false;

//    double En_Co57=0, En_Tc99m=0;

//    for (auto& isotope : Result2) {
//        if (isotope.name == "Co-57") {
//            Co57_Flg = true;
//            En_Co57 = isotope.foundPeaks->at(0).peakEnergy;
//        }

//        if (isotope.name == "Tc-99m") {
//            Tc99m_Flg=true;
//            En_Tc99m = isotope.foundPeaks->at(0).peakEnergy;
//        }
//    }

//    //if two sources are exist
//    //if 662 and 1001

//    if(Co57_Flg && Tc99m_Flg) {
//        bool Flg_Co=false, Flg_Tc=false;

//        auto Thshold1= NcLibrary::Get_Roi_window_by_energy_used_FWHM(123, prop->getFWHM(),
//                                                                     prop->getCoeffcients(),
//                                                                     prop->getWndRatio_Co57_Tc99m());

//        auto Thshold2= NcLibrary::Get_Roi_window_by_energy_used_FWHM(141, prop->getFWHM(),
//                                                                     prop->getCoeffcients(),
//                                                                     prop->getWndRatio_Co57_Tc99m());

//        if(util::isInThreshold(En_Co57, Thshold1)) {
//            Flg_Co=true;
//        }
//        if(util::isInThreshold(En_Tc99m, Thshold2)) {
//            Flg_Tc=true;
//        }

//        Result2.remove_if([&](IsotopeSummary& iso) {
//            if (!Flg_Co) {
//                return iso.name == "Co-57";
//            } else if (!Flg_Tc) {
//                return iso.name == "Tc-99m";
//            }

//            return false;
//        });
//    }
//}

int nChoosek(int n, int r)
{
    int result = n;
    if(r>n)    result=0;

    if(r*2>n)   r=n-r;

    if(r==0)    result=1;

    for (int i=2;i<=r;i++)
    {
        result=result*(n-i+1);

        result=(int)((double) result/(double)i);
    }

    return result;
}

std::vector<std::vector<int>> CalCombination(int n, const int r)
{

    int NCR=nChoosek(n,r); //number combination

    if(NCR>0)
    {
        vector<vector<int>> ret(NCR, vector<int>(r, 0));

        if(r==1)
        {
            int cnt=0;
            for (int i=0;i<n;i++)
            {
                ret[cnt][0]=i+1;
                cnt++;
            }
        }


        //combination of 2
        // C(n,2)

        if(r==2)
        {
            int cnt=0;

            for (int i=0;i<n;i++)
            {
                for(int j=i+1;j<n;j++)
                {
                    ret[cnt][0]=i+1;
                    ret[cnt][1]=j+1;
                    cnt++;
                }
            }
        }


        //combination of 3
        // C(n,3)


        if(r==3)
        {
            int cnt=0;

            for (int i=0;i<n;i++)
            {
                for(int j=i+1;j<n;j++)
                {
                    for(int k=j+1;k<n;k++)
                    {
                        ret[cnt][0]=i+1;
                        ret[cnt][1]=j+1;
                        ret[cnt][2]=k+1;
                        cnt++;
                    }
                }
            }
        }

        return ret;
    } else {
        return vector<vector<int>>();
    }
}

std::vector<double> BRSetup_H(std::vector<double>& PeakEn, std::vector<double>& TrueErg, std::vector<double>& BR0,
                 DetectorProperty* prop)
{
    int N = PeakEn.size();
    vector<double> BR(N, 0);

    int M = TrueErg.size();

    double lowthsld = 0, highthshld = 0;

    double erg = 0, En0 = 0;

    for (int i = 0; i < N; i++)
    {
        erg = PeakEn[i];
        for (int j = 0; j < M; j++)
        {
            En0 = TrueErg[j];

            Threshold Thshold = NcLibrary::Get_Roi_window_by_energy_used_FWHM(En0, prop);

            lowthsld = Thshold.first;
            highthshld=Thshold.second;

            //ROI_Percnt = (72 * Math.pow(En0, -0.43)) / 100d;

            //	lowthsld = (1 - ROI_Percnt) * En0;
            //highthshld = (1 + ROI_Percnt) * En0;

            if (erg >= lowthsld && erg <= highthshld)
            {
                BR[i] = BR0[j];
            }
        }
    }

    return BR;
}

//std::vector<double> GetBR_H(std::vector<double>& PeakEn, std::string Nu, IsotopeSummary& mIso, repository::DetectorProperty* prop)
//{
//    // load Database
//    auto isoPeaks = mIso.isotope->peaks();
//    auto isoPeaksEN = mIso.isotope->isoMinorPeakEn();
//    auto isoPeaksBR = mIso.isotope->isoMinorPeakBR();
//    int num = isoPeaksEN.size() + isoPeaks.size();
//    vector<double> true_en(num, 0);
//    vector<double> true_br(num, 0);

//    for (nucare::uint i = 0; i < isoPeaks.size(); i++)
//    {
//        true_en[i] = isoPeaks[i].peakEnergy;
//        true_br[i] = isoPeaks[i].isotopeGammaEnBR;
//    }
//    for (nucare::uint i = 0; i < isoPeaksEN.size(); i++)
//    {
//        true_en[isoPeaks.size() + i] = isoPeaksEN[i];
//        true_br[isoPeaks.size() + i] = isoPeaksBR[i];
//    }

//    return BRSetup_H(PeakEn, true_en, true_br, prop);
//}

bool BRCompare(Vec2D& BR)
{
    double Error=0.01; //If branching ratio is <1%, they are same HEU and U-235
    int N=BR.size(); // Number of peak energy
    int M=BR[0].size(); //Number of isotope

    Vec1D Diff(M);

    if(N==2)
    {
        for (int k=0;k<M;k++)
        {
            double MS=(BR[0][k]+BR[1][k])/(double) (2.0);
            double TS=BR[0][k]-BR[1][k];
            if(MS>0)
            {
                Diff[k] = abs(TS/MS);
            }
        }
    }

    bool Flg=true;
    for (int k=0;k<M;k++)
    {
        if(Diff[k]>Error) Flg=false;
    }

    bool FLg_Identical=false;

    if(Flg==true) FLg_Identical=true;

    return FLg_Identical;
}

Vec1D ActCorrect_Old(PeakCoefficients& Eff_Coef, Vec1D& Peaken, Vec2D& BR, Vec1D& Y, Vec1D& W, double time) {

    int N = BR.size(); // N: Number of peak erngy
    int M = BR[0].size();// M: number of isotope

    const Vec1D temp(M);
    Vec2D MA(M, temp); // = new double[M][M];
    Vec2D MA1(M, temp); // = new double[M][M];
    Vec1D MB(M);
    Vec1D MC(M);
    Vec1D Act(M);

    double time1 = time;

    double en, eff, tmp;

    for (int i = 0; i < N; i++) {
        en = std::log(Peaken[i]);

        tmp = math::quartic(en, Eff_Coef.data());

        eff = exp(tmp);

        W[i] = 1;// set weighting factor =1

        for (int k = 0; k < M; k++) {
            for (int q = 0; q < M; q++) {
                MA[k][q] = MA[k][q] + BR[i][q] * W[i] * BR[i][k];
            }

            MB[k] = MB[k] + Y[i] / eff / time1 * W[i] * BR[i][k];
        }

    }

    // solve Matrix
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < M; j++)
        {
            if (MA[i][j] == 0)
            {
                MA[i][j] = 0.000000000001;
            }

            //debug 18.01.04
            //MA[i][j] =1;
        }
    }
    math::InverseMatrix(MA1, MA, M);
    math::MultiMatrix2Dby1D(MA1, MB, MC);

    for (int i = 0; i < M; i++) {
        Act[i] = MC[i] / 37000;
    }

    return Act;
}

Vec1D ActCorrect_Processing_IdenticalIso(PeakCoefficients& Eff_Coef, Vec1D& Peaken, Vec2D& BR,
                                         Vec1D& Y, Vec1D& W, double time)
{
    //Modify: 18.15.10
    int N = BR.size(); // N: Number of peak energy
    int M = BR[0].size();// M: number of isotope

    Vec1D Act(M); // = new double[M];

    // Processing: Identical isotope

    Vec2D IDMatrix(M, Vec1D(M)); // = new int[M][M];	//IDentical matrix

    Vec1D FLgMatrix(M); // = new int[M];	//Flg Matrix

    Vec2D BRTemp(2, Vec1D(N)); // = new double[2][N];

    //Step 1: Finding couple of identical isotope

    int NoCoupleIso=0;

    for(int i=0;i<M;i++) //rows
    {
        int count=1;

        for(int j=i+1;j<M;j++)
        {
            if(i!=j && FLgMatrix[j]==0)
            {
                for (int k=0;k<N;k++)
                {
                    BRTemp[0][k]=BR[k][i];
                    BRTemp[1][k]=BR[k][j];
                }

                bool Flg_Identical = BRCompare(BRTemp);

                if(Flg_Identical) // Identical
                {
                    IDMatrix[NoCoupleIso][0]=i+1;
                    IDMatrix[NoCoupleIso][count]=j+1;
                    FLgMatrix[j]=1;
                    FLgMatrix[i]=1;
                    count=count+1;
                }
            }
        }
        if(count>1)
            NoCoupleIso=NoCoupleIso+1;
    }


    //Step 3: Calculate single source
    int SingleSrc=0;
    for(int i=0;i<M;i++)
    {
        if(FLgMatrix[i]==0) SingleSrc=SingleSrc+1;
    }

    //Step 4: Calculate activitivty
    int M1=SingleSrc+NoCoupleIso;
    int N1=N;

    if(N1>=M1)
    {
        if(NoCoupleIso == 0) {
            Act = ActCorrect_Old(Eff_Coef, Peaken, BR, Y, W, time);
        }
        else //identical isotope
        {

            //Processing with identical isotope
            Vec2D BR2(N1, Vec1D(M1)); //=new double [N1][M1];
            int count=0;
            for(int j=0;j<N1;j++)
            {
                for(int i=0;i<NoCoupleIso;i++)
                {
                    int index=IDMatrix[i][0]-1;
                    BR2[j][i]=BR[j][index];
                }
            }

            //Get Branching from single source
            Vec1D IsoSingSrc(M); //=new int [M];
            count=NoCoupleIso;

            for(int i=0;i<M;i++)
            {
                if(FLgMatrix[i]==0)
                {
                    for(int k=0;k<N;k++)
                    {
                        BR2[k][count]=BR[k][i];
                    }

                    IsoSingSrc[i]=count;
                    count=count+1;
                }
            }


            //Solving Equation
            Vec1D ActIDen(M1); // = new double[M1];
            Vec1D WFF(N1, 1); //=new double [N1];

            ActIDen = ActCorrect_Old(Eff_Coef, Peaken, BR2, Y, WFF, time);

            //devide activity
            count=0;
            for(int i=0;i<NoCoupleIso;i++)
            {
                int sum2=0;
                for (int j=0;j<M;j++)
                {
                    if(IDMatrix[i][j]>0)
                        sum2=sum2+1;
                }

                if(sum2>0)
                {
                    for (int j=0;j<M;j++)
                    {
                        if(IDMatrix[i][j]>0)
                        {
                            int index=IDMatrix[i][j]-1;
                            //Act[index]=ActIDen[i]/(double)(sum2);
                            Act[index]=ActIDen[i]; //Though: if isotopes are identical, it mean only 1 source
                        }
                    }
                }

            }

            //add activity for single source
            for(int i=0;i<M;i++)
            {
                if(FLgMatrix[i]==0)
                {
                    int index=IsoSingSrc[i];
                    Act[i]=ActIDen[index];
                }
            }

        }

    }
    else
    {
        Vec1D ActTmp(N1); // = new double[N1];

        double time1 = time;
        double en, eff, tmp;

        double SumAct = 0;
        for (int i = 0; i < N1; i++)
        {
            en = std::log(Peaken[i]);
            tmp = math::quartic(en, Eff_Coef.data());

            eff = exp(tmp);
            int count=0;
            for (int k = 0; k < M; k++)
            {
                if (BR[i][k]>0)
                {
                    ActTmp[i] = ActTmp[i] + Y[i] / (eff*time1 * BR[i][k]);
                    count=count+1;
                }
            }

            if(count==0) count=1;

            ActTmp[i] = ActTmp[i] / (double)37000/(double)count;

            SumAct = SumAct + ActTmp[i];
        }

        for (int i = 0; i < M; i++)
        {
            //Act[i] = SumAct / (double)N1;
            Act[i] = SumAct;
        }
    }

    return Act;
}

Vec1D ActCorrect(PeakCoefficients& Eff_Coef, Vec1D& Peaken, Vec2D& BR, Vec1D& Y, Vec1D& W, double time) {

    int NoLineMeaning=0;
    double sumtmp=0;

    int N = BR.size(); // N: Number of peak erngy
    int M = BR[0].size();// M: number of isotope


    for(int i=0; i< N; i++)
    {
        sumtmp=0;

        for(int j=0;j< M;j++)
        {
            sumtmp=sumtmp+BR[i][j];
        }

        if(sumtmp>0)
        {
            NoLineMeaning=NoLineMeaning+1;
        }
    }

    Vec1D Peaken1(NoLineMeaning);

    Vec1D Y1(NoLineMeaning);

    Vec1D W1(NoLineMeaning);

    Vec2D BR1(NoLineMeaning, Vec1D(M)); // =new double [NoLineMeaning][M];

    int cnt=0;

    for(int i=0; i< N; i++)
    {
        sumtmp=0;

        for(int j=0;j< M;j++)
        {
            sumtmp=sumtmp+BR[i][j];
        }

        if(sumtmp>0)
        {

            Peaken1[cnt]=Peaken[i];
            Y1[cnt]=Y[i];
            W1[cnt]=W[i];

            for(int j=0;j< M;j++)
            {
                BR1[cnt][j]=BR[i][j];
            }
            cnt=cnt+1;
        }
    }

    return ActCorrect_Processing_IdenticalIso(Eff_Coef, Peaken1,  BR1,  Y1,  W1, time);
}

std::vector<double> ActMeaSingle(PeakCoefficients& Eff_Coef, std::vector<double>& Peaken,
                                 std::vector<std::vector<double>>& BR,
                                 std::vector<double>& Y, std::vector<double>& W, double time)
{
    int M = BR.size(); // N: Number of peak erngy
    int N = BR[0].size();// M: number of isotope

    int NoSrc=N;

    Vec2D FlgMix(M, Vec1D(N));
    Vec2D MatrixInter(M, Vec1D(N));
    Vec1D Independ_Src(N);
    Vec1D Act(N);

    double sumBr=0;
    for(int src=0;src<NoSrc;src++)
    {
        for (int i=0;i<M;i++)
        {
            if(BR[i][src]>0)
            {
                sumBr=0;

                for(int j=0;j<N;j++)
                {
                    if(j==src)
                    {

                    }
                    else
                    {
                        sumBr=sumBr+BR[i][j];
                    }
                }

                if(sumBr==0)
                {
                    FlgMix[i][src]=1;
                }

            }
        }

    }


    //Step 2
    int cnt=0;
    for(int i=0;i<M;i++)
    {
        cnt=0;
        for(int j=0;j<N;j++)
        {
            if(BR[i][j]>0)
            {
                cnt=cnt+1;
            }
        }

        if(cnt>1)
        {
            for(int j=0;j<N;j++)
            {
                if(BR[i][j]>0)
                {
                    MatrixInter[i][j]=1;
                }
            }
        }
    }


    //Step 3: Independen source
    double SumMI=0;
    for(int i=0;i<N;i++)
    {
        SumMI=0;

        for(int j=0;j<M;j++)
        {
            SumMI=SumMI+MatrixInter[j][i];
        }

        if(SumMI==0)
        {
            Independ_Src[i]=1;
        }
    }

    //step 4: Calculate activity
    Vec2D BRTmp(M, Vec1D(1)); // = new double[M][1];

    for(int i=0;i<N;i++)
    {
        if(Independ_Src[i]==1)
        {
            for(int j=0;j<M;j++)
            {
                BRTmp[j][0]=BR[j][i];
            }

            auto ActTmp = ActCorrect(Eff_Coef, Peaken, BRTmp, Y, W, time);

            Act[i] = ActTmp[0];
        }
        else
        {
            Act[i]=0;
        }
    }

    return Act;
}

Vec1D ActCal_Used_MajorMinorPeak(PeakCoefficients& Eff_Coef, Vec1D& Peaken, Vec2D& BR, Vec1D& Y,
                                                  Vec1D& W, double time, Vec2D& ListPeakMajor)
{

    int M = BR.size(); // N: Number of peak energy
    int N = BR[0].size();// M: number of isotope

    int NoSrc=N;


    //%find Reference
    int NoIsotope=NoSrc;
    Vec1D Flg1(M);
    int cnt2=0;
    for(int i=0;i<M;i++)
    {
        cnt2=0;
        for(int j=0;j<NoIsotope;j++)
        {
            if(BR[i][j]>0)
            {
                cnt2=cnt2+1;
            }
        }

        Flg1[i]=cnt2;
    }

    //Step
    int NoInfer=0;

    for(int i=0;i<M;i++)
    {
        if(Flg1[i]>1)
        {
            NoInfer=NoInfer+1;
        }
    }

    Vec1D ActTmp(NoIsotope);

    // NoInfer=0;
    if(NoInfer==0) //%% No interfence
    {
        ActTmp=ActCorrect(Eff_Coef, Peaken, BR, Y, W, time);
    }
    else
    {
        if(NoIsotope>=NoInfer)
        {
            ActTmp=ActCorrect(Eff_Coef, Peaken, BR, Y, W, time);
        }
        else
        {

            Vec1D PeakEnMea1(NoInfer);
            Vec2D BR2(NoInfer, ActTmp); //=new double[NoInfer][NoIsotope];
            Vec1D PPNet1(NoInfer);
            Vec1D w1(NoInfer); //=new double[NoInfer];

            int cnt11=0;

            for(int i=0;i<M;i++)
            {
                if(Flg1[i]>1)
                {
                    PeakEnMea1[cnt11]=Peaken[i];
                    PPNet1[cnt11]=Y[i];
                    w1[cnt11]=W[i];

                    for(int j=0;j<NoIsotope;j++)
                    {
                        BR2[cnt11][j]=BR[i][j];
                    }
                    cnt11=cnt11+1;
                }
            }

            ActTmp=ActCorrect(Eff_Coef, PeakEnMea1, BR2, PPNet1, w1, time);


            //Add Peak Information
            //Find Major Peak which are not interference
            Vec1D UnInter_Act(NoIsotope);

            for(int numiso=0;numiso<NoIsotope;numiso++)
            {
                //Find Number Major which are not interference
                for(int j=0;j<M;j++)
                {
                    for (int k=0;k<NoInfer;k++)
                    {
                        if(ListPeakMajor[j][numiso]==PeakEnMea1[k])
                        {
                            ListPeakMajor[j][numiso]=0;
                        }
                    }

                }

                // Count

                int UnInterfereMajorPeak=0;
                for(int j=0;j<M;j++)
                {
                    if(ListPeakMajor[j][numiso]>0)
                    {
                        UnInterfereMajorPeak=UnInterfereMajorPeak+1;
                    }
                }

                //
                if(UnInterfereMajorPeak>0)
                {
                    // Recalculate activity based on UnInterfence
                    Vec1D PeakMajorTmp(UnInterfereMajorPeak);
                    Vec2D BRMajorTmp(UnInterfereMajorPeak, Vec1D(1)); //=new double[UnInterfereMajorPeak][1];
                    Vec1D PPNetMajorTmp(UnInterfereMajorPeak);
                    Vec1D wMajorTmp(UnInterfereMajorPeak);

                    int CntMajorTmp=0;

                    for(int j=0;j<M;j++)
                    {
                        if(ListPeakMajor[j][numiso]>0)
                        {
                            if(ListPeakMajor[j][numiso]==Peaken[j])
                            {
                                PeakMajorTmp[CntMajorTmp]=Peaken[j];
                                PPNetMajorTmp[CntMajorTmp]=Y[j];
                                wMajorTmp[CntMajorTmp]=W[j];

                                BRMajorTmp[CntMajorTmp][0]=BR[j][numiso];

                                CntMajorTmp=CntMajorTmp+1;
                            }
                        }
                    }

                    //Calculate Activity
                    Vec1D ActTmp11 = ActCorrect(Eff_Coef, PeakMajorTmp, BRMajorTmp, PPNetMajorTmp, wMajorTmp, time);
                    UnInter_Act[numiso]=ActTmp11[0];
                }
            }

            //average activitity
            for(int i=0;i<NoIsotope;i++)
            {
                if(ActTmp[i]<=0)
                {
                    ActTmp[i]=0;
                }

                if(UnInter_Act[i]>0)
                {
                    if(ActTmp[i]==0)
                    {
                        ActTmp[i]=UnInter_Act[i];
                    }
                    else
                    {
                        ActTmp[i]=(ActTmp[i]+UnInter_Act[i])/2.0;
                    }
                }
            }

        }
    }
    return ActTmp;
}

Vec1D ActCal_Used_MajorPeak(PeakCoefficients& Eff_Coef, Vec1D& Peaken, Vec2D& BR, Vec1D Y, Vec1D W,double time,
                            Vec2D& ListPeakMajor)
{
    int M = BR.size(); // N: Number of peak energy
    int N = BR[0].size();// M: number of isotope

    int NoIsotope=N;

    //Step 1: Use all Major line for Calculate activity
    double EnTmp;



    //
    int NoMajorPeak=0;

    for(int i=0;i<M;i++)
    {
        EnTmp=0;
        for(int j=0;j<NoIsotope;j++)
        {
            if(ListPeakMajor[i][j]>EnTmp)
            {
                EnTmp=ListPeakMajor[i][j];
            }
        }

        if(EnTmp>0)
        {
            NoMajorPeak=NoMajorPeak+1;
        }

    }

    if(NoMajorPeak>=NoIsotope)
    {
        //Step 3: Using interfrence peak
        Vec1D PeakEnMea1(NoMajorPeak);
        Vec2D BR2(NoMajorPeak, PeakEnMea1); // =new double[NoMajorPeak][NoIsotope];
        Vec1D PPNet1(NoMajorPeak);
        Vec1D w1(NoMajorPeak);

        int cnt11=0;
        for(int i=0;i<M;i++) {
            EnTmp=0;
            for(int j=0;j<NoIsotope;j++)
            {
                if(ListPeakMajor[i][j]>EnTmp)
                {
                    EnTmp=ListPeakMajor[i][j];
                }
            }

            if(EnTmp>0)
            {
                PeakEnMea1[cnt11]=Peaken[i];
                PPNet1[cnt11]=Y[i];
                w1[cnt11]=W[i];

                for(int j=0;j<NoIsotope;j++)
                {
                    BR2[cnt11][j]=BR[i][j];
                }
                cnt11=cnt11+1;
            }
        }

        return ActCorrect(Eff_Coef, PeakEnMea1, BR2, PPNet1, w1, time);
    } else {
        return ActCal_Used_MajorMinorPeak(Eff_Coef,Peaken,BR,Y,W,time,ListPeakMajor);
    }
}

//std::vector<double> ActCal_Optimize(std::list<IsotopeSummary>& result2, std::vector<double>& peakEN,
//                                    std::vector<std::vector<double>>& BR, std::vector<double>& Y,
//                                    std::vector<double>& W, double time,
//                                    std::vector<double>& UnEff, std::vector<double>& UnPeak,
//                                       repository::DetectorProperty* prop)
//{
//    int M = BR.size(); // N: Number of peak energy
//    int N = BR[0].size();// M: number of isotope

//    int NoSrc=N;

//    vector<double> Act(NoSrc);

//    // Step 1: Check Nuclide without interfering with other isotope
//    Vec1D ActInd_Src = ActMeaSingle( prop->getPeakCoefficients(), peakEN,  BR,  Y,  W, time);

//    int cnt = 0;

//    for(int i=0;i<NoSrc;i++)
//    {
//        if(ActInd_Src[i]>0)
//        {
//            cnt=cnt+1;
//            Act[i]=ActInd_Src[i];
//        }
//    }


//    //Condition

//    //Step 2: Find interfering Peak

//    if(cnt < NoSrc) {
//        int NoSrc1 = NoSrc - cnt;
//        const Vec1D idVec(NoSrc1);
//        Vec2D BR1(M, idVec); // = new double[M][NoSrc1];
//        Vec2D  ListPeakMajor(M, idVec); // =new double[M][NoSrc1];

//        int NoMaxEn;
//        double EnTmp;

//        int cnt1=0;
//        int i = 0;

//        for(auto& isotope : result2) {
//            if(ActInd_Src[i] == 0) {
//                for(int j=0;j<M;j++) {
//                    BR1[j][cnt1] = BR[j][i];
//                }

//                //Get Major Peak
//                std::vector<double> isoPeaksEN;

//                for (const auto& peak : isotope.isotope->peaks())
//                {
//                    isoPeaksEN.push_back(peak.peakEnergy);
//                }
//                //auto isoPeaksEN = isotope.isotope->isoMinorPeakEn();
//                //auto isoPeaksBR = isotope.isotope->isoMinorPeakBR();
//                NoMaxEn = isoPeaksEN.size();
//                double Left_thsld,High_thsld;
//                for (int j=0;j<M;j++)
//                {
//                    EnTmp = peakEN[j];

//                    for (int k = 0; k < NoMaxEn; k++)
//                    {
//                        Threshold Thshold = NcLibrary::Get_Roi_window_by_energy_used_FWHM(isoPeaksEN[k], prop);

//                        Left_thsld = Thshold.first;
//                        High_thsld = Thshold.second;

//                        if(EnTmp>=Left_thsld&&EnTmp<=High_thsld)
//                        {
//                            ListPeakMajor[j][cnt1]=EnTmp;
//                        }
//                    }
//                }

//                cnt1=cnt1+1;
//            }

//            i++;
//        }



//        //%find Reference
//        int NoIsotope=cnt1;
//        Vec1D ActTmp(NoIsotope); // = new double[NoIsotope];

//        //Step 1: Used major only to estimate Act
//        if(NoIsotope>2) //if number nuclide >2: Processing all da
//        {
//            ActTmp = ActCal_Used_MajorMinorPeak(prop->getPeakCoefficients(), peakEN, BR1,Y,W,time,ListPeakMajor);
//        }
//        else
//        {
//            ActTmp=ActCal_Used_MajorPeak(prop->getPeakCoefficients(),peakEN,BR1,Y,W,time,ListPeakMajor);

//            bool FLg2=false;
//            for(int i=0;i<NoIsotope;i++)
//            {
//                if(ActTmp[i]<=0)
//                {
//                    FLg2=true;
//                    ActTmp[i]=0;
//                }
//            }

//            if(FLg2==true)
//            {
//                //ActTmp=ActCal_Used_MajorMinorPeak(Eff_Coef,Peaken,BR1,Y,W,time,ListPeakMajor);
//            }
//        }



//        int cnt3=0;
//        for(int i=0;i<NoSrc;i++)
//        {
//            if(ActInd_Src[i]==0)
//            {
//                Act[i]=ActTmp[cnt3];
//                cnt3=cnt3+1;
//            }
//        }

//    }


//    return Act;
//}

double Uncertainty_CValue(double C, Vec1D& PeakEn, Vec1D& UnEff, Vec1D& UnPeakNet, Vec1D& Br,
                                        double MSTime, PeakCoefficients& Eff_Coef) {
    double time1 = MSTime;

    int N = PeakEn.size();

    Vec1D CalNet(N);

    double en, tmp, eff;

    for (int i = 0; i < N; i++) {
        en = std::log(PeakEn[i]);

        tmp = math::quartic(en, Eff_Coef.data());

        eff = exp(tmp);

        CalNet[i] = C * Br[i] * time1 * eff * 37000;
    }

    // calculate uncertainty
    Vec1D UnC(N);
    Vec1D UnC_per(N);
    double tmp1, tmp2;
    for (int i = 0; i < N; i++) {
        UnC[i] = 0;
        if (CalNet[i] > 0) {
            en = std::log(PeakEn[i]);

            tmp = math::quartic(en, Eff_Coef.data());

            eff = exp(tmp);

            tmp1 = UnPeakNet[i] / CalNet[i];
            tmp2 = UnEff[i] / eff;

            UnC[i] = C * sqrt(tmp1 * tmp1 + tmp2 * tmp2);
        }
    }

    for (int i = 0; i < N; i++) {
        UnC_per[i] = UnC[i] / C * 100;
    }

    double avg_un = 0, sum1 = 0;
    for (int i = 0; i < N; i++) {
        if (CalNet[i] > 0) {
            sum1 = sum1 + 1 / (UnC_per[i] * UnC_per[i]);
        }
    }

    avg_un = sqrt(1 / sum1);

    return avg_un;
}

void RemoveBR_Peak_Thshld(Vec1D& BRMinorMajor, Vec1D& PeakEn, double EnThsld , double BrThshld)
{
    for(nucare::uint j=0;j<BRMinorMajor.size();j++)
    {
        if(PeakEn[j]<=EnThsld)
        {
            BRMinorMajor[j]=0;
        }
    }

    for(nucare::uint j=0;j<BRMinorMajor.size();j++)
    {
        if(BRMinorMajor[j]<=BrThshld)
        {
            BRMinorMajor[j]=0;
        }
    }
}

//void RemoveBR_Peak(Vec1D& PeakEn, IsotopeSummary& mIso, Vec1D& BRMinorMajor,
//                   repository::DetectorProperty* prop)
//{
//    auto isoPeakEN = mIso.isotope->isoMinorPeakEn();
//    auto isoPeakBR = mIso.isotope->isoMinorPeakBR();
//    int NoMaxEn = isoPeakEN.size();

//    bool Flg_En=false, Flg_Br=false;
//    double lowthsld,highthshld,erg, erg0,br0;

//    for(int i=0;i<NoMaxEn;i++)
//    {
//        erg0 = isoPeakEN[i];
//        br0 = isoPeakBR[i];

//        Threshold Thshold = NcLibrary::Get_Roi_window_by_energy_used_FWHM(erg0, prop);
//        lowthsld=Thshold.first;
//        highthshld=Thshold.second;

//        for(nucare::uint j = 0; j < BRMinorMajor.size(); j++)
//        {
//            Flg_En=false;

//            Flg_Br=false;

//            if(br0==BRMinorMajor[j])
//            {
//                Flg_Br=true;
//            }


//            erg=PeakEn[j];
//            if (erg >= lowthsld && erg <= highthshld)
//            {
//                Flg_En=true;
//            }


//            if(Flg_Br && Flg_En)
//            {
//                BRMinorMajor[j]=0;
//            }
//        }
//    }
//}

double RMSECal(Vec1D& C0, Vec1D& C1)
{
    double m_rmse = 0;
    int N = C0.size();

    Vec1D bn(N);
    double tmp = 0;
    for(int i = 0; i < N; i++)
    {
        tmp= (C0[i] + C1[i])/ 2;

        bn[i] = abs(C0[i] - C1[i]) / tmp;
    }

    double br = 0;
    double sum1 = 0;
    for(int i = 0; i < N; i++)
    {
        sum1 = sum1 + bn[i];
    }

    br = sum1 / (double)N;

    double sb = 0;

    sum1 = 0;
    for(int i= 0; i < N; i++)
    {
        sum1 = sum1 + (bn[i] - br) * (bn[i] - br);
    }

    if (N > 1)
    {
        sb = sum1 / (double)(N - 1);
    }
    else
    {
        sb = sum1 / (double)N;
    }

    sb = sqrt(sb);


    m_rmse = sqrt(sb*sb+ br*br);

    return m_rmse;
}

double StdErr(double C, Vec1D& Br, Vec1D& PeakEn, Vec1D& PPNet, double MSTime, PeakCoefficients& Eff_Coef)
{
    double rmse = 0;
    double time1 = MSTime;

    int N = PeakEn.size();
    Vec1D CalNet(N);

    double en, tmp, eff;

    for (int i = 0; i < N; i++)
    {
        CalNet[i] = 0; //initialize

        en = std::log(PeakEn[i]);

        tmp = math::quartic(en, Eff_Coef.data());

        eff = exp(tmp);

        CalNet[i] = C * Br[i] * time1 * eff * 37000;
    }


    int cnt = 0;
    for(int i = 0; i < N; i++)
    {
        if (CalNet[i] > 0)
        {
            cnt = cnt + 1;
        }
    }

    if (cnt > 0)
    {
        Vec1D C_Cal(cnt);
        Vec1D C_Mea(cnt);

        cnt = 0;

        for(int i = 0; i < N; i++)
        {
            if (CalNet[i] > 0)
            {
                C_Cal[cnt] = CalNet[i];
                C_Mea[cnt] = PPNet[i];
                cnt = cnt + 1;
            }

        }

        rmse = RMSECal(C_Cal, C_Mea);


    }
    else
    {
        rmse = 100;
    }


    return rmse;
}

// TODO Should avoid to copy result to other result. Use filter only...
//void ActivityCorrection_H1(Spectrum& Spc, std::list<IsotopeSummary>& result2, std::list<NcPeak>& PeakInfo_new,
//                                      const double msTime, repository::DetectorProperty* prop)
//{
//    //Hung Modified: 17/11/22
//    vector<double> TruePeak;
//    TruePeak.reserve(50);

//    int NoMaxEn;
//    double EnTmp,BrTmp;

//    for(auto& isotope : result2) {
//        NoMaxEn = isotope.isotope->getTruePeaksSize();
//        Vec1D peaksEN(NoMaxEn, 0);
//        Vec1D peaksBR(NoMaxEn, 0);
//        isotope.isotope->getTruePeaks(peaksEN.data(), peaksBR.data());

//        for (int j = 0; j < NoMaxEn; j++) {
//            EnTmp = peaksEN[j];
//            BrTmp= peaksBR[j];

//            if(BrTmp>0) {
//                TruePeak.push_back(EnTmp);
//            }
//        }
//    }

//    //remove noise peak
//    int MM = PeakInfo_new.size();
//    double Left_thsld,High_thsld;
//    int Flg[MM];

//    int i = 0;
//    for(auto it = PeakInfo_new.begin(); it != PeakInfo_new.end(); it++) {
//        Flg[i]=0;// reset

//        EnTmp = it->peakEnergy;

//        for(auto& peak : TruePeak) {
//            Threshold Thshold = NcLibrary::Get_Roi_window_by_energy_used_FWHM(peak, prop);

//            Left_thsld = Thshold.first;
//            High_thsld = Thshold.second;

//            if(EnTmp>=Left_thsld&&EnTmp<=High_thsld)
//            {
//                Flg[i]=1;
//            }
//        }

//        i++;
//    }

//    int M222 = PeakInfo_new.size();

//    double PeakInfo[M222][7];
//    int cnt11=0;
//    for (auto& peak : PeakInfo_new) {
//        //if(PeakInfo_new.get(i).Used_for_Boolen >0)
//        {
//            PeakInfo[cnt11][0] = 0; //peak.Peak;        // Look like channel peak isn't used
//            PeakInfo[cnt11][1] = peak.ROI_Left;
//            PeakInfo[cnt11][2] = peak.ROI_Right;
//            PeakInfo[cnt11][3] = peak.peakEnergy;
//            PeakInfo[cnt11][4] = peak.netCount;
//            PeakInfo[cnt11][5] = peak.Background_Net_Count;
//            PeakInfo[cnt11][6] = peak.LC;
//            cnt11=cnt11+1;
//        }
//    }

//    int M = PeakInfo_new.size();
//    int N = 7;

//    int M2 = 0;
//    for (int i = 0; i < M; i++)
//    {
//        M2 = M2 + Flg[i];
//    }


//    // TODO Remove bad this allocation
//    double PeakInfo11[M2][N];

//    int cnt = 0;
//    for (int i = 0; i < M; i++) {
//        if (Flg[i] == 1)
//        {
//            for (int j = 0; j < N; j++)
//            {
//                PeakInfo11[cnt][j] = PeakInfo[i][j];
//            }
//            cnt = cnt + 1;

//        }
//    }

//    M = M2;
//    N = N;

//    vector<double> PeakEn11(M, 0);
//    for (int i = 0; i < M; i++)
//    {
//        PeakEn11[i] = PeakInfo11[i][3];
//    }

//    // Step 2: Calculate BR
//    vector<string> Nu(result2.size(), "");
//    i = 0;
//    for (auto& isotope : result2)
//    {
//        Nu[i] = isotope.name;
//        i++;
//    }

//    int NoIso = Nu.size();
//    double BR11[M][NoIso];

//    i = 0;
//    for (auto& isotope : result2) {
//        auto BRTemp11 = GetBR_H(PeakEn11, Nu[i], isotope, prop);

//        // TODO Proper this data
//        for (int j = 0; j < M; j++)
//        {
//            BR11[j][i] = BRTemp11[j];
//        }

//        i++;
//    }

//    // Step 3.1: Remove BR with row have zeros value
//    int NoLineMeaning=0;
//    double sumtmp=0;
//    for(int i=0; i< M; i++)
//    {
//        sumtmp=0;

//        for(int j=0;j< NoIso;j++)
//        {
//            sumtmp=sumtmp+BR11[i][j];
//        }

//        if(sumtmp>0)
//        {
//            NoLineMeaning=NoLineMeaning+1;
//        }
//    }

//    if (NoLineMeaning == 0) {
//        return;
//    }

//    vector<vector<double>> PeakInfo1(NoLineMeaning, vector<double>(N));
//    int NoLineMeaningTmp=0;
//    for(int i=0; i< M; i++)
//    {
//        sumtmp=0;

//        for(int j=0;j< NoIso;j++)
//        {
//            sumtmp=sumtmp+BR11[i][j];
//        }

//        if(sumtmp>0)
//        {

//            for(int j=0;j<N;j++)
//            {
//                PeakInfo1[NoLineMeaningTmp][j] = PeakInfo11[i][j];
//            }

//            NoLineMeaningTmp=NoLineMeaningTmp+1;
//        }
//    }




//    // step 2: Calcualte Uncertainty of PeakNet and Efficiency
//    M = PeakInfo1.size();
//    N = PeakInfo1[0].size();

//    vector<double> ROI_L(M, 0);
//    vector<double> ROI_R(M, 0);
//    vector<double> PeakEn(M, 0);

//    vector<double> PPNet(M, 0);
//    vector<double> BGNet(M, 0);
//    vector<double> W(M, 0); // weighting factor

//    for (int i = 0; i < M; i++)
//    {
//        ROI_L[i] = PeakInfo1[i][1];
//        ROI_R[i] = PeakInfo1[i][2];
//        PeakEn[i] = PeakInfo1[i][3];
//        PPNet[i] = PeakInfo1[i][4];
//        BGNet[i] = PeakInfo1[i][5];
//        W[i] = 1;
//    }

//    vector<double> UnPeak(M);
//    vector<double> UnEff(M);

//    for (int i = 0; i < M; i++) {
//        UnPeak[i] = NcLibrary::Calculate_PeakUncertainty(Spc, BGNet[i], ROI_L[i], ROI_R[i]);
//        UnEff[i] = NcLibrary::Calculate_EffUncertainty(PeakEn[i], prop);
//    }

//    // Step 3: Calculate BR
//    //String[] Nu = new String[result2.size()];
//    //	for (int i = 0; i < result2.size(); i++)
//    //{
//    //		Nu[i] = result2.get(i).isotopes;
//    //	}

//    NoIso = Nu.size();
//    vector<vector<double>> BR(M, vector<double>(NoIso));

//    vector<double> BRTemp;

//    i = 0;
//    for (auto& isotope : result2) {
//        BRTemp = GetBR_H(PeakEn, Nu[i], isotope, prop);
//        for (int j = 0; j < M; j++) {
//            BR[j][i] = BRTemp[j];
//        }
//        i++;
//    }

//    //Step 3.2: Calculate weighting factor
//    double en,entemp, eff,std_net,std_eff;
//    for (int i=0;i<M;i++)
//    {
//        en = std::log(PeakEn[i]);
//        entemp = math::quartic(en, prop->getPeakCoefficients().data());
//        eff = exp(entemp);

//        std_net = UnPeak[i] / PPNet[i];
//        std_eff = UnEff[i] / eff;

//        W[i] = 1.0 / (std_net * std_net + std_eff * std_eff);
//    }


//    //saving BR,PeakInfo_new,PeakInfo
//    // Step 4: Calculate Activity

//    //	Act = ActCorrect(Eff_Coef, PeakEn, BR, PPNet, W, MsTime);

//    //18.07.31
//    auto Act = ActCal_Optimize(result2,PeakEn, BR, PPNet, W, msTime,UnEff,UnPeak,prop);


//    // step 5: calculate Uncertainty of isotpe
//    Vec1D Uncer(NoIso);
//    Vec1D RMSE(NoIso);

//    i = 0;
//    for (auto& isotope : result2) {
//        for (int j = 0; j < M; j++) {
//            BRTemp[j] = BR[j][i];
//        }

//        isotope.Act = Act[i];
//        isotope.Uncer = Uncertainty_CValue(Act[i], PeakEn, UnEff, UnPeak, BRTemp, msTime, prop->getPeakCoefficients());

//        //Ignore minor peak to Calculate RMSE for Th-232 and Cs-137
//        if (isotope.name == "Th-232" || isotope.name == "Cs-137") {
//            RemoveBR_Peak(PeakEn, isotope, BRTemp, prop);
//        } else if (isotope.name == "Ra-226") {
//            //	BRTemp=RemoveBR_SelectedPeak(PeakEn,FWHMCoeff,coeff, BRTemp,0.6, 2615, 0.008);
//            RemoveBR_Peak_Thshld(BRTemp, PeakEn, 100, 0.036);
//        } else if (isotope.name == "Ba-133") {   //remove Peak 165keV with Br=0.0065
//            RemoveBR_Peak_Thshld(BRTemp, PeakEn, 0, 0.01);
//        }

//        isotope.RMSE = StdErr(Act[i], BRTemp, PeakEn, PPNet, msTime, prop->getPeakCoefficients());

//        i++;
//    }
//}

//void ScreeningProcess_1st(Spectrum& Spc, std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo,
//                          repository::DetectorProperty* prop, double mstime,double Thshld_Un, double Thshld_rmse)
//{
//    int NoIso=Result2.size();


//    if(NoIso>0)
//    {
//        int NoMaxCombine=0;

//        //1st: Determine number combination
//        if(NoIso<=2)
//        {
//            NoMaxCombine=NoIso;
//        } else {
//            NoMaxCombine=2;
//        }


//        // for single source and mix soures
//        int NoRowA=0, NoColA=0;
//        int index;
//        std::vector<std::string> ListIso_Remove;
//        ListIso_Remove.reserve(1000);

//        for(int i=0;i<NoMaxCombine;i++) {
//            auto A = CalCombination(NoIso, i + 1);

//            NoRowA = A.size();
//            NoColA = A[0].size();

//            for(int j=0; j < NoRowA; j++)
//            {

//                std::list<IsotopeSummary> ResultTemp;//=Result2
//                auto it = Result2.begin();

//                for(int k=0;k<NoColA;k++)
//                {
//                    index=A[j][k];
//                    if(index>0)
//                    {
//                        std::advance(it, index-1);
//                         ResultTemp.push_back(*it);
//                        // Reset the iterator for the next use
//                        it = Result2.begin();

//                    }
//                }
//                ActivityCorrection_H1(Spc, ResultTemp, PeakInfo, mstime, prop);

//                 for (auto s = ResultTemp.begin(); s != ResultTemp.end(); ++s)
//                {
//                    if(s->Act<0 || s->Uncer > Thshld_Un)
//                     {
//                         ListIso_Remove.push_back(s->isotope->name());
//                     }
//                }
//            }
//        }


//        //remove isotope
//        if(ListIso_Remove.size()>0)
//        {
//             // Define a lambda function
//            auto shouldRemove = [&](IsotopeSummary& isotope) {
//                std::string iso_str = isotope.name; // Convert isotope to string
//                return std::find(ListIso_Remove.begin(), ListIso_Remove.end(), iso_str) != ListIso_Remove.end();
//            };
//             //remove isotope
//             Result2.erase(
//                 std::remove_if(Result2.begin(), Result2.end(), shouldRemove),
//                 Result2.end());

//        }
//    }


//}

//bool AddExceptLogic_Cs(const IsotopeSummary& result2)
//{
//    bool flg=false;

//    if (result2.name == "Cs-137") {
//        int NoPeak = result2.isoPeaksEn->size();

//        //text file writing
//        // Nopeak,result2.IsoPeakEn.get(0), uncertainty, rmse
//        if(NoPeak == 1) {
//            double abs1 = result2.isoPeaksEn->at(0) - 662;

//            if(abs(abs1) <= 2)
//            {
//                flg=true;
//            }
//        }
//    }

//    return flg;
//}

//void C_Thshold_Filter(std::list<IsotopeSummary>& Result2, double UnLimit)
//{
//    double sumAct = 0;
//    for (auto& iso : Result2) {
//        sumAct += iso.Act;
//    }

//    Result2.remove_if([&](IsotopeSummary& iso) {
//        auto ratio = iso.Act / sumAct;
//        return ratio < UnLimit;
//    });
//}

//void PeakSearch::CValue_Filter_H(Spectrum& smoothSpc, std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo,
//                     repository::DetectorProperty* prop, double Act_Thshld)
//{
//    double msTime = smoothSpc.getAcqTime();
//    // Adding condition to prohibit Co57 and Tc99m
//    AddCondition_Co57_TC99m(Result2, prop) ;

//    //Adding condition to prohibit WGPU by logic when RGPu is IDed
//    // TODO this filter called too many times
//    PeakSearch::AddCondition_WGPu_RGPU(Result2, PeakInfo, prop);

//    if(Result2.size() > 0) {
//        double Thshld_Un = 10;
//        double Thshld_RMSE = 1.2;
//        double Uncer_cs=100;

//        /*
//            double Thshld_Un = 20;
//            double Thshld_RMSE = 2.0;
//            */

//        /*			double Thshld_Un = 20;
//            double Thshld_RMSE = 1.5;*/

//        // Step 1: Remove noise candidate to improve find activity by solving matrix
//        ScreeningProcess_1st(smoothSpc, Result2, PeakInfo, prop, msTime, Thshld_Un, Thshld_RMSE);

//        //Step 2:  Calculate activity

//        while (true) {
//            auto NoIso = Result2.size();

//            if(NoIso==0) {
//                break;
//            } else {
//                ActivityCorrection_H1(smoothSpc, Result2, PeakInfo, msTime, prop);

//                //2019.08.29: Adding condition threadshold
//                //Starting
//                bool FlgThshld=false;
//                Thshld_Un = 10;
//                Thshld_RMSE = 1.2;

//                auto it = find_if(Result2.begin(), Result2.begin(), [](auto& isotope) {
//                    return isotope.Act == 0;
//                });

//                FlgThshld = it != Result2.end();

//                if(FlgThshld==true) {
//                    Thshld_Un = 20;
//                    Thshld_RMSE = 2;
//                } else {
//                    Thshld_Un = 10;
//                    Thshld_RMSE = 1.2;
//                }

//                //end ---- 2019.08.29

//                Result2.remove_if([&](IsotopeSummary& isotope) {        // Return false if not remove
//                    if (isotope.Act > 0) {
//                        if (isotope.Uncer <= Uncer_cs && isotope.RMSE <= 10) {
//                            return false;
//                        } else if (isotope.Uncer <= Thshld_Un && isotope.RMSE <= Thshld_RMSE) {
//                            return false;
//                        }
//                    }

//                    return true;
//                });

//                if (Result2.size() == NoIso || Result2.size() == 0) {
//                    break;
//                }
//            }
//        }

//        if (Result2.size() > 1) {
//            C_Thshold_Filter(Result2, Act_Thshld); //Threshold 5%
//        }
//    }
//}

//std::list<NcPeak> Return_UnclaimedEn(Spectrum& Spc, std::list<IsotopeSummary>& result2, std::list<NcPeak>& PeakInfo_new,
//                                     double MsTime, repository::DetectorProperty* prop)
//{
//    //Hung Modified: 17/11/22
//    Vec1D TruePeak;
//    TruePeak.reserve(50);

//    int NoMaxEn;
//    double EnTmp,BrTmp;

//    for(auto& iso : result2) {
//        NoMaxEn = iso.isotope->getTruePeaksSize();
//        Vec1D isoPeaksEN(NoMaxEn, 0);
//        Vec1D isoPeaksBR(NoMaxEn, 0);
//        iso.isotope->getTruePeaks(isoPeaksEN.data(), isoPeaksBR.data());

//        for (int j = 0; j < NoMaxEn; j++)
//        {
//            EnTmp = isoPeaksEN[j];
//            BrTmp= isoPeaksBR[j];

//            if(BrTmp >= 0) //comment: =0: include Backscarter peak and Compton peak
//                //	if(BrTmp>0) //comment: >0: DO NOT include Backscarter peak and Compton \peak
//            {
//                TruePeak.push_back(EnTmp);
//            }
//        }
//    }


//    //remove noise peak
//    std::list<NcPeak> ret;

//    for(auto peak : PeakInfo_new) {
//        EnTmp= peak.peakEnergy;

//        bool isUnclaim = true;

//        for(int j = 0; j < TruePeak.size(); j++) {
//            auto Thshold = NcLibrary::Get_Roi_window_by_energy_used_FWHM(TruePeak[j], prop);
//            if(util::isInThreshold(EnTmp, Thshold)) {
//                isUnclaim = false;
//                break;
//            }
//        }

//        if (isUnclaim) {
//            ret.push_back(peak);
//        }
//    }

//    return ret;
//}

//std::list<NcPeak> PeakSearch::CValue_Return_UnclaimedEn(Spectrum& smoothSpc, std::list<IsotopeSummary>& Result2,
//                                            std::list<NcPeak>& PeakInfo,
//                                            double mstime, repository::DetectorProperty* prop)
//{
//    // Step 1: Remove noise candidate to improve find activity by solving matrix

//    return Return_UnclaimedEn(smoothSpc, Result2, PeakInfo, mstime , prop);
//}

//void ReListIndMax(Vec1D& ListMax, Vec1D& Data, double thsld) {
//    ListMax.clear();
//    auto max = *max_element(Data.begin(), Data.end());
//    if (max > thsld) {
//        for (nucare::uint i = 0; i < Data.size(); i++) {
//            if (Data[i] == max) {
//                ListMax.push_back(i);
//            }
//        }
//    }
//}

//void PeakSearch::IsotopeID_UnClaimedLine(Spectrum& Spc, std::list<IsotopeSummary>& Result2, std::list<NcPeak>& PeakInfo,
//                                        repository::DetectorProperty* prop)
//{
//    //Hung Modified: 17/11/22

//    int NoSrc=Result2.size();
//    int NoPeak=PeakInfo.size();
//    Vec1D NoLineEn;
//    NoLineEn.reserve(NoSrc);

//    Vec1D TruePeak;
//    TruePeak.reserve(50);

//    int NoMaxEn;

//    double TrueKeV, TrueBr;
//    int CountLineEn;
//    bool Flag;
//    for(auto& iso : Result2) {
//        CountLineEn=0;

//        NoMaxEn = iso.isotope->getTruePeaksSize();
//        Vec1D isoPeaksEN(NoMaxEn);
//        Vec1D isoPeaksBR(NoMaxEn);
//        iso.isotope->getTruePeaks(isoPeaksEN.data(), isoPeaksBR.data());

//        for(int k=0;k<NoMaxEn;k++)
//        {
//            TrueKeV = isoPeaksEN[k];
//            TrueBr = isoPeaksBR[k];

//            Flag=false;

//            if(TrueBr > 0) {
//                auto Thshold = NcLibrary::Get_Roi_window_by_energy_used_FWHM(TrueKeV, prop);

//                for(auto& peak : PeakInfo) {
//                    if( util::isInThreshold(peak.peakEnergy, Thshold)==true)
//                    {
//                        Flag=true;
//                    }
//                }
//            }

//            if(Flag==true)
//            {
//                CountLineEn++;
//            }
//        }

//        NoLineEn.push_back(CountLineEn);
//    }

//    double Thsld1=0.3 * PeakInfo.size();

//    //24.08.01
//    //Hung: addmore code to proccess
//    Vec1D NoLineEn1, NoLineEn2, ListMax1, ListMax2,ListCand1, ListCand2;
//    int cnt_src1=0, cnt_src2=0;
//    NoLineEn1.reserve(NoSrc);
//    NoLineEn2.reserve(NoSrc);
//    ListMax1.reserve(NoSrc);
//    ListMax2.reserve(NoSrc);
//    ListCand1.reserve(NoSrc);
//    ListCand2.reserve(NoSrc);


//    for(int j=0;j<NoSrc; j++)
//    {
//        NoLineEn1.push_back(NoLineEn[j]);
//        NoLineEn2.push_back(NoLineEn[j]);
//        ListMax1.push_back(0);
//        ListMax2.push_back(0);
//        ListCand1.push_back(0);
//        ListCand2.push_back(0);
//    }

//    int index1;

//    for (int k = 0; k < 1; ++k)
//    {
//        ListMax1 = ReListIndMax(ListMax1, NoLineEn1, Thsld1);

//        for (int j = 0; j < NoSrc; ++j)
//        {
//            if (ListMax1[j] > 0)
//            {
//                index1 = ListMax1[j] - 1;
//                ListCand1[index1] = 1;

//                cnt_src1 = cnt_src1 + 1;
//                NoLineEn1[index1] = 0;
//                ListMax1[j] = 0;
//            }
//        }
//    }


//    cnt_src2 = 0;
//    for (int k = 0; k < 2; ++k)
//    {
//        ListMax2 = ReListIndMax(ListMax2, NoLineEn2, Thsld1);

//        for (int j = 0; j < NoSrc; ++j)
//        {
//            if (ListMax2[j] > 0)
//            {
//                index1 = ListMax2[j] - 1;
//                ListCand2[index1] = 1;

//                cnt_src2 = cnt_src2 + 1;
//                NoLineEn2[index1] = 0;
//                ListMax2[j] = 0;
//            }
//        }
//    }


//    //select candidate
//    if (cnt_src2 < NoPeak)
//    {
//        auto it = Result2.begin();

//        for (int i = 0; i < NoSrc && it != Result2.end(); ++i, ++it)
//        {
//            if (ListCand2[i] == 0)
//            {
//                it->Act = -1;
//            }
//        }
//    }
//    else
//    {
//        auto it = Result2.begin();
//        for (int i = 0; i < NoSrc && it != Result2.end(); ++i, ++it)
//        {
//            if (ListCand1[i] == 0)
//            {
//                it->Act = -1;
//            }
//        }
//    }


//    // Remove
//    for (auto it = Result2.begin(); it != Result2.end(); )
//    {
//        if (it->Act == -1)
//        {
//            it = Result2.erase(it);
//        }
//        else
//        {
//            ++it;
//        }
//    }


//    //Calculate activity98

//    if(Result2.size()>0)
//    {
//        ActivityCorrection_H1(Spc, Result2, PeakInfo, Spc.getAcqTime(), prop);
//        Result2.remove_if([](auto& iso) {
//            return iso.Act < 0;
//        });
//    }
//}
//Vec1D PeakSearch::ReListIndMax(Vec1D ListMax, Vec1D Data, double thsld)
//{
//    int NoMax = Data.size();
//    int max1 = 0;

//    for (int i = 0; i < NoMax; ++i)
//    {
//        if (Data[i] > thsld)
//        {
//            if (Data[i] > max1)
//            {
//                max1 = Data[i];
//            }
//        }
//    }

//    int cnt = 0;
//    if (max1 > thsld)
//    {
//        cnt = 0;
//        for (int i = 0; i < NoMax; ++i)
//        {
//            if (Data[i] == max1)
//            {
//                ListMax[cnt] = i + 1;
//                ++cnt;
//            }
//        }
//    }

//    return ListMax;
//}


//std::list<IsotopeSummary> PeakSearch:: UpdateUnclaimedResult( std::list<IsotopeSummary> result2,
//                                                       std::list<IsotopeSummary> Unclaimed_Result)
//{
//    if(Unclaimed_Result.size()>0)
//    {
//        vector<string> Nuiso(result2.size(), "");
//        int i = 0;
//        for (auto& isotope : result2)
//        {
//            Nuiso[i] = isotope.name;
//            i++;
//        }

//        std::vector<IsotopeSummary> result2Vector(result2.begin(), result2.end());

//        // Update Unclaimed_Result based on result2
//        for (auto& unclaimed : Unclaimed_Result) {
//            // Use std::find_if to locate the matching IsotopeSummary in result2Vector
//            auto it = std::find_if(result2Vector.begin(), result2Vector.end(),
//                                   [&unclaimed](const IsotopeSummary& isotope) {
//                                       return isotope.name == unclaimed.name;
//                                   });

//            // Check if a match was found
//            if (it != result2Vector.end()) {
//                // Update the unclaimed isotope with data from result2
//                unclaimed.IndexMax = it->IndexMax; // Corrected from (*it)->IndexMax
//            }
//        }
//    }

//    return Unclaimed_Result;
//}
