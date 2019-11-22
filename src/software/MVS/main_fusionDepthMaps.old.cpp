#include "DepthMap.hpp"
#include "DepthMapFusionParameters.hpp"
#include "PointCloud.hpp"
#include "PutativePoint.hpp"
#include "Util.hpp"

#include "openMVG/image/pixel_types_io_cereal.hpp"

#include "third_party/cmdLine/cmdLine.h"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <cereal/archives/portable_binary.hpp>

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

void InitBBox( openMVG::Vec3& min, openMVG::Vec3& max )
{
  min = openMVG::Vec3( std::numeric_limits<double>::max(),
                       std::numeric_limits<double>::max(),
                       std::numeric_limits<double>::max() );
  max = openMVG::Vec3( -std::numeric_limits<double>::max(),
                       -std::numeric_limits<double>::max(),
                       -std::numeric_limits<double>::max() );
}

void UpdateBBox( openMVG::Vec3& min, openMVG::Vec3& max, const openMVG::Vec3& pt )
{
  min[ 0 ] = std::min( pt[ 0 ], min[ 0 ] );
  min[ 1 ] = std::min( pt[ 1 ], min[ 1 ] );
  min[ 2 ] = std::min( pt[ 2 ], min[ 2 ] );

  max[ 0 ] = std::max( pt[ 0 ], max[ 0 ] );
  max[ 1 ] = std::max( pt[ 1 ], max[ 1 ] );
  max[ 2 ] = std::max( pt[ 2 ], max[ 2 ] );
}

double GetSigma( const openMVG::Vec3& min, const openMVG::Vec3& max, const double perc = 0.01 )
{
  const double dx = max[ 0 ] - min[ 0 ];
  const double dy = max[ 1 ] - min[ 1 ];
  const double dz = max[ 2 ] - min[ 2 ];

  const double max_axis = std::max( dx, std::max( dy, dz ) );

  return perc * max_axis;
}

void ComputeWolffSigma( const std::vector<MVS::Camera>&           all_cams,
                        const std::vector<std::string>&           all_dm_path,
                        MVS::DepthMapFusionComputationParameters& params )
{
  openMVG::Vec3 b_min;
  openMVG::Vec3 b_max;

  InitBBox( b_min, b_max );

  for ( size_t id_cam = 0; id_cam < all_cams.size(); ++id_cam )
  {
    const MVS::DepthMap dm( all_dm_path[ id_cam ] );
    const MVS::Camera&  cam = all_cams[ id_cam ];

    for ( int y = 0; y < dm.height(); ++y )
    {
      for ( int x = 0; x < dm.width(); ++x )
      {

        if ( dm.depth( y, x ) >= 0.0 )
        {
          const openMVG::Vec3 pt = cam.unProject( x, y, dm.depth( y, x ), params.scale() );
          UpdateBBox( b_min, b_max, pt );
        }
      }
    }
  }

  params.setSigma( GetSigma( b_min, b_max ) );
}

