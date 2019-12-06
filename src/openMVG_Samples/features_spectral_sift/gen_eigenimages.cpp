#include <openMVG/numeric/eigen_alias_definition.hpp>
#include <openMVG/numeric/integration.hpp>
#include <openMVG/numeric/special_functions.hpp>

#include "third_party/cmdLine/cmdLine.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace openMVG;
using namespace openMVG::numeric;

double ipow( const double x, const int i )
{
  if ( i == 0 )
    return 1;
  if ( i == 1 )
    return x;
  if ( i == 2 )
    return x * x;

  const double sub = ipow( x, i / 2 );

  return sub * sub * ( ( i % 2 == 0 ) ? 1.0 : x );
}

// K^{Log}         Eq. 18 in paper
// @ref XY-Separable Scale-Space Filtering by Polynomial Representations and Its Applications, Gou KOUTAKI and Keiichi UCHIMURA
class SpectralK
{
public:
  SpectralK( const int i, const int j )
      : _i( i ),
        _j( j )
  {
  }

  double operator()( const double s, const double t ) const
  {
    const double s2     = s * s;
    const double t2     = t * t;
    const double s2t2   = s2 + t2;
    const double s2t2_3 = s2t2 * s2t2 * s2t2;

    return ( 4.0 / M_PI ) * ipow( s, _j + 2 ) * ipow( t, _i + 2 ) / s2t2_3;
  }

private:
  int _i;
  int _j;
};

// S^{Log}         Eq. 19 in paper
// @ref XY-Separable Scale-Space Filtering by Polynomial Representations and Its Applications, Gou KOUTAKI and Keiichi UCHIMURA
class SpectralS
{
public:
  SpectralS( const int i, const int j )
      : _i( i ), _j( j )
  {
  }

  double operator()( const double s )
  {
    return ipow( s, _i + _j );
  }

private:
  int _i;
  int _j;
};

/**
 * @brief Compute eigen vector coeficient at a specific degree (this is internal factor of Eq (20))
 * 
 * @param a   Eigenvector 
 * @param r   radius
 * @param i   current degree 
 * @param n   order of integration
 * @param s1  lower bound of scale 
 * @param s2  upper bound of scale 
 * @return double 
 */
double EigenScalar_order( const Eigen::MatrixXd& a,
                          const double           r,
                          const size_t           i,
                          const size_t           n,
                          const double           s1,
                          const double           s2 )
{
  const double r2   = r * r;
  const double s1_2 = 2.0 * s1 * s1;
  const double s2_2 = 2.0 * s2 * s2;

  const double res = a( i, n ) / ( std::sqrt( 2 ) * M_PI ) * std::pow( 1 / sqrt( 2 ), n ) *
                     std::pow( r, ( (double)n ) - 1.0 ) * ( -generalized_incomplete_gamma( ( 1.0 - n ) / 2.0, r2 / s1_2, r2 / s2_2 ) + generalized_incomplete_gamma( ( 3.0 - n ) / 2.0, r2 / s1_2, r2 / s2_2 ) );

  return res;
}

// Compute eigenimage coeficient at a given polar distance
// This is Eq 20
double EigenScalar( const Eigen::MatrixXd& a,
                    const double           r,
                    const size_t           i,
                    const size_t           N,
                    const double           s1,
                    const double           s2 )
{
  double res = 0.0;

  // Sum over all degree
  for ( size_t n = 0; n < N; ++n )
  {
    res += EigenScalar_order( a, r, i, n, s1, s2 );
  }
  return -res;
}

