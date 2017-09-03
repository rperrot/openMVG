#include "WorkerExportToOpenMVS.hh"


#include "openMVG/cameras/Camera_Pinhole.hpp"
#include "openMVG/cameras/Camera_undistort_image.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/sfm/sfm.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"

#define _USE_EIGEN
#include "software/SfM/InterfaceMVS.h"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"
#include "third_party/progress/progress_display.hpp"

using namespace openMVG;
using namespace openMVG::cameras;
using namespace openMVG::geometry;
using namespace openMVG::image;
using namespace openMVG::sfm;

#include <cstdlib>
#include <string>


namespace openMVG_gui
{

/**
* @brief Ctr
* @param in_sfm_data Input sfm data
* @param out_mvs_path Path of the mvs file to export (not the directory)
* @param out_undist_path Path of the folder containing the undistorted files
*/
WorkerExportToOpenMVS::WorkerExportToOpenMVS( const std::shared_ptr<openMVG::sfm::SfM_Data> in_sfm_data ,
    const std::string & out_mvs_path ,
    const std::string & out_undist_path )
  : m_sfm_data( in_sfm_data ) ,
    m_out_openMVS_file_path( out_mvs_path ) ,
    m_out_undistort_file_path( out_undist_path )
{

}

/**
* @brief get progress range
*/
void WorkerExportToOpenMVS::progressRange( int & min , int & max ) const
{
  min = 0 ;
  max = m_sfm_data->GetViews().size() + 1 ;
}



/**
* @brief Do the computation
*/
void WorkerExportToOpenMVS::process( void )
{

  m_progress_value = 0 ;
  sendProgress() ;

  // Create undistorted images directory structure
  if ( !stlplus::is_folder( m_out_undistort_file_path ) )
  {
    stlplus::folder_create( m_out_undistort_file_path );
    if ( !stlplus::is_folder( m_out_undistort_file_path ) )
    {
      std::cerr << "Cannot access to one of the desired output directory" << std::endl;

      m_progress_value = m_sfm_data->GetViews().size() + 1 ; 
      sendProgress() ; 
    
      emit( finished( NEXT_ACTION_ERROR ) ) ;
      return ;
    }
  }

  // Export data :
  MVS::Interface scene;
  size_t nPoses( 0 );
  const uint32_t nViews( ( uint32_t )m_sfm_data->GetViews().size() );

  // OpenMVG can have not contiguous index, use a map to create the required OpenMVS contiguous ID index
  std::map<openMVG::IndexT, uint32_t> map_intrinsic, map_view;

  // define a platform with all the intrinsic group
  for ( const auto& intrinsic : m_sfm_data->GetIntrinsics() )
  {
    if ( isPinhole( intrinsic.second->getType() ) )
    {
      const Pinhole_Intrinsic * cam = dynamic_cast<const Pinhole_Intrinsic*>( intrinsic.second.get() );
      if ( map_intrinsic.count( intrinsic.first ) == 0 )
      {
        map_intrinsic.insert( std::make_pair( intrinsic.first, scene.platforms.size() ) );
      }
      MVS::Interface::Platform platform;
      // add the camera
      MVS::Interface::Platform::Camera camera;
      camera.K = cam->K();
      // sub-pose
      camera.R = Mat3::Identity();
      camera.C = Vec3::Zero();
      platform.cameras.push_back( camera );
      scene.platforms.push_back( platform );
    }
  }

  // define images & poses
  scene.images.reserve( nViews );
  for ( const auto& view : m_sfm_data->GetViews() )
  {
    map_view[view.first] = scene.images.size();
    MVS::Interface::Image image;
    const std::string srcImage = stlplus::create_filespec( m_sfm_data->s_root_path, view.second->s_Img_path );
    image.name = stlplus::create_filespec( m_out_undistort_file_path, view.second->s_Img_path );
    image.platformID = map_intrinsic.at( view.second->id_intrinsic );
    MVS::Interface::Platform& platform = scene.platforms[image.platformID];
    image.cameraID = 0;
    if ( m_sfm_data->IsPoseAndIntrinsicDefined( view.second.get() ) && stlplus::is_file( srcImage ) )
    {
      MVS::Interface::Platform::Pose pose;
      image.poseID = platform.poses.size();
      const openMVG::geometry::Pose3 poseMVG( m_sfm_data->GetPoseOrDie( view.second.get() ) );
      pose.R = poseMVG.rotation();
      pose.C = poseMVG.center();
      // export undistorted images
      const openMVG::cameras::IntrinsicBase * cam = m_sfm_data->GetIntrinsics().at( view.second->id_intrinsic ).get();
      if ( cam->have_disto() )
      {
        // undistort image and save it
        Image<openMVG::image::RGBColor> imageRGB, imageRGB_ud;
        ReadImage( srcImage.c_str(), &imageRGB );
        UndistortImage( imageRGB, cam, imageRGB_ud, BLACK );
        WriteImage( image.name.c_str(), imageRGB_ud );
      }
      else
      {
        // just copy image
        stlplus::file_copy( srcImage, image.name );
      }
      platform.poses.push_back( pose );
      ++nPoses;
    }
    else
    {
      // image have not valid pose, so set an undefined pose
      image.poseID = NO_ID;
      // just copy the image
      stlplus::file_copy( srcImage, image.name );
    }
    scene.images.emplace_back( image );
    hasIncremented( 1 ) ;
  }

  // define structure
  scene.vertices.reserve( m_sfm_data->GetLandmarks().size() );
  for ( const auto& vertex : m_sfm_data->GetLandmarks() )
  {
    const Landmark & landmark = vertex.second;
    MVS::Interface::Vertex vert;
    MVS::Interface::Vertex::ViewArr& views = vert.views;
    for ( const auto& observation : landmark.obs )
    {
      const auto it( map_view.find( observation.first ) );
      if ( it != map_view.end() )
      {
        MVS::Interface::Vertex::View view;
        view.imageID = it->second;
        view.confidence = 0;
        views.push_back( view );
      }
    }
    if ( views.size() < 2 )
    {
      continue;
    }
    std::sort(
      views.begin(), views.end(),
      [] ( const MVS::Interface::Vertex::View & view0, const MVS::Interface::Vertex::View & view1 )
    {
      return view0.imageID < view1.imageID;
    }
    );
    vert.X = landmark.X.cast<float>();
    scene.vertices.push_back( vert );
  }

  // normalize camera intrinsics
  for ( size_t p = 0; p < scene.platforms.size(); ++p )
  {
    MVS::Interface::Platform& platform = scene.platforms[p];
    for ( size_t c = 0; c < platform.cameras.size(); ++c )
    {
      MVS::Interface::Platform::Camera& camera = platform.cameras[c];
      // find one image using this camera
      MVS::Interface::Image* pImage( nullptr );
      for ( MVS::Interface::Image& image : scene.images )
      {
        if ( image.platformID == p && image.cameraID == c && image.poseID != NO_ID )
        {
          pImage = &image;
          break;
        }
      }
      if ( pImage == nullptr )
      {
        std::cerr << "error: no image using camera " << c << " of platform " << p << std::endl;
        continue;
      }
      // read image meta-data
      ImageHeader imageHeader;
      ReadImageHeader( pImage->name.c_str(), &imageHeader );
      const double fScale( 1.0 / std::max( imageHeader.width, imageHeader.height ) );
      camera.K( 0, 0 ) *= fScale;
      camera.K( 1, 1 ) *= fScale;
      camera.K( 0, 2 ) *= fScale;
      camera.K( 1, 2 ) *= fScale;
    }
  }

  // write OpenMVS data
  if ( !MVS::ARCHIVE::SerializeSave( scene, m_out_openMVS_file_path ) )
  {
    m_progress_value = m_sfm_data->GetViews().size() + 1 ; 
    sendProgress() ; 
  
    emit( finished( NEXT_ACTION_ERROR ) ) ;
    return ;
  }

  std::cout
      << "Scene saved to OpenMVS interface format:\n"
      << "\t" << scene.images.size() << " images (" << nPoses << " calibrated)\n"
      << "\t" << scene.vertices.size() << " Landmarks\n";

  m_progress_value = m_sfm_data->GetViews().size() + 1 ; 
  sendProgress() ; 

  emit( finished( nextAction() ) ) ;
}

/**
* @brief internal progress bar has been incremented, now signal it to the external progress dialog
*/
void WorkerExportToOpenMVS::hasIncremented( int incr )
{
  m_progress_value += incr ;
  sendProgress() ;
}

/**
* @brief Send progress signal
*/
void WorkerExportToOpenMVS::sendProgress( void )
{
  const int p_value = m_progress_value ;
  emit progress( p_value ) ;
}

} // namespace openMVG_gui