void FilterDepthMapWolff( const MVS::Camera&                              reference_cam,
                          MVS::DepthMap&                                  map,
                          const size_t                                    id_reference_cam,
                          const std::vector<std::string>&                 in_dm_paths,
                          const std::vector<MVS::Camera>&                 all_cams,
                          const MVS::DepthMapFusionComputationParameters& params )
{
  // For each points
  // - Project to each depth map to compute signed distance to surface
  //
  // For each point
  // - Filter based on their

  // Projected points for every pixels
  openMVG::image::Image<openMVG::Vec3> projected_pts( map.width(), map.height() );

// Back project each points
#pragma omp parallel for schedule( dynamic )
  for ( int y = 0; y < map.height(); ++y )
  {
    for ( int x = 0; x < map.width(); ++x )
    {
      if ( map.depth( y, x ) >= 0.0 )
      {
        projected_pts( y, x ) = reference_cam.unProject( x, y, map.depth( y, x ), params.scale() );
      }
    }
  }

  // Compute mean signed distance for every points
  openMVG::image::Image<double> sum_distance( map.width(), map.height() );
  openMVG::image::Image<double> sum_weight( map.width(), map.height() );
  openMVG::image::Image<size_t> nb_valid( map.width(), map.height() );

  sum_distance.fill( 0.0 );
  sum_weight.fill( 0.0 );
  nb_valid.fill( 0 );

  const double sigma = params.getSigma(); // should be 1% of the max axis according to the paper

  for ( size_t id_cam = 0; id_cam < all_cams.size(); ++id_cam )
  {
    const MVS::DepthMap other_dm( in_dm_paths[ id_cam ] );
    const MVS::Camera&  other_cam = all_cams[ id_cam ];

    if ( id_cam == id_reference_cam )
    {
#pragma omp parallel for schedule( dynamic )
      for ( int y = 0; y < map.height(); ++y )
      {
        for ( int x = 0; x < map.width(); ++x )
        {
          // P a seen from cam
          const openMVG::Vec3& p = projected_pts( y, x );

          // Normal at P
          const openMVG::Vec4 pl = map.plane( y, x );
          const openMVG::Vec3 loc_n( pl[ 0 ], pl[ 1 ], pl[ 2 ] );
          const openMVG::Vec3 n = reference_cam.unProject( loc_n );

          // sum_distance += 0
          sum_weight( y, x ) += std::fabs( n.dot( ( p - reference_cam.m_C ).normalized() ) );
          nb_valid( y, x ) += 1.0;
        }
      }

      continue;
    }

#pragma omp parallel for schedule( dynamic )
    for ( int y = 0; y < map.height(); ++y )
    {
      for ( int x = 0; x < map.width(); ++x )
      {
        // Already filtered
        if ( map.depth( y, x ) < 0.0 )
        {
          continue;
        }
        // Back project in this camera
        const openMVG::Vec2 other_cam_pos = openMVG::Project( other_cam.m_P, projected_pts( y, x ) );
        if ( other_cam_pos[ 0 ] < 0.0 ||
             other_cam_pos[ 1 ] < 0.0 ||
             other_cam_pos[ 0 ] >= other_cam.m_cam_dims.first - 1 ||
             other_cam_pos[ 1 ] >= other_cam.m_cam_dims.second - 1 )
        {
          continue;
        }

        // P a seen from cam
        const openMVG::Vec3& p = projected_pts( y, x );
        // It's current depth
        const double z = other_cam.depth( p, params.scale() );

        // Normal at P
        const openMVG::Vec4 pl = map.plane( y, x );
        const openMVG::Vec3 loc_n( pl[ 0 ], pl[ 1 ], pl[ 2 ] );
        const openMVG::Vec3 n = reference_cam.unProject( loc_n );

        // Weight for this pixel
        const double wi = std::fabs( n.dot( ( p - other_cam.m_C ).normalized() ) );

        // Compute inverse barycentric coordinates
        // - Computes the base triangle
        const double dx = other_cam_pos[ 0 ] - std::floor( other_cam_pos[ 0 ] );
        const double dy = other_cam_pos[ 1 ] - std::floor( other_cam_pos[ 1 ] );

        /*
        * Given a split :

        A --- B
        |    /|
        |   / |
        |  /  |
        | /   |
        D --- C

        Where :
        A : (0,1)
        B : (1,1)
        C : (1,0)
        D : (0,0)

        Find in which triangle (A,B,D) (B,C,D) a point P (in square) is.

        Just compute the cross product : cr = ( B - D ) x ( P - D ) and test the Z value of cr :
        if cr.z is negative, p is inside (B,C,D), if positive, it's in (A,B,D)

        B - D = ( 1 , 1 , 0 )
        P - D = ( x , y , 0 )

        ( C - D ) ^ ( P - D ) = ( 0 , 0 , - x + y )

        so you only have to test the sign of - x + y
        */
        const double cross = -dx + dy;
        double       z_interp;
        if ( cross > 0.0 )
        {
          // In ABD
          const openMVG::Vec3 a( 0.0, 1.0, 0.0 );
          const openMVG::Vec3 b( 1.0, 1.0, 0.0 );
          const openMVG::Vec3 d( 0.0, 0.0, 0.0 );

          const openMVG::Vec3 bary = openMVG::Vec3( 1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0 ); // MVS::BarycentricCoordinates( a, b, d, openMVG::Vec3( dx, dy, 0.0 ) );

          // Compute interpolation of depth using three consecutive vertex
          const double da = other_dm.depth( other_cam_pos[ 1 ] + 1, other_cam_pos[ 0 ] );
          const double db = other_dm.depth( other_cam_pos[ 1 ] + 1, other_cam_pos[ 0 ] + 1 );
          const double dd = other_dm.depth( other_cam_pos[ 1 ], other_cam_pos[ 0 ] );

          z_interp = da * bary[ 0 ] + db * bary[ 1 ] + dd * bary[ 2 ];
        }
        else
        {
          // In BCD
          const openMVG::Vec3 b( 1.0, 1.0, 0.0 );
          const openMVG::Vec3 c( 1.0, 0.0, 0.0 );
          const openMVG::Vec3 d( 0.0, 0.0, 0.0 );

          const openMVG::Vec3 bary = openMVG::Vec3( 1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0 ); // MVS::BarycentricCoordinates( b, c, d, openMVG::Vec3( dx, dy, 0.0 ) );

          const double db = other_dm.depth( other_cam_pos[ 1 ] + 1, other_cam_pos[ 0 ] + 1 );
          const double dc = other_dm.depth( other_cam_pos[ 1 ], other_cam_pos[ 0 ] + 1 );
          const double dd = other_dm.depth( other_cam_pos[ 1 ], other_cam_pos[ 0 ] );

          z_interp = db * bary[ 0 ] + dc * bary[ 1 ] + dd * bary[ 2 ];
        }

        // Threshold on depth difference
        const double di    = z_interp - z;
        const double activ = ( ( -sigma ) < di ) ? 1.0 : 0.0; // ternary is unnecessary but for educative purpose

        // Accumalate weight
        sum_weight( y, x ) += wi * activ;

        // Accumulate distance
        sum_distance( y, x ) += activ * wi * std::min( sigma, di );

        // Accumulate nb valid
        nb_valid( y, x ) += ( ( -sigma ) < di ) && ( di < sigma ) ? 1.0 : 0.0;
      }
    }
  }

  // Point filtering
  const double td = 0.1 * sigma;
  const double tp = 0.2; // photometric unused now
  const double tv = 7.5 * static_cast<double>( all_cams.size() ) / 100.0;

#pragma omp parallel for schedule( dynamic )
  for ( int y = 0; y < map.height(); ++y )
  {
    for ( int x = 0; x < map.width(); ++x )
    {
      if ( map.depth( y, x ) < 0.0 )
      {
        continue;
      }

      const double d = sum_distance( y, x ) / sum_weight( y, x );

      if ( !( -td < d && d < 0.0 && nb_valid( y, x ) > tv ) )
      {
        map.depth( y, x, -1.0 );
      }
    }
  }
}