int main( int argc, char** argv )
{
  CmdLine cmd;

  int         iOrder           = 3;  // Order of the polynomial used to approximate the eigenfunctions.
  int         iFilterWidth     = 33; // Size of the eigenimage ( iFilterWidth x iFilterWidth )
  double      sLow             = 1.0;
  double      sUp              = 5.0;
  std::string sDirectoryOutput = ".";

  cmd.add( make_option( 'o', iOrder, "Polynomial order" ) );
  cmd.add( make_option( 'n', iFilterWidth, "Filter width" ) );
  cmd.add( make_option( 'd', sDirectoryOutput, "Directory in which eigen_images are written" ) );
  cmd.add( make_option( 'l', sLow, "Lower bound of scale space (s1)" ) );
  cmd.add( make_option( 'u', sUp, "Upper bound of the scale space (s2)" ) );

  // Compute K and S
  Mat K, S;
  K.resize( iOrder + 1, iOrder + 1 );
  S.resize( iOrder + 1, iOrder + 1 );

  for ( size_t i = 0; i <= iOrder; ++i )
  {
    for ( size_t j = 0; j <= iOrder; ++j )
    {
      K( i, j ) = Integrate( SpectralK( i, j ), std::make_pair( sLow, sUp ), std::make_pair( sLow, sUp ) );
      S( i, j ) = Integrate( SpectralS( i, j ), std::make_pair( sLow, sUp ) );
    }
  }

  std::cout << "K: " << std::endl
            << K << std::endl;
  std::cout << "S: " << std::endl
            << S << std::endl;

  //  Solve generalized eigen system Kx = lambda S x:
  Eigen::GeneralizedEigenSolver<Eigen::MatrixXd> ges;

  ges.compute( K, S );

  // Eigenvalues.
  std::cout << "l : " << ges.eigenvalues() << std::endl;
  // Eigenvectors.
  std::cout << "v : " << ges.eigenvectors().transpose() << std::endl;

  Mat v = ges.eigenvectors().real().transpose();

  // Orthonormalize eigenvectors v
  for ( int i = 0; i <= iOrder; ++i )
  {
    const Eigen::VectorXd r     = v.row( i );
    const double          res   = ( r.transpose() * S ).dot( r );
    const double          scale = ( i % 2 == 0 ? -1.0 : 1.0 ) * std::sqrt( res );

    for ( int j = 0; j <= iOrder; ++j )
    {
      v( i, j ) = v( i, j ) / scale;
    }
  }
  std::cout << "b: " << v << std::endl;

  // compute eigenimages F_i
  for ( int i = 0; i <= iOrder; ++i )
  {
    Mat F;
    F.resize( 2 * iFilterWidth + 1, 2 * iFilterWidth + 1 );

    for ( int x = -iFilterWidth; x <= iFilterWidth; ++x )
    {
      for ( int y = -iFilterWidth; y <= iFilterWidth; ++y )
      {
        double r = std::sqrt( x * x + y * y );
        if ( r == 0.0 )
          r += std::numeric_limits<double>::epsilon();

        F( y + iFilterWidth, x + iFilterWidth ) = EigenScalar( v /* eigenvector */, r /* distance */, i /* cur order */, iOrder /* order */, sLow, sUp );
      }
    }

    std::stringstream filename;
    filename << sDirectoryOutput << "/F_" << i << "_[" << iFilterWidth << "x" << iFilterWidth << "]_[" << iOrder << "]_[" << sLow << "_" << sUp << "]"
             << ".txt";
    std::ofstream file( filename.str() );
    if ( !file )
    {
      std::cerr << "Could not write " << filename.str() << std::endl;
    }
    else
    {
      for ( size_t i = 0; i < 2 * iFilterWidth + 1; ++i )
      {
        for ( size_t j = 0; j < 2 * iFilterWidth + 1; ++j )
        {
          file << std::setprecision( 10 ) << F( i, j ) << " ";
        }
        file << "\n";
      }
    }
  }

  // Write a_i
  std::stringstream filename;
  filename << sDirectoryOutput << "/a_" << iOrder << "_[" << sLow << "_" << sUp << "]"
           << ".txt";
  std::ofstream file( filename.str() );
  for ( size_t i = 0; i <= iOrder; ++i )
  {
    for ( size_t j = 0; j <= iOrder; ++j )
    {
      file << std::setprecision( 10 ) << v( i, j ) << " ";
    }
    file << "\n";
  }

  return EXIT_SUCCESS;
}