#include "software/SfMGui/SfMProject.hpp"

#include "openMVG/cameras/cameras.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"

#include "openMVG/exif/sensor_width_database/ParseDatabase.hpp"

#include "software/SfM/SfMInitImageListingHelper.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <iostream>

using namespace stlplus ;
using namespace openMVG::sfm ;
using namespace openMVG::cameras ;

using namespace software::SfM ;

namespace openMVG
{
namespace SfMGui
{
/**
* @brief Open a SfM Project given it's base path
*/
SfMProject::SfMProject( const std::string & input_folder )
  : m_project_root_path( input_folder )
{
  if( ! folder_exists( input_folder ) )
  {
    BuildProjectStructure( input_folder ) ;
  }
  else
  {
    // Try to load sfm_data
    const std::string sfm_data_path =
      folder_append_separator( folder_append_separator( m_project_root_path ) + "sfm" ) + "sfm_data.json" ;

    if( file_exists( sfm_data_path ) )
    {
      Load( m_sfm_data , sfm_data_path , ALL ) ;
    }

    // TODO : load project structure
    // TODO : load project settings
  }
}

/**
* @brief Save all changes
*/
bool SfMProject::Save( void )
{
  // Save SfMData file
  const std::string sfm_folder = folder_append_separator( m_project_root_path ) + "sfm" ;

  if ( ! openMVG::sfm::Save( m_sfm_data ,
                             stlplus::create_filespec( sfm_folder , "sfm_data.json" ).c_str(),
                             ESfM_Data( ALL ) ) )
  {
    return false ;
  }

  // TODO : save project structure
  // TODO : save project settings

  return true ;
}


/**
* @brief Save project in a new location (move everything to the new folder )
* @retval true if save can be done
*/
bool SfMProject::SaveAs( const std::string & path )
{
  BuildProjectStructure( path ) ;

  // Copy project to new path


  // Update new root
  m_project_root_path = path ;
}



/**
* @brief Open an image folder (replace existing images)
*/
bool SfMProject::OpenImageFolder( const std::string & image_folder , const std::string & sensor_width_database_path )
{
  Reset() ;

  std::vector<std::string> vec_image = stlplus::folder_files( image_folder );
  std::sort( vec_image.begin(), vec_image.end() );
  std::ostringstream error_report_stream;

  EINTRINSIC e_User_camera_model = ConvertToEINTRINSIC( m_settings.m_camera_model ) ;


  std::vector<Datasheet> vec_database;
  if ( !sensor_width_database_path.empty() )
  {
    if ( !parseDatabase( sensor_width_database_path, vec_database ) )
    {
      std::cerr
          << "\nInvalid input database: " << sensor_width_database_path
          << ", please specify a valid file." << std::endl;
      return false ;
    }
  }


  // Create SFM data for all images
  if( ! FillSfMImageData( m_sfm_data , image_folder , vec_image , error_report_stream , e_User_camera_model , vec_database ) )
  {
    return false ;
  }
  
  // Save SfM data 
  if( ! Save() )
  {
    return false ;
  }

  return true ;
}

/**
* @brief Remove everyting done in the project
*/
void SfMProject::Reset( void )
{
  // Reset sfm data
  SfM_Data dummy ;
  m_sfm_data = dummy ;

  // Reset directory structure
  folder_delete( m_project_root_path ) ;
  BuildProjectStructure( m_project_root_path ) ;
}

/**
* @brief get GUI folder path
* @return GUI folder path for this project
*/
std::string SfMProject::GetGUIFolder( void ) const
{
  return folder_append_separator( m_project_root_path ) + "gui" ;
}

/**
* @brief Get Thumbnail folder path
* @return thumbnail folder path for this project
*/
std::string SfMProject::GetThumbailFolder( void ) const
{
  return folder_append_separator( folder_append_separator( m_project_root_path ) + "gui" ) + "thumbnails" ;
}

/**
* @brief Get SFM folder path
* @return SFM folder path for this project
*/
std::string SfMProject::GetSFMFolder( void ) const
{
  return folder_append_separator( m_project_root_path ) + "sfm" ;
}

/**
* @brief Set current project settings
* @param set new settings
*/
void SfMProject::SetSettings( const SfMSettings & set )
{
  m_settings = set ;
}

/**
* @brief Get current project settings
* @return Current settings
*/
SfMSettings SfMProject::GetSettings( void ) const
{
  return m_settings ;
}

/**
* @brief Given input images, generate thumbnails
*/
void SfMProject::GenerateThumbnails( void )
{

}

/**
* Folder
* Folder/gui/thumbnail
* Folder/sfm/
* Folder/sfm/sfm_data/...
*/
bool SfMProject::BuildProjectStructure( const std::string & input_folder )
{
  // base_folder
  if( ! folder_exists( input_folder ) )
  {
    if( ! folder_create( input_folder ) )
    {
      std::cerr << "Could not create \"" << input_folder << "\" folder" << std::endl ;
      return false ;
    }
  }

  // gui folder
  const std::string gui_folder = folder_append_separator( input_folder ) + "gui" ;
  if( ! folder_exists( gui_folder ) )
  {
    if( ! folder_create( gui_folder ) )
    {
      std::cerr << "Could not create \"" << gui_folder << "\" folder" << std::endl ;
      return false ;
    }
  }

  // gui/thumbnails folder
  const std::string thumbnail_folder = folder_append_separator( gui_folder ) + "thumbnails" ;
  if( ! folder_exists( thumbnail_folder ) )
  {
    if( ! folder_create( thumbnail_folder ) )
    {
      std::cerr << "Could not create \"" << thumbnail_folder << "\" folder" << std::endl ;
      return false ;
    }
  }

  // sfm folder
  const std::string sfm_folder = folder_append_separator( input_folder ) + "sfm" ;
  if( ! folder_exists( sfm_folder ) )
  {
    if( ! folder_create( sfm_folder ) )
    {
      std::cerr << "Could not create \"" << sfm_folder << "\" folder" << std::endl ;
      return false ;
    }
  }
  return true ;
}

} // namespace SfMGui
} // namespace openMVG