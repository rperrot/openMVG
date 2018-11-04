#include "SFOPSlice.hpp"

#include "openMVG/features/sfop/SFOPUtilNumeric.hpp"

#include "openMVG/image/image_filtering.hpp"

#include "openMVG/features/sfop/SFOPDebug.hpp"

namespace openMVG
{
namespace features
{
namespace sfop
{

/**
   * @brief Construct a new SFOPSlice 
   * 
   * @param img       Input image 
   * @param idSlice   Id of the slice (relative to the current octave, from -1 to nbSlice )
   * @param idOctave  Id of the current octave 
   * @param NbSlice   Number of slice in this octave 
   */
SFOPSlice::SFOPSlice( const image::Image<float>& img, const int idSlice, const int idOctave, const int nbSlice )
    : m_base_img( img ),
      m_id_slice( idSlice ),
      m_id_octave( idOctave ),
      m_nb_slice( nbSlice )
{
}

void SFOPSlice::detect( const SFOPSlice& below, const SFOPSlice& above, std::vector<SFOPFeature>& kpts ) const
{
  // note: sigma is with respect to the current octave not an absolute one
  const float sigma         = std::pow( 2.0f, 1.0f + ( 1.0f + static_cast<float>( m_id_slice ) ) / static_cast<float>( m_nb_slice ) );
  const float scale_octave  = 1 << m_id_octave; //  ie: std::pow( 2.0f, static_cast<float>( m_id_octave ) );
  const float scale_octave2 = scale_octave * scale_octave;

  const image::Image<float>& cur_prec   = m_precision;
  const image::Image<float>& above_prec = above.m_precision;
  const image::Image<float>& below_prec = below.m_precision;

  float cube[ 3 ][ 3 ][ 3 ];

// Detect features using local max
#pragma omp parallel for schedule( dynamic, 8 )
  for ( int idRow = 1; idRow < cur_prec.Height() - 1; ++idRow )
    for ( int idCol = 1; idCol < cur_prec.Width() - 1; ++idCol )
    {
      const float cur_p = cur_prec( idRow, idCol );
      // Compute 3x3 cube
      // Limit memory access and prefer cached one
      if ( idCol == 1 )
      {
        for ( int dy = -1; dy <= 1; ++dy )
          for ( int dx = -1; dx <= 1; ++dx )
          {
            cube[ dx + 1 ][ dy + 1 ][ 0 ] = below_prec( dy + idRow, dx + idCol );
            cube[ dx + 1 ][ dy + 1 ][ 1 ] = cur_prec( dy + idRow, dx + idCol );
            cube[ dx + 1 ][ dy + 1 ][ 2 ] = above_prec( dy + idRow, dx + idCol );
          }
      }
      else
      {
        // Copy data from last positions (hope preceding will stay in cache)
        // 1. copy old x to current x-1
        cube[ 0 ][ 0 ][ 0 ] = cube[ 1 ][ 0 ][ 0 ];
        cube[ 0 ][ 1 ][ 0 ] = cube[ 1 ][ 1 ][ 0 ];
        cube[ 0 ][ 2 ][ 0 ] = cube[ 1 ][ 2 ][ 0 ];

        cube[ 0 ][ 0 ][ 1 ] = cube[ 1 ][ 0 ][ 1 ];
        cube[ 0 ][ 1 ][ 1 ] = cube[ 1 ][ 1 ][ 1 ];
        cube[ 0 ][ 2 ][ 1 ] = cube[ 1 ][ 2 ][ 1 ];

        cube[ 0 ][ 0 ][ 2 ] = cube[ 1 ][ 0 ][ 2 ];
        cube[ 0 ][ 1 ][ 2 ] = cube[ 1 ][ 1 ][ 2 ];
        cube[ 0 ][ 2 ][ 2 ] = cube[ 1 ][ 2 ][ 2 ];

        // 2. copy old (x+1) to current x
        cube[ 1 ][ 0 ][ 0 ] = cube[ 2 ][ 0 ][ 0 ];
        cube[ 1 ][ 1 ][ 0 ] = cube[ 2 ][ 1 ][ 0 ];
        cube[ 1 ][ 2 ][ 0 ] = cube[ 2 ][ 2 ][ 0 ];

        cube[ 1 ][ 0 ][ 1 ] = cube[ 2 ][ 0 ][ 1 ];
        cube[ 1 ][ 1 ][ 1 ] = cube[ 2 ][ 1 ][ 1 ];
        cube[ 1 ][ 2 ][ 1 ] = cube[ 2 ][ 2 ][ 1 ];

        cube[ 1 ][ 0 ][ 2 ] = cube[ 2 ][ 0 ][ 2 ];
        cube[ 1 ][ 1 ][ 2 ] = cube[ 2 ][ 1 ][ 2 ];
        cube[ 1 ][ 2 ][ 2 ] = cube[ 2 ][ 2 ][ 2 ];

        // 3 get new (x+1) values
        cube[ 2 ][ 0 ][ 0 ] = below_prec( idRow - 1, idCol + 1 );
        cube[ 2 ][ 1 ][ 0 ] = below_prec( idRow, idCol + 1 );
        cube[ 2 ][ 2 ][ 0 ] = below_prec( idRow + 1, idCol + 1 );

        cube[ 2 ][ 0 ][ 1 ] = cur_prec( idRow - 1, idCol + 1 );
        cube[ 2 ][ 1 ][ 1 ] = cur_prec( idRow, idCol + 1 );
        cube[ 2 ][ 2 ][ 1 ] = cur_prec( idRow + 1, idCol + 1 );

        cube[ 2 ][ 0 ][ 2 ] = above_prec( idRow - 1, idCol + 1 );
        cube[ 2 ][ 1 ][ 2 ] = above_prec( idRow, idCol + 1 );
        cube[ 2 ][ 2 ][ 2 ] = above_prec( idRow + 1, idCol + 1 );
      }

      const bool isLocalMax =
          // Max on bellow slice
          cur_p > cube[ 0 ][ 0 ][ 0 ] &&
          cur_p > cube[ 0 ][ 1 ][ 0 ] &&
          cur_p > cube[ 0 ][ 2 ][ 0 ] &&
          cur_p > cube[ 1 ][ 0 ][ 0 ] &&
          cur_p > cube[ 1 ][ 1 ][ 0 ] &&
          cur_p > cube[ 1 ][ 2 ][ 0 ] &&
          cur_p > cube[ 2 ][ 0 ][ 0 ] &&
          cur_p > cube[ 2 ][ 1 ][ 0 ] &&
          cur_p > cube[ 2 ][ 2 ][ 0 ] &&
          // Max on current slice
          cur_p > cube[ 0 ][ 0 ][ 1 ] &&
          cur_p > cube[ 0 ][ 1 ][ 1 ] &&
          cur_p > cube[ 0 ][ 2 ][ 1 ] &&
          cur_p > cube[ 1 ][ 0 ][ 1 ] &&
          //
          cur_p > cube[ 1 ][ 2 ][ 1 ] &&
          cur_p > cube[ 2 ][ 0 ][ 1 ] &&
          cur_p > cube[ 2 ][ 1 ][ 1 ] &&
          cur_p > cube[ 2 ][ 2 ][ 1 ] &&
          // Max on above slice
          cur_p > cube[ 0 ][ 0 ][ 2 ] &&
          cur_p > cube[ 0 ][ 1 ][ 2 ] &&
          cur_p > cube[ 0 ][ 2 ][ 2 ] &&
          cur_p > cube[ 1 ][ 0 ][ 2 ] &&
          cur_p > cube[ 1 ][ 1 ][ 2 ] &&
          cur_p > cube[ 1 ][ 2 ][ 2 ] &&
          cur_p > cube[ 2 ][ 0 ][ 2 ] &&
          cur_p > cube[ 2 ][ 1 ][ 2 ] &&
          cur_p > cube[ 2 ][ 2 ][ 2 ];

      if ( !isLocalMax )
        continue;

      // Compute 3d hessian
      Mat3f H;
      computeHessian( cube, H );

      if ( !checkNegativeDefinite( H ) )
        continue;

      // Compute dx,dy update to have optimal direction
      const Vec3f grad   = computeGradient( cube );
      const Vec3f update = -H.colPivHouseholderQr().solve( grad );

      // Check if optimal position is valid
      // not a nan and update norm <= 1.0
      if ( update.squaredNorm() > 1.0 ||
           update[ 0 ] != update[ 0 ] ||
           update[ 1 ] != update[ 1 ] ||
           update[ 2 ] != update[ 2 ] )
      {
        continue;
      }

      const float x    = idCol + update[ 0 ];
      const float y    = idRow + update[ 1 ];
      const float s    = sigma * std::pow( 2.0, update[ 2 ] / m_nb_slice );
      const float prec = cur_p + 0.5 * grad.dot( update );
      const float l2   = m_lambda2( static_cast<int>( std::floor( y + 0.5f ) ), static_cast<int>( std::floor( x + 0.5f ) ) );

#pragma omp critical
      {
        kpts.push_back( SFOPFeature( x * scale_octave,
                                     y * scale_octave,
                                     s * scale_octave,
                                     m_id_slice,
                                     m_id_octave,
                                     l2 / scale_octave2,
                                     prec * scale_octave2 ) );
      }
    }
}

/**
  * @brief Build slice (ie: compute eigenvalue and associated precision)
  * 
  */
void SFOPSlice::buildSlice( void )
{
  // Integration scale:
  const float sigma = std::pow( 2.0f, 1.0f + ( 1.0f + static_cast<float>( m_id_slice ) ) / static_cast<float>( m_nb_slice ) );
  // Differenciation scale: tau = sigma / k
  const int   k   = 3;
  const float tau = sigma / static_cast<float>( k );
  // Neighboring size:
  const float M = 12.0f * sigma * sigma + 1.0f;

  // Differenciation gradients
  image::Image<float> gx, gy;

  ImageGaussianXDerivativeFilter( m_base_img, tau, gx, 8 * tau, 8 * tau );
  ImageGaussianYDerivativeFilter( m_base_img, tau, gy, 8 * tau, 8 * tau );

  // Squares of the gradient
  const image::Image<float> gx2  = image::Image<float>( gx.cwiseProduct( gx ) );
  const image::Image<float> gy2  = image::Image<float>( gy.cwiseProduct( gy ) );
  const image::Image<float> gxgy = image::Image<float>( gx.cwiseProduct( gy ) );

  // Gaussian filtering of the gradients (using sigma)
  {
    image::Image<float> G_gx2, G_gxgy, G_gy2;

    image::ImageGaussianFilter( gx2, sigma, G_gx2 );
    image::ImageGaussianFilter( gy2, sigma, G_gy2 );
    image::ImageGaussianFilter( gxgy, sigma, G_gxgy );

    // 1. Compute lambda2: the smallest eigenvalues
    computeEigenValue( M, G_gx2, G_gxgy, G_gy2 );
  }
  // 2. Compute precision of the model
  image::Image<float> omega[ 3 ]; // Image model for 3 angles : [ 0 ; 60 ; 120 ]
  const float         angles[] = {0.f, M_PI / 3.f, 2.f * M_PI / 3.f};

  // Convolution kernels
  const Vec G   = image::ComputeGaussianKernel( 8 * sigma, sigma, false );
  const Vec xG  = image::ComputeGaussianTimesXKernel( 8 * sigma, sigma );
  const Vec x2G = image::ComputeGaussianTimesXSquaredKernel( 8 * sigma, sigma );

  for ( int idAngle = 0; idAngle < 3; ++idAngle )
  {
    // Compute model error for current angle
    const float cur_angle = angles[ idAngle ];

    // Compute rotated gradients with current angle
    image::Image<float> rx, ry;
    ImageRotatedGradient( gx, gy, cur_angle, rx, ry );

    // Compute structure tensor of the given model
    image::Image<float> rx2  = image::Image<float>( rx.cwiseProduct( rx ) );
    image::Image<float> rxry = image::Image<float>( 2.f * rx.cwiseProduct( ry ) );
    image::Image<float> ry2  = image::Image<float>( ry.cwiseProduct( ry ) );

    // Compute Image model (eq1 of poster)
    image::Image<float> filtered_rx2, filtered_ry2, filtered_rxry;

    ImageSeparableConvolution( rx2, x2G, G, filtered_rx2 );
    ImageSeparableConvolution( ry2, G, x2G, filtered_ry2 );
    ImageSeparableConvolution( rxry, xG, xG, filtered_rxry );

    omega[ idAngle ] = filtered_rx2 + filtered_rxry + filtered_ry2;
  }
  // Compute best omega (note: omega is not yet scaled by M is not )
  const image::Image<float> bestOmega = computeBestOmega( omega[ 0 ], omega[ 1 ], omega[ 2 ] );

  // (M-2)/M factor comes by the fact that bestOmega was not yet scaled by M.
  // As stated in the paper, the correct scaling is :
  // ( M - 2 ) * lambda2 / ( M * omega )
  // ->
  // ( ( M - 2 ) / M ) * lambda2 / omega
  computePrecision( ( M - 2 ) / M, bestOmega );
}

/**
   * @brief Compute eigen value 
   * 
   * The scaled smaller eigenvalue lambda2 is defined as
   *   lambda2 = M * (trace / 2 - sqrt(trace^2 / 4 - determinant))
   * with trace and determinant being
   *   trace = Nxx + Nyy
   *   determinant = Nxx * Nyy - Nxy^2;
   */
void SFOPSlice::computeEigenValue( const float                M,
                                   const image::Image<float>& Nxx,
                                   const image::Image<float>& Nxy,
                                   const image::Image<float>& Nyy )
{
  const image::Image<float> tr2 = image::Image<float>( 0.5f * ( Nxx + Nyy ) );
  const image::Image<float> det = image::Image<float>( Nxx.cwiseProduct( Nyy ) - Nxy.cwiseProduct( Nxy ) );

  m_lambda2 = M * ( tr2 - ( tr2.cwiseProduct( tr2 ) - det ).array().sqrt().matrix() );
}

/**
   * @brief Compute best omega 
   * 
   * @param omega0              Omega for angle = 0 
   * @param omega_60            Omega for angle = 60
   * @param omega_120           Omega for angle = 120 
   * 
   * @return compute omega that gives lowest model error   
   */
image::Image<float> SFOPSlice::computeBestOmega( const image::Image<float>& omega_0,
                                                 const image::Image<float>& omega_60,
                                                 const image::Image<float>& omega_120 )
{
  // 1/3 a + 2/3 b
  image::Image<float> tmp  = omega_0 + omega_60 + omega_120;
  image::Image<float> tmp2 = image::Image<float>( 2.f * ( omega_0.array().square().matrix() +
                                                          omega_60.array().square().matrix() +
                                                          omega_120.array().square().matrix() -
                                                          ( omega_0.cwiseProduct( omega_60 ) +
                                                            omega_60.cwiseProduct( omega_120 ) +
                                                            omega_0.cwiseProduct( omega_120 ) ) )
                                                            .array()
                                                            .sqrt()
                                                            .matrix() );
  return image::Image<float>( ( 1.f / 3.f ) * ( tmp - tmp2 ) );
}

void SFOPSlice::computePrecision( const float scale, const image::Image<float>& omega )
{
  m_precision = image::Image<float>( scale * ( m_lambda2.cwiseQuotient( omega ) ) );
}

} // namespace sfop
} // namespace features
} // namespace openMVG
