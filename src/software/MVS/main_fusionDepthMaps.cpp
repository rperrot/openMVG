#include "DepthMap.hpp"
#include "DepthMapFusionParameters.hpp"
#include "PointCloud.hpp"
#include "PutativePoint.hpp"
#include "Util.hpp"

#include "openMVG/image/pixel_types_io_cereal.hpp"

#include "third_party/cmdLine/cmdLine.h"

#include <cereal/archives/portable_binary.hpp>


#include <fstream>
#include <iostream>
#include <sstream>

void FilterDepthMap( const MVS::Camera & reference_cam ,
                     MVS::DepthMap & map ,
                     const size_t id_reference_cam ,
                     const std::vector< std::string > & in_dm_paths ,
                     const std::vector< MVS::Camera > & cams ,
                     const MVS::DepthMapFusionComputationParameters & params )
{
  // Number of view per pixel
  openMVG::image::Image<int> nb_view( map.Width() , map.Height() , true , 0 ) ;
  // inverse projected points
  openMVG::image::Image< openMVG::Vec3 > projected_pts( map.Width() , map.Height() ) ;

  // 1- project points in 3d
  for( int y = 0 ; y < map.Height() ; ++y )
  {
    for( int x = 0 ; x < map.Width() ; ++x )
    {
      projected_pts( y , x ) = reference_cam.UnProject( x , y , map.Depth( y , x ) ) ;
      nb_view( y , x ) = 0 ;
    }
  }

  // 2 - Inverse project points in the corresponding cameras
  for( size_t id_cam = 0 ; id_cam < in_dm_paths.size() ; ++id_cam )
  {
    if( id_cam == id_reference_cam )
    {
      continue ;
    }

    MVS::DepthMap other_dm( in_dm_paths[ id_cam ] ) ;
    const MVS::Camera & other_cam = cams[ id_cam ] ;

    const double baseline = ( other_cam.m_C - reference_cam.m_C ).norm() ;

    // Project all points in the given image
    for( int y = 0 ; y < map.Height() ; ++y )
    {
      for( int x = 0 ; x < map.Width() ; ++x )
      {
        // Point is already valid, skip useless projection
        if( nb_view( y , x ) >= params.NbMinimumView() )
        {
          continue ;
        }

        const openMVG::Vec2 other_cam_pos   = openMVG::Project( other_cam.m_P , projected_pts( y , x ) ) ;
        if( other_cam_pos[0] < 0 ||
            other_cam_pos[1] < 0 ||
            other_cam_pos[0] >= other_dm.Width() ||
            other_cam_pos[1] >= other_dm.Height() ||
            std::isinf( other_cam_pos[0] ) || std::isinf( other_cam_pos[1] ) ||
            std::isnan( other_cam_pos[0] ) || std::isnan( other_cam_pos[1] ) )
        {
          continue ;
        }

        const openMVG::Vec4 & cur_plane   = map.Plane( y , x ) ;
        const openMVG::Vec4 & other_plane = other_dm.Plane( other_cam_pos[1] , other_cam_pos[0] ) ;
        const openMVG::Vec3 cur_normal( cur_plane[0] , cur_plane[1] , cur_plane[2] ) ;
        const openMVG::Vec3 other_normal( other_plane[0] , other_plane[1] , other_plane[2] ) ;
        const double projected_depth = other_cam.Depth( projected_pts( y , x ) ) ;
        const double other_depth     = other_dm.Depth( other_cam_pos[1] , other_cam_pos[0] ) ;

        const double projected_disparity = other_cam.DepthDisparityConversion( projected_depth , baseline ) ;
        const double other_disparity     = other_cam.DepthDisparityConversion( other_depth , baseline ) ;

        const double delta_disparity         = projected_depth - other_depth ; //  projected_disparity - other_disparity ;

        const double angle_between = MVS::AngleBetween( cur_normal , other_normal ) ;

        if( fabs( delta_disparity ) < params.DepthThreshold() &&
            angle_between < params.AngleThreshold() && cur_normal.dot( other_normal ) > 0.0 )
        {
          ++ nb_view( y , x ) ;
        }
      }
    }
  }

  // 2 - filter depending on the number of valid camera
  for( int y = 0 ; y < map.Height() ; ++y )
  {
    for( int x = 0 ; x < map.Width() ; ++x )
    {
      if( nb_view( y , x ) < params.NbMinimumView() )
      {
        map.Depth( y , x , -1.0 ) ;
      }
    }
  }
}

