#ifndef EVENT_H
#define EVENT_H

#include <vector>
#include <string>
#include "Spectrum.h"
#include "Time.h"
#include <memory>
class Background;
class Calibration;

class Event
{
private:

    QString mSoftwareVersion;
    double mLiveTime;
    double mRealTime;
    double mAvgDose;
    double mMaxDose;
    double mAvgGamma_nSv;
    double mMaxGamma_nSv;
    double mMinGamma_nSv;
    double mAvgFillCps;
    long mBackgroundId;
    long mCalibrationId;
    double mAvgCps;
    double mMaxCps;
    double mMinCps;
    double mE1Energy;
    double mE1Branching;
    double mE1Netcount;
    double mE2Energy;
    double mE2Branching;
    double mE2Netcount;

    QString mPipeMaterial;
    double mPipeThickness;
    double mPipeDiameter;
    QString mClogMaterial;
    double mClogDensity;
    double mClogThickness;
    double mClogRatio;

    nucare::Timestamp mTimeStarted;
    nucare::Timestamp mTimeFinished;
    std::shared_ptr<Spectrum> mSpc;
    std::shared_ptr<Background> mBackground;
    std::shared_ptr<Calibration> mCalibration;
    long mId;
    long mDetectorId;
    int mAcqTime;
    bool mIsFavorite = false;

public:
    Event();
    Event(const Event& ev);
    Event(Event&& ev);
    inline long getId() { return mId; }
    inline void setId(long id) { mId = id; }

    inline long getBackgroundId() const { return mBackgroundId; }
    inline void setBackgroundId(long id) { mBackgroundId = id; }

    inline long getCalibrationId() const { return mCalibrationId; }
    inline void setCalibrationId(long id) { mCalibrationId = id; }

    inline int getAcqTime() { return mAcqTime; }
    inline void setAcqTime(int acqTime) { mAcqTime = acqTime; }
    inline void setStartedTime(const nucare::Timestamp& time) { mTimeStarted = time; }
    inline void setFinishedTime(const nucare::Timestamp& time) { mTimeFinished = time; }
    inline auto getStartedTime() const { return mTimeStarted; }
    inline auto getFinishedTime() const { return mTimeFinished; }
    inline std::shared_ptr<Spectrum> getSpectrum() { return mSpc; }
    void setSpectrum(std::shared_ptr<Spectrum> spc);
    void setSpectrum(const Spectrum::SPC_DATA data);
    void setBackground(std::shared_ptr<Background> background);
    inline std::shared_ptr<Background> getBackground() { return mBackground; }
    inline std::shared_ptr<Calibration> getCalibration() { return mCalibration; }
    inline void setCalibration(std::shared_ptr<Calibration> calibration) { mCalibration = calibration; }

    // Setters
    inline void setSoftwareVersion(const QString& version) { mSoftwareVersion = version; }
    inline void setLiveTime(double time) { mLiveTime = time; }
    inline void setAvgGamma_nSv(double value) { mAvgGamma_nSv = value; }
    inline void setMaxGamma_nSv(double value) { mMaxGamma_nSv = value; }
    inline void setMinGamma_nSv(double value) { mMinGamma_nSv = value; }
    inline void setAvgFillCps(double value) { mAvgFillCps = value; }
    inline void setDetectorId(long id) { mDetectorId = id; }
    inline void setAvgCps(double value) { mAvgCps = value; }
    inline void setMaxCps(double value) { mMaxCps = value; }
    inline void setMinCps(double value) { mMinCps = value; }
    inline void setE1Energy(double value) { mE1Energy = value; }
    inline void setE1Branching(double value) { mE1Branching = value; }
    inline void setE1Netcount(double value) { mE1Netcount = value; }
    inline void setE2Energy(double value) { mE2Energy = value; }
    inline void setE2Branching(double value) { mE2Branching = value; }
    inline void setE2Netcount(double value) { mE2Netcount = value; }

    inline void setPipeMaterial(const QString& value) { mPipeMaterial = value; }
    inline void setPipeThickness(double value) { mPipeThickness = value; }
    inline void setPipeDiameter(double value) { mPipeDiameter = value; }
    inline void setClogMaterial(const QString& value) { mClogMaterial = value; }
    inline void setClogDensity(double value) { mClogDensity = value; }
    inline void setClogThickness(double value) { mClogThickness = value; }
    inline void setClogRatio(double value) { mClogRatio = value; }

    Event& operator=(const Event& ev);

    inline double getAvgDose() const { return mAvgDose; }
    inline void setAvgDose(double dose) { mAvgDose = dose; }
    inline double getMaxDose() const { return mMaxDose; }
    inline void setMaxDose(double dose) { mMaxDose = dose; }
    inline double getRealTime() const { return mRealTime; }
    inline void setRealTime(double time) { mRealTime = time; }
    bool isFavorite() const { return mIsFavorite; } // Made const
    void setFavorite(bool isFavorite) { mIsFavorite = isFavorite; }

    // Getters for members used in insertEvent (already const)
    inline QString getSoftwareVersion() const { return mSoftwareVersion; }
    inline double getLiveTime() const { return mLiveTime; }
    inline double getAvgGamma_nSv() const { return mAvgGamma_nSv; }
    inline double getMaxGamma_nSv() const { return mMaxGamma_nSv; }
    inline double getMinGamma_nSv() const { return mMinGamma_nSv; }
    inline double getAvgFillCps() const { return mAvgFillCps; }
    inline double getAvgCps() const { return mAvgCps; }
    inline double getMaxCps() const { return mMaxCps; }
    inline double getMinCps() const { return mMinCps; }
    inline double getE1Energy() const { return mE1Energy; }
    inline double getE1Branching() const { return mE1Branching; }
    inline double getE1Netcount() const { return mE1Netcount; }
    inline double getE2Energy() const { return mE2Energy; }
    inline double getE2Branching() const { return mE2Branching; }
    inline double getE2Netcount() const { return mE2Netcount; }
    inline QString getPipeMaterial() const { return mPipeMaterial; }
    inline double getPipeThickness() const { return mPipeThickness; }
    inline double getPipeDiameter() const { return mPipeDiameter; }
    inline QString getClogMaterial() const { return mClogMaterial; }
    inline double getClogDensity() const { return mClogDensity; }
    inline double getClogThickness() const { return mClogThickness; }
    inline double getClogRatio() const { return mClogRatio; }
    inline long getDetectorId() const { return mDetectorId; }
};

#endif // EVENT_H
