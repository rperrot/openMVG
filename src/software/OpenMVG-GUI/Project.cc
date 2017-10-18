// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Project.hh"

#include "Version.hh"

#include "openMVG/exif/exif_IO_EasyExif.hpp"
#include "openMVG/exif/sensor_width_database/ParseDatabase.hpp"

#include "openMVG/image/image_io.hpp"

#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QImage>

#include <cereal/archives/xml.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/utility.hpp>

#include <algorithm>
#include <exception>
#include <fstream>
#include <vector>

using namespace openMVG ;
using namespace openMVG::cameras ;
using namespace openMVG::exif ;
using namespace openMVG::image ;
using namespace openMVG::sfm ;

namespace openMVG_gui
{

/**
* @brief Constructor from an existing project
* @param projectFile The path of the project file
*/
Project::Project( const std::string & projectFile , std::shared_ptr<SceneManager> scn )
  : m_project_paths( "" ) ,
    m_scene_mgr( scn ) ,
    m_sfm_data( nullptr ) ,
    m_sparse_point_cloud( nullptr )
{
  open( projectFile ) ;
}

/**
* @brief Constructor for a new project
* @param base_path The path of the project
* @param img_dir The path containing images of the project
* @param intrin_params Intrinsic params used for the initialisation
* @param camera_sensor_width_database_file File containing the sensor database
* @todo : need to add a third parameter for intrinsic data ?
*/
Project::Project( const std::string & base_path ,
                  const std::string & image_path ,
                  const IntrinsicParams & intrin_params ,
                  const std::string camera_sensor_width_database_file ,
                  std::shared_ptr<SceneManager> scn ,
                  C_Progress * progress )
  :
  m_project_base_path( base_path ) ,
  m_project_image_path( image_path ) ,
  m_project_paths( base_path ) ,
  m_sfm_method( SFM_METHOD_INCREMENTAL ) ,
  m_scene_mgr( scn ) ,
  m_sparse_point_cloud( nullptr )
{
  createProject( base_path , image_path , intrin_params , camera_sensor_width_database_file , progress ) ;
}

/**
* @brief The number of image in the project
*/
size_t Project::nbImage( void ) const
{
  return m_sfm_data->GetViews().size() ;
}

/**
* @brief Save project in base path
*/
void Project::save( void )
{
  std::ofstream file( stlplus::create_filespec( m_project_base_path , "project.omvg" ) ) ;
  if( ! file )
  {
    std::cerr << "Could not save project" << std::endl ;
    throw std::runtime_error( "Could not save project" ) ;
    return ;
  }

  // Save global project state
  cereal::XMLOutputArchive archive( file );

  int major_version = GUIVersionMajorNumber() ;
  int minor_version = GUIVersionMinorNumber() ;
  int revision_version = GUIVersionRevisionNumber() ;

  archive( cereal::make_nvp( "major_version" , major_version ) ) ;
  archive( cereal::make_nvp( "minor_version" , minor_version ) ) ;
  archive( cereal::make_nvp( "revision_version" , revision_version ) ) ;

  archive( cereal::make_nvp( "project_path" , m_project_base_path ) ) ;
  archive( cereal::make_nvp( "image_path" , m_project_image_path ) ) ;
  archive( cereal::make_nvp( "sfm_method" , m_sfm_method ) ) ;

  // Save Feature computation params
  archive( cereal::make_nvp( "features_params" , m_feature_params ) ) ;

  // Save Feature Matching params
  archive( cereal::make_nvp( "matching_params" , m_match_params ) ) ;

  // Save SfM computation params
  archive( cereal::make_nvp( "incremental_sfm_params" , m_incremental_sfm_params ) ) ;
  archive( cereal::make_nvp( "global_sfm_params" , m_global_sfm_params ) );

  // Save mask enabled/disabled param
  archive( cereal::make_nvp( "mask_enabled" , m_mask_enabled ) ) ;

  // Viewport camera
  archive( cereal::make_nvp( "viewport_camera" , m_viewport_camera ) ) ;


  m_saved = true ;
}

/**
* @brief Open project file and associated data
* @param project File Project file
*/
void Project::open( const std::string & projectFile )
{
  std::ifstream file( projectFile ) ;
  if( ! file )
  {
    std::cerr << "Could not load project" << std::endl ;
    throw std::runtime_error( "Unable to load file" );
    return  ;
  }

  // Save global project state
  cereal::XMLInputArchive archive( file );

  int major_version ;
  int minor_version ;
  int revision_version ;
  archive( cereal::make_nvp( "major_version" , major_version ) ) ;
  archive( cereal::make_nvp( "minor_version" , minor_version ) ) ;
  archive( cereal::make_nvp( "revision_version" , revision_version ) ) ;

  archive( cereal::make_nvp( "project_path" , m_project_base_path ) ) ;
  m_project_paths = ProjectPaths( m_project_base_path ) ;
  archive( cereal::make_nvp( "image_path" , m_project_image_path ) ) ;
  archive( cereal::make_nvp( "sfm_method" , m_sfm_method ) ) ;

  // Load Feature computation params
  archive( cereal::make_nvp( "features_params" , m_feature_params ) ) ;

  // Load Feature Matching params
  archive( cereal::make_nvp( "matching_params" , m_match_params ) ) ;

  // Load SfM computation params
  archive( cereal::make_nvp( "incremental_sfm_params" , m_incremental_sfm_params ) ) ;
  archive( cereal::make_nvp( "global_sfm_params" , m_global_sfm_params ) );

  // Save mask enabled/disabled param
  archive( cereal::make_nvp( "mask_enabled" , m_mask_enabled ) ) ;

  // Load sfm_data ?
  // either from reconstruction path or at least from matches path
  const std::string matchesSfMDataPath = stlplus::create_filespec( m_project_paths.matchesPath() , "sfm_data.json" );
  const std::string reconstructionSeqSfMDataPath = stlplus::create_filespec( m_project_paths.reconstructionSequentialPath() , "sfm_data.bin" ) ;
  const std::string reconstructionGloSfMDataPath = stlplus::create_filespec( m_project_paths.reconstructionGlobalPath() , "sfm_data.bin" ) ;

  if( stlplus::file_exists( reconstructionSeqSfMDataPath ) )
  {
    // Load it
    m_sfm_data = std::make_shared<SfM_Data>() ;
    if( ! Load( *m_sfm_data , reconstructionSeqSfMDataPath , openMVG::sfm::ESfM_Data( openMVG::sfm::ESfM_Data::ALL ) ) )
    {
      std::cerr << "Could not load sfm_data" ;
    }
  }
  else if( stlplus::file_exists( reconstructionGloSfMDataPath ) )
  {
    // Load it
    m_sfm_data = std::make_shared<SfM_Data>() ;
    if( ! Load( *m_sfm_data , reconstructionGloSfMDataPath , openMVG::sfm::ESfM_Data( openMVG::sfm::ESfM_Data::ALL ) ) )
    {
      std::cerr << "Could not load sfm_data" ;
    }

  }
  else if( stlplus::file_exists( matchesSfMDataPath ) )
  {
    // Load it
    m_sfm_data = std::make_shared<SfM_Data>() ;
    if( ! Load( *m_sfm_data , matchesSfMDataPath , openMVG::sfm::ESfM_Data( openMVG::sfm::ESfM_Data::VIEWS | openMVG::sfm::ESfM_Data::INTRINSICS ) ) )
    {
      std::cerr << "Could not load sfm_data" ;
    }
  }
  // Load camera
  archive( cereal::make_nvp( "viewport_camera" , m_viewport_camera ) ) ;

  m_saved = true ;
}


/**
* @brief Get path of a given image
* @param id_image Id of the image to get
* @node indice start at 0
* @note if id_image >= nbImage() return an empty string
* @return image path
*/
std::string Project::getImagePath( const size_t id_image ) const
{
  return stlplus::create_filespec( m_sfm_data->s_root_path, m_sfm_data->GetViews().at( id_image )->s_Img_path ) ;
}

/**
* @brief Get mask image path of a specified image
* @param id_image Id of the image to get
* @return mask image path
*/
std::string Project::getMaskImagePath( const size_t id_image ) const
{
  return stlplus::create_filespec( m_sfm_data->s_root_path ,
                                   stlplus::basename_part( m_sfm_data->GetViews().at( id_image )->s_Img_path ) + "_mask", "png" );
}


/**
* @brief Indicate if all image have features computed
*/
bool Project::hasAllFeaturesComputed( void ) const
{
  const std::string matches_dir = m_project_paths.featuresPath( featureParams() ) ;
  for( int i = 0; i < static_cast<int>( m_sfm_data->views.size() ); ++i )
  {
    openMVG::sfm::Views::const_iterator iterViews = m_sfm_data->views.begin();
    std::advance( iterViews, i );
    const openMVG::sfm::View * view = iterViews->second.get();
    const std::string sView_filename = stlplus::create_filespec( m_sfm_data->s_root_path, view->s_Img_path ) ;
    const std::string sFeat = stlplus::create_filespec( matches_dir, stlplus::basename_part( sView_filename ), "feat" ) ;
    const std::string sDesc = stlplus::create_filespec( matches_dir, stlplus::basename_part( sView_filename ), "desc" );

    //If features or descriptors file are missing, we haven't done all, so conclude
    if ( !stlplus::file_exists( sFeat ) || !stlplus::file_exists( sDesc ) )
    {
      return false ;
    }
  }
  return true ;
}

/**
* @brief Indicate if some of the images have features computed
*/
bool Project::hasPartialFeaturesComputed( void ) const
{
  size_t nb_computed = 0 ;
  const std::string matches_dir = m_project_paths.matchesPath() ;
  for( int i = 0; i < static_cast<int>( m_sfm_data->views.size() ); ++i )
  {
    openMVG::sfm::Views::const_iterator iterViews = m_sfm_data->views.begin();
    std::advance( iterViews, i );
    const openMVG::sfm::View * view = iterViews->second.get();
    const std::string sView_filename = stlplus::create_filespec( m_sfm_data->s_root_path, view->s_Img_path ) ;
    const std::string sFeat = stlplus::create_filespec( matches_dir, stlplus::basename_part( sView_filename ), "feat" ) ;
    const std::string sDesc = stlplus::create_filespec( matches_dir, stlplus::basename_part( sView_filename ), "desc" );

    //If features or descriptors file are missing, we haven't done all, so conclude
    if ( !stlplus::file_exists( sFeat ) || !stlplus::file_exists( sDesc ) )
    {
      return false ;
    }
    else
    {
      ++nb_computed ;
    }
  }
  return ( nb_computed > 0 ) && ( nb_computed < m_sfm_data->views.size() ) ;
}



/**
* @brief Indicate if all matches are computed
*/
bool Project::hasAllMatchesComputed( void ) const
{
  // Just check if the matches.f.bin (or matches.e.bin or matches.h.bin) is here
  // TODO : check if it's a reasonable test :
  // -> If any of the matches.X.bin is present, it should be enough ?
  const std::string matchesPath = projectPaths().featuresPath( featureParams() ) ;
  const MatchingParams matchesParams = this->matchingParams() ;

  std::string matchesName ;
  switch( matchesParams.geometricModel() )
  {
    case MATCHING_GEOMETRY_TYPE_HOMOGRAPHY:
    {
      matchesName = "matches.h.bin";
      break ;
    }
    case MATCHING_GEOMETRY_TYPE_FUNDAMENTAL:
    {
      matchesName = "matches.f.bin" ;
      break ;
    }
    case MATCHING_GEOMETRY_TYPE_ESSENTIAL:
    {
      matchesName = "matches.e.bin" ;
      break ;
    }
  }

  return stlplus::file_exists( stlplus::create_filespec( matchesPath , matchesName ) ) ;
}

/**
* @brief Indicate if SfM is computed
*/
bool Project::hasSfMComputed( void ) const
{
  // Check if the cloud_and_poses is present
  const std::string cloudPath = m_project_paths.plyCloud( sfMMethod() ) ;
  return stlplus::file_exists( cloudPath ) ;
}

/**
* @brief Indicate if Color is computed
*/
bool Project::hasColorComputed( void ) const
{
  // Check if the colorized is present
  const std::string colorizedPath = m_project_paths.colorizedPlyCloud( sfMMethod() ) ;
  return stlplus::file_exists( colorizedPath ) ;
}

/**
* @brief Indicate if matches.f.bin is present
* @retval true if file exists
* @retval false if file does not exist
*/
bool Project::hasMatchesFundamentalFiltered( void ) const
{
  const std::string matches_path = m_project_paths.featuresPath( featureParams() ) ;
  return stlplus::file_exists( stlplus::create_filespec( matches_path , "matches.f.bin" ) );
}

/**
* @brief Indicate if matches.e.bin is present
* @retval true if file exists
* @retval false if file does not exist
*/
bool Project::hasMatchesEssentialFiltered( void ) const
{
  const std::string matches_path = m_project_paths.featuresPath( featureParams() ) ;
  return stlplus::file_exists( stlplus::create_filespec( matches_path , "matches.e.bin" ) );
}

/**
* @brief Indicate if matches.h.bin is present
* @param true if file exists
* @retval false if file does not exist
*/
bool Project::hasMatchesHomographyFiltered( void ) const
{
  const std::string matches_path = m_project_paths.featuresPath( featureParams() ) ;
  return stlplus::file_exists( stlplus::create_filespec( matches_path , "matches.h.bin" ) );
}

/**
* @brief Get feature parameters
* @return current feature computation parameters
*/
FeatureParams Project::featureParams( void ) const
{
  return m_feature_params ;
}

/**
* @brief Set feature computation parameters
* @param f_params New params
*/
void Project::setFeatureParams( const FeatureParams & f_params )
{
  m_feature_params = f_params ;
  // TODO : not exactly : if params are the same, save is still ok
  m_saved = false ;
}

/**
* @brief get matching parameters
* @return curent matching parameters
*/
MatchingParams & Project::matchingParams( void )
{
  return m_match_params ;
}

/**
* @brief get matching parameters
* @return curent matching parameters
*/
MatchingParams Project::matchingParams( void ) const
{
  return m_match_params ;
}


/**
* @brief Set Matching parameters
* @param m_params New params
*/
void Project::setMatchingParams( const MatchingParams & m_params )
{
  m_match_params = m_params ;
  // TODO : not exactly : if params are the same, save is still ok
  m_saved = false ;
}

/**
* @brief Get incremental SfM Parameters
* @return incremental SfM parameters
*/
IncrementalSfMParams Project::incrementalSfMParams( void ) const
{
  return m_incremental_sfm_params ;
}

/**
* @brief Set incremental SfM Parameters
* @param i_sfm_params New params
*/
void Project::setIncrementalSfMParams( const IncrementalSfMParams & i_sfm_params )
{
  m_incremental_sfm_params = i_sfm_params ;
  // TODO : not exactly : if params are the same, save is still ok
  m_saved = false ;
}

/**
* @brief Get global SfM Parameters
* @return incremental SfM params
*/
GlobalSfMParams Project::globalSfMParams( void ) const
{
  return m_global_sfm_params ;
}

/**
* @brief Set global SfM Parameters
* @param g_sfm_params New params
*/
void Project::setGlobalSfMParams( const GlobalSfMParams & g_sfm_params )
{
  m_global_sfm_params = g_sfm_params ;
  // TODO : not exactly : if params are the same, save is still ok
  m_saved = false ;
}

/**
* @brief Get images IDs
* @return get current image ids
*/
std::vector< int > Project::imageIds( void ) const
{
  std::vector< int > res ;

  for( auto it : m_sfm_data->GetViews() )
  {
    res.emplace_back( it.first ) ;
  }
  return res ;
}

/**
 * @brief Get warper to get project paths
 * @return object containing project paths
 */
ProjectPaths Project::projectPaths( void ) const
{
  return m_project_paths ;
}

/**
* @brief Create project (structure and SfM_data)
* @param base_path The path of the project
* @param img_dir The path containing images of the project
* @param intrin_params Intrinsic params used for the initialisation
* @param camera_sensor_width_database_file File containing the sensor database
* @todo : need to add a third parameter for intrinsic data ?
*/
void Project::createProject( const std::string & base_path ,
                             const std::string & image_path ,
                             const IntrinsicParams & intrin_params ,
                             const std::string camera_sensor_width_database_file ,
                             C_Progress * progress )
{
  m_project_base_path = base_path ;
  m_project_image_path = image_path ;

  // 1 - create directory structure
  if( ! createDirectoryStructure( base_path ) )
  {
    std::cerr << "Could not create directory structure" << std::endl ;
    throw std::runtime_error( "Could not create project directory" ) ;
    return ;
  }

  // 2 - load camera db
  std::vector<Datasheet> vec_database;
  if ( !parseDatabase( camera_sensor_width_database_file , vec_database ) )
  {
    std::cerr << "Could not parse database" << std::endl ;
    throw std::runtime_error( "Could not load sensor width database" ) ;
    return ;
  }

  // 3 - create empty Sfm_data
  m_sfm_data = std::make_shared<SfM_Data>() ;

  // Setup main image root_path
  Views & views = m_sfm_data->views;
  Intrinsics & intrinsics = m_sfm_data->intrinsics;
  m_sfm_data->s_root_path = image_path;

  // 4 - get input images
  std::vector<std::string> vec_image = stlplus::folder_files( image_path );
  std::sort( vec_image.begin(), vec_image.end() );

  double width , height , ppx , ppy , focal ;

  if( progress )
  {
    progress->restart( vec_image.size() ) ;
  }

  // 5 - get valid images and initialize intrinsics for each views
  for ( std::vector<std::string>::const_iterator iter_image = vec_image.begin();
        iter_image != vec_image.end();
        ++iter_image  )
  {
    // Read meta data to fill camera parameter (w,h,focal,ppx,ppy) fields.
    width = height = ppx = ppy = focal = -1.0;

    const std::string sImageFilename = stlplus::create_filespec( image_path, *iter_image );
    const std::string sImFilenamePart = stlplus::filename_part( sImageFilename );

    // Test if the image format is supported:
    if ( openMVG::image::GetFormat( sImageFilename.c_str() ) == openMVG::image::Unknown )
    {
      std::cout << "Unknown image format : " << sImageFilename.c_str() << std::endl ;
      if( progress )
      {
        ++( *progress ) ;
      }
      continue; // image cannot be opened
    }

    if( sImFilenamePart.find( "mask.png" ) != std::string::npos
        || sImFilenamePart.find( "_mask.png" ) != std::string::npos )
    {
      if( progress )
      {
        ++( *progress ) ;
      }
      continue;
    }

    ImageHeader imgHeader;
    QImage img( sImageFilename.c_str() ) ;
    if ( img.isNull() ) // openMVG::image::ReadImageHeader( sImageFilename.c_str(), &imgHeader ) )
    {
      if( progress )
      {
        ++( *progress ) ;
      }
      continue;  // image cannot be read
    }

    width = img.width();
    height = img.height() ;
    ppx = width / 2.0;
    ppy = height / 2.0;

    std::unique_ptr<Exif_IO> exifReader( new Exif_IO_EasyExif );
    exifReader->open( sImageFilename );

    const bool bHaveValidExifMetadata =
      exifReader->doesHaveExifInfo()
      && !exifReader->getModel().empty();

    // RPERROT : need to find how to provide it from intrin_params
    // Consider the case where the focal is provided manually
    /*
    if ( !bHaveValidExifMetadata || focal_pixels != -1 )
    {
      if ( sKmatrix.size() > 0 ) // Known user calibration K matrix
      {
        if ( !checkIntrinsicStringValidity( sKmatrix, focal, ppx, ppy ) )
        {
          focal = -1.0;
        }
      }
      else // User provided focal length value
        if ( focal_pixels != -1 )
        {
          focal = focal_pixels;
        }
    }
    else
    */
    // If image contains meta data
    {
      const std::string sCamModel = exifReader->getModel();

      // Handle case where focal length is equal to 0
      if ( exifReader->getFocal() == 0.0f )
      {
        focal = -1.0;
      }
      else
        // Create the image entry in the list file
      {
        Datasheet datasheet;
        if ( getInfo( sCamModel, vec_database, datasheet ) )
        {
          // The camera model was found in the database so we can compute it's approximated focal length
          const double ccdw = datasheet.sensorSize_;
          focal = std::max ( width, height ) * exifReader->getFocal() / ccdw;
        }
        else
        {

        }
      }
    }

    // Build intrinsic parameter related to the view
    std::shared_ptr<IntrinsicBase> intrinsic ( NULL );

    if ( focal > 0 && ppx > 0 && ppy > 0 && width > 0 && height > 0 )
    {
      intrinsic = intrin_params.getIntrinsic( width , height , focal , ppx , ppy ) ;
    }


    // rperrot : need to find how to provide GPS data
    // Build the view corresponding to the image
    /*
    const std::pair<bool, Vec3> gps_info = checkGPS( sImageFilename, i_GPS_XYZ_method );
    if ( gps_info.first && cmd.used( 'P' ) )
    {
      ViewPriors v( *iter_image, views.size(), views.size(), views.size(), width, height );

      // Add intrinsic related to the image (if any)
      if ( intrinsic == NULL )
      {
        //Since the view have invalid intrinsic data
        // (export the view, with an invalid intrinsic field value)
        v.id_intrinsic = UndefinedIndexT;
      }
      else
      {
        // Add the defined intrinsic to the sfm_container
        intrinsics[v.id_intrinsic] = intrinsic;
      }

      v.b_use_pose_center_ = true;
      v.pose_center_ = gps_info.second;
      // prior weights
      if ( prior_w_info.first == true )
      {
        v.center_weight_ = prior_w_info.second;
      }

      // Add the view to the sfm_container
      views[v.id_view] = std::make_shared<ViewPriors>( v );
    }
    else
    */
    {
      View v( *iter_image, views.size(), views.size(), views.size(), width, height );

      // Add intrinsic related to the image (if any)
      if ( intrinsic == NULL )
      {
        //Since the view have invalid intrinsic data
        // (export the view, with an invalid intrinsic field value)
        v.id_intrinsic = UndefinedIndexT;
      }
      else
      {
        // Add the defined intrinsic to the sfm_container
        intrinsics[v.id_intrinsic] = intrinsic;
      }

      // Add the view to the sfm_container
      views[v.id_view] = std::make_shared<View>( v );
    }

    if( progress )
    {
      ++( *progress ) ;
    }
  }

  // Create default camera
  m_viewport_camera = std::make_shared<Camera>() ;
  m_scene_mgr->setCamera( m_viewport_camera ) ;

  m_saved = false ;
}

/**
* @brief Create directory structure for a new project
* @param base_path base directory
* @retval true if creation is OK
* @retval false if folder creation failed
*/
bool Project::createDirectoryStructure( const std::string & base_path )
{
  /**
  *
  * base_path / gui / thumbnails
  * base_path / sfm / matches
  * base_path / sfm / reconstruction_sequential
  * base_path / sfm / reconstruction_global
  * base_path / sfm / exporters
  */

  ProjectPaths prjPaths( base_path ) ;
  const std::string gui_path = prjPaths.guiPath( ) ;
  const std::string thumb_path = prjPaths.thumbnailsPath( ) ;
  const std::string sfm_path = prjPaths.sfmBasePath( ) ;
  const std::string matches_path = prjPaths.matchesPath( ) ;
  const std::string rec_global_path = prjPaths.reconstructionGlobalPath( ) ;
  const std::string rec_sequen_path = prjPaths.reconstructionSequentialPath( ) ;
  const std::string global_features_dir = prjPaths.globalFeaturePath( ) ;
  const std::string exporter_dir = prjPaths.exportPath( ) ;

  if( !stlplus::folder_exists( base_path ) )
  {
    std::cerr << "base does not exists" << std::endl ;
    return false ;
  }

  // gui
  if( ! stlplus::folder_exists( gui_path ) )
  {
    if( ! stlplus::folder_create( gui_path ) )
    {
      std::cerr << "Could not create gui" << std::endl ;
      // TODO : do we have to remove the path already created ?
      return false ;
    }
    if( ! stlplus::folder_exists( gui_path ) )
    {
      // TODO : do we have to remove the path already created ?
      return false ;
    }
  }


  // sfm
  if( ! stlplus::folder_exists( sfm_path ) )
  {
    if( ! stlplus::folder_create( sfm_path ) )
    {
      std::cerr << "Could not create sfm" << std::endl ;
      // TODO : do we have to remove the path already created ?
      return false ;
    }
    if( ! stlplus::folder_exists( sfm_path ) )
    {
      // TODO : do we have to remove the path already created ?
      return false ;
    }
  }


  // gui/thumbs
  if( ! stlplus::folder_exists( thumb_path ) )
  {
    if( ! stlplus::folder_create( thumb_path ) )
    {
      std::cerr << "Could not create thumb" << std::endl ;
      // TODO : do we have to remove the path already created ?
      return false ;
    }
    if( ! stlplus::folder_exists( thumb_path ) )
    {
      // TODO : do we have to remove the path already created ?
      return false ;
    }
  }

  // sfm/matches
  if( ! stlplus::folder_exists( matches_path ) )
  {
    if( ! stlplus::folder_create( matches_path ) )
    {
      // TODO : do we have to remove the path already created ?
      return false ;
    }
    if( ! stlplus::folder_exists( matches_path ) )
    {
      // TODO : do we have to remove the path already created ?
      return false ;
    }
  }

  // sfm/features
  if( ! stlplus::folder_exists( global_features_dir ) )
  {
    if( ! stlplus::folder_create( global_features_dir ) )
    {
      return false ;
    }
    if( ! stlplus::folder_exists( global_features_dir ) )
    {
      return false ;
    }
  }

  // sfm/reconstruction_global
  if( ! stlplus::folder_exists( rec_global_path ) )
  {
    if( ! stlplus::folder_create( rec_global_path ) )
    {
      // TODO : do we have to remove the path already created ?
      return false ;
    }
    if( ! stlplus::folder_exists( rec_global_path ) )
    {
      // TODO : do we have to remove the path already created ?
      return false ;
    }
  }

  // sfm/reconstruction_sequential
  if( ! stlplus::folder_exists( rec_sequen_path ) )
  {
    if( ! stlplus::folder_create( rec_sequen_path ) )
    {
      // TODO : do we have to remove the path already created ?
      return false ;
    }
    if( ! stlplus::folder_exists( rec_sequen_path ) )
    {
      // TODO : do we have to remove the path already created ?
      return false ;
    }
  }

  // sfm/exporters
  if( ! stlplus::folder_exists( exporter_dir ) )
  {
    if( ! stlplus::folder_create( exporter_dir ) )
    {
      return false ;
    }
    if( ! stlplus::folder_exists( exporter_dir ) )
    {
      return false ;
    }
  }

  return true ;
}


/**
* @brief get access to the sfm data structure
* @return The sfm_data structure
*/
std::shared_ptr<openMVG::sfm::SfM_Data> Project::SfMData( void ) const
{
  return m_sfm_data ;
}

/**
* @brief Get list of image names
* @note this is only the base name without the full path
* @return id of the image, filename
*/
std::vector< std::pair< int , std::string > > Project::GetImageNames( void ) const
{
  std::vector< std::pair< int , std::string > > res ;

  const int nb_view = m_sfm_data->GetViews().size();
  for( int id_image = 0 ; id_image < nb_view ; ++id_image )
  {
    openMVG::sfm::Views::const_iterator iterViews = m_sfm_data->views.begin();
    std::advance( iterViews, id_image );
    const openMVG::sfm::View * view = iterViews->second.get();

    const int index = view->id_view ;
    const std::string path = view->s_Img_path ;
    res.emplace_back( std::make_pair( index , path ) ) ;
  }
  return res ;
}

/**
* @brief Get current SfM method to use
* return current sfm method to use
*/
SfMMethod Project::sfMMethod( void ) const
{
  return m_sfm_method ;
}

/**
* @brief Set current SfM method to use
* @param meth new sfm method to use
*/
void Project::setSfMMethod( const SfMMethod & meth )
{
  if( meth != m_sfm_method )
  {
    m_saved = false ;
  }
  m_sfm_method = meth ;
}

/**
* @brief get 3d scene manager
* @return scene manager
*/
std::shared_ptr<SceneManager> Project::sceneManager( void ) const
{
  return m_scene_mgr ;
}

/**
* @brief The sparse point cloud associated with the scene
* @return the current sparse point cloud
* @note can be nullptr
*/
std::shared_ptr<RenderableObject> Project::sparsePointCloud( void ) const
{
  return m_sparse_point_cloud ;
}

/**
* @brief Set the current sparse point cloud
* @param obj New sparse point cloud
*/
void Project::setSparsePointCloud( std::shared_ptr<RenderableObject> obj )
{
  m_sparse_point_cloud = obj ;
}

/**
* @brief Indicate if some parameters have changed since the last save on disk
* @retval true Something has changed since the last save
* @retval false Everything is up to date wrt to the project file
*/
bool Project::hasUnsavedChange( void ) const
{
  return ! m_saved ;
}

/**
* @brief Indicate if mask is enabled for a specified image
* @param id Id of the image
*/
bool Project::maskEnabled( const int id ) const
{
  if( ! m_mask_enabled.count( id ) )
  {
    return false ;
  }
  else
  {
    return m_mask_enabled.at( id ) ;
  }
}

/**
 * @brief Enable/disable mask on selected id
 */
void Project::setMaskEnabled( const int id , const bool value )
{
  m_mask_enabled[ id ] = value ;
}

/**
* @brief Get current view camera
* @return current view camera
*/
std::shared_ptr<Camera> Project::viewportCamera( void ) const
{
  return m_viewport_camera ;
}




} // namespace openMVG_gui