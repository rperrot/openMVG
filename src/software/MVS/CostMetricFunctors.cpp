#include "CostMetricFunctors.hpp"
#include "MatchingCost.hpp"

#include "Util.hpp"

namespace MVS
{

/**
 * @brief Ctr
 * @param img_ref Reference image
 * @param img_other Target image
 * @param params Various parameters of the functor
 */
AbstractCostMetric::AbstractCostMetric( const Image& img_ref, const Image& img_other, const DepthMapComputationParameters& params )
    : m_image_ref( img_ref ),
      m_image_other( img_other ),
      m_params( params )
{
}

///////////////////////////////
///////////// ZNCC ////////////
///////////////////////////////

/**
 * @brief Ctr
 * @param img_ref Reference image
 * @param img_other Target image
 * @param params Various parameters of the functor
 */
ZNCCCostMetric::ZNCCCostMetric( const Image& img_ref, const Image& img_other, const DepthMapComputationParameters& params )
    : AbstractCostMetric( img_ref, img_other, params )
{
}

/**
 * @brief Compute aggregated matching cost at a given pixel
 * @param id_row Row index of the queried pixel
 * @param id_col Col index of the queried pixel
 * @param H Homography that maps pixels from Reference image to the Target image
 * @return Aggregated matching cost at specified pixel
 */
double ZNCCCostMetric::operator()( const int id_row, const int id_col, const openMVG::Mat3& H ) const
{
  // TODO get width directly using the parameters
  static constexpr int window = 15;
  static constexpr int half_w = window / 2;

  Eigen::Matrix<int, 2, window * window> refValues;
  Eigen::Matrix<int, 2, window * window> otherValues;

  // 1- retreive (ref|other) pixels positions
  int nb = 0;
  for ( int y = id_row - half_w; y <= id_row + half_w; y += 2 )
  {
    for ( int x = id_col - half_w; x <= id_col + half_w; x += 2 )
    {
      if ( !m_image_ref.inside( y, x ) )
      {
        return DepthMapComputationParameters::MAX_COST_NCC;
      }

      refValues( 0, nb ) = x;
      refValues( 1, nb ) = y;

      const openMVG::Vec3 p( x, y, 1.0 );
      const openMVG::Vec3 q = H * p;

      // Handle division by 0
      if ( fabs( q[ 2 ] ) <= std::numeric_limits<double>::epsilon() )
      {
        return DepthMapComputationParameters::MAX_COST_NCC;
      }

      const double inv = 1.0 / q[ 2 ];
      const int    qx  = q[ 0 ] * inv;
      const int    qy  = q[ 1 ] * inv;

      if ( !m_image_other.inside( qy, qx ) )
      {
        return DepthMapComputationParameters::MAX_COST_NCC;
      }

      otherValues( 0, nb ) = qx;
      otherValues( 1, nb ) = qy;

      ++nb;
    }
  }

  // Perform precomputation
  double sum1 = 0.0;
  double sum2 = 0.0;
  double sum3 = 0.0;
  double sum4 = 0.0;
  double sum5 = 0.0;
  for ( int id_pair = 0; id_pair < nb; ++id_pair )
  {
    const int x  = refValues( 0, id_pair );
    const int y  = refValues( 1, id_pair );
    const int qx = otherValues( 0, id_pair );
    const int qy = otherValues( 1, id_pair );

    const double v1 = static_cast<double>( m_image_ref.intensity( y, x ) );     // / 255.0 ;
    const double v2 = static_cast<double>( m_image_other.intensity( qy, qx ) ); // / 255.0 ;

    sum1 += v1;
    sum2 += v2;
    sum3 += v1 * v1;
    sum4 += v2 * v2;
    sum5 += v1 * v2;
  }
  /*
    nb = 0 ;
    for( int y = id_row - half_w ; y <= id_row + half_w ; y += 2 )
    {
      for( int x = id_col - half_w ; x <= id_col + half_w ; x += 2 )
      {
        if( ! m_image_ref.inside( y , x ) )
        {
          return DepthMapComputationParameters::MAX_COST_NCC ;
        }
        const openMVG::Vec3 p( x , y , 1.0 ) ;
        const openMVG::Vec3 q = H * p ;

        // Handle division by 0
        if( fabs( q[2] ) <= std::numeric_limits<double>::epsilon() )
        {
          return DepthMapComputationParameters::MAX_COST_NCC ;
        }

        const double inv = 1.0 / q[2] ;
        const int qx = q[0] * inv ;
        const int qy = q[1] * inv ;

        if( ! m_image_other.inside( qy , qx ) )
        {
          return DepthMapComputationParameters::MAX_COST_NCC ;
        }

        const double v1 = static_cast<double>( m_image_ref.intensity( y , x ) ) ; // / 255.0 ;
        const double v2 = static_cast<double>( m_image_other.intensity( qy , qx ) ) ; // / 255.0 ;

        sum1 += v1 ;
        sum2 += v2 ;
        sum3 += v1 * v1 ;
        sum4 += v2 * v2 ;
        sum5 += v1 * v2 ;

        ++nb ;
      }
    }
    */

  const double inv_nb = 1.0 / nb;

  const double v1m  = sum1 * inv_nb;
  const double v2m  = sum2 * inv_nb;            // / ( double ) nb ;
  const double v12m = ( sum1 * sum1 ) * inv_nb; // / ( double ) nb ;
  const double v22m = ( sum2 * sum2 ) * inv_nb; // / ( double ) nb ;

  const double denom = std::sqrt( ( sum3 - v12m ) * ( sum4 - v22m ) );
  const double ncc   = ( sum5 - ( (double)nb ) * v1m * v2m ) / denom;

  if ( std::isinf( ncc ) || std::isnan( ncc ) )
  {
    return DepthMapComputationParameters::MAX_COST_NCC;
  }
  else
  {
    // Result in range [0;2]
    return 1.0 - Clamp( ncc, -1.0, 1.0 );
  }
}

///////////////////////////////
///////////// PM //////////////
///////////////////////////////

/**
 * @brief Ctr
 * @param img_ref Reference image
 * @param img_other Target image
 * @param params Various parameters of the functor
 */
PatchMatchCostMetric::PatchMatchCostMetric( const Image& img_ref, const Image& img_other, const DepthMapComputationParameters& params )
    : AbstractCostMetric( img_ref, img_other, params )
{
}

/**
 * @brief Compute aggregated matching cost at a given pixel
 * @param id_row Row index of the queried pixel
 * @param id_col Col index of the queried pixel
 * @param H Homography that maps pixels from Reference image to the Target image
 * @return Aggregated matching cost at specified pixel
 */
double PatchMatchCostMetric::operator()( const int id_row, const int id_col, const openMVG::Mat3& H ) const
{
  const int                        Ic        = m_image_ref.intensity( id_row, id_col );
  static const std::vector<double> exp_table = GetExpTable( m_params.gamma() );

  // TODO get width directly using the parameters
  static constexpr int window = 15;
  static constexpr int half_w = window / 2;

  Eigen::Matrix<int, 2, window * window> refValues;
  Eigen::Matrix<int, 2, window * window> otherValues;

  // 1- retreive (ref|other) pixels positions
  int nb = 0;
  for ( int y = id_row - half_w; y <= id_row + half_w; y += 2 )
  {
    for ( int x = id_col - half_w; x <= id_col + half_w; x += 2 )
    {
      if ( !m_image_ref.inside( y, x ) )
      {
        return DepthMapComputationParameters::MAX_COST_NCC;
      }

      refValues( 0, nb ) = x;
      refValues( 1, nb ) = y;

      const openMVG::Vec3 p( x, y, 1.0 );
      const openMVG::Vec3 q = H * p;

      // Handle division by 0
      if ( fabs( q[ 2 ] ) <= std::numeric_limits<double>::epsilon() )
      {
        return DepthMapComputationParameters::MAX_COST_NCC;
      }

      const double inv = 1.0 / q[ 2 ];
      const int    qx  = q[ 0 ] * inv;
      const int    qy  = q[ 1 ] * inv;

      if ( !m_image_other.inside( qy, qx ) )
      {
        return DepthMapComputationParameters::MAX_COST_NCC;
      }

      otherValues( 0, nb ) = qx;
      otherValues( 1, nb ) = qy;

      ++nb;
    }
  }

  //    assert( id == window * window ) ;

  // Compute error
  double res = 0.0;
  for ( size_t id_pair = 0; id_pair < nb; ++id_pair )
  {
    const int x  = refValues( 0, id_pair );
    const int y  = refValues( 1, id_pair );
    const int qx = otherValues( 0, id_pair );
    const int qy = otherValues( 1, id_pair );

    const unsigned char  Ip = m_image_ref.intensity( y, x );
    const openMVG::Vec4& Gp = m_image_ref.gradient( y, x );

    const unsigned char  Iq = m_image_other.intensity( qy, qx );
    const openMVG::Vec4& Gq = m_image_other.gradient( qy, qx );

    const int    normI = std::abs( Ic - static_cast<int>( Ip ) );
    const double w     = exp_table[ normI ]; // std::exp( - normI / m_params.Gamma() ) ;

    const double cost = ComputeMatchingCost( Ip, Gp, Iq, Gq, m_params.alpha(), m_params.tauI(), m_params.tauG() );

    res += w * cost;
  }
  return res;
}

////////////////////////////////
///////////// Census ///////////
////////////////////////////////
/**
* @brief Ctr
* @param img_ref Reference image
* @param img_other Target image
* @param params Various parameters of the functor
*/
CensusCostMetric::CensusCostMetric( const Image& img_ref, const Image& img_other, const DepthMapComputationParameters& params )
    : AbstractCostMetric( img_ref, img_other, params )
{
}

/**
* @brief Convert value to a propabilistic value using exponential prior
* @param val Value to convert
* @param lambda Exponential prior
* @return probabilistic value
*/
static inline double Proba( const double val, const double lambda )
{
  return 1.0 - std::exp( -val / lambda );
}

/**
* @brief Compute aggregated matching cost at a given pixel
* @param id_row Row index of the queried pixel
* @param id_col Col index of the queried pixel
* @param H Homography that maps pixels from Reference image to the Target image
* @return Aggregated matching cost at specified pixel
*/
double CensusCostMetric::operator()( const int id_row, const int id_col, const openMVG::Mat3& H ) const
{
  static constexpr int window = 15;
  static constexpr int half_w = window / 2;

#define MODIFIED_AD_COST 1
  // @todo: pass as parameters instead of fixed values
#ifdef STANDARD_AD_COST
  const double lambda_census = 20.0;
  const double lambda_ad     = 20.0; // 60 if use RGB difference
  const double tau           = 60.0;
#elif defined MODIFIED_AD_COST
  static const double lambda_census = 30.0;
  static const double lambda_ad     = 10.0; // 60 if use RGB difference
#endif

  // Precomputed exponential table
  static const std::vector<double> exp_census_table = GetExpTable( lambda_census );
  static const std::vector<double> exp_ad_table     = GetExpTable( lambda_ad );

  double total_distance = 0.0;
  int    nb             = 0;
  for ( int y = id_row - half_w; y <= id_row + half_w; y += 2 )
  {
    for ( int x = id_col - half_w; x <= id_col + half_w; x += 2 )
    {
      if ( !m_image_ref.inside( y, x ) )
      {
        return DepthMapComputationParameters::MAX_COST_CENSUS;
      }
      const openMVG::Vec3 p( x, y, 1.0 );
      const openMVG::Vec3 q = H * p;

      // Handle division by 0
      if ( fabs( q[ 2 ] ) <= std::numeric_limits<double>::epsilon() )
      {
        return DepthMapComputationParameters::MAX_COST_CENSUS;
      }

      const double inv = 1.0 / q[ 2 ];
      const int    qx  = q[ 0 ] * inv;
      const int    qy  = q[ 1 ] * inv;

      if ( !m_image_other.inside( qy, qx ) )
      {
        return DepthMapComputationParameters::MAX_COST_CENSUS;
      }

      // Census distance
      const uint64_t val_p_census = m_image_ref.census( y, x );
      const uint64_t val_q_census = m_image_other.census( qy, qx );
      const int      census_dist  = popcount( val_p_census ^ val_q_census );

      // AD distance
      const int val_p_grayscale = m_image_ref.intensity( y, x );
      const int val_q_grayscale = m_image_other.intensity( qy, qx );
#ifdef STANDARD_AD_COST
      const double grayscale_dist = std::min( tau, std::fabs( val_p_grayscale - val_q_grayscale ) );
      total_distance += std::min( census_dist, lambda_census ) + std::min( grayscale_dist, lambda_ad );
#elif defined MODIFIED_AD_COST
      // Modified version :
      // http://www.nlpr.ia.ac.cn/2011papers/gjhy/gh75.pdf
      const int grayscale_dist = std::abs( val_p_grayscale - val_q_grayscale );

      // TODO : precompute exp values :
      // AD range may be between 0-256
      // census range may be between 0-64
      //
      total_distance += 2.0 - exp_census_table[ census_dist ] - exp_ad_table[ grayscale_dist ]; // ( std::exp( - census_dist / lambda_census ) + std::exp( - grayscale_dist / lambda_ad ) ) ;

      /* Proba( census_dist , lambda_census ) +
                        Proba( grayscale_dist , lambda_ad ) ;
                        */
#endif

      ++nb;
    }
  }
  return ( nb == 0 ) ? DepthMapComputationParameters::MAX_COST_CENSUS : ( total_distance / static_cast<double>( nb ) );
}

std::map<Image, std::shared_ptr<daisy>> DaisyCostMetric::all_daisy_descs;

/**
 * @brief Ctr
 * @param img_ref Reference image
 * @param img_other Target image
 * @param params Various parameters of the functor
 */
DaisyCostMetric::DaisyCostMetric( const Image& img_ref, const Image& img_other, const DepthMapComputationParameters& params )
    : AbstractCostMetric( img_ref, img_other, params )
{
  // Reference
  if ( all_daisy_descs.count( img_ref ) > 0 )
  {
    m_desc_ref = all_daisy_descs.at( img_ref );
  }
  else
  {
    m_desc_ref = std::make_shared<daisy>();
    // TODO: pass it in parameters
    const double rad = 8;
    const int    q   = 2;
    const int    t   = 4;
    const int    h   = 4;

    m_desc_ref->set_image( img_ref.intensity().data(), img_ref.height(), img_ref.width() );
    m_desc_ref->verbose( 0 );
    m_desc_ref->set_parameters( rad, q, t, h );
    m_desc_ref->set_normalization( NRM_FULL );
    m_desc_ref->initialize_single_descriptor_mode();
    m_desc_ref->compute_descriptors();
    m_desc_ref->normalize_descriptors();

    all_daisy_descs[ img_ref ] = m_desc_ref;
  }

  // Other
  if ( all_daisy_descs.count( img_other ) > 0 )
  {
    m_desc_other = all_daisy_descs.at( img_other );
  }
  else
  {
    m_desc_other = std::make_shared<daisy>();
    // TODO: pass it in parameters
    const double rad = 8;
    const int    q   = 2;
    const int    t   = 4;
    const int    h   = 4;

    m_desc_other->set_image( img_other.intensity().data(), img_other.height(), img_other.width() );
    m_desc_other->verbose( 0 );
    m_desc_other->set_parameters( rad, q, t, h );
    m_desc_other->set_normalization( NRM_FULL );
    m_desc_other->initialize_single_descriptor_mode();
    m_desc_other->compute_descriptors();
    m_desc_other->normalize_descriptors();

    all_daisy_descs[ img_other ] = m_desc_other;
  }
}

/**
 * @brief Compute aggregated matching cost at a given pixel
 * @param id_row Row index of the queried pixel
 * @param id_col Col index of the queried pixel
 * @param H Homography that maps pixels from Reference image to the Target image
 * @return Aggregated matching cost at specified pixel
 */
double DaisyCostMetric::operator()( const int id_row, const int id_col, const openMVG::Mat3& H ) const
{
  const openMVG::Vec3 p( id_col, id_row, 1.0 );
  const openMVG::Vec3 q = H * p;

  if ( fabs( q[ 2 ] ) <= std::numeric_limits<double>::epsilon() )
  {
    return DepthMapComputationParameters::MAX_COST_DAISY;
  }

  const double inv = 1.0 / q[ 2 ];
  const int    qx  = q[ 0 ] * inv;
  const int    qy  = q[ 1 ] * inv;

  if ( !m_image_ref.inside( id_row, id_col ) )
  {
    return DepthMapComputationParameters::MAX_COST_DAISY;
  }

  if ( !m_image_other.inside( qy, qx ) )
  {
    return DepthMapComputationParameters::MAX_COST_DAISY;
  }

  float* descA;
  float* descB;

  m_desc_ref->get_descriptor( id_row, id_col, descA );
  m_desc_other->get_descriptor( qy, qx, descB );

  const int size = 36; //  m_desc_ref->descriptor_size() ;

  Eigen::Map<Eigen::Matrix<float, 1, 36>> mapDescA( descA );
  Eigen::Map<Eigen::Matrix<float, 1, 36>> mapDescB( descB );

  const float sum = ( mapDescA - mapDescB ).squaredNorm();

  return 1.0 - std::exp( -static_cast<double>( sum ) );
}

/**
 * @brief Release internal memory
 */
void DaisyCostMetric::releaseInternalMemory( void )
{
  all_daisy_descs.clear();
}

/**
    * @brief Ctr
    * @param img_ref Reference image
    * @param img_other Target image
    * @param params Various parameters of the functor
    */
BilateralWeightedNCC::BilateralWeightedNCC( const Image& img_ref, const Image& img_other, const DepthMapComputationParameters& params )
    : AbstractCostMetric( img_ref, img_other, params )
{
}

static inline double weight( const double square_col_diff,
                             const double color_dispersion,
                             const double square_spatial_diff,
                             const double spatial_dispersion )
{
  return std::exp( -square_col_diff * color_dispersion - square_spatial_diff * spatial_dispersion );
}

/**
  * @brief Compute aggregated matching cost at a given pixel
  * @param id_row Row index of the queried pixel
  * @param id_col Col index of the queried pixel
  * @param H Homography that maps pixels from Reference image to the Target image
  * @return Aggregated matching cost at specified pixel
  */
double BilateralWeightedNCC::operator()( const int id_row, const int id_col, const openMVG::Mat3& H ) const
{
  // TODO: lots of computation could be saved in constructor then reused later
  const double sigma_color        = 0.3;
  const double sigma_dist         = 3.0;
  const double color_dispersion   = 1.0 / ( 2.0 * sigma_color * sigma_color );
  const double spatial_dispersion = 1.0 / ( 2.0 * sigma_dist * sigma_dist );
  const int    window             = 15;
  const int    half_w             = window / 2;
  const int    step               = 2;

  if ( ( ( id_row - half_w ) < 0 ) || ( ( id_row + half_w ) >= m_image_ref.height() ) ||
       ( ( id_col - half_w ) < 0 ) || ( ( id_col + half_w ) >= m_image_ref.width() ) )
  {
    return DepthMapComputationParameters::MAX_COST_BILATERAL_NCC;
  }

  double sum_w_ref       = 0.0;
  double sum_w_ref_sq    = 0.0;
  double sum_w_other     = 0.0;
  double sum_w_other_sq  = 0.0;
  double sum_w_ref_other = 0.0;
  double sum_w1          = 0.0;

  const double center_ref = static_cast<double>( m_image_ref.intensity( id_row, id_col ) ) / 255.0;

  for ( int y = id_row - half_w; y <= id_row + half_w; y += step )
  {
    for ( int x = id_col - half_w; x <= id_col + half_w; x += step )
    {
      if ( !m_image_ref.inside( y, x ) )
      {
        return DepthMapComputationParameters::MAX_COST_BILATERAL_NCC;
      }
      const openMVG::Vec3 p( x, y, 1.0 );
      const openMVG::Vec3 q = H * p;

      // Handle division by 0
      if ( fabs( q[ 2 ] ) <= std::numeric_limits<double>::epsilon() )
      {
        return DepthMapComputationParameters::MAX_COST_BILATERAL_NCC;
      }

      const double inv = 1.0 / q[ 2 ];
      const int    qx  = q[ 0 ] * inv;
      const int    qy  = q[ 1 ] * inv;

      if ( !m_image_other.inside( qy, qx ) )
      {
        return DepthMapComputationParameters::MAX_COST_BILATERAL_NCC;
      }

      // spatial diff
      const double dx                  = x - id_col;
      const double dy                  = y - id_row;
      const double square_spatial_diff = dx * dx + dy * dy;
      // Color diff

      const double v1 = static_cast<double>( m_image_ref.intensity( y, x ) ) / 255.0;
      const double v2 = static_cast<double>( m_image_other.intensity( qy, qx ) ) / 255.0;

      const double diff_ref = v1 - center_ref;                                                                          // TODO: precompute
      const double w_ref    = weight( diff_ref * diff_ref, color_dispersion, square_spatial_diff, spatial_dispersion ); // TODO: precompute

      sum_w_ref += w_ref * v1;         // TODO: precompute
      sum_w_ref_sq += w_ref * v1 * v1; // TODO: precompute

      sum_w_other += w_ref * v2;
      sum_w_other_sq += w_ref * v2 * v2;

      sum_w_ref_other += w_ref * v1 * v2;

      sum_w1 += w_ref; // TODO: precompute
    }
  }

  sum_w_ref /= sum_w1;
  sum_w_ref_sq /= sum_w1;

  sum_w_other /= sum_w1;
  sum_w_other_sq /= sum_w1;
  sum_w_ref_other /= sum_w1;

  const double variance_w_ref       = sum_w_ref_sq - ( sum_w_ref * sum_w_ref );
  const double variance_w_other     = sum_w_other_sq - ( sum_w_other * sum_w_other );
  const double variance_w_ref_other = sum_w_ref_other - ( sum_w_ref * sum_w_other );

  if ( variance_w_ref < 1e-6 ||
       variance_w_other < 1e-6 )
  {
    return DepthMapComputationParameters::MAX_COST_BILATERAL_NCC;
  }

  const double ncc = variance_w_ref_other / std::sqrt( variance_w_other * variance_w_ref );

  return 1.0 - Clamp( ncc, -1.0, 1.0 );
}

} // namespace MVS