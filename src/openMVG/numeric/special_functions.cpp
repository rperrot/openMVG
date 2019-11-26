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
  if ( a == 0.0 )
  {
    // Special case for a == 0 ->  GammaUpper(0,x) == E1(x)
    return exponential_integral( 1, z );
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

double exponential_integral( const int i, const double x )
{
  // Note: use expint since c++17
  // @ref: numerical recipes
  // Euler Mascheroni constant.
  static const double EULER_g  = 0.5772156649015328606065120900824024310421593359399235988057672348848677267776646709369470632917467495;
  static const int    MAX_ITER = 2048;

  if ( ( i < 0 ) || ( x < 0.0 ) || ( ( x == 0.0 ) && ( ( i == 0 ) || ( i == 1 ) ) ) )
  {
    // Undefinite cases:
    // i negative
    // x negative
    // x==0 and (i==0) or (i==1) -> division by zero.
    return std::numeric_limits<double>::infinity();
  }

  if ( i == 0 )
  {
    // Special case
    return std::exp( -x ) / x;
  }

  // here n>1
  if ( x == 0.0 )
  {
    // Special case x == 0
    return 1.0 / static_cast<double>( i - 1 );
  }

  // Here we compute E_i(x) using two different schemes depending on the value of x.
  // if x > 1.0, we use the continous fraction (and Lentz algorithm)
  // f_n = A_n/B_n
  //
  //  A_n = -n(i-1+n)
  //  B_n = x + i + 2 * n
  //
  // if x < 1.0, we use series expansion
  //
  // f_n = (-x)^(i-1)/ (i-1) ! * (-ln x + psi(i)) - sum_k (-x)^k / (k-i+1) k!
  //
  if ( x > 1.0 )
  {
    // Continuous fraction using Lentz scheme.

    double b = x + (double)i;
    double c = std::numeric_limits<double>::max() * std::numeric_limits<double>::epsilon();
    double d = 1.0 / b;
    double f = d;

    for ( int k = 1; k <= MAX_ITER; ++k )
    {
      const double an = -k * ( i - 1 + k );
      b += 2.0;
      d = 1.0 / ( an * d + b );
      c = b + an / c;

      const double delta = c * d;

      f *= delta;

      if ( abs( delta - 1.0 ) < std::numeric_limits<double>::epsilon() )
      {
        // Early exit
        break;
      }
    }

    return f * std::exp( -x );
  }
  else
  {
    // Series expansion

    // First term
    double f    = ( i - 1 ) != 0 ? 1.0 / static_cast<double>( i - 1 ) : -log( x ) - EULER_g;
    double frac = 1.0; // Store -x ^ k / k !

    for ( int k = 1; k < MAX_ITER; ++k )
    {
      frac *= -x / k;

      double delta;
      if ( k != ( i - 1 ) )
      {
        delta = -frac / static_cast<double>( k - (i - 1) );
      }
      else
      {
        double psi = -EULER_g;
        for ( int p = 0; p < ( i - 1 ); ++p )
        {
          psi += 1.0 / p;
        }
        delta = frac * ( -std::log( x ) + psi );
      }

      f += delta;
      if ( abs( delta ) < abs( f ) * std::numeric_limits<double>::epsilon() )
      {
        // Early exit.
        break;
      }
    }

    return f;
  }
}

} // namespace numeric
} // namespace openMVG