void FilterDepthMap( const MVS::Camera&                              reference_cam,
                     MVS::DepthMap&                                  map,
                     const size_t                                    id_reference_cam,
                     const std::vector<std::string>&                 in_dm_paths,
                     const std::vector<MVS::Camera>&                 cams,
                     const MVS::DepthMapFusionComputationParameters& params )
{
  // Number of view per pixel
  openMVG::image::Image<int> nb_view( map.width(), map.height(), true, 0 );
  // inverse projected points
  openMVG::image::Image<openMVG::Vec3> projected_pts( map.width(), map.height() );

// 1- project points in 3d
#pragma omp parallel for schedule( dynamic )
  for ( int y = 0; y < map.height(); ++y )
  {
    for ( int x = 0; x < map.width(); ++x )
    {
      projected_pts( y, x ) = reference_cam.unProject( x, y, map.depth( y, x ), params.scale() );
      nb_view( y, x )       = 0;
    }
  }

  // 2 - Inverse project points in the corresponding cameras
  for ( size_t id_cam = 0; id_cam < in_dm_paths.size(); ++id_cam )
  {
    if ( id_cam == id_reference_cam )
    {
      continue;
    }

    MVS::DepthMap      other_dm( in_dm_paths[ id_cam ] );
    const MVS::Camera& other_cam = cams[ id_cam ];

    const double baseline = ( other_cam.m_C - reference_cam.m_C ).norm();

// Project all points in the given image
#pragma omp parallel for schedule( dynamic )
    for ( int y = 0; y < map.height(); ++y )
    {
      for ( int x = 0; x < map.width(); ++x )
      {
        // Point is already valid, skip useless projection
        if ( nb_view( y, x ) >= params.nbMinimumView() )
        {
          continue;
        }

        const openMVG::Vec2 other_cam_pos = openMVG::Project( other_cam.m_P, projected_pts( y, x ) );
        if ( other_cam_pos[ 0 ] < 0 ||
             other_cam_pos[ 1 ] < 0 ||
             other_cam_pos[ 0 ] >= other_dm.width() ||
             other_cam_pos[ 1 ] >= other_dm.height() ||
             std::isinf( other_cam_pos[ 0 ] ) || std::isinf( other_cam_pos[ 1 ] ) ||
             std::isnan( other_cam_pos[ 0 ] ) || std::isnan( other_cam_pos[ 1 ] ) )
        {
          continue;
        }

        const openMVG::Vec4& cur_plane   = map.plane( y, x );
        const openMVG::Vec4& other_plane = other_dm.plane( other_cam_pos[ 1 ], other_cam_pos[ 0 ] );
        const openMVG::Vec3  cur_normal_local( cur_plane[ 0 ], cur_plane[ 1 ], cur_plane[ 2 ] );
        const openMVG::Vec3  other_normal_local( other_plane[ 0 ], other_plane[ 1 ], other_plane[ 2 ] );
        const openMVG::Vec3  cur_normal   = reference_cam.unProject( cur_normal_local );
        const openMVG::Vec3  other_normal = other_cam.unProject( other_normal_local );

        const double projected_depth = other_cam.depth( projected_pts( y, x ), params.scale() );
        const double other_depth     = other_dm.depth( other_cam_pos[ 1 ], other_cam_pos[ 0 ] );

        const double projected_disparity = other_cam.depthDisparityConversion( projected_depth, baseline );
        const double other_disparity     = other_cam.depthDisparityConversion( other_depth, baseline );

        const double delta_disparity = projected_depth - other_depth; //  projected_disparity - other_disparity ;

        const double angle_between = MVS::AngleBetween( cur_normal, other_normal );

        if ( fabs( delta_disparity ) < params.depthThreshold() &&
             angle_between < params.angleThreshold() && cur_normal.dot( other_normal ) > 0.0 )
        {
          ++nb_view( y, x );
        }
      }
    }
  }

  // 2 - filter depending on the number of valid camera
  for ( int y = 0; y < map.height(); ++y )
  {
    for ( int x = 0; x < map.width(); ++x )
    {
      if ( nb_view( y, x ) < params.nbMinimumView() )
      {
        map.depth( y, x, -1.0 );
      }
    }
  }
}

