#include "openMVG/numeric/integration.hpp"

#include <array>
#include <cmath>

namespace openMVG
{
namespace numeric
{

static double Integrate1d_recursion( const std::function<double( double )>& f,
                                     const double                           a,
                                     const double                           b,
                                     const double                           fa,
                                     const double                           fb,
                                     const double                           is )
{
  // @see description in function Integrate1d for full explaination.
  static const double alpha = sqrt( 2.0 / 3.0 );
  static const double beta  = 1.0 / sqrt( 5.0 );

  const double m = ( a + b ) / 2.0; // Midpoint
  const double h = ( b - a ) / 2.0; // Half interval size

  // Evaluation points
  const std::array<double, 7> x{
      {a,             // 0
       m - alpha * h, // 1
       m - beta * h,  // 2
       m,             // 3
       m + beta * h,  // 4
       m + alpha * h, // 5
       b}};           // 6
  // Evalution of the function at the given points.
  std::array<double, 7> fx;

  fx[ 0 ] = fa;
  for ( int i = 1; i < 6; ++i )
  {
    fx[ i ] = f( x[ i ] );
  }
  fx[ 6 ] = fb;

  // Gauss estimation
  const double i1 = ( h / 6.0 ) *
                    ( fx[ 0 ] + fx[ 6 ] +            // f(a) + f(b)
                      5.0 * ( fx[ 2 ] + fx[ 4 ] ) ); // 5.0 * ( f(m-beta h)  + f( m + beta h ) )
  // Lobatto estimation
  const double i2 = ( h / 1470.0 ) * ( 77.0 * ( fx[ 0 ] + fx[ 6 ] ) +  // 77 * ( fa + fb )
                                       432.0 * ( fx[ 1 ] + fx[ 5 ] ) + // 432 * ( f( m - alpha h ) + f( m + alpha h ) )
                                       625.0 * ( fx[ 2 ] + fx[ 4 ] ) + // 625 * ( f( m - beta h ) + f( m + beta h ) )
                                       672.0 * fx[ 3 ] );              // 672 * f( m )

  // Stopping criterion
  // either max precision reached or too small interval
  if ( is + ( i1 - i2 ) == is || x[ 1 ] < a || b < x[ 5 ] )
  {
    return i1;
  }
  else
  {
    // Split in 6 intervals and recurse.
    return Integrate1d_recursion( f, x[ 0 ], x[ 1 ], fx[ 0 ], fx[ 1 ], is ) +
           Integrate1d_recursion( f, x[ 1 ], x[ 2 ], fx[ 1 ], fx[ 2 ], is ) +
           Integrate1d_recursion( f, x[ 2 ], x[ 3 ], fx[ 2 ], fx[ 3 ], is ) +
           Integrate1d_recursion( f, x[ 3 ], x[ 4 ], fx[ 3 ], fx[ 4 ], is ) +
           Integrate1d_recursion( f, x[ 4 ], x[ 5 ], fx[ 4 ], fx[ 5 ], is ) +
           Integrate1d_recursion( f, x[ 5 ], x[ 6 ], fx[ 5 ], fx[ 6 ], is );
  }
}

double Integrate( const std::function<double( double )>& f, const std::pair<double, double>& x_range, const double tolerance )
{
  // The idea is to compute integral using two schemes
  // i1 = (scheme=Lobatto) ; i2 (scheme=Gauss)
  // Then we compare the difference between the two schemes.
  // If it is below a given threshold, we accept it. If not we
  // subdivide the interval and process each subdomains.
  //
  // Subdivision of interval is made on 6 sub-intervals:
  // [a;b] =>
  //          [ a           ; m - alpha h ]
  //          [ m - alpha h ; m - beta h ]
  //          [ m - beta h  ; m ]
  //          [ m           ; m + beta h ]
  //          [ m + beta h  ; m + alpha h ]
  //          [ m + alpha h ; b ]
  // With:
  //    h = 1/2 ( b - a )
  //    m = 1/2 ( a + b )
  //    alpha = sqrt( 2 ) / sqrt( 3 )
  //    beta  = 1.0 / sqrt( 5 )
  //
  // Adaptative gauss estimation:
  //
  // i1 = h / 1470 * ( 77 * ( f( a ) + f( b ) ) +
  //                   432 * ( f( m - alpha h ) + f( m + alpha h ) ) +
  //                   625 * ( f( m - beta h ) + f( m + beta h ) ) +
  //                   672 * f( m ) )
  // i2 = ( h / 6 ) * ( f(a) + f(b) + 5.0 * ( f( m - beta h ) + f( m + beta h ) )
  //
  //
  // Error of the integral is computed using the Kronrod extension of the Gauss estimation
  //
  // is = A ( f( a ) + f( b ) ) +
  //      B ( f( m - x1 * h ) + f( m + x1 * h ) ) +
  //      C ( f( m - alpha * h ) + f( m + alpha * h ) ) +
  //      D ( f( m - x2 * h ) + f( m + x2 * h ) ) +
  //      E ( f( m - beta * h ) + f( m + beta * h ) ) +
  //      F ( f( m - x3 * h ) + f( m + x3 * h ) ) +
  //      G ( f( m ) )
  //
  // With:
  // - x1, x2 , x3 the Kronrod points
  // - A,B,C,D,E,F coefficients used to etablish the Kronrod extension.
  //
  // errors are computed as follow:
  //  - err1 = abs( i1 - is )
  //  - err2 = abs( i2 - is )
  //
  // stoping criterion is:
  //  if( is + ( i1 - i2 ) == is or m <= a or b <= m )
  //    Stop ( and integral is i1)
  //  else
  //    recursive split.

  // The procedure is as follow:
  // 1. compute first value of is.
  // 2. Apply recursive split.

  // In recursive split:
  // - Compute the 13 values of f
  // - Compute i1, i2
  // - Test end of recursion

  static const double alpha = sqrt( 2.0 / 3.0 );
  static const double beta  = 1.0 / sqrt( 5.0 );
  // Constants from paper.
  static const double x1 = 0.94288241569547971905635175843185720232;
  static const double x2 = 0.64185334234578130578123554132903188354;
  static const double x3 = 0.23638319966214988028222377349205292599;
  static const double A  = 0.015827191973480183087169986733305510591;
  static const double B  = 0.094273840218850045531282505077108171960;
  static const double C  = 0.15507198733658539625363597980210298680;
  static const double D  = 0.18882157396018245442000533937297167125;
  static const double E  = 0.19977340522685852679206802206648840246;
  static const double F  = 0.22492646533333952701601768799639508076;
  static const double G  = 0.24261107190140773379964095790325635233;

  const double a = x_range.first;
  const double b = x_range.second;
  const double m = ( a + b ) / 2.0;
  const double h = ( b - a ) / 2.0;

  // Evaluation points
  std::array<double, 13> x{
      {a,
       m - x1 * h,
       m - alpha * h,
       m - x2 * h,
       m - beta * h,
       m - x3 * h,
       m,
       m + x3 * h,
       m + beta * h,
       m + x2 * h,
       m + alpha * h,
       m + x1 * h,
       b}};

  // Evaluate f on all points
  std::array<double, 13> fx;
  for ( int i = 0; i < 13; ++i )
  {
    fx[ i ] = f( x[ i ] );
  }

  // Gauss estimation
  const double i1 = ( h / 6.0 ) * ( fx[ 0 ] + fx[ 12 ] + 5.0 * ( fx[ 4 ] + fx[ 8 ] ) );
  // Lobatto estimation
  const double i2 = ( h / 1470.0 ) * ( 77.0 * ( fx[ 0 ] + fx[ 12 ] ) + 432.0 * ( fx[ 2 ] + fx[ 10 ] ) + 625.0 * ( fx[ 4 ] + fx[ 8 ] ) + 672.0 * fx[ 6 ] );
  // Compute is
  const double is = h * ( A * ( fx[ 0 ] + fx[ 12 ] ) +
                          B * ( fx[ 1 ] + fx[ 11 ] ) +
                          C * ( fx[ 2 ] + fx[ 10 ] ) +
                          D * ( fx[ 3 ] + fx[ 9 ] ) +
                          E * ( fx[ 4 ] + fx[ 8 ] ) +
                          F * ( fx[ 5 ] + fx[ 7 ] ) +
                          G * fx[ 6 ] );

  const double erri1 = fabs( i1 - is );
  const double erri2 = fabs( i2 - is );
  const double R     = ( erri2 != 0.0 ) ? erri1 / erri2 : 1.0; // Avoid division by 0.

  // Tolerance relaxation
  const double tol = ( R > 0.0 || R < 1.0 ) ? tolerance / R : tolerance;

  // Compute error is for the next of the computation.
  double integral_is = fabs( is ) * tol / std::numeric_limits<double>::epsilon();
  if ( integral_is == 0.0 )
    integral_is = b - a;

  // Now apply recursion.
  return Integrate1d_recursion( f, a, b, fx[ 0 ], fx[ 12 ], integral_is );
}

// Integration of Function f(x,y) for a fixed x
struct f_fixedX
{
public:
  f_fixedX( const std::function<double( double, double )>& f, const std::pair<double, double>& y_range, const double tolerance )
      : _f( f ), _yRange( y_range ), _tolerance( tolerance )
  {
  }

  // Integration
  double operator()( const double x )
  {
    std::function<double( double )> fx = std::bind( _f, x, std::placeholders::_1 );
    return Integrate( fx, _yRange, _tolerance );
  }

private:
  std::function<double( double, double )> _f;
  std::pair<double, double>               _yRange;
  double                                  _tolerance;
};

double Integrate( const std::function<double( double, double )>& f,
                  const std::pair<double, double>&               x_range,
                  const std::pair<double, double>&               y_range,
                  const double                                   tolerance )
{
  f_fixedX fx( f, y_range, tolerance );
  return Integrate( fx, x_range, tolerance );
}

} // namespace numeric
} // namespace openMVG