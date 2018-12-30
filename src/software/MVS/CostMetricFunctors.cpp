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

AbstractCostMetric::~AbstractCostMetric( void )
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

ZNCCCostMetric::~ZNCCCostMetric( void )
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
  * @brief Dtr
  * 
  */
PatchMatchCostMetric::~PatchMatchCostMetric( void )
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

CensusCostMetric::~CensusCostMetric( void )
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

DaisyCostMetric::~DaisyCostMetric( void )
{
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

  // Map between 0.0-2.0
  return 2.0 * ( 1.0 - std::exp( -static_cast<double>( sum ) ) );
}

/**
 * @brief Release internal memory
 */
void DaisyCostMetric::releaseInternalMemory( void )
{
  all_daisy_descs.clear();
}

/*
static inline double weight( const double square_col_diff,
                             const double color_dispersion,
                             const double square_spatial_diff,
                             const double spatial_dispersion )
{
  return std::exp( -square_col_diff * color_dispersion - square_spatial_diff * spatial_dispersion );
}
*/

std::vector<double> BilateralWeightedNCC::m_precomputed_spatial_weight;
std::vector<double> BilateralWeightedNCC::m_precomputed_color_weight;

/**
  * @brief Ctr
  * @param img_ref Reference image
  * @param img_other Target image
  * @param params Various parameters of the functor
  */
BilateralWeightedNCC::BilateralWeightedNCC( const Image& img_ref, const Image& img_other, const DepthMapComputationParameters& params )
    : AbstractCostMetric( img_ref, img_other, params )
{
  static const double norm_color = 1.0 / 255.0;

  // TODO: get sigma_color and sigma distance from parameters
  const double sigma_color    = 0.3;
  const double sigma_distance = 3.0;

  m_inv_sigma_color_2    = 1.0 / ( 2.0 * sigma_color * sigma_color );
  m_inv_sigma_distance_2 = 1.0 / ( 2.0 * sigma_distance * sigma_distance );

  // TODO: get window and step from parameters
  m_window      = 15;
  m_half_window = m_window / 2;
  m_step        = 2;

  if ( m_precomputed_spatial_weight.size() < ( ( 2 * m_half_window + 1 ) * ( 2 * m_half_window + 1 ) + 1 ) )
  {
    m_precomputed_spatial_weight.resize( ( 2 * m_half_window + 1 ) * ( 2 * m_half_window + 1 ) + 1 );
    for ( int window_row = -m_half_window; window_row <= m_half_window; window_row++ )
    {
      for ( int window_col = -m_half_window; window_col <= m_half_window; window_col++ )
      {
        const int id_y  = window_row + m_half_window;
        const int id_x  = window_col + m_half_window;
        const int index = id_x + ( 2 * m_half_window + 1 ) * id_y;
        assert( index < m_precomputed_spatial_weight.size() );

        m_precomputed_spatial_weight[ index ] = std::exp( -( window_row * window_row + window_col * window_col ) * m_inv_sigma_distance_2 );
      }
    }
  }
  if ( m_precomputed_color_weight.size() < 512 )
  {
    m_precomputed_color_weight.resize( 512 );
    for ( int color_diff = -255; color_diff <= 255; ++color_diff )
    {
      const double floatDiff = static_cast<double>( color_diff ) * norm_color;
      const size_t cur_index = 255 + color_diff;
      assert( cur_index < m_precomputed_color_weight.size() );
      m_precomputed_color_weight[ cur_index ] = std::exp( -( floatDiff * floatDiff ) * m_inv_sigma_color_2 );
    }
  }

  m_inv_sum_weights.resize( m_image_ref.width(), m_image_ref.height() );
  m_sum_w_ref.resize( m_image_ref.width(), m_image_ref.height() );
  m_variance_w_ref.resize( m_image_ref.width(), m_image_ref.height() );

  for ( int id_row = m_half_window; id_row < m_image_ref.height() - m_half_window - 1; ++id_row )
  {
    for ( int id_col = m_half_window; id_col < m_image_ref.width() - m_half_window - 1; ++id_col )
    {
      const int center_ref = m_image_ref.intensity( id_row, id_col );

      double sum_w        = 0.0;
      double sum_w_ref    = 0.0;
      double sum_w_ref_sq = 0.0;

      for ( int y = id_row - m_half_window; y <= id_row + m_half_window; y += m_step )
      {
        for ( int x = id_col - m_half_window; x <= id_col + m_half_window; x += m_step )
        {
          // spatial diff
          const double dx = x - id_col;
          const double dy = y - id_row;
          // Color diff

          const int    i1 = m_image_ref.intensity( y, x );
          const double v1 = static_cast<double>( i1 ) * norm_color;

          const double w_ref = weight( dy, dx, i1 - center_ref ); // diff_ref * diff_ref, m_inv_sigma_color_2, square_spatial_diff, m_inv_sigma_distance_2 );

          const double w_ref_v1 = w_ref * v1;

          sum_w += w_ref;
          sum_w_ref += w_ref_v1;
          sum_w_ref_sq += w_ref_v1 * v1;
        }
      }

      const double inv_sum             = 1.0 / sum_w;
      const double sum_w_ref_scaled    = sum_w_ref * inv_sum;
      const double sum_w_ref_sq_scaled = sum_w_ref_sq * inv_sum;

      m_inv_sum_weights( id_row, id_col ) = inv_sum;
      m_sum_w_ref( id_row, id_col )       = sum_w_ref_scaled;

      m_variance_w_ref( id_row, id_col ) = sum_w_ref_sq_scaled - ( sum_w_ref_scaled * sum_w_ref_scaled );
    }
  }
}

