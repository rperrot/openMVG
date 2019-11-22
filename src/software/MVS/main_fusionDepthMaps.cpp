#include "BoundingBox.hpp"
#include "Camera.hpp"
#include "DepthMap.hpp"
#include "DepthMapFusionParameters.hpp"
#include "Image.hpp"
#include "PointCloud.hpp"
#include "Util.hpp"

#include "normal_estimation/HoppeNormalEstimation.hpp"

#include "third_party/cmdLine/cmdLine.h"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include "openMVG/image/pixel_types.hpp"
#include "openMVG/image/pixel_types_io_cereal.hpp"

#include <cereal/archives/portable_binary.hpp>

#include <fstream>
#include <iostream>
#include <vector>

// Depth map fusion using Wolff method
// Katja Wolff, Changil Kim, Henning Zimmer, Christopher Schroers, Mario Botsch, Olga Sorkine-Hornung, Alexander Sorkine-Hornung
// "Point Cloud Noise and Outlier Removal for Image-Based 3D Reconstruction"
// Proceedings of International Conference on 3D Vision

void usage( const char* arg0 )
{
  std::cout << "Usage: " << arg0 << " [params]" << std::endl;
  std::cout << "Mandatory params" << std::endl;
  std::cout << "  -i | --input-dir                  Input MVS directory" << std::endl;
  std::cout << "Optional parameters" << std::endl;
  std::cout << "  -s | --scale                      Scale of the computation.                             [ default : -1 ]" << std::endl;
  std::cout << "  -n | --normal-neighbor-size       Neighboring size for the initial normal estimation.   [ default : 7  ]" << std::endl;
}

/**
 * @brief Get the Input Depth Maps Paths
 * 
 * @param base_path       Base directory of the MVS project 
 * @param params          Parameters of the MVS project 
 * @return Vector of paths of the depth map of the project
 */
static std::vector<std::string> GetInputDepthMapsPaths( const std::string&                              base_path,
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

static std::vector<std::string> GetInputGrayscaleImagesPaths( const std::string& base_path, const MVS::DepthMapFusionComputationParameters& params )
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
    const std::string gray_path = params.getGrayscalePath( id_cam );
    if ( !stlplus::file_exists( gray_path ) )
    {
      break;
    }

    res.emplace_back( gray_path );

    ++id_cam;
  }
  return res;
}

static std::vector<std::string> GetInputColorImagesPaths( const std::string& base_path, const MVS::DepthMapFusionComputationParameters& params )
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
    const std::string color_path = params.getColorPath( id_cam );
    if ( !stlplus::file_exists( color_path ) )
    {
      break;
    }

    res.emplace_back( color_path );

    ++id_cam;
  }
  return res;
}

static std::vector<std::string> GetRecomputedNormalDepthMapsPaths( const std::string&                              base_path,
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

    res.emplace_back( dm_path + ".normals" );

    ++id_cam;
  }

  return res;
}

/**
 * @brief Get the Input Camera Paths 
 * 
 * @param base_path         Base directory of the MVS project
 * @param params            Parameters of the MVS project 
 * @return Vector of paths of the depth map of the project 
 */
static inline std::vector<std::string>
GetInputCameraPaths( const std::string&                              base_path,
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

std::vector<std::string> GetOutputDepthMapsPaths( const std::string&                              base_path,
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

    const std::string dm_path = params.getFilteredDepthPath( id_cam );

    res.emplace_back( dm_path );

    ++id_cam;
  }

  return res;
}

/**
 * @brief (Re)Compute for each depth maps normals for each points 
 * 
 * @param depthMapPaths   Paths for all depth maps  
 * @param cameraPaths     Paths for all cameras 
 * @param params          Computation parameters 
 */
void ComputeInitialNormals( const std::vector<std::string>&                 depthMapPaths,
                            const std::vector<std::string>&                 cameraPaths,
                            const MVS::DepthMapFusionComputationParameters& params )
{
  for ( size_t id_dm = 0; id_dm < depthMapPaths.size(); ++id_dm )
  {
    std::cout << "Depth map [" << id_dm + 1 << "/" << depthMapPaths.size() << "]" << std::endl;
    MVS::Camera   cam = MVS::Camera( cameraPaths[ id_dm ] );
    MVS::DepthMap dm  = MVS::DepthMap( depthMapPaths[ id_dm ] );

    // Estimate normal
    MVS::HoppeNormalEstimation normalEstimator;
    normalEstimator.computeNormals( cam, dm, params.scale() );

    // Save depth map to a file ( with .normals suffix)
    const std::string out_dm_path = params.getDepthPath( id_dm ) + ".normals";
    dm.save( out_dm_path );
  }
}