// Filter a depth map by removing it's points
void FilterDepthMaps( const std::vector<std::string>&           in_dm_paths,
                      const std::vector<std::string>&           in_cams_paths,
                      const std::vector<std::string>&           out_dm_paths,
                      MVS::DepthMapFusionComputationParameters& params )
{
  const bool use_wolfe = params.useWolff();

  std::vector<MVS::Camera> all_cams;
  for ( size_t id_cam = 0; id_cam < in_cams_paths.size(); ++id_cam )
  {
    all_cams.emplace_back( MVS::Camera( in_cams_paths[ id_cam ] ) );
  }

  if ( use_wolfe )
  {
    ComputeWolffSigma( all_cams, in_dm_paths, params );
  }

  for ( size_t id_dm = 0; id_dm < in_dm_paths.size(); ++id_dm )
  {
    std::cout << "Filtering depth map : " << id_dm << std::endl;
    MVS::DepthMap in_dm( in_dm_paths[ id_dm ] );
    MVS::Camera&  cur_cam = all_cams[ id_dm ];

    if ( use_wolfe )
    {
      FilterDepthMapWolff( cur_cam, in_dm, id_dm, in_dm_paths, all_cams, params );
    }
    else
    {
      FilterDepthMap( cur_cam, in_dm, id_dm, in_dm_paths, all_cams, params );
    }
    in_dm.save( out_dm_paths[ id_dm ] );
  }
}

openMVG::image::Image<openMVG::image::RGBColor> ReadColorFile( const std::string& path )
{
  openMVG::image::Image<openMVG::image::RGBColor> res;

  std::ifstream in_color( path, std::ios::binary );
  if ( !in_color )
  {
    std::cerr << "Impossible to read the file " << std::endl;
    return res;
  }

  cereal::PortableBinaryInputArchive ar_color( in_color );

  try
  {
    ar_color( res );
  }
  catch ( ... )
  {
    std::cerr << "Serialization impossible" << std::endl;
  }
  return res;
}

