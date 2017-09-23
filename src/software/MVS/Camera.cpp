#include "Camera.hpp"

#include "Util.hpp"

#include "openMVG/cameras/cameras.hpp"
#include "openMVG/multiview/essential.hpp"
#include "openMVG/numeric/numeric_io_cereal.hpp"

#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp> // std::string 
#include <cereal/types/utility.hpp> // std::pair
#include <cereal/types/vector.hpp> // std::vector

#include <fstream>

// using namespace cereal ;

namespace MVS
{
// Compute stereo rig motion, assuming cam1 is the reference
std::pair< openMVG::Mat3 , openMVG::Vec3 > RelativeMotion( const Camera & cam1 , const Camera & cam2 )
{
  openMVG::Mat3 R ;
  openMVG::Vec3 t ;

  openMVG::RelativeCameraMotion( cam1.m_R , cam1.m_t , cam2.m_R , cam2.m_t , &R , &t ) ;

  return std::make_pair( R , t ) ;
}

std::pair< int , int > Rescale( const std::pair< int , int > & dim , const int scale )
{
  int div = 1 ;
  for( int i = 0 ; i < scale ; ++i )
  {
    div *= 2 ;
  }
  return std::make_pair( dim.first / div , dim.second / div ) ;
}

Camera::Camera( void )
{

}


// Load cameras from serialization file
Camera::Camera( const std::string & cam )
{
  if( ! load( cam ) )
  {
    std::cerr << "Could not load camera file from serialization" << std::endl;
  }
}

bool Camera::save( const std::string & path )
{
  std::ofstream file( path , std::ios::binary ) ;
  if( ! file )
  {
    std::cerr << "Could not open file : " << path << std::endl ;
    return false ;
  }

  cereal::PortableBinaryOutputArchive ar( file ) ;

  // Store all but the intrinsic (not used because image are undistorted)
  try
  {
    ar( m_K ) ;
    ar( m_K_inv ) ;
    ar( m_K_scaled ) ;
    ar( m_K_inv_scaled ) ;
    ar( m_R ) ;
    ar( m_t ) ;
    ar( m_C ) ;
    ar( m_P ) ;
    ar( m_P_scaled ) ;
    ar( m_M_inv ) ;
    ar( m_M_inv_scaled ) ;
    ar( m_img_path ) ;
    ar( m_cam_dims ) ;
    ar( m_ground_truth ) ;
    ar( m_min_depth ) ;
    ar( m_max_depth ) ;
    ar( m_min_baseline ) ;
    ar( m_max_baseline ) ;
    ar( m_baseline ) ;
    ar( m_mean_baseline ) ;
    ar( m_view_neighbors ) ;
  }
  catch( ... )
  {
    std::cerr << "Could not serialize-out elements " << std::endl ;
    return false ;
  }

  return true ;
}

bool Camera::load( const std::string & path )
{
  std::ifstream file( path , std::ios::binary ) ;
  if( ! file )
  {
    std::cerr << "Could not open file : " << path << std::endl ;
    return false ;
  }

  cereal::PortableBinaryInputArchive ar( file ) ;

  // Store all but the intrinsic (not used because image are undistorted)
  try
  {
    ar( m_K ) ;
    ar( m_K_inv ) ;
    ar( m_K_scaled ) ;
    ar( m_K_inv_scaled ) ;
    ar( m_R ) ;
    ar( m_t ) ;
    ar( m_C ) ;
    ar( m_P ) ;
    ar( m_P_scaled ) ;
    ar( m_M_inv ) ;
    ar( m_M_inv_scaled ) ;
    ar( m_img_path ) ;
    ar( m_cam_dims ) ;
    ar( m_ground_truth ) ;
    ar( m_min_depth ) ;
    ar( m_max_depth ) ;
    ar( m_min_baseline ) ;
    ar( m_max_baseline ) ;
    ar( m_baseline ) ;
    ar( m_mean_baseline ) ;
    ar( m_view_neighbors ) ;
    m_intrinsic = nullptr ;
  }
  catch( ... )
  {
    std::cerr << "Could not serialize-in elements " << std::endl ;
    return false ;
  }

  return true ;
}



/**
 * Extract camera informations from sfm_data
 */
std::vector< Camera > LoadCameras( const openMVG::sfm::SfM_Data & sfm_data , const DepthMapComputationParameters & params )
{
  std::vector< Camera > cams ;

  int id = 0 ;
  std::map< const openMVG::sfm::View * , int > map_view_id ;

  for( openMVG::sfm::Views::const_iterator iter = sfm_data.GetViews().begin();
       iter != sfm_data.GetViews().end();
       ++iter )
  {
    const openMVG::sfm::View * view = iter->second.get();

    if ( sfm_data.IsPoseAndIntrinsicDefined( view ) )
    {
      const std::string srcImage = stlplus::create_filespec( sfm_data.s_root_path, view->s_Img_path );

      openMVG::sfm::Intrinsics::const_iterator iterIntrinsic = sfm_data.GetIntrinsics().find( view->id_intrinsic );
      const openMVG::cameras::IntrinsicBase * intrinsic = iterIntrinsic->second.get();

      const openMVG::geometry::Pose3 pose = sfm_data.GetPoseOrDie( view );
      const openMVG::cameras::Pinhole_Intrinsic * pinhole_cam = static_cast<const openMVG::cameras::Pinhole_Intrinsic *>( intrinsic );

      // Get R,t,C, K
      const openMVG::Mat3 rotation    = pose.rotation() ;
      const openMVG::Vec3 translation = pose.translation() ;
      const openMVG::Vec3 center      = pose.center() ;

      // TODO: be more robust if not a pinhole (but what to do ?)
      const openMVG::cameras::Pinhole_Intrinsic * cam = dynamic_cast<const openMVG::cameras::Pinhole_Intrinsic*>( intrinsic );
      const openMVG::Mat3 K = ScaleK( cam->K() , params.scale() ) ;

      Camera tmp ;
      tmp.m_K = K ;
      tmp.m_K_inv = K.inverse() ;

      // Compute some intrinsic matrix from scale = 0 to scale = 6 ( maybe enough ? )
      for( int scale = 0 ; scale < 5 ; ++scale )
      {
        const openMVG::Mat3 cur_K = ScaleK( cam->K() , scale ) ;
        tmp.m_K_scaled.push_back( cur_K ) ;
        tmp.m_K_inv_scaled.push_back( cur_K.inverse() ) ;
      }

      tmp.m_R = rotation ;
      tmp.m_t = translation ;
      tmp.m_C = center ;
      tmp.m_img_path = srcImage ;
      tmp.m_cam_dims = Rescale( std::make_pair( cam->w() , cam->h() ) , params.scale() ) ;
      tmp.m_intrinsic = const_cast<openMVG::cameras::IntrinsicBase *>( intrinsic ) ;
      openMVG::P_From_KRt( tmp.m_K , tmp.m_R , tmp.m_t , &tmp.m_P ) ;

      for( int scale = 0 ; scale < 5 ; ++scale )
      {
        const openMVG::Mat3 & cur_K = tmp.m_K_scaled[ scale ] ;
        openMVG::Mat34 P ;
        openMVG::P_From_KRt( cur_K , tmp.m_R , tmp.m_t , &P ) ;
        tmp.m_P_scaled.push_back( P ) ;
      }

      {
        // Block of 3x3 starting at (0,0)
        const openMVG::Mat3 M = tmp.m_P.block<3, 3>( 0 , 0 ) ;
        tmp.m_M_inv = M.inverse() ;
      }

      for( int scale = 0 ; scale < 5 ; ++scale )
      {
        const openMVG::Mat34 & P = tmp.m_P_scaled[ scale ] ;
        const openMVG::Mat3 M = tmp.m_P_scaled[ scale ].block<3, 3>( 0 , 0 ) ;
        tmp.m_M_inv_scaled.push_back( M.inverse() ) ;
      }

      cams.push_back( tmp ) ;
      map_view_id[ view ] = id ;
      ++id ;
    }
  }

  // Now compute depth values (min and max) for all views
  for( size_t id_cam = 0 ; id_cam < cams.size() ; ++id_cam )
  {
    cams[ id_cam ].m_min_depth = std::numeric_limits<double>::max() ;
    cams[ id_cam ].m_max_depth = -std::numeric_limits<double>::max() ;
  }


  // Pass all observations to compute depth range
  for( openMVG::sfm::Landmarks::const_iterator iterTracks = sfm_data.structure.begin();
       iterTracks != sfm_data.structure.end();
       ++iterTracks )
  {
    // The observations
    const openMVG::sfm::Observations & obs = iterTracks->second.obs;

    // The 3d position
    const openMVG::Vec3 & X = iterTracks->second.X ;

    for( openMVG::sfm::Observations::const_iterator itObs = obs.begin();
         itObs != obs.end();
         ++itObs )
    {
      const openMVG::sfm::View * view = sfm_data.views.at( itObs->first ).get();
      if( ! map_view_id.count( view ) )
      {
        continue ;
      }

      // Project in the given camera
      const int view_id = map_view_id.at( view ) ;
      Camera & cur_cam = cams[ view_id ] ;

      const openMVG::Vec2 x = itObs->second.x ;

      const double cur_depth = cur_cam.depth( X ) ;  //  openMVG::Depth( cur_cam.m_R , cur_cam.m_t , X ) ;
      const double tmp_depth = cur_depth ;
      if( cur_depth > 0.0 )
      {
        cur_cam.m_min_depth = std::min( cur_cam.m_min_depth , cur_depth ) ;
        cur_cam.m_max_depth = std::max( cur_cam.m_max_depth , cur_depth ) ;

        // Push the ground gruth data
        cur_cam.m_ground_truth.push_back( std::make_pair( x , X ) ) ;
      }
    }
  }

  // Compute neighbors (todo: use a more robust scheme) - shen, goesle, bailer, ...
  const double aRadMin = openMVG::D2R( params.minimumViewAngle() ) ;
  const double aRadMax = openMVG::D2R( params.maximumViewAngle() ) ;
  const int K = params.nbMaximumViewSelection() ;

  for( size_t id_ref_cam = 0 ; id_ref_cam < cams.size() ; ++id_ref_cam )
  {
    // Compute view angle wrt all others cameras
    std::vector< std::pair<int, double> > angle_cam ;
    Camera & cur_ref = cams[ id_ref_cam ] ;
    const openMVG::Vec3 ref_dir = cur_ref.getRay( openMVG::Vec2( cur_ref.m_cam_dims.first / 2 , cur_ref.m_cam_dims.second / 2 ) ).second ;

    std::vector< int > putative_list ;

    for( size_t id_cam = 0 ; id_cam < cams.size() ; ++id_cam )
    {
      if( id_ref_cam == id_cam )
      {
        continue ;
      }

      const Camera & cur_cam = cams[ id_cam ] ;
      const openMVG::Vec3 cur_dir = cur_cam.getRay( openMVG::Vec2( cur_cam.m_cam_dims.first / 2 , cur_cam.m_cam_dims.second / 2 ) ).second ;

      const double angle = AngleBetween( cur_dir , ref_dir ) ;

      if( angle > aRadMin && angle < aRadMax && cur_dir.dot( ref_dir ) > 0.0 )
      {
        putative_list.push_back( id_cam ) ;
      }
    }

    if( putative_list.size() > K )
    {
      std::random_shuffle( putative_list.begin() , putative_list.end() ) ;
    }
    for( size_t id_putative = 0 ; id_putative < putative_list.size() && id_putative < static_cast<size_t>( K ) ; ++id_putative )
    {
      cur_ref.m_view_neighbors.push_back( putative_list[ id_putative ] ) ;
    }
  }

  // Compute baseline
  for( size_t id_cam = 0 ; id_cam < cams.size() ; ++id_cam )
  {
    Camera & cur_ref = cams[ id_cam ] ;

    cur_ref.m_min_baseline = std::numeric_limits<double>::max() ;
    cur_ref.m_max_baseline = - std::numeric_limits<double>::max() ;

    for( size_t id_other = 0 ; id_other < cur_ref.m_view_neighbors.size() ; ++id_other )
    {
      if( id_other != id_cam )
      {
        const Camera & other = cams[ cur_ref.m_view_neighbors[id_other] ] ;

        const double b = ( cur_ref.m_C - other.m_C ).norm() ;
        cur_ref.m_baseline.push_back( b ) ;

        cur_ref.m_min_baseline = std::min( cur_ref.m_min_baseline , b ) ;
        cur_ref.m_max_baseline = std::max( cur_ref.m_max_baseline , b ) ;
      }
    }

    // Mean baseline
    double sum = 0.0 ;
    for( size_t i = 0 ; i < cur_ref.m_baseline.size() ; ++i )
    {
      sum += cur_ref.m_baseline[ i ] ;
    }
    cur_ref.m_mean_baseline = sum / static_cast<double>( cur_ref.m_baseline.size() ) ;
  }

  return cams ;
}

/**
 * @brief Compute Ray (origin, direction), given a 2d pixel
 */
std::pair< openMVG::Vec3 , openMVG::Vec3 > Camera::getRay( const openMVG::Vec2 & x ) const
{
  const openMVG::Vec3 pt = m_R.transpose() * ( m_K_inv * openMVG::Vec3( x[0] , x[1] , 1.0 ) ) ;
  const openMVG::Vec3 dir = pt.normalized() ;
  return std::make_pair( m_C , dir ) ;
}

/**
* @brief Get 3d point for a 2d position and it's depth
*/
openMVG::Vec3 Camera::unProject( const double x , const double y , const double depth , const int scale ) const
{
  const openMVG::Mat3 & Kinv = ( scale == -1 ) ? m_K_inv : m_K_inv_scaled[ scale ] ;
  return m_C + m_R.transpose() * depth * ( Kinv * openMVG::Vec3( x , y , 1.0 ) ) ;
}

/**
* @brief Get 3d point in local coordinate frame (ie assuming the camera is at origin)
*/
openMVG::Vec3 Camera::unProjectLocal( const double x , const double y , const double depth ) const
{
  return depth * m_K_inv * openMVG::Vec3( x , y , 1.0 ) ;
}

double Camera::depth( const openMVG::Vec3 & pt ) const
{
  return m_P( 2 , 0 ) * pt[0] +
         m_P( 2 , 1 ) * pt[1] +
         m_P( 2 , 2 ) * pt[2] +
         m_P( 2 , 3 ) ;
}

double Camera::depthDisparityConversion( const double d , const int scale ) const
{
  const openMVG::Mat3 & K = ( scale == -1 ) ? m_K : m_K_scaled[ scale ] ;
  return K( 0, 0 ) * m_mean_baseline / d ;
}

double Camera::depthDisparityConversion( const double d , const double baseline ) const
{
  return m_K( 0 , 0 ) * baseline / d ;
}


/**
* @brief Compute homography induced by a given stereo rig and a plane
* @param R Rotation from reference (at origin) and the second camera
* @param t Translation vector (from origin to second)
* @param K Intrinsic matrix of origin
* @param Kp Intrinsic matrix of second
* @param n Plane normal
* @param d depth (relative to the origin)
*/
openMVG::Mat3 HomographyTransformation( const openMVG::Mat3 & R ,
                                        const openMVG::Vec3 & t ,
                                        const openMVG::Mat3 & K ,
                                        const openMVG::Mat3 & Kp ,
                                        const openMVG::Vec3 & n ,
                                        const double d )
{
  return Kp * ( R - ( t * n.transpose() ) / d ) * K.inverse() ;
}

openMVG::Mat3 HomographyTransformation( const openMVG::Mat3 & R ,
                                        const openMVG::Vec3 & t ,
                                        const MVS::Camera & cam_ref ,
                                        const MVS::Camera & cam_other ,
                                        const openMVG::Vec4 & pl ,
                                        const int scale )
{
  const openMVG::Mat3 other_K   = ( scale == -1 ) ? cam_other.m_K   : cam_other.m_K_scaled[ scale ] ;
  const openMVG::Mat3 ref_K_inv = ( scale == -1 ) ? cam_ref.m_K_inv : cam_ref.m_K_inv_scaled[ scale ] ;
  const openMVG::Vec3 n( pl[0] , pl[1] , pl[2] );
  return other_K * ( R - ( t * n.transpose() ) / pl[3] ) * ref_K_inv ;
}


/*
 * @param K Input intrinsic matrix
 * @brief scale factor ( 1 -> No change, else / 2^scale )
 */
openMVG::Mat3 ScaleK( const openMVG::Mat3 & K , const int scale )
{
  openMVG::Mat3 res = K ;

  int div = 1 ;
  for( int i = 0 ; i < scale ; ++i )
  {
    div *= 2 ;
  }

  const double factor = 1.0 / static_cast<double>( div ) ;

  res( 0 , 0 ) *= factor ;
  res( 1 , 1 ) *= factor ;

  res( 0 , 2 ) *= factor ;
  res( 1 , 2 ) *= factor ;

  return res ;
}

openMVG::Vec3 Camera::get3dPoint( const double x , const double y , const int scale ) const
{
  openMVG::Vec3 pt ;

  const openMVG::Mat34 & P    = ( scale == -1 ) ? m_P : m_P_scaled[ scale ] ;
  const openMVG::Mat3 & M_inv = ( scale == -1 ) ? m_M_inv : m_M_inv_scaled[ scale ] ;

  pt[0] = x - P( 0 , 3 ) ;
  pt[1] = y - P( 1 , 3 ) ;
  pt[2] = 1.0 - P( 2 , 3 ) ;

  return M_inv * pt ;
}

openMVG::Vec3 Camera::getViewVector( const double x , const double y , const int scale ) const
{
  return ( get3dPoint( x , y , scale ) - m_C ).normalized() ;
}

/**
* @brief Get intrinsic matrix at a specified scale
* @param scale The requested scale
* @return The intrinsic at specified scale
*/
openMVG::Mat3 Camera::getK( const int scale )
{
  return m_K_scaled[ scale ] ;
}


double ComputeDepth( const openMVG::Vec4 & plane , const int id_row , const int id_col , const Camera & cam , const int scale )
{
  const openMVG::Vec3 plane_n( plane[0] , plane[1] , plane[2] ) ;
  const double plane_d = plane[3] ;

  if( std::isinf( plane_d ) || std::isnan( plane_d ) )
  {
    return cam.m_max_depth ;
  }

  // The plane [n,d]
#ifdef FULL_INTERSECTION
  // The ray (src,dst)
  const std::pair< openMVG::Vec3 , openMVG::Vec3 > ray = cam.GetRay( openMVG::Vec2( id_col , id_row ) ) ;

  // Intersection point
  const double t = ( -plane_d - plane_n.dot( ray.first ) ) / plane_n.dot( ray.second ) ;
  openMVG::Vec3 ptX = ray.first + t * ray.second ;

  // Ensure it's in a valid range
  return Clamp( openMVG::Depth( cam.m_R , cam.m_t , ptX ) , cam.m_min_depth , cam.m_max_depth ) ;
#else

  return Clamp( DepthFromPlane( cam , plane_n , plane_d , id_col , id_row , scale ) , 0.0 , cam.m_max_depth * 1.3 ) ;
#endif
}

} // namespace MVS
