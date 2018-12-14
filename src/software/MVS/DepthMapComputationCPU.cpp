#include "DepthMapComputationCPU.hpp"

#include "DepthMapComputationCommon.hpp"
#include "MatchingCost.hpp"
#include "Util.hpp"

#include <array>
#include <random>

namespace MVS
{

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
void ComputeImagePairCost( openMVG::image::Image<double>&                 cost,
                           const openMVG::image::Image<openMVG::Vec4>&    planes,
                           const Camera&                                  reference_cam,
                           const Camera&                                  other_cam,
                           const std::pair<openMVG::Mat3, openMVG::Vec3>& stereo_rig,
                           const Image&                                   image_ref,
                           const Image&                                   image_other,
                           const DepthMapComputationParameters&           params,
                           const std::shared_ptr<AbstractCostMetric>      cost_metric,
                           const int                                      scale )
{
  const double MAX_COST = DepthMapComputationParameters::metricMaxCostValue( params.metric() );

  cost.resize( image_ref.width(), image_ref.height(), true, MAX_COST );

#pragma omp parallel for
  for ( int id_row = 0; id_row < image_ref.height(); ++id_row )
  {
    for ( int id_col = 0; id_col < image_ref.width(); ++id_col )
    {
      // 1 - Compute homography at this pixel
      const openMVG::Vec4 cur_plane = planes( id_row, id_col );
      const openMVG::Mat3 H         = HomographyTransformation( stereo_rig.first,  // R
                                                        stereo_rig.second, // t
                                                        reference_cam,     // first camera
                                                        other_cam,         // second camera
                                                        cur_plane,
                                                        scale );

      // Compute cost at this pixel
      cost( id_row, id_col ) = ( *cost_metric )( id_row, id_col, H );
    }
  }
}

// Helper used to build cost metrics
static inline std::shared_ptr<AbstractCostMetric> CostMetricFactoryBuilder( const Image& image_ref, const Image& image_other, const DepthMapComputationParameters& params )
{
  switch ( params.metric() )
  {
  case COST_METRIC_NCC:
  {
    return std::make_shared<ZNCCCostMetric>( image_ref, image_other, params );
  }
  case COST_METRIC_PM:
  {
    return std::make_shared<PatchMatchCostMetric>( image_ref, image_other, params );
  }
  case COST_METRIC_CENSUS:
  {
    return std::make_shared<CensusCostMetric>( image_ref, image_other, params );
  }
  case COST_METRIC_DAISY:
  {
    return std::make_shared<DaisyCostMetric>( image_ref, image_other, params );
  }
  case COST_METRIC_BILATERAL_NCC:
  {
    return std::make_shared<BilateralWeightedNCC>( image_ref, image_other, params );
  }
  }
  return nullptr;
}

static inline void CostMetricFactoryClean( const DepthMapComputationParameters& params )
{
  switch ( params.metric() )
  {
  case COST_METRIC_DAISY:
  {
    DaisyCostMetric::releaseInternalMemory();
    break;
  }
  default:
  {
    ;
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
void ComputeMultipleViewCost( openMVG::image::Image<double>&                              cost,
                              const openMVG::image::Image<openMVG::Vec4>&                 planes,
                              const Camera&                                               reference_cam,
                              const std::vector<Camera>&                                  cams,
                              const std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>>& stereo_rig,
                              const Image&                                                image_ref,
                              const std::vector<Image>&                                   neigh_imgs,
                              const DepthMapComputationParameters&                        params,
                              const int                                                   scale )
{
  std::vector<openMVG::image::Image<double>> all_costs( reference_cam.m_view_neighbors.size() );

  // 1 compute cost for all images
  for ( size_t id_cam = 0; id_cam < reference_cam.m_view_neighbors.size(); ++id_cam )
  {
    const int                                      id_neigh  = reference_cam.m_view_neighbors[ id_cam ];
    const Camera&                                  other_cam = cams[ id_neigh ];
    const std::pair<openMVG::Mat3, openMVG::Vec3>& cur_rig   = stereo_rig[ id_cam ];

    std::shared_ptr<AbstractCostMetric> c_metric = CostMetricFactoryBuilder( image_ref, neigh_imgs[ id_cam ], params );

    ComputeImagePairCost( all_costs[ id_cam ], planes, reference_cam, other_cam, cur_rig, image_ref, neigh_imgs[ id_cam ], params, c_metric, scale );
  }

  const double MAX_COST = DepthMapComputationParameters::metricMaxCostValue( params.metric() );
  cost.resize( image_ref.width(), image_ref.height(), true, MAX_COST );

  // 2 Sort costs and compute final value
  const size_t K = std::min( (size_t)params.nbMultiViewImageForCost(), reference_cam.m_view_neighbors.size() );
#pragma omp parallel for
  for ( int id_row = 0; id_row < image_ref.height(); ++id_row )
  {
    for ( int id_col = 0; id_col < image_ref.width(); ++id_col )
    {
      std::vector<double> cur_costs( reference_cam.m_view_neighbors.size() );
      size_t              nb_valid = 0;
      // 1 - retreive all costs
      for ( size_t id_cam = 0; id_cam < reference_cam.m_view_neighbors.size(); ++id_cam )
      {
        const double cur_c  = all_costs[ id_cam ]( id_row, id_col );
        const bool   valid  = cur_c < MAX_COST && !std::isnan( cur_c ) && !std::isinf( cur_c );
        cur_costs[ id_cam ] = valid ? cur_c : MAX_COST;
        nb_valid += valid ? 1 : 0;
      }

      // 2 - Sort cost
      std::sort( cur_costs.begin(), cur_costs.end() );

      double cur_sum = 0.0;
      //
      int nb_used = 0;
      for ( size_t id = 0; id < K && id < nb_valid; ++id, ++nb_used )
      {
        cur_sum += cur_costs[ id ];
      }

      // Compute final value
      cur_sum /= static_cast<double>( nb_used );
      if ( nb_valid > 0 &&
           cur_sum < MAX_COST &&
           cur_sum >= 0.0 &&
           !std::isinf( cur_sum ) &&
           !std::isnan( cur_sum ) )
      {
        cost( id_row, id_col ) = cur_sum;
      }
      else
      {
        cost( id_row, id_col ) = MAX_COST;
      }
    }
  }

  CostMetricFactoryClean( params );
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
double ComputeMultiViewCost( const int id_row, const int id_col,
                             const openMVG::Vec3&                                        cur_normal, // Normal parameter
                             const double&                                               cur_d,      // Plane d
                             const Camera&                                               reference_cam,
                             const std::vector<Camera>&                                  cams,
                             const std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>>& stereo_rig,
                             const Image&                                                image_ref,
                             const std::vector<Image>&                                   neigh_imgs,
                             const DepthMapComputationParameters&                        params,
                             std::vector<std::shared_ptr<AbstractCostMetric>>            cost_metric,
                             const int                                                   scale )
{
  const double MAX_COST = DepthMapComputationParameters::metricMaxCostValue( params.metric() );

  const int K        = params.nbMultiViewImageForCost();
  int       nb_valid = 0;

  std::vector<double> costs( reference_cam.m_view_neighbors.size() );

  // Get reference intrinsic

  // Compute cost for all camera
  for ( size_t id_cam = 0; id_cam < reference_cam.m_view_neighbors.size(); ++id_cam )
  {
    const int                                      id_neigh = reference_cam.m_view_neighbors[ id_cam ];
    const Camera&                                  cur_cam  = cams[ id_neigh ];
    const std::pair<openMVG::Mat3, openMVG::Vec3>& cur_rig  = stereo_rig[ id_cam ];

    // Get neigh intrinsic matrix
    const openMVG::Mat3 H = HomographyTransformation( cur_rig.first,                                                             // R
                                                      cur_rig.second,                                                            // t
                                                      reference_cam,                                                             // first camera
                                                      cur_cam,                                                                   // second camera
                                                      openMVG::Vec4( cur_normal[ 0 ], cur_normal[ 1 ], cur_normal[ 2 ], cur_d ), // plane normal
                                                      scale );                                                                   // scale parameter

    // Compute cost at specified position
    const double cur_cost = ( *cost_metric[ id_cam ] )( id_row, id_col, H );

    if ( cur_cost < MAX_COST )
    {
      costs[ id_cam ] = cur_cost;
      nb_valid++;
    }
    else
    {
      costs[ id_cam ] = MAX_COST;
    }
  }

  // Sort cost and compute sum of the K first
  std::sort( costs.begin(), costs.end() );

  double cost = 0.0;
  int    nb   = 0;
  for ( int k = 0; ( k < K ) && ( k < static_cast<int>( costs.size() ) ) && ( k < nb_valid ) ; ++k )
  {
    cost += costs[ k ];
    ++nb;
  }

  cost /= static_cast<double>( nb );

  // No valid
  if ( nb == 0 || std::isnan( cost ) || std::isinf( cost ) || cost < 0.0 || cost > MAX_COST )
  {
    return MAX_COST;
  }
  return cost;
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
void ComputeCost( DepthMap&                                                   map,
                  const Camera&                                               reference_cam,
                  const std::vector<Camera>&                                  cams,
                  const std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>>& stereo_rig,
                  const Image&                                                image_ref,
                  const std::vector<Image>&                                   neigh_imgs,
                  const DepthMapComputationParameters&                        params,
                  const int                                                   scale )
{
  openMVG::image::Image<double>               costs;
  const openMVG::image::Image<openMVG::Vec4>& planes = map.planes();

  // 1 - Compute cost
  ComputeMultipleViewCost( costs, planes, reference_cam, cams, stereo_rig, image_ref, neigh_imgs, params, scale );

  // 2 - Store back cost
  for ( int id_row = 0; id_row < map.height(); ++id_row )
  {
    for ( int id_col = 0; id_col < map.width(); ++id_col )
    {
      map.cost( id_row, id_col, costs( id_row, id_col ) );
    }
  }
}

void DetermineNeighborBySampling( const DepthMap&                      map,
                                  const int                            id_row,
                                  const int                            id_col,
                                  const std::vector<std::vector<int>>& putative,
                                  std::vector<int>&                    res )
{
  int    best_id   = -1;
  double best_cost = std::numeric_limits<double>::max();
  for ( int id = 0; id < putative.size(); ++id )
  {
    const int y = id_row + putative[ id ][ 1 ];
    const int x = id_col + putative[ id ][ 0 ];

    if ( map.inside( y, x ) )
    {
      const double cost = map.cost( y, x );
      if ( cost < best_cost )
      {
        best_cost = cost;
        best_id   = id;
      }
    }
  }

  if ( best_id != -1 )
  {
    res[ 0 ] = putative[ best_id ][ 0 ];
    res[ 1 ] = putative[ best_id ][ 1 ];
  }
  else
  {
    res[ 0 ] = 0;
    res[ 1 ] = 0;
  }
}

/**
 * @brief Get the best neighbors to use for sampling 
 * 
 * @param map 
 * @param id_row 
 * @param id_col 
 * @return std::vector<std::vector<int>> 
 */
void DetermineNeighborBySampling( const DepthMap&                map,
                                  const int                      id_row,
                                  const int                      id_col,
                                  std::vector<std::vector<int>>& res )
{
  // Checkboard close north - asymetric
  static const std::vector<std::vector<int>> neigh_idx_asymetric_close_north =
      {
          {0, -1},
          {-1, -2},
          {1, -2},
          {-2, -3},
          {2, -3},
          {-3, -4},
          {3, -4}};
  // Checkboard close south - asymetric
  static const std::vector<std::vector<int>> neigh_idx_asymetric_close_south =
      {
          {0, 1},
          {-1, 2},
          {1, 2},
          {-2, 3},
          {2, 3},
          {-3, 4},
          {3, 4}};
  // Checkboard close west - asymetric
  static const std::vector<std::vector<int>> neigh_idx_asymetric_close_west =
      {
          {-1, 0},
          {-2, -1},
          {-2, 1},
          {-3, -2},
          {-3, 2},
          {-4, -3},
          {-4, 3}};
  // Checkboard close est - asymetric
  static const std::vector<std::vector<int>> neigh_idx_asymetric_close_east =
      {
          {1, 0},
          {2, -1},
          {2, 1},
          {3, -2},
          {3, 2},
          {4, -3},
          {4, 3}};
  static const std::vector<std::vector<int>> neigh_idx_asymetric_far_north =
      {
          {0, -3},
          {0, -5},
          {0, -7},
          {0, -9},
          {0, -11},
          {0, -13},
          {0, -15},
          {0, -17},
          {0, -19},
          {0, -21},
          {0, -23}};
  static const std::vector<std::vector<int>> neigh_idx_asymetric_far_south =
      {
          {0, 3},
          {0, 5},
          {0, 7},
          {0, 9},
          {0, 11},
          {0, 13},
          {0, 15},
          {0, 17},
          {0, 19},
          {0, 21},
          {0, 23}};
  static const std::vector<std::vector<int>> neigh_idx_asymetric_far_west =
      {
          {-3, 0},
          {-5, 0},
          {-7, 0},
          {-9, 0},
          {-11, 0},
          {-13, 0},
          {-15, 0},
          {-17, 0},
          {-19, 0},
          {-21, 0},
          {-23, 0}};
  static const std::vector<std::vector<int>> neigh_idx_asymetric_far_east =
      {
          {3, 0},
          {5, 0},
          {7, 0},
          {9, 0},
          {11, 0},
          {13, 0},
          {15, 0},
          {17, 0},
          {19, 0},
          {21, 0},
          {23, 0}};

  DetermineNeighborBySampling( map, id_row, id_col, neigh_idx_asymetric_close_north, res[ 0 ] );
  DetermineNeighborBySampling( map, id_row, id_col, neigh_idx_asymetric_close_south, res[ 1 ] );
  DetermineNeighborBySampling( map, id_row, id_col, neigh_idx_asymetric_close_west, res[ 2 ] );
  DetermineNeighborBySampling( map, id_row, id_col, neigh_idx_asymetric_close_east, res[ 3 ] );
  DetermineNeighborBySampling( map, id_row, id_col, neigh_idx_asymetric_far_north, res[ 4 ] );
  DetermineNeighborBySampling( map, id_row, id_col, neigh_idx_asymetric_far_south, res[ 5 ] );
  DetermineNeighborBySampling( map, id_row, id_col, neigh_idx_asymetric_far_west, res[ 6 ] );
  DetermineNeighborBySampling( map, id_row, id_col, neigh_idx_asymetric_far_east, res[ 7 ] );
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
void Propagate( DepthMap& map, const int id_start, const Camera& cam, const std::vector<Camera>& cams, const std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>>& stereo_rig, const Image& image_ref, const std::vector<Image>& neigh_imgs, const DepthMapComputationParameters& params, const int scale )
{
  // Build metrics
  std::vector<std::shared_ptr<AbstractCostMetric>> c_metrics;
  for ( const auto& image_other : neigh_imgs )
  {
    c_metrics.emplace_back( CostMetricFactoryBuilder( image_ref, image_other, params ) );
  }

  // (x,y) For full preset
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
  const std::vector<std::vector<int>> neighs_idx_full = //[20][2] =
      {
          {0, -5},

          {0, -3},

          {-1, -2},
          {1, -2},

          {-2, -1},
          {0, -1},
          {2, -1},

          {-5, 0},
          {-3, 0},
          {-1, 0},
          {1, 0},
          {3, 0},
          {5, 0},

          {-2, 1},
          {0, 1},
          {2, 1},

          {-1, 2},
          {1, 2},

          {0, 3},

          {0, 5}};

  // (x,y) For speed preset
  /*
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   | X |   |   |   | X | O | X |   |   |   | X |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   */
  const std::vector<std::vector<int>> neighs_idx_speed = //[8][2] =
      {
          {0, -5},
          {0, -1},
          {-5, 0},
          {-1, 0},
          {1, 0},
          {5, 0},
          {0, 1},
          {0, 5}};

  bool                          sampling = false;
  int                           nb_neigh = 0;
  std::vector<std::vector<int>> neighs_idx; // = neighs_idx_speed ;
  switch ( params.propagationScheme() )
  {
  case PROPAGATION_SCHEME_FULL:
  {
    nb_neigh   = 20;
    neighs_idx = neighs_idx_full;
    sampling   = false;
    break;
  }
  case PROPAGATION_SCHEME_SPEED:
  {
    nb_neigh   = 8;
    neighs_idx = neighs_idx_speed;
    sampling   = false;
    break;
  }
  case PROPAGATION_SCHEME_ASYMETRIC:
  {
    sampling = true;
    nb_neigh = 8; // 1 per region
    neighs_idx.resize( 8 );
    for ( int i = 0; i < 8; ++i )
    {
      neighs_idx[ i ].resize( 2 );
    }
    break;
  }
  default:
  {
    nb_neigh   = 20;
    neighs_idx = neighs_idx_full;
    sampling   = false;
  }
  }

#pragma omp parallel for
  for ( int id_row = 0; id_row < map.height(); ++id_row )
  {
    const int pad = ( id_row % 2 == 0 ) ? id_start : ( ( id_start + 1 ) % 2 );
    for ( int id_col = pad; id_col < map.width(); id_col += 2 )
    {
      if ( sampling )
      {
        // Sample each region to determine the neighbors to use
        DetermineNeighborBySampling( map, id_row, id_col, neighs_idx );
      }

      // Get neighbors in a fixed scheme
      for ( int id_n = 0; id_n < nb_neigh; ++id_n )
      {
        const int dx = neighs_idx[ id_n ][ 0 ];
        const int dy = neighs_idx[ id_n ][ 1 ];
        if ( dx == 0 && dy == 0 )
          continue;

        const int x = id_col + dx;
        const int y = id_row + dy;

        if ( map.inside( y, x ) )
        {
          // Compute cost at given pixel using the other planes
          const openMVG::Vec4& plane = map.plane( y, x );
          const openMVG::Vec3  plane_n( plane[ 0 ], plane[ 1 ], plane[ 2 ] );
          const double         plane_d = plane[ 3 ];

          // Given the depth, compute the d value of the plane (ie intersection between ray and the plane)
          const double new_cost = ComputeMultiViewCost( id_row, id_col, plane_n, plane_d, cam, cams, stereo_rig, image_ref, neigh_imgs, params, c_metrics, scale );

          if ( new_cost < map.cost( id_row, id_col ) )
          {
            // Copy cost
            map.cost( id_row, id_col, new_cost );
            // Copy plane
            map.plane( id_row, id_col, plane );

            // Compute current depth at this pixel
            const double z = ComputeDepth( plane, id_row, id_col, cam, scale );
            map.depth( id_row, id_col, z );
          }
        }
      }
    }
  }

  CostMetricFactoryClean( params );
} // namespace MVS

static inline void GenerateRandomVectorAndDisparity( double&              dispOut,
                                                     openMVG::Vec3&       normOut,
                                                     const double         disparity,
                                                     const openMVG::Vec3& normal,
                                                     const double         maxDeltaZ,
                                                     const double         maxDeltaN,
                                                     const double         minDisparity,
                                                     const double         maxDisparity,
                                                     const openMVG::Vec3& viewVector,
                                                     const Camera&        cam,
                                                     const int            scale,
                                                     std::mt19937_64&     rng )
{
  static std::uniform_real_distribution<double> distrib_01( 0.0, 1.0 );

  const double u1   = distrib_01( rng );
  const double n_u1 = distrib_01( rng );
  const double n_u2 = distrib_01( rng );
  const double n_u3 = distrib_01( rng );

  const double cur_d = cam.depthDisparityConversion( disparity, scale );

  float minDelta, maxDelta;
  minDelta = -std::min( maxDeltaZ, minDisparity + cur_d );
  maxDelta = std::min( maxDeltaZ, maxDisparity - cur_d );

  const double deltaZ = minDelta + u1 * ( maxDelta - minDelta );

  dispOut = std::min( std::max( cur_d + deltaZ, minDisparity ), maxDisparity );
  dispOut = cam.depthDisparityConversion( dispOut, scale );

  normOut[ 0 ] = normal[ 0 ] - maxDeltaN + 2.0 * n_u1 * maxDeltaN;
  normOut[ 1 ] = normal[ 1 ] - maxDeltaN + 2.0 * n_u2 * maxDeltaN;
  normOut[ 2 ] = normal[ 2 ] - maxDeltaN + 2.0 * n_u3 * maxDeltaN;
  normOut.normalize();

  if ( viewVector.dot( normOut ) > 0.0 )
  {
    normOut = -normOut;
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
void Refinement( DepthMap&                                                   map,
                 const Camera&                                               cam,
                 const std::vector<Camera>&                                  cams,
                 const std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>>& stereo_rig,
                 const Image&                                                image_ref,
                 const std::vector<Image>&                                   neigh_imgs,
                 const DepthMapComputationParameters&                        params,
                 const int                                                   scale )
{
  // Initialize RNG
  std::mt19937_64    rng;
  std::random_device device;
  std::seed_seq      seq{device(), device(), device(), device()};
  rng.seed( seq );

  std::uniform_real_distribution<double> distrib_01( 0.0, 1.0 );
  std::uniform_real_distribution<double> distrib_11( -1.0, 1.0 );

  // Build metrics
  std::vector<std::shared_ptr<AbstractCostMetric>> c_metrics;
  for ( const auto& image_other : neigh_imgs )
  {
    c_metrics.emplace_back( CostMetricFactoryBuilder( image_ref, image_other, params ) );
  }

  // TODO: use another value (more data oriented)
  const double disparity_threshold = 0.01;

  const int    nb_step  = 6;
  const double MAX_COST = DepthMapComputationParameters::metricMaxCostValue( params.metric() );

#pragma omp parallel for
  for ( int id_row = 0; id_row < map.height(); ++id_row )
  {
    for ( int id_col = 0; id_col < map.width(); ++id_col )
    {
      const openMVG::Vec3 cam_dir = cam.getViewVector( id_col, id_row, scale ); //  cam.GetRay( openMVG::Vec2( id_col , id_row ) ).second ;
#define USE_GIPUMA_REFINEMENT 
#ifdef USE_GIPUMA_REFINEMENT

      const double min_disparity = cam.depthDisparityConversion( cam.m_max_depth, scale );
      const double max_disparity = cam.depthDisparityConversion( cam.m_min_depth, scale );

      //      const double disparity_range = ( max_disparity - min_disparity ) ;

      double delta_disparity = max_disparity / 2.0;
      //      double delta_angle     = openMVG::D2R( 60.0 ) ;
      double delta_N = 1.0;

      while ( delta_disparity > disparity_threshold )
      {
        // Compute new depth :
        const openMVG::Vec4& plane = map.plane( id_row, id_col );
        const openMVG::Vec3  cur_n( plane[ 0 ], plane[ 1 ], plane[ 2 ] );
        const double         cur_depth = map.depth( id_row, id_col );

        double        new_depth;
        openMVG::Vec3 new_n;

        GenerateRandomVectorAndDisparity( new_depth, new_n, cur_depth, cur_n, delta_disparity, delta_N, min_disparity, max_disparity, cam_dir, cam, scale, rng );

        // Compute three planes : (new_n,new_d) - (new_n,old_d) - (old_n,new_d)

        // Compute plane parameter d with new_d, new_n
        const double d_plane_new_d_new_n = GetPlaneD( cam, id_row, id_col, new_depth, new_n, scale );
        const double d_plane_old_d_new_n = GetPlaneD( cam, id_row, id_col, cur_depth, new_n, scale );
        const double d_plane_new_d_old_n = GetPlaneD( cam, id_row, id_col, new_depth, cur_n, scale );

        // Compute cost
        const double cost_new_d_new_n = ComputeMultiViewCost( id_row, id_col, new_n, d_plane_new_d_new_n, cam, cams, stereo_rig, image_ref, neigh_imgs, params, c_metrics, scale );
        const double cost_old_d_new_n = ComputeMultiViewCost( id_row, id_col, new_n, d_plane_old_d_new_n, cam, cams, stereo_rig, image_ref, neigh_imgs, params, c_metrics, scale );
        const double cost_new_d_old_n = ComputeMultiViewCost( id_row, id_col, cur_n, d_plane_new_d_old_n, cam, cams, stereo_rig, image_ref, neigh_imgs, params, c_metrics, scale );

        double        best_cost = std::numeric_limits<double>::max();
        openMVG::Vec3 best_n;
        double        best_d_plane;
        double        best_depth;

        if ( cost_new_d_new_n < cost_new_d_old_n &&
             cost_new_d_new_n < cost_old_d_new_n )
        {
          best_cost    = cost_new_d_new_n;
          best_n       = new_n;
          best_d_plane = d_plane_new_d_new_n;
          best_depth   = new_depth;
        }
        else if ( cost_new_d_old_n < cost_new_d_new_n &&
                  cost_new_d_old_n < cost_old_d_new_n )
        {
          best_cost    = cost_new_d_old_n;
          best_n       = cur_n;
          best_d_plane = d_plane_new_d_old_n;
          best_depth   = new_depth;
        }
        else
        {
          best_cost    = cost_old_d_new_n;
          best_n       = new_n;
          best_d_plane = d_plane_old_d_new_n;
          best_depth   = cur_depth;
        }

        // {
        //   const double best_cost = std::min( cost_new_d_new_n , std::min( cost_new_d_old_n , cost_old_d_new_n ) ) ;
        // }
        //        const double new_cost = ComputeMultiViewCost( id_row , id_col , new_n , d_plane , cam , cams , stereo_rig , image_ref , neigh_imgs , params , c_metrics , scale ) ;

        if ( best_cost < map.cost( id_row, id_col ) )
        {
          // Update value
          map.cost( id_row, id_col, best_cost );
          map.plane( id_row, id_col, openMVG::Vec4( best_n[ 0 ], best_n[ 1 ], best_n[ 2 ], best_d_plane ) );
          map.depth( id_row, id_col, best_depth );
        }

        // Halve the range
        delta_disparity /= 10.0;
        delta_N /= 4.0;
      }
#else
      const int nb_refinement_step = 6;
      double    depth_min          = cam.m_min_depth;
      double    depth_max          = cam.m_max_depth;

      double delta_N = 1.0;

      for ( int id_ref = 0; id_ref < nb_refinement_step; ++id_ref )
      {
        const double d_u1 = distrib_01( rng );
        const double n_u1 = distrib_11( rng );
        const double n_u2 = distrib_11( rng );
        const double n_u3 = distrib_11( rng );

        // Current values
        const openMVG::Vec4& plane = map.plane( id_row, id_col );
        const openMVG::Vec3  cur_n( plane[ 0 ], plane[ 1 ], plane[ 2 ] );
        const double         cur_depth = map.depth( id_row, id_col );

        // Compute depth and normal
        const double new_d = depth_min + d_u1 * ( depth_max - depth_min );

        openMVG::Vec3 new_n;
        new_n[ 0 ] = cur_n[ 0 ] + n_u1 * delta_N;
        new_n[ 1 ] = cur_n[ 1 ] + n_u2 * delta_N;
        new_n[ 2 ] = cur_n[ 2 ] + n_u3 * delta_N;
        new_n.normalize();

        if ( cam_dir.dot( new_n ) > 0.0 )
        {
          new_n = -new_n;
        }

        // Compute three planes : (new_n,new_d) - (new_n,old_d) - (old_n,new_d)
        // Compute plane parameter d with new_d, new_n
        const double d_plane_new_d_new_n = GetPlaneD( cam, id_row, id_col, new_d, new_n, scale );
        const double d_plane_old_d_new_n = GetPlaneD( cam, id_row, id_col, cur_d, new_n, scale );
        const double d_plane_new_d_old_n = GetPlaneD( cam, id_row, id_col, new_d, cur_n, scale );

        // Compute cost
        const double cost_new_d_new_n = ComputeMultiViewCost( id_row, id_col, new_n, d_plane_new_d_new_n, cam, cams, stereo_rig, image_ref, neigh_imgs, params, c_metrics, scale );
        const double cost_old_d_new_n = ComputeMultiViewCost( id_row, id_col, new_n, d_plane_old_d_new_n, cam, cams, stereo_rig, image_ref, neigh_imgs, params, c_metrics, scale );
        const double cost_new_d_old_n = ComputeMultiViewCost( id_row, id_col, cur_n, d_plane_new_d_old_n, cam, cams, stereo_rig, image_ref, neigh_imgs, params, c_metrics, scale );

        double        best_cost = MAX_COST;
        openMVG::Vec3 best_n;
        double        best_d_plane;
        double        best_depth;

        if ( cost_new_d_new_n < cost_new_d_old_n &&
             cost_new_d_new_n < cost_old_d_new_n )
        {
          best_cost    = cost_new_d_new_n;
          best_n       = new_n;
          best_d_plane = d_plane_new_d_new_n;
          best_depth   = new_d;
        }
        else if ( cost_new_d_old_n < cost_new_d_new_n &&
                  cost_new_d_old_n < cost_old_d_new_n )
        {
          best_cost    = cost_new_d_old_n;
          best_n       = cur_n;
          best_d_plane = d_plane_new_d_old_n;
          best_depth   = new_d;
        }
        else
        {
          best_cost    = cost_old_d_new_n;
          best_n       = new_n;
          best_d_plane = d_plane_old_d_new_n;
          best_depth   = cur_d;
        }

        if ( ( best_cost < map.cost( id_row, id_col ) ) && ( best_cost < MAX_COST ) )
        {
          // Update value
          map.cost( id_row, id_col, best_cost );
          map.plane( id_row, id_col, openMVG::Vec4( best_n[ 0 ], best_n[ 1 ], best_n[ 2 ], best_d_plane ) );
          map.depth( id_row, id_col, best_depth );
        }

        const double half_delta_depth = ( depth_max - depth_min ) / 2.0;
        depth_min += half_delta_depth;
        depth_max -= half_delta_depth;
        delta_N /= 4.0;
      }
#endif
    }
  }

  CostMetricFactoryClean( params );
}

} // namespace MVS