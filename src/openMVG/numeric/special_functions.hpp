#ifndef OPENMVG_NUMERIC_SPECIAL_FUNCTIONS_HPP
#define OPENMVG_NUMERIC_SPECIAL_FUNCTIONS_HPP

namespace openMVG
{
namespace numeric
{

/**
 * @brief Compute the unnormalized upper incomplete gamma function.
 * 
 * @param a 
 * @param z > 0
 * @return double 
 */
double upper_incomplete_gamma( const double a, const double z );

/**
 * @brief Compute the unnormalized generalized incomplete gamma function.
 * 
 * @param a 
 * @param z0 
 * @param z1 
 * @return double 
 */
double generalized_incomplete_gamma( const double a, const double z0, const double z1 );

} // namespace numeric
} // namespace openMVG

#endif