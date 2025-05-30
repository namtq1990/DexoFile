#ifndef DETECTORINFO_H
#define DETECTORINFO_H

#include "DetectorCode.h"
#include <QString>
#include <memory>

struct DetectorInfo {
    int id = -1; // Added id member
    QString manufacture = "Nucare";
    QString model = "";
    QString serialNumber;
    QString detectorType = "SPRD300";
    QString probeType = "None";

    std::shared_ptr<DetectorCode> detectorCode = nullptr;
};

#endif // DETECTORINFO_H
