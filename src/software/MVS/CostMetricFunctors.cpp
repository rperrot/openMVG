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
AbstractCostMetric::AbstractCostMetric( const Image & img_ref , const Image & img_other , const DepthMapComputationParameters & params )
  : m_image_ref( img_ref ) ,
    m_image_other( img_other ) ,
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
ZNCCCostMetric::ZNCCCostMetric( const Image & img_ref , const Image & img_other , const DepthMapComputationParameters & params )
  : AbstractCostMetric( img_ref , img_other , params )
{

}

/**
 * @brief Compute aggregated matching cost at a given pixel
 * @param id_row Row index of the queried pixel
 * @param id_col Col index of the queried pixel
 * @param H Homography that maps pixels from Reference image to the Target image
 * @return Aggregated matching cost at specified pixel
 */
double ZNCCCostMetric::operator()( const int id_row , const int id_col , const openMVG::Mat3 & H ) const
{
  // TODO get width directly using the parameters
  static constexpr int window = 15 ;
  static constexpr int half_w = window / 2 ;

  double sum1 = 0.0 ;
  double sum2 = 0.0 ;
  double sum3 = 0.0 ;
  double sum4 = 0.0 ;
  double sum5 = 0.0 ;

  int nb = 0 ;
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

      const double v1 = static_cast<double>( m_image_ref.intensity( y , x ) ) / 255.0 ;
      const double v2 = static_cast<double>( m_image_other.intensity( qy , qx ) ) / 255.0 ;

      sum1 += v1 ;
      sum2 += v2 ;
      sum3 += v1 * v1 ;
      sum4 += v2 * v2 ;
      sum5 += v1 * v2 ;

      ++nb ;
    }
  }

  const double inv_nb = 1.0 / nb ;

  const double v1m = sum1 * inv_nb ;
  const double v2m = sum2 * inv_nb ; // / ( double ) nb ;
  const double v12m = ( sum1 * sum1 ) * inv_nb ; // / ( double ) nb ;
  const double v22m = ( sum2 * sum2 ) * inv_nb ; // / ( double ) nb ;

  const double denom = std::sqrt( ( sum3 - v12m ) * ( sum4 - v22m ) ) ;
  const double ncc = ( sum5 - ( ( double ) nb ) * v1m * v2m ) / denom ;

  if( std::isinf( ncc ) || std::isnan( ncc ) )
  {
    return DepthMapComputationParameters::MAX_COST_NCC ;
  }
  else
  {
    // Result in range [0;2]
    return 1.0 - Clamp( ncc , -1.0 , 1.0 ) ;
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
PatchMatchCostMetric::PatchMatchCostMetric( const Image & img_ref , const Image & img_other , const DepthMapComputationParameters & params )
  : AbstractCostMetric( img_ref , img_other , params )
{

}

/**
 * @brief Compute aggregated matching cost at a given pixel
 * @param id_row Row index of the queried pixel
 * @param id_col Col index of the queried pixel
 * @param H Homography that maps pixels from Reference image to the Target image
 * @return Aggregated matching cost at specified pixel
 */
double PatchMatchCostMetric::operator()( const int id_row , const int id_col , const openMVG::Mat3 & H ) const
{
  // TODO : bring it to a parameter
  const int window = 15 ;
  const int half_w = window / 2 ;

  const int Ic = m_image_ref.intensity( id_row , id_col );
  static const std::vector<double> exp_table = GetExpTable( m_params.gamma() ) ;
  std::vector< std::pair< openMVG::Vec2i , openMVG::Vec2i > > points_pair( window * window ) ;

  // Compute sampling positions
  int id = 0 ;
  for( int y = id_row - half_w ; y <= id_row + half_w ; y += 2 )
  {
    for( int x = id_col - half_w ; x <= id_col + half_w ; x += 2 )
    {
      if( ! m_image_ref.inside( y , x ) )
      {
        return DepthMapComputationParameters::MAX_COST_PM ;
      }

      openMVG::Vec2i posP( x , y ) ;

      const openMVG::Vec3 p( x , y , 1.0 ) ;
      const openMVG::Vec3 q = H * p ;

      // Handle division by 0
      if( fabs( q[2] ) <= std::numeric_limits<double>::epsilon() )
      {
        return DepthMapComputationParameters::MAX_COST_PM ;
      }

      const double inv = 1.0 / q[2] ;

      const int qx = q[0] * inv ;
      const int qy = q[1] * inv ;

      if( ! m_image_other.inside( qy , qx ) )
      {
        return DepthMapComputationParameters::MAX_COST_PM ;
      }

      openMVG::Vec2i posQ( qx , qy ) ;

      points_pair[ id ] = std::make_pair( posP , posQ ) ;
      ++id ;
    }
  }

  //    assert( id == window * window ) ;

  // Compute error
  double res = 0.0 ;
  for( size_t id_pair = 0 ; id_pair < id ; ++id_pair )
  {
    const openMVG::Vec2i & p = points_pair[ id_pair ].first ;
    const openMVG::Vec2i & q = points_pair[ id_pair ].second ;

    const unsigned char Ip = m_image_ref.intensity( p[1] , p[0] ) ;
    const openMVG::Vec4 & Gp = m_image_ref.gradient( p[1] , p[0] ) ;

    const unsigned char Iq = m_image_other.intensity( q[1] , q[0] ) ;
    const openMVG::Vec4 & Gq = m_image_other.gradient( q[1] , q[0] ) ;

    const int normI = std::abs( Ic - static_cast<int>( Ip ) ) ;
    const double w = exp_table[ normI ] ; // std::exp( - normI / m_params.Gamma() ) ;

    const double cost = ComputeMatchingCost( Ip , Gp , Iq , Gq , m_params.alpha() , m_params.tauI() , m_params.tauG() ) ;

    res += w * cost ;
  }
  return res ;
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
CensusCostMetric::CensusCostMetric( const Image & img_ref , const Image & img_other , const DepthMapComputationParameters & params )
  : AbstractCostMetric( img_ref , img_other , params )
{

}


/**
* @brief Convert value to a propabilistic value using exponential prior
* @param val Value to convert
* @param lambda Exponential prior
* @return probabilistic value
*/
static inline double Proba( const double val , const double lambda )
{
  return 1.0 - std::exp( - val / lambda ) ;
}

/**
* @brief Compute aggregated matching cost at a given pixel
* @param id_row Row index of the queried pixel
* @param id_col Col index of the queried pixel
* @param H Homography that maps pixels from Reference image to the Target image
* @return Aggregated matching cost at specified pixel
*/
double CensusCostMetric::operator()( const int id_row , const int id_col , const openMVG::Mat3 & H ) const
{
  static constexpr int window = 15 ;
  static constexpr int half_w = window / 2 ;


#define MODIFIED_AD_COST 1 
  // @todo: pass as parameters instead of fixed values
#ifdef STANDARD_AD_COST
  const double lambda_census = 20.0 ;
  const double lambda_ad     = 20.0 ; // 60 if use RGB difference
  const double tau = 60.0 ;
#elif defined MODIFIED_AD_COST
  const double lambda_census = 30.0 ;
  const double lambda_ad     = 10.0 ; // 60 if use RGB difference
#endif

  double total_distance = 0.0 ;
  for( int y = id_row - half_w ; y <= id_row + half_w ; y += 2 )
  {
    for( int x = id_col - half_w ; x <= id_col + half_w ; x += 2 )
    {
      if( ! m_image_ref.inside( y , x ) )
      {
        return DepthMapComputationParameters::MAX_COST_CENSUS ;
      }
      const openMVG::Vec3 p( x , y , 1.0 ) ;
      const openMVG::Vec3 q = H * p ;

      // Handle division by 0
      if( fabs( q[2] ) <= std::numeric_limits<double>::epsilon() )
      {
        return DepthMapComputationParameters::MAX_COST_CENSUS ;
      }

      const double inv = 1.0 / q[2] ;
      const int qx = q[0] * inv ;
      const int qy = q[1] * inv ;

      if( ! m_image_other.inside( qy , qx ) )
      {
        return DepthMapComputationParameters::MAX_COST_CENSUS ;
      }

      // Census distance
      const unsigned long long val_p_census = m_image_ref.census( y , x ) ;
      const unsigned long long val_q_census = m_image_other.census( qy , qx ) ;
      const double census_dist = popcount( val_p_census ^ val_q_census ) ;

      // AD distance
      const double val_p_grayscale = m_image_ref.intensity( y , x ) ;
      const double val_q_grayscale = m_image_other.intensity( qy , qx ) ;
#ifdef STANDARD_AD_COST
      const double grayscale_dist = std::min( tau , std::fabs( val_p_grayscale - val_q_grayscale ) ) ;
      total_distance += std::min( census_dist , lambda_census ) + std::min( grayscale_dist , lambda_ad ) ;
#elif defined MODIFIED_AD_COST
      // Modified version :
      // http://www.nlpr.ia.ac.cn/2011papers/gjhy/gh75.pdf
      const double grayscale_dist = /* std::min( tau , */ std::fabs( val_p_grayscale - val_q_grayscale ) ; // ) ;

      total_distance += Proba( census_dist , lambda_census ) +
                        Proba( grayscale_dist , lambda_ad ) ;
#endif


    }
  }

  return total_distance ;
}

} // namespace MVS