// Filter a depth map by removing it's points
void FilterDepthMaps( const std::vector< std::string > & in_dm_paths ,
                      const std::vector< std::string > & in_cams_paths ,
                      const std::vector< std::string > & out_dm_paths ,
                      const MVS::DepthMapFusionComputationParameters & params )
{
  std::vector< MVS::Camera > all_cams ;
  for( size_t id_cam = 0 ; id_cam < in_cams_paths.size() ; ++id_cam )
  {
    all_cams.push_back( MVS::Camera( in_cams_paths[ id_cam ] ) ) ;
  }

  for( size_t id_dm = 0 ; id_dm < in_dm_paths.size() ; ++id_dm )
  {
    std::cout << "Filtering depth map : " << id_dm << std::endl ;
    MVS::DepthMap in_dm( in_dm_paths[ id_dm ] ) ;
    MVS::Camera & cur_cam = all_cams[ id_dm ] ;

    FilterDepthMap( cur_cam , in_dm , id_dm , in_dm_paths , all_cams , params ) ;
    in_dm.Save( out_dm_paths[ id_dm ] ) ;
  }
}

openMVG::image::Image<openMVG::image::RGBColor> ReadColorFile( const std::string & path )
{
  openMVG::image::Image<openMVG::image::RGBColor> res ;

  std::ifstream in_color( path , std::ios::binary ) ;
  if( ! in_color )
  {
    std::cerr << "Impossible to read the file " << std::endl;
    return res ;
  }

  cereal::PortableBinaryInputArchive ar_color( in_color ) ;

  try
  {
    ar_color( res ) ;
  }
  catch( ... )
  {
    std::cerr << "Serialization impossible" << std::endl ;
  }
  return res ;
}

