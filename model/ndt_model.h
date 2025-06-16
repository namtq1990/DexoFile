#ifndef NDT_MODEL_H
#define NDT_MODEL_H

#include <QObject>
#include "Types.h"
#include "util/nc_exception.h"

class IsoProfile : public QObject {
    Q_OBJECT
public:

    enum Isotope {
        Ba133,
        Eu152
    } isotope;

    Q_ENUM(Isotope)

    static const QMetaEnum s_enum_isotope;

    Threshold threshold_channel;
    Threshold threshold_energy;
    Threshold threshold_branching;

    IsoProfile(const Isotope isotope);
    static const char* name(const Isotope isotope) { return s_enum_isotope.valueToKey(isotope); }
    static Isotope isotopeFromName(const char* name);
    static IsoProfile* fromIsotope(const Isotope isotope);
};

struct ClogEstimation {
    double thickness = 0;
    double netCount1 = 0;
    double netCount2 = 0;
};

#endif // NDT_MODEL_H
