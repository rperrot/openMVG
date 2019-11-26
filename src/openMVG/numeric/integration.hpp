#ifndef OPENMVG_NUMERIC_INTEGRATION_HPP
#define OPENMVG_NUMERIC_INTEGRATION_HPP

#include <functional>
#include <limits>

namespace openMVG
{
namespace numeric
{

/**
 * @brief Integration using the Gander and Gautschi method 
 * 
 * @param x_range 
 * @param tolerance 
 * @return double 
 * 
 * @ref ADAPTIVE QUADRATURE—REVISITED - WALTER GANDER and WALTER GAUTSCHI.
 */
double Integrate( const std::function<double( double )>& f,
                  const std::pair<double, double>&       x_range,
                  const double                           tolerance = std::numeric_limits<double>::epsilon() );

/**
  * @brief Perform 2D integration of a function f 
  * 
  * @param f        Function to integrate.
  * @param x_range  Range of integration on first variable 
  * @param y_range  Range of integration on second variable 
  * @param nbSplitX Number of subdivision on X range.
  * @param nbSplitY Number of subdivision on Y range.
  * 
  * @note f must be definite and continous on integration range (including bounds).
  * @ref ADAPTIVE QUADRATURE—REVISITED - WALTER GANDER and WALTER GAUTSCHI.
  */
double Integrate( const std::function<double( double, double )>& f,
                  const std::pair<double, double>&               x_range,
                  const std::pair<double, double>&               y_range,
                  const double                                   tolerance = 10e-6 );

} // namespace numeric
} // namespace openMVG

#endif //
