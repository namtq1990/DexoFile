#include "Event.h"
#include <sstream>
using namespace std;



Event::Event()
    : mAcqTime(0),
    mId(0),
    mBackgroundId(-1),
    mCalibrationId(-1),
    mSpc(nullptr),
    mAvgDose(0),
    mMaxDose(0)
{

}

Event::Event(const Event &ev)
{
    *this = ev;
}

Event::Event(Event &&ev) {
    *this = ev;
}

void Event::setSpectrum(std::shared_ptr<Spectrum> spc)
{
    mSpc = spc;
}

void Event::setSpectrum(const Spectrum::SPC_DATA data)
{
    if (mSpc == nullptr) {
        mSpc = std::make_shared<Spectrum>(data);
    } else {
        mSpc->setData((const Spectrum::Channel*) data);
    }
}

void Event::setBackground(std::shared_ptr<Background> background)
{
    mBackground = background;
}

Event &Event::operator=(const Event &ev)
{
    mAcqTime = ev.mAcqTime;
    mId = ev.mId;
    mTimeStarted = ev.mTimeStarted;
    mTimeFinished = ev.mTimeFinished;
    mAvgDose = ev.mAvgDose;
    mMaxDose = ev.mMaxDose;
    mSpc = ev.mSpc;
    mDetectorId = ev.mDetectorId;

    return *this;
}
