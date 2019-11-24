#include <openMVG/numeric/special_functions.hpp>

#include <cmath>
#include <limits>

#include <iostream>

namespace openMVG
{
namespace numeric
{

static double incomplete_gamma_continous_fraction( const double a, const double z )
{
  //
  // Continous fractions:
  //
  // Gamma( a , z ) = e^(-z) * z^a * F
  //
  //      F =         1
  //          ------------------------------------------------------
  //            z + 1 - a -               1 . ( 1 - a )
  //                       -----------------------------------------
  //                       z + 3 - a -        2 . ( 2 - a )
  //                                  ------------------------------
  //                                  z + 5 - a -       ....
  // Evaluation is made using the modified Lentz algorithm with recurence terms :
  //
  // a_n = - ( n . ( n - a ) )
  // b_n = z + ( 2n + 1 ) - a
  // a_0 = 1.0
  // b_0 = z + 1 - a

  // Remember Lentz :
  // Compute f_i = A_i / B_i with ratio being a continous fraction (A_i/B_i being a generic recurence relation or terms a_i and b_i)
  //
  // C_i = A_i / A_(i-1)
  //     = b_j + a_j / C_(i-i)            // Using recurence relation of continuous fractions.
  // D_i = B_(i-1) / B_i
  //     = 1.0 / ( b_i + a_i / D_i )      // Using recurence relation of continuous fractions.
  // f_i = f_(i-1) * C_i * D_i
  //
  // With:
  // f_0 = B_0 == 0 ? TINY : B_0
  // C_0 = f_0
  // D_0 = 0.0
  //
  // So the algorithm is:
  //
  // For i : 1..n
  //    D_i <- 1.0 / ( b_i + a_i * D_i )
  //    C_i <- b_i + a_i / C_i
  //    f_i <- C_i * D_i
  //
  // Modification by Thomson and Barret (to avoid null term C_i and division by zero in D_i):
  //
  // For i : 1..n
  //    D_i = b_i + a_i * D_i
  //    C_i = b_i + a_i / C_i
  //    if( D_i == 0 )
  //      D_i = TINY
  //    if( C_i == 0 )
  //      C_i = TINY
  //    D_i = 1.0 / D_i
  //    f_i = f_i * C_i * D_i

  // e^( a * log( z ) - z ) -- z^a * e^ (-z)
  static const double EPSILON = std::numeric_limits<double>::epsilon();
  static const double TINY    = std::numeric_limits<double>::min() / EPSILON;
  double              b, c, d, f;

  static const int MAX_ITER = 2048;

  b = z + 1.0 - a;

  // Note initialization is tricky because first term is not really A_0 / B_0
  c = 1.0 / TINY;
  d = 1.0 / b; // /!\ if z = a - 1.0
  f = d;

  for ( int i = 1; i < MAX_ITER; ++i )
  {
    const double a_i = i * ( a - i );
    b += 2.0; // b = z + 1.0 -a + 2 * i
    d = b + a_i * d;
    if ( fabs( d ) < TINY )
      d = TINY;
    c = b + a_i / c;
    if ( fabs( c ) < TINY )
      c = TINY;

    d                  = 1.0 / d;
    const double delta = c * d;
    f *= delta;

    // early exit
    if ( fabs( delta - 1.0 ) < EPSILON )
      break;
  }

  return f * exp( a * log( z ) - z );
}

static double incomplete_gamma_series( const double a, const double z )
{
  // Compute lower gamma function.
  // gamma( a , z ) = e^-z * z^a * F
  //              F = Sum_{i=1..n} z^i * ( Gamma(a) / Gamma( a + 1 + n ) )
  //
  // Since Gamma( a + 1 ) = a * Gamma( a )
  //
  // We got :
  //  - Gamma( a ) / Gamma( a + 1 )     = 1.0 / a
  //  - Gamma( a ) / Gamma( a + 2 )     = Gamma( a ) / ( ( a + 1 ) * Gamma( a ) )
  //                                    = 1.0 / ( a + 1 )
  //  - Gamma( a ) / Gamma( a + 3 )     = Gamma( a ) / ( ( a + 2 ) * Gamma( a + 1 ) )
  //                                    = Gamma( a ) / ( ( a + 2 ) * ( a + 1 ) * Gamma( a ) )
  //                                    = 1.0 / ( ( a + 2 ) * ( a + 1 ) )
  //                                ...
  //  - Gamma( a ) / Gamma( a + 1 + n ) = Gamma( a ) / ( ( a + n ) * Gamma( a + n ) )
  //                                    = 1.0 / ( ( a + n ) * Gamma( a + n - 1 + 1 ) )
  //                                    = 1.0 / ( ( a + n ) * ( a + n - 1 ) * Gamma( a + n - 1 ) )
  //                                    = ...
  //                                    = 1.0 / ( a + n ) ! n !
  //
  // So the sum reduces to:
  //
  // F = Sum_{i=1..n} z^i / ( a + i ) ! * i !

  static const int MAX_ITER = 2048;

  double ap  = a;
  double f   = 1.0 / a;
  double del = f;

  static const double EPSILON = std::numeric_limits<double>::epsilon();

  for ( int i = 0; i < MAX_ITER; ++i )
  {
    ap += 1.0;
    del *= z / ap;

    f += del;

    // early exit
    if ( fabs( del ) < fabs( f ) * EPSILON )
      break;
  }
  return f * exp( a * log( z ) - z );
}

double upper_incomplete_gamma( const double a, const double z )
{
  if ( z < 0.0 )
  {
    return std::numeric_limits<double>::infinity();
  }
  if ( z == 0.0 )
  {
    return 1.0;
  }
  if ( z < a + 1.0 )
  {
    // Better using series
    // GammaUpper = Gamma( x ) - Gamma_Lower( x , z )
    return tgamma( a ) - incomplete_gamma_series( a, z );
  }
  else
  {
    // Better using continous fractions
    return incomplete_gamma_continous_fraction( a, z );
  }
}

double generalized_incomplete_gamma( const double a, const double z0, const double z1 )
{
  return upper_incomplete_gamma( a, z0 ) - upper_incomplete_gamma( a, z1 );
}

} // namespace numeric
} // namespace openMVG