/**
 * @brief 
 * 
 * @param all_cams 
 * @param all_dm_path 
 * @param params 
 * 
 * From the paper : 
 * "The value of s should be chosen according to the scale of the scene, so we set it to 1% of the depth range (e.g., the length of the bounding box along the z-axis)" 
 *
 */
void ComputeWolffSigma( const std::vector<std::string>&           depthMapPaths,
                        const std::vector<std::string>&           cameraPaths,
                        MVS::DepthMapFusionComputationParameters& params )
{
  openMVG::Vec3 b_min;
  openMVG::Vec3 b_max;

  MVS::InitBBox( b_min, b_max );

  double depth_min = std::numeric_limits<double>::max();
  double depth_max = -std::numeric_limits<double>::max();

  for ( size_t id_dm = 0; id_dm < depthMapPaths.size(); ++id_dm )
  {
    MVS::Camera   cam = MVS::Camera( cameraPaths[ id_dm ] );
    MVS::DepthMap dm  = MVS::DepthMap( depthMapPaths[ id_dm ] );

    for ( int y = 0; y < dm.height(); ++y )
    {
      for ( int x = 0; x < dm.width(); ++x )
      {
        const double d = dm.depth( y, x );

        if ( d > 0.0 )
        {
          depth_min = std::min( depth_min, d );
          depth_max = std::max( depth_max, d );

          const openMVG::Vec3 pt = cam.unProject( x, y, d, params.scale() );
          MVS::UpdateBBox( b_min, b_max, pt );
        }
      }
    }
  }

  const double ext[] = {b_max[ 0 ] - b_min[ 0 ],
                        b_max[ 1 ] - b_min[ 1 ],
                        b_max[ 2 ] - b_min[ 2 ]};

  const double sigma = 0.05 * ( depth_max - depth_min );
  //( std::max( ext[ 0 ], std::max( ext[ 1 ], ext[ 2 ] ) ) );
  params.setSigma( sigma );
}

static bool IsDegenerate( const openMVG::Vec3& a,
                          const openMVG::Vec3& b,
                          const openMVG::Vec3& c )
{
  const openMVG::Vec3 ABn = ( b - a ).normalized();
  const openMVG::Vec3 ACn = ( c - a ).normalized();
  const openMVG::Vec3 BCn = ( c - b ).normalized();

  const double angle_a = std::acos( std::fabs( ABn.dot( ACn ) ) );
  const double angle_b = std::acos( std::fabs( ABn.dot( BCn ) ) );
  const double angle_c = std::acos( std::fabs( ACn.dot( BCn ) ) );

  static const double th = openMVG::D2R( 1 );

  return angle_a < th || angle_b < th || angle_c < th;
}

