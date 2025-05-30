#include "util.h"
#include "nc_exception.h"

#include <math.h>

double nucare::math::quadraticEquation(const double a, const double b, const double c)
{
    auto delta = b * b - 4 * a * c;
    if (delta <= 0)
        NC_THROW_ARG_ERROR("Quadratic equation can't be solved.");

    return (-b + sqrt(delta)) / (2 * a);
}

double nucare::math::linear(const double xStart, const double xEnd, const double yStart, const double yEnd, double position)
{
    auto xRange = xEnd - xStart;
    auto yRange = yEnd - yStart;

    return yStart + yRange * (position - xStart) / xRange;
}