MVS::PointCloud CreatePCLFromView( const MVS::Camera&                              cur_cam,
                                   const MVS::DepthMap&                            cur_dm,
                                   const size_t                                    id_dm,
                                   const MVS::DepthMapFusionComputationParameters& params )
{
  const std::string                               color_path = params.getColorPath( id_dm );
  openMVG::image::Image<openMVG::image::RGBColor> cur_img    = ReadColorFile( color_path );

  MVS::PointCloud res;
  for ( int y = 0; y < cur_dm.height(); ++y )
  {
    for ( int x = 0; x < cur_dm.width(); ++x )
    {
      const double cur_depth = cur_dm.depth( y, x );

      if ( cur_depth > 0.0 )
      {
        const openMVG::Vec4&           cur_plane = cur_dm.plane( y, x );
        const openMVG::Vec3            pt        = cur_cam.unProject( x, y, cur_depth, params.scale() );
        const openMVG::Vec3            n( cur_plane[ 0 ], cur_plane[ 1 ], cur_plane[ 2 ] );
        const openMVG::image::RGBColor cur_col = cur_img( y, x );
        const openMVG::Vec3            cur_color( cur_col.r() / 255.0, cur_col.g() / 255.0, cur_col.b() / 255.0 );

        res.addPoint( pt, n, cur_color );
      }
    }
  }

  return res;
}

