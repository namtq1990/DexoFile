#include "DetectorCode.h"
#include "util/util.h"
#include <util/nc_exception.h>

#include <QMap>

QMap<LibType_E, QString> MAP_LIB_TYPE = {
    { LibType_E::LIB_ANSI, "ANSI" },
    { LibType_E::LIB_ANSI15, "ANSI15" },
    { LibType_E::LIB_IND, "IND" },
    { LibType_E::LIB_MED, "MED" }
};
QMap<CrystalType, QString> MAP_CRYSTAL_TYPE = {
    { CrystalType::NaI, "NaI" },
    { CrystalType::CeBr, "CeBr" },
    { CrystalType::LaBr, "LaBr" },
    { CrystalType::CLLBC, "CLLBC" },
    { CrystalType::CLYC, "CLYC" },
    { CrystalType::CSI, "CSI" }
};

CrystalType fromDetCode(const DetectorCode_E &code) {
    switch (code) {
    case CeBr_1_5x1_5: {
        return CeBr;
    }
    case CeBr_2x2: {
        return CeBr;
    }
    case CeBr_3x3 : {
        return CeBr;
    }
    case LaBr_1_5x1_5 : {
        return LaBr;
    }
    case LaBr_2x2: {
        return LaBr;
    }
    case LaBr_3x3: {
        return LaBr;
    }
    case NaI_1x1: {
        return NaI;
    }
        //        int NaI_2x2 = 0x0D;
    case NaI_3x3: {
        return NaI;
    }
    case NaI_2x3: {
        return NaI;
    }
    case NaI_1_5x1_5: {
        return NaI;
    }
    case NaI_2x4x16: {
        return NaI;
    }
    case NaI_3x5x16: {
        return NaI;
    }
    case NaI_4x4x16: {
        return NaI;
    }
    case NaI_2x2_HH300: {
        return NaI;
    }
    case CLLBC_1_5x1_5: {
        return CLLBC;
    }
    case CLLBC_2x2: {
        return CLLBC;
    }
    case CLYC_1_5x1_5: {
        return CLYC;
    }
    case CLYC_2x2: {
        return CLYC;
    }
    case CSI_SPRD: {
        return CSI;
    }
    case CLLBC_SPRD: {
        return CLLBC;
    }
    case NaI_2x2_GP: {
        return NaI;
    }
    case NaI_3x3_GP: {
        return NaI;
    }
    default:
        NC_THROW_ARG_ERROR(QString::asprintf("Not support detector code: %d", code));
    }
}

template<> QString nucare::toString<CrystalType>(const CrystalType &type)
{
    return MAP_CRYSTAL_TYPE[type];
}

template<> QString nucare::toString<LibType_E>(const LibType_E &type)
{
    return MAP_LIB_TYPE[type];
}

template<>
QString nucare::toString<LibType>(const LibType &library)
{
    return QString("%1_%2").arg(toString<LibType_E>(library.type), toString<CrystalType>(library.cType));
}

template<>
QString nucare::toString<DetectorCode_E>(const DetectorCode_E &code)
{
    switch (code) {
    case CeBr_1_5x1_5: {
        return "CeBr_1_5x1_5";
    }
    case CeBr_2x2: {
        return "CeBr_2x2";
    }
    case CeBr_3x3 : {
        return "CeBr_3x3";
    }
    case LaBr_1_5x1_5 : {
        return "LaBr_1_5x1_5";
    }
    case LaBr_2x2: {
        return "LaBr_2x2";
    }
    case LaBr_3x3: {
        return "LaBr_3x3";
    }
    case NaI_1x1: {
        return "NaI_1x1";
    }
        //        int NaI_2x2 = 0x0D;
    case NaI_3x3: {
        return "NaI_3x3";
    }
    case NaI_2x3: {
        return "NaI_2x3";
    }
    case NaI_1_5x1_5: {
        return "NaI_1_5x1_5";
    }
    case NaI_2x4x16: {
        return "NaI_2x4x16";
    }
    case NaI_3x5x16: {
        return "NaI_3x5x16";
    }
    case NaI_4x4x16: {
        return "NaI_4x4x16";
    }
    case NaI_2x2_HH300: {
        return "NaI_2x2";
    }
    case CLLBC_1_5x1_5: {
        return "CLLBC_1_5x1_5";
    }
    case CLLBC_2x2: {
        return "CLLBC_2x2";
    }
    case CLYC_1_5x1_5: {
        return "CLYC_1_5x1_5";
    }
    case CLYC_2x2: {
        return "CLYC_2x2";
    }
    case CSI_SPRD: {
        return "CsI";
    }
    case CLLBC_SPRD: {
        return "CLLBC";
    }
    case NaI_2x2_GP: {
        return "NaI_2x2_GP";
    }
    case NaI_3x3_GP: {
        return "NaI_3x3_GP";
    }
    default:
        NC_THROW_ARG_ERROR(QString("Not support detector code: ") + code);
    }
}

template<>
CrystalType nucare::fromString<CrystalType>(const QString &s)
{
    for (auto it = MAP_CRYSTAL_TYPE.begin(); it != MAP_CRYSTAL_TYPE.end(); ++it) {
        if (it.value() == s)
            return it.key();
    }

    NC_THROW_ARG_ERROR(QString("Can't support crystal type: ") + s);
}

template<> LibType_E nucare::fromString<LibType_E>(const QString &s)
{
    for (auto it = MAP_LIB_TYPE.begin(); it != MAP_LIB_TYPE.end(); it++) {
        if (it.value() == s) {
            return it.key();
        }
    }

    NC_THROW_ARG_ERROR(QString("Can't support lib type: ") + s);
}

template<> LibType nucare::fromString<LibType>(const QString& s)
{
    LibType ret;
    ret.name = s;

    if (s == "ANSI") {
        ret.cType = CrystalType::CSI;
        ret.type = LibType_E::LIB_ANSI;
    } else if (s == "IND") {
        ret.cType = CrystalType::CSI;
        ret.type = LibType_E::LIB_IND;
    } else if (s == "MED") {
        ret.cType = CrystalType::CSI;
        ret.type = LibType_E::LIB_MED;
    } else if (s == "ANSI_HG") {
        ret.cType = CrystalType::CLLBC;
        ret.type = LibType_E::LIB_ANSI;
    } else if (s == "MED_HG") {
        ret.cType = CrystalType::CLLBC;
        ret.type = LibType_E::LIB_MED;
    } else if (s == "IND_HG") {
        ret.cType = CrystalType::CLLBC;
        ret.type = LibType_E::LIB_IND;
    } else {
        NC_THROW_ARG_ERROR(QString("Doesnt support lib type ") + s + " for sprd");
    }


    return ret;
}
