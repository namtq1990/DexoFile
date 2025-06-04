#ifndef NCMANAGER_H
#define NCMANAGER_H

#include "component/component.h"
#include "model/Spectrum.h" // Assuming Spectrum_t is the data type from Detector
#include "model/DetectorModels.h"
#include "model/Calibration.h"
#include "model/Types.h"

namespace nucare { class DetectorComponent; }
class DetectorPackage;
class Calibration;

class NcManager : public QObject, public Component
{
    Q_OBJECT
public:
    explicit NcManager(const QString& tag);
    ~NcManager() override;

    void updateCalibFromRawPeak(Calibration *calib, const Coeffcients &foundPeaks);
    void computeCalibration(nucare::DetectorComponent* dev,
                                std::shared_ptr<Spectrum> spc,
                                std::shared_ptr<HwSpectrum> hwSpc,
                                Calibration::Mode mode = Calibration::HH300_CS_137,
                                bool updateStdPeaks = false);

    nucare::DetectorComponent* getCurrentDetector();

public slots:
    void onRecvPackage(nucare::DetectorComponent* dev, std::shared_ptr<DetectorPackage> pkg);
    void onRecvGC(nucare::DetectorComponent* dev, std::shared_ptr<GcResponse> message);

signals:
    // Signal to forward processed data to other components
    void spectrumReceived(std::shared_ptr<Spectrum> spc);

private:
    nucare::Average<double, 5> mAvgCps;
};

#endif // NCMANAGER_H