MVS::PointCloud CreatePCLFromView( const MVS::Camera & cur_cam ,
                                   const MVS::DepthMap & cur_dm ,
                                   const size_t id_dm ,
                                   const std::vector< std::string > & dm_paths ,
                                   const std::vector< MVS::Camera > & all_cams ,
                                   const MVS::DepthMapFusionComputationParameters & params )
{

  openMVG::image::Image< MVS::PutativePoint > points( cur_dm.Width() , cur_dm.Height() ) ;
  openMVG::image::Image< openMVG::Vec3 > base_points( cur_dm.Width() , cur_dm.Height() ) ;

  // set initial points
  {
    const std::string color_path = params.GetColorPath( id_dm ) ;
    openMVG::image::Image<openMVG::image::RGBColor> cur_img = ReadColorFile( color_path ) ;

    for( int y = 0 ; y < cur_dm.Height() ; ++y )
    {
      for( int x = 0 ; x < cur_dm.Width() ; ++x )
      {
        const double cur_depth = cur_dm.Depth( y , x ) ;

        if( cur_depth > 0.0 )
        {
          const openMVG::Vec4 & cur_plane = cur_dm.Plane( y , x ) ;
          const openMVG::Vec3 pt = cur_cam.UnProject( x , y , cur_depth ) ;
          const openMVG::Vec3 n( cur_plane[0] , cur_plane[1] , cur_plane[2] ) ;
          const openMVG::image::RGBColor cur_col = cur_img( y , x ) ;
          const openMVG::Vec3 cur_color( cur_col.r() / 255.0 , cur_col.g() / 255.0 , cur_col.b() / 255.0 ) ;

          points( y , x ).SetInitialPoint( pt , n , cur_color ) ;
          base_points( y , x ) = pt ;
        }
      }
    }
  }

  // Pass all cameras and get corresponding points
  for( size_t id_cam = 0 ; id_cam < all_cams.size() ; ++id_cam )
  {
    if( id_cam == id_dm )
    {
      continue ;
    }

    MVS::DepthMap other_dm( dm_paths[ id_cam ] ) ;
    const MVS::Camera & other_cam = all_cams[ id_cam ] ;
    openMVG::image::Image<openMVG::image::RGBColor> other_img = ReadColorFile( params.GetColorPath( id_cam ) ) ;

    const double baseline = ( other_cam.m_C - cur_cam.m_C ).norm() ;


    openMVG::image::Image<bool> used( other_dm.Width() , other_dm.Height() , true , false ) ;

    for( int y = 0 ; y < cur_dm.Height() ; ++y )
    {
      for( int x = 0 ; x < cur_dm.Width() ; ++x )
      {
        // Current depth is not valid or already used
        if( cur_dm.Depth( y , x ) < 0.0 )
        {
          continue ;
        }
        const openMVG::Vec2 other_cam_pos   = openMVG::Project( other_cam.m_P , base_points( y , x ) ) ;
        if( other_cam_pos[0] < 0 || other_cam_pos[1] < 0 ||
            other_cam_pos[0] >= other_dm.Width() ||
            other_cam_pos[1] >= other_dm.Height() )
        {
          continue ;
        }
        if( used( other_cam_pos[ 1 ] , other_cam_pos[ 0 ] ) )
        {
          continue ;
        }

        const openMVG::Vec4 & cur_plane   = cur_dm.Plane( y , x ) ;
        const openMVG::Vec4 & other_plane = other_dm.Plane( other_cam_pos[1] , other_cam_pos[0] ) ;
        const openMVG::Vec3 cur_normal( cur_plane[0] , cur_plane[1] , cur_plane[2] ) ;
        const openMVG::Vec3 other_normal( other_plane[0] , other_plane[1] , other_plane[2] ) ;

        // Projection
        const double projected_depth = other_cam.Depth( base_points( y , x ) ) ;
        // Existing value
        const double other_depth   = other_dm.Depth( other_cam_pos[1] , other_cam_pos[0] ) ;

        if( other_depth < 0.0 )
        {
          continue ;
        }
        const double projected_disparity = other_cam.DepthDisparityConversion( projected_depth , baseline ) ;
        const double other_disparity     = other_cam.DepthDisparityConversion( other_depth , baseline ) ;

        const double delta_disparity         =  projected_depth - other_depth ; // projected_disparity - other_disparity ;

        const double angle_between = MVS::AngleBetween( cur_normal , other_normal ) ;

        if( fabs( delta_disparity ) < params.DepthThreshold() &&
            angle_between < params.AngleThreshold() )
        {
          const openMVG::image::RGBColor & cur_col = other_img( other_cam_pos[1] , other_cam_pos[0] ) ;
          const openMVG::Vec3 other_color( cur_col.r() / 255.0 , cur_col.g() / 255.0 , cur_col.b() / 255.0 ) ;
          used( other_cam_pos[1] , other_cam_pos[0] ) = true ;
          const openMVG::Vec3 other_point = other_cam.UnProject( other_cam_pos[0] , other_cam_pos[1] , other_depth ) ;
          points( y , x ).AddCandidate( other_point , other_normal , other_color ) ;
        }
      }
    }

    // Todo save the depth map in order to remove the points already used
  }

  MVS::PointCloud res ;
  for( int y = 0 ; y < cur_dm.Height() ; ++y )
  {
    for( int x = 0 ; x < cur_dm.Width() ; ++x )
    {
      if( points( y , x ).NbCandidate() >= params.NbMinimumView() )
      {
        const std::tuple< openMVG::Vec3 , openMVG::Vec3 , openMVG::Vec3 > cur_pt = points( y , x ).GetPoint() ;
        res.AddPoint( std::get<0>( cur_pt ) , std::get<1>( cur_pt ) , std::get<2>( cur_pt ) ) ;
      }
    }
  }

  return res ;
}


MVS::PointCloud FusionDepthMap( const std::vector< std::string > & dm_paths ,
                                const std::vector< std::string > & cam_paths ,
                                const MVS::DepthMapFusionComputationParameters & params )
{
  std::vector< MVS::Camera > all_cams ;
  for( size_t id_cam = 0 ; id_cam < cam_paths.size() ; ++id_cam )
  {
    all_cams.push_back( MVS::Camera( cam_paths[ id_cam ] ) ) ;
  }

  MVS::PointCloud pcloud ;

  for( size_t id_dm = 0 ; id_dm < cam_paths.size() ; ++id_dm )
  {
    std::cout << "Fusion map : " << id_dm << std::endl ;
    MVS::Camera & cur_camera = all_cams[ id_dm ] ;
    MVS::DepthMap cur_dm( dm_paths[ id_dm ] ) ;

    pcloud.Append( CreatePCLFromView( cur_camera , cur_dm , id_dm , dm_paths , all_cams , params ) ) ;
  }

  return pcloud ;
}