BilateralWeightedNCC::~BilateralWeightedNCC( void )
{
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
  static const double norm_color = 1.0 / 255.0;
  if ( ( ( id_row - m_half_window ) < 0 ) || ( ( id_row + m_half_window ) >= m_image_ref.height() ) ||
       ( ( id_col - m_half_window ) < 0 ) || ( ( id_col + m_half_window ) >= m_image_ref.width() ) )
  {
    return DepthMapComputationParameters::MAX_COST_BILATERAL_NCC;
  }

  double sum_w_other     = 0.0;
  double sum_w_other_sq  = 0.0;
  double sum_w_ref_other = 0.0;

  const int center_ref = m_image_ref.intensity( id_row, id_col );

  for ( int y = id_row - m_half_window; y <= id_row + m_half_window; y += m_step )
  {
    for ( int x = id_col - m_half_window; x <= id_col + m_half_window; x += m_step )
    {
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
      const int dx = x - id_col;
      const int dy = y - id_row;
      // Color diff
      const int i1 = m_image_ref.intensity( y, x );
      const int i2 = m_image_other.intensity( qy, qx );

      const double v1 = static_cast<double>( i1 ) * norm_color;
      const double v2 = static_cast<double>( i2 ) * norm_color;

      const double w_ref = weight( dy, dx, i1 - center_ref ); // weight( diff_ref * diff_ref, m_inv_sigma_color_2, square_spatial_diff, m_inv_sigma_distance_2 );

      const double w_ref_v2 = w_ref * v2;

      sum_w_other += w_ref_v2;
      sum_w_other_sq += w_ref_v2 * v2;
      sum_w_ref_other += w_ref_v2 * v1;
    }
  }

  const double inv = m_inv_sum_weights( id_row, id_col );

  sum_w_other *= inv;
  sum_w_other_sq *= inv;
  sum_w_ref_other *= inv;

  const double sum_w_ref = m_sum_w_ref( id_row, id_col ); // Note: already scaled

  const double variance_w_ref       = m_variance_w_ref( id_row, id_col );
  const double variance_w_other     = sum_w_other_sq - ( sum_w_other * sum_w_other );
  const double variance_w_ref_other = sum_w_ref_other - ( sum_w_other * sum_w_ref );

  if ( variance_w_ref < 1e-6 ||
       variance_w_other < 1e-6 )
  {
    return DepthMapComputationParameters::MAX_COST_BILATERAL_NCC;
  }

  const double ncc = variance_w_ref_other / std::sqrt( variance_w_other * variance_w_ref );

  return 1.0 - Clamp( ncc, -1.0, 1.0 );
}

double BilateralWeightedNCC::weight( const int row_diff, const int col_diff, const int delta_color ) const
{
  // uses precomputed weigth
  // exp( a + b ) = exp( a ) * exp( b )
  // exp( a ) could be precomputed (row_diff and col_diff are limited to 2 * window size)
  // exp( b ) also color_diff is limited since it corresponds to image pixel values (0;255)
  // for history, the computed value is:
  // return std::exp( -( color_diff * color_diff ) * color_dispersion - ( row_diff * row_diff + col_diff * col_diff ) * spatial_dispersion );

  const int    id_y           = row_diff + m_half_window;
  const int    id_x           = col_diff + m_half_window;
  const int    index          = id_x + id_y * m_window;
  const double spatial_weight = m_precomputed_spatial_weight[ index ];

  const int    i_color_diff = Clamp( delta_color + 255, 0, 510 );
  const double color_weight = m_precomputed_color_weight[ i_color_diff ];

  return spatial_weight * color_weight;
}

} // namespace MVS