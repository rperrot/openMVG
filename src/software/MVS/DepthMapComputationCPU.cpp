#include "DepthMapComputationCPU.hpp"

#include "DepthMapComputationCommon.hpp"
#include "MatchingCost.hpp"
#include "Util.hpp"

#include <random>
#include <array>

namespace MVS
{
/**
* @brief Compute cost value at specified pixel position
* @param id_row Y coordinate of the pixel
* @param id_col X coordinate of the pixel
* @param H Homography that maps points in first image to points in the second image
* @param params Computation parameters
* @param image_ref Image data of the first view
* @param image_other Image data of the second view
* @return Pixel cost using NCC value
*/
double ComputePixelCostNCC( const int id_row , const int id_col ,
                            const openMVG::Mat3 & H ,
                            const DepthMapComputationParameters & params ,
                            const Image & image_ref ,
                            const Image & image_other )
{
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
      if( ! image_ref.inside( y , x ) )
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

      if( ! image_other.inside( qy , qx ) )
      {
        return DepthMapComputationParameters::MAX_COST_NCC ;
      }

      const double v1 = static_cast<double>( image_ref.intensity( y , x ) ) / 255.0 ;
      const double v2 = static_cast<double>( image_other.intensity( qy , qx ) ) / 255.0 ;

      sum1 += v1 ;
      sum2 += v2 ;
      sum3 += v1 * v1 ;
      sum4 += v2 * v2 ;
      sum5 += v1 * v2 ;

      ++nb ;
    }
  }

  const double v1m = sum1 / ( double ) nb ;
  const double v2m = sum2 / ( double ) nb ;
  const double v12m = ( sum1 * sum1 ) / ( double ) nb ;
  const double v22m = ( sum2 * sum2 ) / ( double ) nb ;

  const double denom = std::sqrt( ( sum3 - v12m ) * ( sum4 - v22m ) ) ;
  const double ncc = ( sum5 - ( ( double ) nb ) * v1m * v2m ) / denom ;

  if( std::isinf( ncc ) || std::isnan( ncc ) )
  {
    return DepthMapComputationParameters::MAX_COST_NCC ;
  }
  else
  {
    return 1.0 - Clamp( ncc , -1.0 , 1.0 ) ;
  }
}