std::vector< std::string > GetInputDepthMapsPaths( const std::string & base_path ,
    const int scale ,
    const MVS::DepthMapFusionComputationParameters & params )
{
  std::vector< std::string > res ;
  const std::string depth = stlplus::create_filespec( base_path , "depth" ) ;
  int id_cam = 0 ;
  while( 1 )
  {
    const std::string cur_cam_folder = params.GetCameraDirectory( id_cam ) ;
    if( ! stlplus::folder_exists( cur_cam_folder ) )
    {
      break ;
    }

    const std::string dm_path = params.GetDepthPath( id_cam ) ;
    if( ! stlplus::file_exists( dm_path ) )
    {
      break ;
    }

    res.push_back( dm_path ) ;

    ++id_cam ;
  }

  return res ;
}

std::vector< std::string > GetInputCameraPaths( const std::string & base_path ,
    const int scale ,
    const MVS::DepthMapFusionComputationParameters & params )
{
  std::vector< std::string > res ;
  int id_cam = 0 ;
  while( 1 )
  {
    const std::string cur_cam_folder = params.GetCameraDirectory( id_cam ) ;
    if( ! stlplus::folder_exists( cur_cam_folder ) )
    {
      break ;
    }

    const std::string cam_path = params.GetCameraPath( id_cam ) ;
    if( ! stlplus::file_exists( cam_path ) )
    {
      break ;
    }

    res.push_back( cam_path ) ;

    ++id_cam ;
  }

  return res ;
}

std::vector< std::string > GetOutputDepthMapsPaths( const std::string & base_path , const int scale , const MVS::DepthMapFusionComputationParameters & params )
{
  std::vector< std::string > res ;
  const std::string depth = stlplus::create_filespec( base_path , "depth" ) ;
  int id_cam = 0 ;
  while( 1 )
  {
    const std::string cur_cam_folder = params.GetCameraDirectory( id_cam ) ;
    if( ! stlplus::folder_exists( cur_cam_folder ) )
    {
      break ;
    }

    const std::string dm_path = params.GetFilteredDepthPath( id_cam ) ;

    res.push_back( dm_path ) ;

    ++id_cam ;
  }

  return res ;
}

int main( int argc , char ** argv )
{
  CmdLine cmd;

  std::string sInOutDir = "" ;
  double fe = 0.1 ;
  double fa = 30.0 ;
  int fcomp = 3 ;
  int scale = 1 ;

  cmd.add( make_option( 'i', sInOutDir, "input_dir" ) ) ;
  cmd.add( make_option( 'e', fe , "depth_threshold" ) ) ;
  cmd.add( make_option( 'a', fa , "angle_threshold" ) ) ;
  cmd.add( make_option( 'c', fcomp , "minimum_view" ) ) ;
  cmd.add( make_option( 's', scale , "scale" ) ) ;

  cmd.process( argc, argv );


  std::cout << "You called fusion with parameters : " << std::endl ;
  std::cout << "In/Out path : " << sInOutDir << std::endl ;
  std::cout << "Max depth threshold (fe) : " << fe << std::endl ;
  std::cout << "Max angle threshold (fa) : " << fa << std::endl ;
  std::cout << "Minimum view (fcomp)     : " << fcomp << std::endl ;
  std::cout << "scale                    : " << scale << std::endl ;

  MVS::DepthMapFusionComputationParameters params( sInOutDir , scale , fe , fa , fcomp ) ;

  std::vector< std::string > in_depth_maps = GetInputDepthMapsPaths( sInOutDir , scale , params ) ;
  std::vector< std::string > in_camera_paths = GetInputCameraPaths( sInOutDir , scale , params ) ;
  std::vector< std::string > out_depth_maps = GetOutputDepthMapsPaths( sInOutDir , scale , params ) ;


  // Pass 1 : filter the depth map and remove some points
  FilterDepthMaps( in_depth_maps , in_camera_paths , out_depth_maps , params ) ;

  // Pass 2 : generate point cloud using the valid points
  const MVS::PointCloud pcl = FusionDepthMap( out_depth_maps , in_camera_paths , params ) ;

  // [ Pass 3 : remove duplicate points ? (if not done in pass2) ]

  // Final pass : save the point cloud
  pcl.ExportToPly( params.GetModelPath() , true ) ;


  return EXIT_SUCCESS ;
}