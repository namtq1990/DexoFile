#ifndef NDT_UTIL_H
#define NDT_UTIL_H

#include "model/Types.h"
#include "model/Material.h"
#include <vector> // Add this line

namespace ndt {

/**
 * @brief Calculates the mass attenuation coefficient for Iron.
 * @param En Photon energy in KeV.
 * @return Mass attenuation coefficient in cm2/g.
 * @throws std::out_of_range if En is outside the known data range.
 */
double Mass_Attenuation_coefficient_Iron(const Energy En, const Material material);

/**
 * @brief Estimates tc_est iteratively based on given parameters and mu values.
 *
 * This function performs an iterative calculation to refine the estimate of tc_est.
 * It uses a linear update rule derived from the provided input parameters.
 *
 * @param A A double value used in the numerator calculation.
 * @param B A double value used in the numerator calculation.
 * @param params A std::vector<double> containing polynomial coefficients (a1, b1, c1).
 * Expected to have at least 3 elements.
 * @param mu_ce1 A double value for mu_ce1.
 * @param mu_ce2 A double value for mu_ce2.
 * @param mu_pe1 A double value for mu_pe1.
 * @param mu_pe2 A double value for mu_pe2.
 * @param tp A double value for tp.
 * @param tc_est_initial The initial estimate for tc_est. It will be updated iteratively.
 * @return The refined estimate of tc_est after iteration, or the initial value if conditions are met.
 */
double estimate_tc_from_Est_E2(double A, double B, const std::vector<double>& params,
                               double mu_ce1, double mu_ce2,
                               double mu_pe1, double mu_pe2,
                               double tp, double tc_est_initial);

}

#endif // NDT_UTIL_H