/**
* @brief Compute pixel cost using Patch Match metric
* @param id_row Y-coordinate of the pixel
* @param id_col X-coordinate of the pixel
* @param H Homography that maps points in first image to points in the second image
* @param params Computation parameters
* @param image_ref Image data of the first view
* @param image_other Image data of the second view
* @return Pixel cost using PatchMatch metric
*/
double ComputePixelCostPM( const int id_row ,
                           const int id_col ,
                           const openMVG::Mat3 & H ,
                           const DepthMapComputationParameters & params ,
                           const Image & image_ref ,
                           const Image & image_other )
{
  // TODO : bring it to a parameter
  const int window = 15 ;
  const int half_w = window / 2 ;

  const int Ic = image_ref.intensity( id_row , id_col );
  static const std::vector<double> exp_table = GetExpTable( params.gamma() ) ;
  std::vector< std::pair< openMVG::Vec2i , openMVG::Vec2i > > points_pair( window * window ) ;

  // Compute sampling positions
  int id = 0 ;
  for( int y = id_row - half_w ; y <= id_row + half_w ; y += 2 )
  {
    for( int x = id_col - half_w ; x <= id_col + half_w ; x += 2 )
    {
      if( ! image_ref.inside( y , x ) )
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

      if( ! image_other.inside( qy , qx ) )
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

    const unsigned char Ip = image_ref.intensity( p[1] , p[0] ) ;
    const openMVG::Vec4 & Gp = image_ref.gradient( p[1] , p[0] ) ;

    const unsigned char Iq = image_other.intensity( q[1] , q[0] ) ;
    const openMVG::Vec4 & Gq = image_other.gradient( q[1] , q[0] ) ;

    const int normI = std::abs( Ic - static_cast<int>( Ip ) ) ;
    const double w = exp_table[ normI ] ; // std::exp( - normI / params.Gamma() ) ;

    const double cost = ComputeMatchingCost( Ip , Gp , Iq , Gq , params.alpha() , params.tauI() , params.tauG() ) ;

    res += w * cost ;
  }
  return res ;
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
* @brief Compute cost value at specified pixel position
* @param id_row Y coordinate of the pixel
* @param id_col X coordinate of the pixel
* @param H Homography that maps points in first image to points in the second image
* @param params Computation parameters
* @param image_ref Image data of the first view
* @param image_other Image data of the second view
* @return Pixel cost using Census transform value
*/
double ComputePixelCostCensus( const int id_row , const int id_col ,
                               const openMVG::Mat3 & H ,
                               const DepthMapComputationParameters & params ,
                               const Image & image_ref ,
                               const Image & image_other )
{
  static constexpr int window = 15 ;
  static constexpr int half_w = window / 2 ;


  // @todo: pass as parameters instead of fixed values
  const double lambda_census = 30.0 ;
  const double lambda_ad     = 10.0 ;
  const double tau = 60.0 ;

  double total_distance = 0.0 ;
  for( int y = id_row - half_w ; y <= id_row + half_w ; y += 2 )
  {
    for( int x = id_col - half_w ; x <= id_col + half_w ; x += 2 )
    {
      if( ! image_ref.inside( y , x ) )
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

      if( ! image_other.inside( qy , qx ) )
      {
        return DepthMapComputationParameters::MAX_COST_CENSUS ;
      }

      // Census distance
      const unsigned long long val_p_census = image_ref.census( y , x ) ;
      const unsigned long long val_q_census = image_other.census( qy , qx ) ;
      const double census_dist = popcount( val_p_census ^ val_q_census ) ;

      // AD distance
      const double val_p_grayscale = image_ref.intensity( y , x ) ;
      const double val_q_grayscale = image_other.intensity( qy , qx ) ;
      const double grayscale_dist = std::min( tau , std::fabs( val_p_grayscale - val_q_grayscale ) ) ;

      total_distance += Proba( census_dist , lambda_census ) +
                        Proba( grayscale_dist , lambda_ad ) ;
    }
  }

  return total_distance ;
}


/**
* @brief compute cost between two images
* @param[out] cost Cost for all pixels
* @param planes The planes per pixel
* @param reference_cam First view camera
* @param other_cam Second view camera
* @param stereo_rig Stereo parameters (Rotation, translation) motion from first to second view
* @param image_ref Image data of the first view
* @param image_other Image data of the second view
* @param params Computation parameters
* @param scale Optionnal scale of the computation (if not specified , used the user specified resolution)
*/
void ComputeImagePairCost( openMVG::image::Image<double> & cost ,
                           const openMVG::image::Image<openMVG::Vec4> & planes ,
                           const Camera & reference_cam ,
                           const Camera & other_cam ,
                           const std::pair< openMVG::Mat3 , openMVG::Vec3 > & stereo_rig ,
                           const Image & image_ref ,
                           const Image & image_other ,
                           const DepthMapComputationParameters & params ,
                           const int scale )
{
  const double MAX_COST = DepthMapComputationParameters::metricMaxCostValue( params.metric() ) ;

  cost.resize( image_ref.width() , image_ref.height() , true , MAX_COST ) ;

  const cost_metric cur_metric = params.metric() ;

  const openMVG::Mat3 & cur_k = ( scale == -1 ) ? reference_cam.m_K : reference_cam.m_K_scaled[ scale ] ;
  const openMVG::Mat3 & other_k = ( scale == -1 ) ? other_cam.m_K : other_cam.m_K_scaled[ scale ] ;

  #pragma omp parallel for
  for( int id_row = 0 ; id_row < image_ref.height() ; ++id_row )
  {
    for( int id_col = 0 ; id_col < image_ref.width() ; ++id_col )
    {
      // 1 - Compute homography at this pixel
      const openMVG::Vec4 cur_plane = planes( id_row , id_col ) ;
      const openMVG::Vec3 cur_plane_n( cur_plane[0] , cur_plane[1] , cur_plane[2] ) ;
      const double cur_plane_d = cur_plane[3] ;
      const openMVG::Mat3 H = HomographyTransformation( stereo_rig.first ,    // R
                              stereo_rig.second ,   // t
                              cur_k ,   // first camera
                              other_k ,       // second camera
                              cur_plane_n , cur_plane_d ) ;

      // Compute cost at this pixel
      if( cur_metric == COST_METRIC_PM )
      {
        cost( id_row , id_col ) = ComputePixelCostPM( id_row , id_col , H , params , image_ref , image_other ) ;
      }
      else if( cur_metric == COST_METRIC_NCC )
      {
        cost( id_row , id_col ) = ComputePixelCostNCC( id_row , id_col , H , params , image_ref , image_other ) ;
      }
      else if( cur_metric == COST_METRIC_CENSUS )
      {
        cost( id_row , id_col ) = ComputePixelCostCensus( id_row , id_col , H , params , image_ref , image_other ) ;
      }
      else
      {
        std::cerr << "Undefined cost metric" << std::endl;
      }
    }
  }
}

/**
* @brief Compute cost using all neighboring images
* @param[out] Final cost value for all pixels
* @param planes The planes parameters for each pixel
* @param reference_cam The reference view camera
* @param cams Array of all cameras
* @param stereo_rig Array of all stereo rigs between reference and it's neighboring
* @param image_ref Image data of the reference view
* @param neigh_imgs Neighboring images
* @param params Computation parameters
*/
void ComputeMultipleViewCost( openMVG::image::Image<double> & cost ,
                              const openMVG::image::Image<openMVG::Vec4> & planes ,
                              const Camera & reference_cam ,
                              const std::vector< Camera > & cams ,
                              const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                              const Image & image_ref ,
                              const std::vector< Image > & neigh_imgs ,
                              const DepthMapComputationParameters & params ,
                              const int scale )
{
  std::vector< openMVG::image::Image<double> > all_costs( reference_cam.m_view_neighbors.size() ) ;

  // 1 compute cost for all images
  for( size_t id_cam = 0 ; id_cam < reference_cam.m_view_neighbors.size() ; ++id_cam )
  {
    const int id_neigh = reference_cam.m_view_neighbors[ id_cam ] ;
    const Camera & other_cam = cams[ id_neigh ] ;
    const std::pair< openMVG::Mat3 , openMVG::Vec3 > & cur_rig = stereo_rig[ id_cam ] ;

    ComputeImagePairCost( all_costs[ id_cam ] , planes , reference_cam , other_cam , cur_rig , image_ref , neigh_imgs[ id_cam ] , params , scale ) ;
  }

  const double MAX_COST = DepthMapComputationParameters::metricMaxCostValue( params.metric() ) ;
  cost.resize( image_ref.width() , image_ref.height() , true , MAX_COST ) ;

  // 2 Sort costs and compute final value
  const size_t K = std::min( ( size_t )params.nbMultiViewImageForCost() , reference_cam.m_view_neighbors.size() ) ;
  #pragma omp parallel for
  for( int id_row = 0 ; id_row < image_ref.height() ; ++id_row )
  {
    for( int id_col = 0 ; id_col < image_ref.width() ; ++id_col )
    {
      std::vector< double > cur_costs( reference_cam.m_view_neighbors.size() ) ;
      size_t nb_valid = 0 ;
      // 1 - retreive all costs
      for( size_t id_cam = 0 ; id_cam < reference_cam.m_view_neighbors.size() ; ++id_cam )
      {
        const double cur_c = all_costs[ id_cam ]( id_row , id_col ) ;
        const bool valid = cur_c < MAX_COST && ! std::isnan( cur_c ) && ! std::isinf( cur_c ) ;
        cur_costs[ id_cam ] =  valid ? cur_c : MAX_COST ;
        nb_valid += valid ? 1 : 0 ;
      }

      // 2 - Sort cost
      std::sort( cur_costs.begin() , cur_costs.end() ) ;

      double cur_sum = 0.0 ;
      //
      int nb_used = 0 ;
      for( size_t id = 0 ; id < K && id < nb_valid ; ++id , ++nb_used )
      {
        cur_sum += cur_costs[ id ] ;
      }

      // Compute final value
      cur_sum /= static_cast<double>( nb_used ) ;
      if( nb_valid > 0 &&
          cur_sum < MAX_COST &&
          cur_sum >= 0.0 &&
          ! std::isinf( cur_sum ) &&
          ! std::isnan( cur_sum ) )
      {
        cost( id_row , id_col ) = cur_sum ;
      }
      else
      {
        cost( id_row , id_col ) = MAX_COST ;
      }
    }
  }
}

/**
* @brief Compute multiple view cost for specified pixel
* @param id_row Y-coordinate of the pixel
* @param id_col X-coordinate of the pixel
* @param cur_normal Normal of the plane
* @param cur_d Plane parameter
* @param cams Array of all neighboring cameras
* @param stereo_rig Stereo motion wrt all neighboring cameras
* @param image_ref Image data of the reference view
* @param neigh_imgs Image data of all the neighbors of the camera
* @param params Computation parameters
* @return Multiple view cost for the specified pixel
*/
double ComputeMultiViewCost( const int id_row , const int id_col ,
                             const openMVG::Vec3 & cur_normal , // Normal parameter
                             const double & cur_d , // Plane d
                             const Camera & reference_cam ,
                             const std::vector< Camera > & cams ,
                             const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                             const Image & image_ref ,
                             const std::vector< Image > & neigh_imgs ,
                             const DepthMapComputationParameters & params ,
                             const int scale )
{
  const double MAX_COST = DepthMapComputationParameters::metricMaxCostValue( params.metric() ) ;

  const int K = params.nbMultiViewImageForCost() ;
  int nb_valid = 0 ;

  std::vector< double > costs( reference_cam.m_view_neighbors.size() ) ;

  // Get reference intrinsic
  const openMVG::Mat3 & reference_K = ( scale == -1 ) ? reference_cam.m_K : reference_cam.m_K_scaled[ scale ] ;

  // Compute cost for all camera
  for( size_t id_cam = 0 ; id_cam < reference_cam.m_view_neighbors.size() ; ++id_cam )
  {
    const int id_neigh = reference_cam.m_view_neighbors[ id_cam ] ;
    const Camera & cur_cam = cams[ id_neigh ] ;
    const std::pair< openMVG::Mat3 , openMVG::Vec3 > & cur_rig = stereo_rig[ id_cam ] ;

    // Get neigh intrinsic matrix
    const openMVG::Mat3 & cur_K = ( scale == -1 ) ? cur_cam.m_K : cur_cam.m_K_scaled[ scale ] ;

    const openMVG::Mat3 H = HomographyTransformation( cur_rig.first ,     // R
                            cur_rig.second ,    // t
                            reference_K ,       // first camera
                            cur_K ,             // second camera
                            cur_normal ,        // plane normal
                            cur_d ) ;           // plane parameter

    double cur_cost ;
    // Compute cost at specified position
    if( params.metric() == COST_METRIC_NCC )
    {
      cur_cost = ComputePixelCostNCC( id_row , id_col , H , params , image_ref , neigh_imgs[ id_cam ] ) ;
    }
    else if( params.metric() == COST_METRIC_PM )
    {
      cur_cost = ComputePixelCostPM( id_row , id_col , H , params , image_ref , neigh_imgs[ id_cam ] ) ;
    }
    else if( params.metric() == COST_METRIC_CENSUS )
    {
      cur_cost = ComputePixelCostCensus( id_row , id_col , H , params , image_ref , neigh_imgs[ id_cam ] ) ;
    }

    if( cur_cost < MAX_COST )
    {
      costs[ id_cam ] = cur_cost ;
      nb_valid ++ ;
    }
    else
    {
      costs[ id_cam ] = MAX_COST ;
    }
  }

  // Sort cost and compute sum of the K first
  std::sort( costs.begin() , costs.end() ) ;

  double cost = 0.0 ;
  int nb = 0 ;
  for( int k = 0 ; k < K && k < static_cast<int>( costs.size() ) && k < nb_valid ; ++k )
  {
    cost += costs[ k ] ;
    ++nb ;
  }

  cost /= static_cast<double>( nb ) ;

  // No valid
  if( nb == 0 || std::isnan( cost ) || std::isinf( cost ) || cost < 0.0 )
  {
    return MAX_COST ;
  }
  return cost ;
}

/**
* @brief Compute initial matching cost of specified camera
* @param map The depth map of the reference image
* @param reference_cam The reference view camera
* @param cams Array of neighboring cameras
* @param stereo_rig Array of all motions between reference and it's neighbors
* @param image_ref Reference image
* @param params Computation parameters
*/
void ComputeCost( DepthMap & map ,
                  const Camera & reference_cam ,
                  const std::vector< Camera > & cams ,
                  const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                  const Image & image_ref ,
                  const DepthMapComputationParameters & params )
{
  // Load images for the neighbors
  const ImageLoadType load_type = ComputeLoadType( params.metric() );
  std::vector< Image > neigh_imgs = LoadNeighborImages( reference_cam , params , load_type ) ;

  openMVG::image::Image<double> costs ;
  const openMVG::image::Image<openMVG::Vec4> & planes = map.planes() ;

  // 1 - Compute cost
  ComputeMultipleViewCost( costs , planes , reference_cam , cams , stereo_rig , image_ref , neigh_imgs , params ) ;

  // 2 - Store back cost
  for( int id_row = 0 ; id_row < map.height() ; ++id_row )
  {
    for( int id_col = 0 ; id_col < map.width() ; ++id_col )
    {
      map.cost( id_row , id_col , costs( id_row , id_col ) ) ;
    }
  }
}

/**
* @brief Compute initial cost at a specific scale
* @param map The depth map of the reference image
* @param reference_cam The reference view camera
* @param cams Array of neighboring cameras
* @param stereo_rig Array of all motions between reference and it's neighbors
* @param image_ref Reference image
* @param neigh_imgs Neighboring images
* @param params Computation parameters
* @param scale Scale of the computation
*/
void ComputeCost( DepthMap & map ,
                  const Camera & reference_cam ,
                  const std::vector< Camera > & cams ,
                  const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                  const Image & image_ref ,
                  const std::vector< Image > & neigh_imgs ,
                  const DepthMapComputationParameters & params ,
                  const int scale )
{
  openMVG::image::Image<double> costs ;
  const openMVG::image::Image<openMVG::Vec4> & planes = map.planes() ;

  // 1 - Compute cost
  ComputeMultipleViewCost( costs , planes , reference_cam , cams , stereo_rig , image_ref , neigh_imgs , params , scale ) ;

  // 2 - Store back cost
  for( int id_row = 0 ; id_row < map.height() ; ++id_row )
  {
    for( int id_col = 0 ; id_col < map.width() ; ++id_col )
    {
      map.cost( id_row , id_col , costs( id_row , id_col ) ) ;
    }
  }
}


/**
* @brief Perform propagation using Red or Black scheme
* @param[in,out] map The depth map to optimize
* @param id_start 0 if propagate Red , 1 if propagate Black
* @param cam Reference camera
* @param cams Neighboring cameras
* @param stereo_rig Array of motion between reference and it's neighbors
* @param image_ref Image data of the reference view
* @param params Computation parameters
*/
void Propagate( DepthMap & map , const int id_start ,
                Camera & cam ,
                const std::vector< Camera > & cams ,
                const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                const Image & image_ref ,
                const DepthMapComputationParameters & params )
{
  // Load images for the neighbors
  const ImageLoadType load_type = ComputeLoadType( params.metric() ) ;
  std::vector< Image > neigh_imgs = LoadNeighborImages( cam , params , load_type ) ;


  // (x,y)
  /*
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   | X |   | X |   |   |   |   |
   *   |   |   |   | X |   | X |   | X |   |   |   |
   *   | X |   | X |   | X | O | X |   | X |   | X |
   *   |   |   |   | X |   | X |   | X |   |   |   |
   *   |   |   |   |   | X |   | X |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   */
  const int neighs_idx[20][2] =
  {
    {0, -5} ,

    {0, -3} ,

    { -1, -2} ,
    {1, -2} ,

    { -2, -1} ,
    {0, -1} ,
    {2, -1} ,

    { -5, 0} ,
    { -3, 0} ,
    { -1, 0} ,
    {1, 0} ,
    {3, 0} ,
    {5, 0} ,

    { -2, 1} ,
    {0, 1} ,
    {2, 1} ,

    { -1, 2} ,
    {1, 2} ,

    {0, 3} ,

    {0, 5}
  } ;

  #pragma omp parallel for
  for( int id_row = 0 ; id_row < map.height() ; ++id_row )
  {
    const int pad = ( id_row % 2 == 0 ) ? id_start : ( id_start + 1 ) % 2 ;
    for( int id_col = pad ; id_col < map.width() ; id_col += 2  )
    {
      // Get neighbors using
      for( int id_n = 0 ; id_n < 20 ; ++id_n )
      {
        const int x = id_col + neighs_idx[ id_n ][ 0 ] ;
        const int y = id_row + neighs_idx[ id_n ][ 1 ] ;

        if( map.inside( y , x ) )
        {
          // Compute cost at given pixel using the other planes
          const openMVG::Vec4 & plane      = map.plane( y , x ) ;
          const openMVG::Vec3 plane_n( plane[0] , plane[1] , plane[2] ) ;
          const double plane_d           = plane[3] ;

          // Given the depth, compute the d value of the plane (ie intersection between ray and the plane)
          const double new_cost = ComputeMultiViewCost( id_row , id_col , plane_n , plane_d , cam , cams , stereo_rig , image_ref , neigh_imgs , params ) ;

          if( new_cost < map.cost( id_row , id_col ) )
          {
            // Copy cost
            map.cost( id_row , id_col , new_cost ) ;
            // Copy plane
            map.plane( id_row , id_col , plane ) ;

            // Compute current depth at this pixel
            const double z = ComputeDepth( plane , id_row , id_col , cam ) ;
            map.depth( id_row , id_col , z ) ;
          }
        }
      }
    }
  }
}


/**
* @brief Perform propagation using Red or Black scheme at specific scale
* @param[in,out] map The depth map to optimize
* @param id_start 0 if propagate Red , 1 if propagate Black
* @param cam Reference camera
* @param cams Neighboring cameras
* @param stereo_rig Array of motion between reference and it's neighbors
* @param image_ref Image data of the reference view
* @param params neigh_imgs Neighboring images
* @param params Computation parameters
* @param scale Scale of the computation
*/
void Propagate( DepthMap & map , const int id_start ,
                const Camera & cam ,
                const std::vector< Camera > & cams ,
                const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                const Image & image_ref ,
                const std::vector< Image > & neigh_imgs ,
                const DepthMapComputationParameters & params ,
                const int scale )
{
  // (x,y)
  /*
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   | X |   | X |   |   |   |   |
   *   |   |   |   | X |   | X |   | X |   |   |   |
   *   | X |   | X |   | X | O | X |   | X |   | X |
   *   |   |   |   | X |   | X |   | X |   |   |   |
   *   |   |   |   |   | X |   | X |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   */
  const int neighs_idx[20][2] =
  {
    {0, -5} ,

    {0, -3} ,

    { -1, -2} ,
    {1, -2} ,

    { -2, -1} ,
    {0, -1} ,
    {2, -1} ,

    { -5, 0} ,
    { -3, 0} ,
    { -1, 0} ,
    {1, 0} ,
    {3, 0} ,
    {5, 0} ,

    { -2, 1} ,
    {0, 1} ,
    {2, 1} ,

    { -1, 2} ,
    {1, 2} ,

    {0, 3} ,

    {0, 5}
  } ;

  #pragma omp parallel for
  for( int id_row = 0 ; id_row < map.height() ; ++id_row )
  {
    const int pad = ( id_row % 2 == 0 ) ? id_start : ( id_start + 1 ) % 2 ;
    for( int id_col = pad ; id_col < map.width() ; id_col += 2  )
    {
      // Get neighbors using
      for( int id_n = 0 ; id_n < 20 ; ++id_n )
      {
        const int x = id_col + neighs_idx[ id_n ][ 0 ] ;
        const int y = id_row + neighs_idx[ id_n ][ 1 ] ;

        if( map.inside( y , x ) )
        {
          // Compute cost at given pixel using the other planes
          const openMVG::Vec4 & plane      = map.plane( y , x ) ;
          const openMVG::Vec3 plane_n( plane[0] , plane[1] , plane[2] ) ;
          const double plane_d           = plane[3] ;

          // Given the depth, compute the d value of the plane (ie intersection between ray and the plane)
          const double new_cost = ComputeMultiViewCost( id_row , id_col , plane_n , plane_d , cam , cams , stereo_rig , image_ref , neigh_imgs , params , scale ) ;

          if( new_cost < map.cost( id_row , id_col ) )
          {
            // Copy cost
            map.cost( id_row , id_col , new_cost ) ;
            // Copy plane
            map.plane( id_row , id_col , plane ) ;

            // Compute current depth at this pixel
            const double z = ComputeDepth( plane , id_row , id_col , cam , scale ) ;
            map.depth( id_row , id_col , z ) ;
          }
        }
      }
    }
  }
}


/**
* @brief Perform plane refinement
* @param map Depth map to refine
* @param cam Reference camera
* @param cams Array of all neighboring cameras
* @param stereo_rig Array of motion between reference and its neighbors
* @param image_ref Image data of the reference view
* @param params Computation parameters
*/
void Refinement( DepthMap & map ,
                 const Camera & cam ,
                 const std::vector< Camera > & cams ,
                 const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                 const Image & image_ref ,
                 const DepthMapComputationParameters & params )
{
  // Load images for the neighbors
  const ImageLoadType load_type = ComputeLoadType( params.metric() ) ;
  const std::vector< Image > neigh_imgs = LoadNeighborImages( cam , params , load_type ) ;

  // Initialize RNG
  std::mt19937_64 rng ;
  std::random_device device;
  std::seed_seq seq{device(), device(), device(), device()};
  rng.seed( seq ) ;

  std::uniform_real_distribution<double> distrib_01( 0.0 , 1.0 ) ;

  // TODO: use another value (more data oriented)
  const double disparity_threshold = 0.01 ;


  #pragma omp parallel for
  for( int id_row = 0 ; id_row < map.height() ; ++id_row )
  {
    for( int id_col = 0 ; id_col < map.width() ; ++id_col )
    {
      openMVG::Vec3 cam_dir = cam.getViewVector( id_col , id_row ) ;  //  cam.GetRay( openMVG::Vec2( id_col , id_row ) ).second ;

      const double min_disparity = cam.depthDisparityConversion( cam.m_max_depth ) ;
      const double max_disparity = cam.depthDisparityConversion( cam.m_min_depth ) ;

      double delta_disparity = max_disparity / 2.0 ;
      double delta_angle = openMVG::D2R( 60.0 ) ;

      while( delta_disparity > disparity_threshold )
      {
        const double u1 = distrib_01( rng ) ;
        const double u2 = distrib_01( rng ) ;
        const double u3 = distrib_01( rng ) ;
        // const double u4 = distrib_01( rng ) ;


        // Compute new depth :
        const openMVG::Vec4 & plane = map.plane( id_row , id_col ) ;
        const openMVG::Vec3 cur_n( plane[0] , plane[1] , plane[2] ) ;
        const double        cur_d = map.depth( id_row , id_col ) ;
        const double        cur_disp = cam.depthDisparityConversion( cur_d ) ;

        const double min_delta  = - std::min( delta_disparity , cur_disp + min_disparity ) ;
        const double max_delta  = std::min( delta_disparity , max_disparity - cur_disp ) ;
        const double delta_disp = u1 * ( max_delta - min_delta ) + min_delta ;
        const double new_disp   = Clamp( cur_disp + delta_disp , min_disparity , max_disparity ) ;
        const double new_d      = cam.depthDisparityConversion( new_disp ) ;

        // Compute new normal
        // Sampling around a cone around current normal
        openMVG::Vec3 new_n = UniformSampleWRTSolidAngle( u2 , u3 , delta_angle , cur_n ) ;
        // Handle vector in same range of the view vector
        if( cam_dir.dot( new_n ) > 0.0 )
        {
          new_n = - new_n ;
        }

        // Compute plane d
        const double d_plane    = GetPlaneD( cam , id_row , id_col , new_d , new_n ) ;

        // Compute cost
        const double new_cost = ComputeMultiViewCost( id_row , id_col , new_n , d_plane , cam , cams , stereo_rig , image_ref , neigh_imgs , params ) ;

        if( new_cost < map.cost( id_row , id_col ) )
        {
          // Update value
          map.cost( id_row , id_col , new_cost ) ;
          map.plane( id_row , id_col , openMVG::Vec4( new_n[0] , new_n[1] , new_n[2] , d_plane ) ) ;
          map.depth( id_row , id_col , new_d ) ;
        }

        // Halve the range
        delta_disparity /= 2.0 ;
        delta_angle /= 2.0 ;
      }
    }
  }
}

/**
* @brief Perform plane refinement at specific scale
* @param map Depth map to refine
* @param cam Reference camera
* @param cams Array of all neighboring cameras
* @param stereo_rig Array of motion between reference and its neighbors
* @param image_ref Image data of the reference view
* @param params Computation parameters
*/
void Refinement( DepthMap & map ,
                 const Camera & cam ,
                 const std::vector< Camera > & cams ,
                 const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                 const Image & image_ref ,
                 const std::vector< Image > & neigh_imgs ,
                 const DepthMapComputationParameters & params ,
                 const int scale )
{
  // Initialize RNG
  std::mt19937_64 rng ;
  std::random_device device;
  std::seed_seq seq{device(), device(), device(), device()};
  rng.seed( seq ) ;

  std::uniform_real_distribution<double> distrib_01( 0.0 , 1.0 ) ;

  // TODO: use another value (more data oriented)
  const double disparity_threshold = 0.01 ;

  #pragma omp parallel for
  for( int id_row = 0 ; id_row < map.height() ; ++id_row )
  {
    for( int id_col = 0 ; id_col < map.width() ; ++id_col )
    {
      openMVG::Vec3 cam_dir = cam.getViewVector( id_col , id_row , scale ) ;  //  cam.GetRay( openMVG::Vec2( id_col , id_row ) ).second ;

      const double min_disparity = cam.depthDisparityConversion( cam.m_max_depth , scale ) ;
      const double max_disparity = cam.depthDisparityConversion( cam.m_min_depth , scale ) ;

      double delta_disparity = max_disparity / 2.0 ;
      double delta_angle     = openMVG::D2R( 60.0 ) ;

      while( delta_disparity > disparity_threshold )
      {
        const double u1 = distrib_01( rng ) ;
        const double u2 = distrib_01( rng ) ;
        const double u3 = distrib_01( rng ) ;


        // Compute new depth :
        const openMVG::Vec4 & plane = map.plane( id_row , id_col ) ;
        const openMVG::Vec3 cur_n( plane[0] , plane[1] , plane[2] ) ;
        const double        cur_d = map.depth( id_row , id_col ) ;
        const double        cur_disp = cam.depthDisparityConversion( cur_d , scale ) ;

        const double min_delta  = - std::min( delta_disparity , cur_disp + min_disparity ) ;
        const double max_delta  = std::min( delta_disparity , max_disparity - cur_disp ) ;
        const double delta_disp = u1 * ( max_delta - min_delta ) + min_delta ;
        const double new_disp   = Clamp( cur_disp + delta_disp , min_disparity , max_disparity ) ;
        const double new_d      = cam.depthDisparityConversion( new_disp , scale ) ;

        // Compute new normal
        openMVG::Vec3 new_n = UniformSampleWRTSolidAngle( u2 , u3 , delta_angle , cur_n ) ;
        // Handle vector in same range of the view vector
        if( cam_dir.dot( new_n ) > 0.0 )
        {
          new_n = - new_n ;
        }

        // Compute plane d
        const double d_plane    = GetPlaneD( cam , id_row , id_col , new_d , new_n , scale ) ;

        // Compute cost
        const double new_cost = ComputeMultiViewCost( id_row , id_col , new_n , d_plane , cam , cams , stereo_rig , image_ref , neigh_imgs , params , scale ) ;

        if( new_cost < map.cost( id_row , id_col ) )
        {
          // Update value
          map.cost( id_row , id_col , new_cost ) ;
          map.plane( id_row , id_col , openMVG::Vec4( new_n[0] , new_n[1] , new_n[2] , d_plane ) ) ;
          map.depth( id_row , id_col , new_d ) ;
        }

        // Halve the range
        delta_disparity /= 2.0 ;
        delta_angle /= 2.0 ;
      }
    }
  }
}

} // namespace MVS