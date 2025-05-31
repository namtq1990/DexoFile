#include "ncmanager.h"
#include "model/Calibration.h"
#include "model/DetectorProp.h"
#include "detectorcomponent.h"
#include "util/NcLibrary.h"

using namespace nucare;
using namespace std;

NcManager::NcManager(const QString& tag)
    : Component(tag)
{
    logI() << "NcManager initialized.";
}

NcManager::~NcManager()
{
    logI() << "NcManager destroyed.";
}

void NcManager::updateCalibFromRawPeak(Calibration *calib, const Coeffcients &foundPeaks)
{
    auto ratio = calib->getRatio();
    auto chPeaks = calib->chCoefficients();
    bool needUpdate = ratio == 0;
    if (!needUpdate) {
        for (nucare::uint i = 0; i < foundPeaks.size(); i++) {
            if (((int) chPeaks[i] * ratio) != (int) foundPeaks[i]) {
                needUpdate = true;
                break;
            }
        }
    }

    if (!needUpdate) return;

    Coeffcients stdPeaks = {nucare::CS137_PEAK1, nucare::CS137_PEAK2, nucare::K40_PEAK};
    Coeffcients fitParam = NcLibrary::computeCalib(foundPeaks, stdPeaks);
    Coeffcients convCoeff;
    ratio = NcLibrary::calibConvert(fitParam.data(), convCoeff.data());

    calib->setRatio(ratio);
    calib->setCoefficients(convCoeff);

    for (nucare::uint i = 0; i < convCoeff.size(); i++) {
        convCoeff[i] = foundPeaks[i] / ratio;
    }

    calib->setChCoefficients(convCoeff);
    calib->setDate(nucare::Timestamp());
}

void NcManager::computeCalibration(nucare::DetectorComponent *dev, std::shared_ptr<Spectrum> spc, std::shared_ptr<HwSpectrum> hwSpc, Calibration::Mode mode, bool updateStdPeaks)
{

}

void NcManager::onRecvGC(nucare::DetectorComponent* dev, std::shared_ptr<GcResponse> message) {
    logD() << "Received gc " << message->serial << ':' << message->gc << "," << message->detType;
    std::shared_ptr<Calibration> calib = dev->properties()->getCalibration();

    if (calib != nullptr) {
        Coeffcients foundPeaks = {
            (double) message->cs137Ch1,
            (double) message->cs137Ch2,
            (double) message->k40Ch
        };
        updateCalibFromRawPeak(calib.get(), foundPeaks);
        calib->setGC(message->gc);
    }
}

void NcManager::onRecvPackage(nucare::DetectorComponent* dev, std::shared_ptr<DetectorPackage> pkg)
{
    if (!pkg) {
        logE() << "Received null spectrum data from detector.";
        return;
    }

    auto prop = dev->properties();
    if (!prop || !prop->isInitialized()) return;

    shared_ptr<Spectrum> spc = make_shared<Spectrum>();
    prop->mOriginSpc = pkg->spc;

    if (pkg->spc->getSize() != Spectrum::getSize()) {
        auto ratio = prop->getCalibration()->getRatio();
//        Spectrum::Channel* data = new Spectrum::Channel[Spectrum::getSize()];
//        NcLibrary::convertSpectrum(spc->data(), spc->getSize(), data, Spectrum::getSize(), ratio);
//        spc = make_shared<Spectrum>(data);
        HwSpectrum::convertSpectrum<Spectrum>(*pkg->spc, *spc, ratio);
        spc->update();
    }

    spc->setFillCps(pkg->pileup);
    spc->setDetectorID(prop->getId());
    prop->mSpc = spc;
    prop->mSpc->setRealTime(pkg->realtime);

    auto cps0=spc->getTotalCount()/(pkg->realtime == 0 ? 1 : pkg->realtime);

    prop->setCps0(cps0);

    //Smoothing CPS in 3 sec
    prop->setCps(mAvgCps.addedValue(cps0));
    logD() << "cps: " << spc->getTotalCount()
           << ", avg: " << prop->mCPS
           << " , pilup: " << spc->getFillCps()
           << ", realtime: " << pkg->realtime;

    //end smoothing

//    auto doserate = NcLibrary::computeDoserate(spc, prop->getCoeffcients(),
//                                                 prop->getGeCoeffcients());
//    if (spc->getTotalCount()) {
//        doserate += (doserate * (spc->getFillCps()) / spc->getTotalCount());
//    }
//    prop->mDoserate = mAvgDoserate.addedValue(doserate);

    prop->setGM(pkg->gm);

    prop->debugInfo.spcDoserate0 = prop->mDoserate;
    prop->debugInfo.spcDoserate1= prop->mDoserate;


    //Calcuate Dose GM
//    auto avgGM = prop->getGmCount();
//    auto DoseGM = NcLibrary::GM_to_nSV(avgGM);

//    prop->debugInfo.gmDoserate = DoseGM;

//    prop->mDoserate = NcLibrary::DoseCSI_Factor_nSV(prop->mDoserate);

//    if (prop->mDoserate > nucare::config::DOSE_RATE_THRSHLD_NAI_LOW) //100 uSv/h
//    {
//       // prop->mDoserate = NcLibrary::DoseCSI_Factor_nSV(prop->mDoserate);
//        prop->debugInfo.spcDoserate1 = prop->mDoserate;

//        if( prop->mDoserate >  nucare::config::DOSE_RATE_THRSHLD_NAI )
//        {
//            if(DoseGM > nucare::config::DOSE_RATE_THRSHLD_NAI)
//            {
//                prop->mDoserate = DoseGM;
//                prop->setCps(avgGM);
//            }
//        }
//        else
//        {
//            if(DoseGM > nucare::config::DOSE_RATE_THRSHLD_NAI)
//            {
//                prop->mDoserate = DoseGM;
//                prop->setCps(avgGM);
//            }
//        }
//    }

//    prop->setHasNeutron(pkg->hasNeutron);
//    if (pkg->hasNeutron) {
//        prop->setNeutron(mAvgNeutron.addedValue(pkg->neutron));
//    }


    // TODO DEBUG
//    if (prop->mDoserate > 100) {
//        prop->setCps(10000);
//        prop->setCps0(10000);
//        prop->mDoserate = 100000;
//    }

    prop->setGC(pkg->gc);
    prop->setTemperature(pkg->temperature);
    prop->setRawTemperature(pkg->temperatureRaw);

//    mThreadMgr->mainScheduler().create_worker().schedule([=](const rxsc::schedulable&) {
//        if (mAlarmMgr != nullptr && prop != nullptr) {
//            mAlarmMgr->checkAlarm(prop);
//        }
//    });
    emit spectrumReceived(spc);
}
