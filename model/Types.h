#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <memory>
#include <vector>
#include <array>
#include <QString>

namespace nucare {

typedef unsigned int uint;
typedef uint32_t suint;     // Small uint
typedef uint8_t tuint;      // Tiny uint

} // namespace nucare


typedef std::array<double, 3> Coeffcients;
typedef std::array<double, 2> FWHM;
typedef std::array<double, 6> GeCoefficients;
typedef std::array<double, 5> PeakCoefficients;
typedef std::array<double, 2> FHM;
typedef std::array<double, 10> StdEff;

typedef std::pair<double, double> Threshold;
typedef std::pair<double, double> SmoothP;

template<class P>
using NcSP = std::shared_ptr<P>;

typedef std::vector<double> Vec1D;
typedef std::vector<Vec1D> Vec2D;

#include "model/Time.h"
// #include "DetectorCode.h"

#endif // TYPES_H