MVS::PointCloud CreatePCLFromView( const MVS::Camera&                              cur_cam,
                                   const MVS::DepthMap&                            cur_dm,
                                   const size_t                                    id_dm,
                                   const std::vector<std::string>&                 dm_paths,
                                   const std::vector<MVS::Camera>&                 all_cams,
                                   const MVS::DepthMapFusionComputationParameters& params )
{

  openMVG::image::Image<MVS::PutativePoint> points( cur_dm.width(), cur_dm.height() );
  openMVG::image::Image<openMVG::Vec3>      base_points( cur_dm.width(), cur_dm.height() );

  // set initial points
  {
    const std::string                               color_path = params.getColorPath( id_dm );
    openMVG::image::Image<openMVG::image::RGBColor> cur_img    = ReadColorFile( color_path );

#pragma omp parallel for schedule( dynamic )
    for ( int y = 0; y < cur_dm.height(); ++y )
    {
      for ( int x = 0; x < cur_dm.width(); ++x )
      {
        const double cur_depth = cur_dm.depth( y, x );

        if ( cur_depth > 0.0 )
        {
          const openMVG::Vec4& cur_plane = cur_dm.plane( y, x );
          const openMVG::Vec3  pt        = cur_cam.unProject( x, y, cur_depth, params.scale() );
          // TODO: unproject since normal is in view coordinate
          const openMVG::Vec3            n_loc( cur_plane[ 0 ], cur_plane[ 1 ], cur_plane[ 2 ] );
          const openMVG::Vec3            n       = cur_cam.unProject( n_loc );
          const openMVG::image::RGBColor cur_col = cur_img( y, x );
          const openMVG::Vec3            cur_color( cur_col.r() / 255.0, cur_col.g() / 255.0, cur_col.b() / 255.0 );

          points( y, x ).setInitialPoint( pt, n, cur_color );
          base_points( y, x ) = pt;
        }
      }
    }
  }

  // Pass all cameras and get corresponding points
  for ( size_t id_cam = 0; id_cam < all_cams.size(); ++id_cam )
  {
    if ( id_cam == id_dm )
    {
      continue;
    }

    MVS::DepthMap                                   other_dm( dm_paths[ id_cam ] );
    const MVS::Camera&                              other_cam = all_cams[ id_cam ];
    openMVG::image::Image<openMVG::image::RGBColor> other_img = ReadColorFile( params.getColorPath( id_cam ) );

    const double baseline = ( other_cam.m_C - cur_cam.m_C ).norm();

    openMVG::image::Image<bool> used( other_dm.width(), other_dm.height(), true, false );

#pragma omp parallel for schedule( dynamic )
    for ( int y = 0; y < cur_dm.height(); ++y )
    {
      for ( int x = 0; x < cur_dm.width(); ++x )
      {
        // Current depth is not valid or already used
        if ( cur_dm.depth( y, x ) < 0.0 )
        {
          continue;
        }
        const openMVG::Vec2 other_cam_pos = openMVG::Project( other_cam.m_P, base_points( y, x ) );
        if ( other_cam_pos[ 0 ] < 0 || other_cam_pos[ 1 ] < 0 ||
             other_cam_pos[ 0 ] >= other_dm.width() ||
             other_cam_pos[ 1 ] >= other_dm.height() )
        {
          continue;
        }
        if ( used( other_cam_pos[ 1 ], other_cam_pos[ 0 ] ) )
        {
          continue;
        }

        const openMVG::Vec4& cur_plane   = cur_dm.plane( y, x );
        const openMVG::Vec4& other_plane = other_dm.plane( other_cam_pos[ 1 ], other_cam_pos[ 0 ] );
        const openMVG::Vec3  cur_normal_local( cur_plane[ 0 ], cur_plane[ 1 ], cur_plane[ 2 ] );
        const openMVG::Vec3  other_normal_local( other_plane[ 0 ], other_plane[ 1 ], other_plane[ 2 ] );
        const openMVG::Vec3  cur_normal   = cur_cam.unProject( cur_normal_local );
        const openMVG::Vec3  other_normal = other_cam.unProject( other_normal_local );

        // Projection
        const double projected_depth = other_cam.depth( base_points( y, x ), params.scale() );
        // Existing value
        const double other_depth = other_dm.depth( other_cam_pos[ 1 ], other_cam_pos[ 0 ] );

        if ( other_depth < 0.0 )
        {
          continue;
        }
        const double projected_disparity = other_cam.depthDisparityConversion( projected_depth, baseline, params.scale() );
        const double other_disparity     = other_cam.depthDisparityConversion( other_depth, baseline, params.scale() );

        const double delta_disparity = projected_depth - other_depth; // projected_disparity - other_disparity ;

        const double angle_between = MVS::AngleBetween( cur_normal, other_normal );

        if ( fabs( delta_disparity ) < params.depthThreshold() &&
             angle_between < params.angleThreshold() )
        {
          const openMVG::image::RGBColor& cur_col = other_img( other_cam_pos[ 1 ], other_cam_pos[ 0 ] );
          const openMVG::Vec3             other_color( cur_col.r() / 255.0, cur_col.g() / 255.0, cur_col.b() / 255.0 );
          used( other_cam_pos[ 1 ], other_cam_pos[ 0 ] ) = true;
          const openMVG::Vec3 other_point                = other_cam.unProject( other_cam_pos[ 0 ], other_cam_pos[ 1 ], other_depth, params.scale() );
          points( y, x ).addCandidate( other_point, other_normal, other_color );
        }
      }
    }

    // Todo save the depth map in order to remove the points already used
  }

  MVS::PointCloud res;
  for ( int y = 0; y < cur_dm.height(); ++y )
  {
    for ( int x = 0; x < cur_dm.width(); ++x )
    {
      if ( points( y, x ).nbCandidate() >= params.nbMinimumView() )
      {
        const std::tuple<openMVG::Vec3, openMVG::Vec3, openMVG::Vec3> cur_pt = points( y, x ).getPoint();
        res.addPoint( std::get<0>( cur_pt ), std::get<1>( cur_pt ), std::get<2>( cur_pt ) );
      }
    }
  }

  return res;
}

MVS::PointCloud FusionDepthMap( const std::vector<std::string>&                 dm_paths,
                                const std::vector<std::string>&                 cam_paths,
                                const MVS::DepthMapFusionComputationParameters& params )
{
  const bool use_wolff = params.useWolff();

  std::vector<MVS::Camera> all_cams;
  for ( size_t id_cam = 0; id_cam < cam_paths.size(); ++id_cam )
  {
    all_cams.emplace_back( MVS::Camera( cam_paths[ id_cam ] ) );
  }

  MVS::PointCloud pcloud;

  for ( size_t id_dm = 0; id_dm < cam_paths.size(); ++id_dm )
  {
    std::cout << "Fusion map : " << id_dm << std::endl;
    MVS::Camera&  cur_camera = all_cams[ id_dm ];
    MVS::DepthMap cur_dm( dm_paths[ id_dm ] );

    if ( use_wolff )
    {
      pcloud.append( CreatePCLFromView( cur_camera, cur_dm, id_dm, params ) );
    }
    else
    {
      pcloud.append( CreatePCLFromView( cur_camera, cur_dm, id_dm, dm_paths, all_cams, params ) );
    }
  }

  return pcloud;
}

