#ifndef DETECTORCODE_H
#define DETECTORCODE_H

//#include "Common/BaseCommon.h"
#include <QString>

enum CrystalType {
    NaI = 1,
    CeBr = 2,
    LaBr = 5,
    CLLBC = 3,
    CLYC = 4,
    CSI = 6
};

enum DetectorCode_E {
    CeBr_1_5x1_5 = 0x0A,
    CeBr_2x2 = 0x1B,
    CeBr_3x3 = 0x2A,
    LaBr_1_5x1_5 = 0x0B,
    LaBr_2x2 = 0x1C,
    LaBr_3x3 = 0x2B,
    NaI_1x1 = 0x1D,
    //        int NaI_2x2 = 0x0D;
    NaI_3x3 = 0x0C,
    NaI_2x3 = 0x1E,
    NaI_1_5x1_5 = 0x0E,
    NaI_2x4x16 = 0x0F,
    NaI_3x5x16 = 0x1A,
    NaI_4x4x16 = 0x1F,
    NaI_2x2_HH300 = 0x0D,
    CLLBC_1_5x1_5 = 0x2C,
    CLLBC_2x2 = 304,
    CLYC_1_5x1_5 = 0x2D,
    CLYC_2x2 = 303,
    CSI_SPRD = 0x2E,
    CLLBC_SPRD = 0x2F,
    NaI_2x2_GP = 0x3A,
    NaI_3x3_GP = 0x3B
};

CrystalType fromDetCode(const DetectorCode_E& code);

enum LibType_E {
    LIB_ANSI15,
    LIB_ANSI,
    LIB_IND,
    LIB_MED
};

struct LibType {
    LibType_E type;
    CrystalType cType;
    QString name;

    QString toString();
};

struct DetectorCode {
public:
    const DetectorCode_E code;
    const CrystalType cType;

    DetectorCode(const DetectorCode_E code) : code(code), cType(fromDetCode(code)) {
    }

    DetectorCode(const DetectorCode& code) : code(code.code), cType(code.cType) {}
};

#endif // DETECTORCODE_H
