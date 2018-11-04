#ifndef OPENMVG_FEATURES_SFOP_SFOP_UTIL_NUMERIC_HPP
#define OPENMVG_FEATURES_SFOP_SFOP_UTIL_NUMERIC_HPP

#include "openMVG/numeric/eigen_alias_definition.hpp"

namespace openMVG
{
namespace features
{
namespace sfop
{

/**
 * @brief Compute 3d hessian of the cube M 
 * 
 * @param M Input 3x3x3 Cube 
 * @param H Output hessian 
 */
void computeHessian( const float M[ 3 ][ 3 ][ 3 ], Mat3f& H );

/**
 * @brief Compute 3d gradient 
 * 
 * @param M       3x3x3 cube 
 * @return 3d Gradient 
 */
Vec3f computeGradient( const float M[ 3 ][ 3 ][ 3 ] );

/**
 * @brief Check if 3x3 matrix is negative definite
 * @param M Input matrix 
 * @retval true if M is negative definite 
 * @retval false if M is not negative definite 
 */
bool checkNegativeDefinite( const Mat3f& M );

/**
  * @brief Compute the inverse chi-square distribution
  *
  * @param[in] P Probability
  * @param[in] dim Degrees of freedom
  *
  * @return Inverse of the chi-square cumulative distribution function
  */
double chi2inv( const double P, const int dim );

} // namespace sfop
} // namespace features
} // namespace openMVG

#endif