void FilterDepthMapWolff( const MVS::Camera&                              reference_cam,
                          MVS::DepthMap&                                  map,
                          const size_t                                    id_reference_cam,
                          const std::vector<std::string>&                 in_dm_paths,
                          const std::vector<std::string>&                 in_color_path,
                          const std::vector<MVS::Camera>&                 all_cams,
                          const MVS::DepthMapFusionComputationParameters& params )
{
  // Projected points for every pixels
  openMVG::image::Image<openMVG::Vec3> projected_pts( map.width(), map.height() );
  openMVG::image::Image<double>        wi( map.width(), map.height() );

  // Back project each points
  double depth_min = std::numeric_limits<double>::max();
  double depth_max = -std::numeric_limits<double>::max();

#pragma omp parallel for schedule( dynamic )
  for ( int y = 0; y < map.height(); ++y )
  {
    for ( int x = 0; x < map.width(); ++x )
    {
      if ( map.depth( y, x ) > 0.0 )
      {
        const openMVG::Vec3 p = reference_cam.unProject( x, y, map.depth( y, x ), params.scale() );
        projected_pts( y, x ) = p;

        const double d = reference_cam.depth( p, params.scale() );

        depth_min = std::min( depth_min, d );
        depth_max = std::max( depth_max, d );

        const openMVG::Vec4 pl = map.plane( y, x );
        const openMVG::Vec3 n( pl[ 0 ], pl[ 1 ], pl[ 2 ] );

        wi( y, x ) = std::fabs( n.dot( ( p - reference_cam.m_C ).normalized() ) );
      }
    }
  }

  // Compute mean signed distance for every points
  openMVG::image::Image<double>        sum_distance( map.width(), map.height() );
  openMVG::image::Image<double>        sum_weight( map.width(), map.height() );
  openMVG::image::Image<openMVG::Vec3> s( map.width(), map.height() );  // Color
  openMVG::image::Image<double>        s2( map.width(), map.height() ); // Color^2
  openMVG::image::Image<size_t>        nb_valid( map.width(), map.height() );

  sum_distance.fill( 0.0 );
  sum_weight.fill( 0.0 );
  nb_valid.fill( 0 );
  s.fill( openMVG::Vec3::Zero() );
  s2.fill( 0.0 );

  const double sigma = params.getSigma(); // should be 1% of the max axis according to the paper
  // Point filtering
  const double td = 0.1 * sigma;
  const double tp = 0.2;
  const double tv = std::max( 2.0, 7.5 * static_cast<double>( all_cams.size() ) / 100.0 );

  for ( size_t id_cam = 0; id_cam < all_cams.size(); ++id_cam )
  {
    const MVS::DepthMap other_dm( in_dm_paths[ id_cam ] );
    const MVS::Camera&  other_cam = all_cams[ id_cam ];
    const MVS::Image    other_image( in_color_path[ id_cam ], "", "", "", MVS::ImageLoadType::IMAGE_COLOR );

    for ( int y = 0; y < map.height(); ++y )
    {
      for ( int x = 0; x < map.width(); ++x )
      {
        if ( map.depth( y, x ) < 0.0 )
        {
          continue;
        }

        if ( id_cam == id_reference_cam )
        {
          const double d = 0;
          const double w = wi( y, x );

          sum_distance( y, x ) = ( sum_weight( y, x ) * sum_distance( y, x ) + ( w * d ) / sigma ) / ( sum_weight( y, x ) + w );
          sum_weight( y, x ) += w;
          nb_valid( y, x ) += 1.0;

          const openMVG::Vec3 c = other_image.color( y, x );
          s( y, x ) += c;
          s2( y, x ) += c.dot( c );

          continue;
        }

        // P as seen from reference camera.
        const openMVG::Vec3& p = projected_pts( y, x );
        // Viewing direction in reference cam.
        const openMVG::Vec3 vi = ( p - reference_cam.m_C );

        // Skip points if viewing direction is opposite.
        const openMVG::Vec3 vj = ( p - other_cam.m_C );
        if ( vi.dot( vj ) < 0.0 )
          continue;

        // Project the point into id_cam to get it's coordinates.
        const openMVG::Vec2 other_cam_pos = openMVG::Project( other_cam.m_P_scaled[ params.scale() ], p );

        const double u = other_cam_pos[ 0 ];
        const double v = other_cam_pos[ 1 ];
        const double z = other_cam.depth( p, params.scale() );

        if ( floor( u ) < 0.0 ||
             floor( v ) < 0.0 ||
             std::ceil( u ) >= other_cam.m_cam_dims.first - 1 ||
             std::ceil( v ) >= other_cam.m_cam_dims.second - 1 ||
             std::isnan( u ) || std::isnan( v ) || std::isinf( u ) || std::isinf( v ) ||
             z < 0.0 )
        {
          continue;
        }

        // Check in which triangle is the point
        const double dx = u - std::floor( u );
        const double dy = v - std::floor( v );

        /*
          A: (0,0)

          A --- B
          |    /|
          |   / |
          |  /  |
          | /   |
          D --- C
        */

        // Get/Compute : normal, depth, weight for each corners of the mesh
        const openMVG::Vec2i A( floor( u ), floor( v ) );
        const openMVG::Vec2i B( ceil( u ), floor( v ) );
        const openMVG::Vec2i D( floor( u ), ceil( v ) );
        const openMVG::Vec2i C( ceil( u ), ceil( v ) );

        const double da = other_dm.depth( A );
        const double db = other_dm.depth( B );
        const double dc = other_dm.depth( C );
        const double dd = other_dm.depth( D );

        const openMVG::Vec3 pa = other_cam.unProject( A[ 0 ], A[ 1 ], da, params.scale() );
        const openMVG::Vec3 pb = other_cam.unProject( B[ 0 ], B[ 1 ], db, params.scale() );
        const openMVG::Vec3 pc = other_cam.unProject( C[ 0 ], C[ 1 ], dc, params.scale() );
        const openMVG::Vec3 pd = other_cam.unProject( D[ 0 ], D[ 1 ], dd, params.scale() );

        const openMVG::Vec4 plA = other_dm.plane( A );
        const openMVG::Vec4 plB = other_dm.plane( B );
        const openMVG::Vec4 plC = other_dm.plane( C );
        const openMVG::Vec4 plD = other_dm.plane( D );

        const openMVG::Vec3 na( plA[ 0 ], plA[ 1 ], plA[ 2 ] );
        const openMVG::Vec3 nb( plB[ 0 ], plB[ 1 ], plB[ 2 ] );
        const openMVG::Vec3 nc( plC[ 0 ], plC[ 1 ], plC[ 2 ] );
        const openMVG::Vec3 nd( plD[ 0 ], plD[ 1 ], plD[ 2 ] );

        const double wa = std::fabs( na.dot( ( pa - other_cam.m_C ).normalized() ) );
        const double wb = std::fabs( nb.dot( ( pb - other_cam.m_C ).normalized() ) );
        const double wc = std::fabs( nc.dot( ( pc - other_cam.m_C ).normalized() ) );
        const double wd = std::fabs( nd.dot( ( pd - other_cam.m_C ).normalized() ) );

        const openMVG::Vec3 ca = other_image.color( A );
        const openMVG::Vec3 cb = other_image.color( B );
        const openMVG::Vec3 cc = other_image.color( C );
        const openMVG::Vec3 cd = other_image.color( D );

        // Interpolated depth and weight
        double        z_interp;
        double        w;
        openMVG::Vec3 c;
        if ( dx + dy > 1.0 )
        {
          // Lower-right corner BCD
          // If no triangle skip it.
          if ( db < 0.0 || dc < 0.0 || dd < 0.0 )
            continue;

          // Test if triangle is degenerate
          if ( IsDegenerate( pb, pc, pd ) )
            continue;

          const openMVG::Vec3 Gb( 1.0, 0.0, 0.0 );
          const openMVG::Vec3 Gc( 1.0, 1.0, 0.0 );
          const openMVG::Vec3 Gd( 0.0, 1.0, 0.0 );

          const openMVG::Vec3 bary = MVS::BarycentricCoordinates( Gb, Gc, Gd, openMVG::Vec3( dx, dy, 0.0 ) );

          z_interp = db * bary[ 0 ] + dc * bary[ 1 ] + dd * bary[ 2 ];
          w        = wb * bary[ 0 ] + wc * bary[ 1 ] + wd * bary[ 2 ];
          c        = cb * bary[ 0 ] + cc * bary[ 1 ] + cd * bary[ 2 ];
        }
        else
        {
          // Upper left corner ABD
          // If no triangle skip it.
          if ( db < 0.0 || da < 0.0 || dd < 0.0 )
            continue;
          if ( IsDegenerate( pa, pb, pd ) )
            continue;

          const openMVG::Vec3 Ga( 0.0, 0.0, 0.0 );
          const openMVG::Vec3 Gb( 1.0, 0.0, 0.0 );
          const openMVG::Vec3 Gd( 0.0, 1.0, 0.0 );

          const openMVG::Vec3 bary = MVS::BarycentricCoordinates( Ga, Gb, Gd, openMVG::Vec3( dx, dy, 0.0 ) );

          z_interp = da * bary[ 0 ] + db * bary[ 1 ] + dd * bary[ 2 ];
          w        = wa * bary[ 0 ] + wb * bary[ 1 ] + wd * bary[ 2 ];
          c        = ca * bary[ 0 ] + cb * bary[ 1 ] + cd * bary[ 2 ];
        }

        double d = z_interp - z;

        // Distance to the range surface is too far (observed point is behind the surface and it's distance is above abs( sigma ) )
        if ( d < -sigma )
          continue;
        if ( d > sigma )
          d = sigma;

        sum_distance( y, x ) = ( sum_weight( y, x ) * sum_distance( y, x ) + ( w * d ) / sigma ) / ( sum_weight( y, x ) + w );
        sum_weight( y, x ) += w;

        if ( d != sigma )
        {
          s( y, x ) += c;
          s2( y, x ) += c.dot( c );

          nb_valid( y, x ) += 1;
        }
      }
    }
  }

#pragma omp parallel for schedule( dynamic )
  for ( int y = 0; y < map.height(); ++y )
  {
    for ( int x = 0; x < map.width(); ++x )
    {
      if ( map.depth( y, x ) < 0.0 )
      {
        continue;
      }

      const double        _s2 = s2( y, x );
      const openMVG::Vec3 _s  = s( y, x );

      const double d = sum_distance( y, x );
      const double p = std::sqrt( ( _s2 - ( _s.dot( _s ) / nb_valid( y, x ) ) ) / nb_valid( y, x ) ) * 2.0 / ( 255.0 * std::sqrt( 3.0 ) );

      if ( !( ( -td < d && d < 0 ) && ( nb_valid( y, x ) > tv ) && ( p < tp ) ) )
      {
        map.depth( y, x, -1.0 );
      }
    }
  }

  return;

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
          if ( map.depth( y, x ) < 0.0 )
          {
            continue;
          }

          // Normal at P
          const openMVG::Vec4 pl = map.plane( y, x );
          const openMVG::Vec3 n( pl[ 0 ], pl[ 1 ], pl[ 2 ] );
          // const openMVG::Vec3 n = reference_cam.unProject( loc_n, params.scale() );

          // sum_distance += 0
          sum_weight( y, x ) += wi( y, x ); // std::fabs( n.dot( ( p - reference_cam.m_C ).normalized() ) );
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
        const openMVG::Vec3 n( pl[ 0 ], pl[ 1 ], pl[ 2 ] );
        // const openMVG::Vec3 n = reference_cam.unProject( loc_n, params.scale() );

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

          const openMVG::Vec3 bary = // openMVG::Vec3( 1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0 ); //
              MVS::BarycentricCoordinates( a, b, d, openMVG::Vec3( dx, dy, 0.0 ) );

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

          const openMVG::Vec3 bary = // openMVG::Vec3( 1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0 ); //
              MVS::BarycentricCoordinates( b, c, d, openMVG::Vec3( dx, dy, 0.0 ) );

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
}