std::vector<std::string> GetInputDepthMapsPaths( const std::string&                              base_path,
                                                 const int                                       scale,
                                                 const MVS::DepthMapFusionComputationParameters& params )
{
  std::vector<std::string> res;
  const std::string        depth  = stlplus::create_filespec( base_path, "depth" );
  int                      id_cam = 0;
  while ( 1 )
  {
    const std::string cur_cam_folder = params.getCameraDirectory( id_cam );
    if ( !stlplus::folder_exists( cur_cam_folder ) )
    {
      break;
    }

    const std::string dm_path = params.getDepthPath( id_cam );
    if ( !stlplus::file_exists( dm_path ) )
    {
      break;
    }

    res.emplace_back( dm_path );

    ++id_cam;
  }

  return res;
}

std::vector<std::string> GetInputCameraPaths( const std::string&                              base_path,
                                              const int                                       scale,
                                              const MVS::DepthMapFusionComputationParameters& params )
{
  std::vector<std::string> res;
  int                      id_cam = 0;
  while ( 1 )
  {
    const std::string cur_cam_folder = params.getCameraDirectory( id_cam );
    if ( !stlplus::folder_exists( cur_cam_folder ) )
    {
      break;
    }

    const std::string cam_path = params.getCameraPath( id_cam );
    if ( !stlplus::file_exists( cam_path ) )
    {
      break;
    }

    res.emplace_back( cam_path );

    ++id_cam;
  }

  return res;
}

std::vector<std::string> GetOutputDepthMapsPaths( const std::string& base_path, const int scale, const MVS::DepthMapFusionComputationParameters& params )
{
  std::vector<std::string> res;
  const std::string        depth  = stlplus::create_filespec( base_path, "depth" );
  int                      id_cam = 0;
  while ( 1 )
  {
    const std::string cur_cam_folder = params.getCameraDirectory( id_cam );
    if ( !stlplus::folder_exists( cur_cam_folder ) )
    {
      break;
    }

    const std::string dm_path = params.getFilteredDepthPath( id_cam );

    res.emplace_back( dm_path );

    ++id_cam;
  }

  return res;
}

int main( int argc, char** argv )
{
  CmdLine cmd;

  std::string sInOutDir = "";
  double      fe        = 0.1;
  double      fa        = 30.0;
  int         fcomp     = 3;
  int         scale     = 1;

  cmd.add( make_option( 'i', sInOutDir, "input_dir" ) );
  cmd.add( make_option( 'e', fe, "depth_threshold" ) );
  cmd.add( make_option( 'a', fa, "angle_threshold" ) );
  cmd.add( make_option( 'c', fcomp, "minimum_view" ) );
  cmd.add( make_option( 's', scale, "scale" ) );
  cmd.add( make_switch( 'w', "wolff" ) );

  cmd.process( argc, argv );

  std::cout << "You called fusion with parameters : " << std::endl;
  std::cout << "In/Out path : " << sInOutDir << std::endl;
  std::cout << "Max depth threshold (fe) : " << fe << std::endl;
  std::cout << "Max angle threshold (fa) : " << fa << std::endl;
  std::cout << "Minimum view (fcomp)     : " << fcomp << std::endl;
  std::cout << "scale                    : " << scale << std::endl;

  MVS::DepthMapFusionComputationParameters params( sInOutDir, scale, fe, fa, fcomp );

  if ( cmd.used( 'w' ) )
  {
    std::cout << "Set use Wolff" << std::endl;
    params.setUseWolff( true );
  }

  std::vector<std::string> in_depth_maps   = GetInputDepthMapsPaths( sInOutDir, scale, params );
  std::vector<std::string> in_camera_paths = GetInputCameraPaths( sInOutDir, scale, params );
  std::vector<std::string> out_depth_maps  = GetOutputDepthMapsPaths( sInOutDir, scale, params );

  // Pass 1 : filter the depth map and remove some points
  FilterDepthMaps( in_depth_maps, in_camera_paths, out_depth_maps, params );

  // Pass 2 : generate point cloud using the valid points
  const MVS::PointCloud pcl = FusionDepthMap( out_depth_maps, in_camera_paths, params );

  // [ Pass 3 : remove duplicate points ? (if not done in pass2) ]

  // Final pass : save the point cloud
  pcl.exportToPly( params.getModelPath(), true );

  return EXIT_SUCCESS;
}