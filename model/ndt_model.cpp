#include "ndt_model.h"

const QMetaEnum IsoProfile::s_enum_isotope = QMetaEnum::fromType<Isotope>();

static const QMap<IsoProfile::Isotope, IsoProfile*> S_ISOTOPE_PROFILES = {
    {IsoProfile::Ba133, new IsoProfile(IsoProfile::Ba133)},
    {IsoProfile::Eu152, new IsoProfile(IsoProfile::Eu152)}
};

IsoProfile::IsoProfile(const Isotope isotope) : isotope(isotope) {
    switch (isotope) {
    case Ba133:
        threshold_channel = {25, 46};
        threshold_energy = {80, 360};
        threshold_branching = {0.367, 0.71};
        break;
    case Eu152:
        threshold_channel = {125, 162};
        threshold_energy = {122, 344};
        threshold_branching = {0.284, 0.266};
        break;
    default:
        NC_THROW_ARG_ERROR(QString("Invalid isotope %1").arg(name(isotope)));
    }
}

IsoProfile::Isotope IsoProfile::isotopeFromName(const char *name)
{
    auto ret = s_enum_isotope.keyToValue(name);
    if (ret < 0) NC_THROW_ARG_ERROR(QString("Invaid isotope ") + name);
    return (IsoProfile::Isotope) ret;
}

IsoProfile* IsoProfile::fromIsotope(const Isotope isotope)
{
    return S_ISOTOPE_PROFILES.value(isotope, nullptr);
}