/**
 * @brief Main function for depth map filtering 
 * 
 * @param in_dm_paths       Path for all depth maps 
 * @param in_cams_paths     Path for all cameras 
 * @param out_dm_paths      Path for output of the depth maps after filtering
 * @param params            Computation parameters 
 */
void FilterDepthMaps( const std::vector<std::string>&           in_dm_paths,
                      const std::vector<std::string>&           in_color_paths,
                      const std::vector<std::string>&           in_cams_paths,
                      const std::vector<std::string>&           out_dm_paths,
                      MVS::DepthMapFusionComputationParameters& params )
{
  std::vector<MVS::Camera> all_cams;
  for ( size_t id_cam = 0; id_cam < in_cams_paths.size(); ++id_cam )
  {
    all_cams.emplace_back( MVS::Camera( in_cams_paths[ id_cam ] ) );
  }

  for ( size_t id_dm = 0; id_dm < in_dm_paths.size(); ++id_dm )
  {
    std::cout << "Filtering depth map: [" << id_dm + 1 << "/" << in_dm_paths.size() << "]" << std::endl;
    MVS::DepthMap in_dm( in_dm_paths[ id_dm ] + ".normals" );
    MVS::Camera&  cur_cam = all_cams[ id_dm ];

    FilterDepthMapWolff( cur_cam, in_dm, id_dm, in_dm_paths, in_color_paths, all_cams, params );
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

MVS::PointCloud FusionDepthMap( const std::vector<std::string>&                 dm_paths,
                                const std::vector<std::string>&                 cam_paths,
                                const MVS::DepthMapFusionComputationParameters& params )
{
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

    pcloud.append( CreatePCLFromView( cur_camera, cur_dm, id_dm, params ) );
  }

  return pcloud;
}

int main( int argc, char** argv )
{
  CmdLine cmd;

  std::string sInOutDir             = "";
  int         iScale                = -1;
  int         iNormalEstimationSize = 7;

  cmd.add( make_option( 'i', sInOutDir, "input_dir" ) );
  cmd.add( make_option( 's', iScale, "scale" ) );
  cmd.add( make_option( 'n', iNormalEstimationSize, "normal-neighbor-size" ) );

  try
  {
    cmd.process( argc, argv );
  }
  catch ( ... )
  {
    std::cerr << "Command line parsing error." << std::endl;
    usage( argv[ 0 ] );
    exit( EXIT_FAILURE );
  }

  MVS::DepthMapFusionComputationParameters params( sInOutDir, iScale );
  const std::vector<std::string>           in_depth_maps_paths                    = GetInputDepthMapsPaths( sInOutDir, params );
  const std::vector<std::string>           in_color_paths                         = GetInputColorImagesPaths( sInOutDir, params );
  const std::vector<std::string>           in_depth_maps_normals_recomputed_paths = GetRecomputedNormalDepthMapsPaths( sInOutDir, params );
  const std::vector<std::string>           in_camera_paths                        = GetInputCameraPaths( sInOutDir, params );
  const std::vector<std::string>           out_dm_paths                           = GetOutputDepthMapsPaths( sInOutDir, params );

  const bool debug_intermediate = false;

  // 1 Compute initial normals for all depth maps
  if ( debug_intermediate )
  {
    std::cout << "[DEBUG] Creating raw point cloud" << std::endl;
    const MVS::PointCloud pcl = FusionDepthMap( in_depth_maps_paths, in_camera_paths, params );
    pcl.exportToPly( "raw.ply", true );
  }

  std::cout << "1. Computing normals for all depth maps." << std::endl;
  ComputeInitialNormals( in_depth_maps_paths, in_camera_paths, params );

  if ( debug_intermediate )
  {
    std::cout << "[DEBUG] Creting raw point cloud with recomputed normals." << std::endl;
    const MVS::PointCloud pcl = FusionDepthMap( in_depth_maps_normals_recomputed_paths, in_camera_paths, params );
    pcl.exportToPly( "raw.normals.ply", true );
  }

  // 2 Compute sigma parameter
  std::cout << "2. Computing sigma parameter." << std::endl;
  ComputeWolffSigma( in_depth_maps_paths, in_camera_paths, params );

  // 3. Filter depth maps
  std::cout << "3. Filter depth maps." << std::endl;
  FilterDepthMaps( in_depth_maps_paths, in_color_paths, in_camera_paths, out_dm_paths, params );

  // 4. Fusion depth maps
  std::cout << "4. Fusion depth maps." << std::endl;
  const MVS::PointCloud pcl = FusionDepthMap( out_dm_paths, in_camera_paths, params );

  // 5. Save
  std::cout << "5. Saving model." << std::endl;
  pcl.exportToPly( params.getModelPath(), true );

  // 1. Project depth maps:
  // For all depth map di
  //   p <- UnProject
  //   N <- PCA
  //   w <- (equation (3))

  // 2. Filter points
  // For all depths map Di
  //    For all p in Di
  //        w(p) <- 0
  //        d(p) <- 0
  //        v(p) <- 0
  //        s    <- 0
  //        s2   <- 0

  //        For all depths map Dj
  //            (u,v) <- Project depth p on Dj
  //            (z)   <- depth of p wrt Dj
  //            Get triangle (u,v)            // Figure (2)
  //            If triangle is flat OR no triangle OR Invalid
  //                Continue;
  //            z(p) <- Interpolation of depth of the triangle vertices
  //            d <- z(p) - z
  //            If d < - sigma
  //                Continue
  //            If d > sigma
  //                Continue
  //            d(p) <- ( w(p) d(p) + (wd)/ sigma ) / ( w(p) + w )
  //            w(p) <- w(p) + w
  //            if d is not sigma
  //                c <- interpolation of color values of the triangle
  //                s <- s + c
  //                s2 <- s2 + c^c
  //                v(p) <- v(p) + 1
  //
  //        p(p) <- sqrt( s2 - s^s/v(p)) / v(p) ) . 2 / (255 * sqrt(3))
  //
  //        If -td < d(p) < 0 AND p(p) < tp AND v(p) > tv
  //           Add point to the valid set

  return EXIT_SUCCESS;
}