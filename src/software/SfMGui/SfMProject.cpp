#include "software/SfMGui/SfMProject.hpp"

#include "openMVG/cameras/cameras.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"

#include "openMVG/exif/sensor_width_database/ParseDatabase.hpp"

#include "software/SfM/SfMInitImageListingHelper.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <cereal/archives/json.hpp>

#include <iostream>
#include <fstream>

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
  if( ! folder_exists( input_folder ) || ! ValidProjectStructure( input_folder ) )
  {
    // Directory structure is not valid
    BuildProjectStructure( input_folder ) ;
    m_has_unsaved_changes = true ;
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

    const std::string project_file_path = folder_append_separator( input_folder ) + "project.json" ;
    if( file_exists( project_file_path ) )
    {
      std::ifstream file( project_file_path ) ;
      cereal::JSONInputArchive archive( file )  ;
      archive( m_settings ) ;
    }
    m_has_unsaved_changes = false ;
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

  std::ofstream file( folder_append_separator( m_project_root_path ) + "project.json" ) ;
  cereal::JSONOutputArchive archive( file ) ;

  archive( m_project_root_path ) ;
  archive( m_settings ) ;

  m_has_unsaved_changes = false ;

  return true ;
}


/**
* @brief Save project in a new location (move everything to the new folder )
* @retval true if save can be done
*/
bool SfMProject::SaveAs( const std::string & path )
{
  if( ! BuildProjectStructure( path ) )
  {
    return false ;
  }

  // Update new root
  m_project_root_path = path ;

  // Save
  return Save( ) ;
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

  // 1   - Reset directory structure
  // 1.1 - Remove project file
  const std::string project_filename = create_filespec( m_project_root_path , "project.json" ) ;
  if( file_exists( project_filename ) )
  {
    file_delete( project_filename ) ;
  }
  // 1.2 - Remove contents of gui folder if it exists
  const std::string gui_folder = GetGUIFolder() ;
  if( folder_exists( gui_folder ) )
  {
    folder_delete( gui_folder , true ) ;
  }
  // 1.3 - Remove contents of sfm folder if it exists
  const std::string sfm_folder = GetSFMFolder() ;
  if( folder_exists( sfm_folder ) )
  {
    folder_delete( sfm_folder , true ) ;
  }


  // 2 - Rebuild a clean structure
  BuildProjectStructure( m_project_root_path ) ;
  m_has_unsaved_changes = true ;
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
  if( m_settings != set )
  {
    return ;
  }
  m_settings = set ;
  m_has_unsaved_changes = true ;
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
* @brief Compute SfM
*/
void SfMProject::ComputeSfM( void )
{

}

/**
* @brief Export to MVE subdirectory
*/
void SfMProject::ExportToMVE( void )
{

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
      std::cerr << "Could not create \"" << input_folder << "\" folder (input) " << std::endl ;
      return false ;
    }
  }

  // gui folder
  const std::string gui_folder = folder_append_separator( input_folder ) + "gui" ;
  if( ! folder_exists( gui_folder ) )
  {
    if( ! folder_create( gui_folder ) )
    {
      std::cerr << "Could not create \"" << gui_folder << "\" folder (gui)" << std::endl ;
      return false ;
    }
  }

  // gui/thumbnails folder
  const std::string thumbnail_folder = folder_append_separator( gui_folder ) + "thumbnails" ;
  if( ! folder_exists( thumbnail_folder ) )
  {
    if( ! folder_create( thumbnail_folder ) )
    {
      std::cerr << "Could not create \"" << thumbnail_folder << "\" folder (thumbnail)" << std::endl ;
      return false ;
    }
  }

  // sfm folder
  const std::string sfm_folder = folder_append_separator( input_folder ) + "sfm" ;
  if( ! folder_exists( sfm_folder ) )
  {
    if( ! folder_create( sfm_folder ) )
    {
      std::cerr << "Could not create \"" << sfm_folder << "\" folder (sfm)" << std::endl ;
      return false ;
    }
  }
  return true ;
}

bool SfMProject::ValidProjectStructure( const std::string & input_folder )
{
  if( ! folder_exists( input_folder ) )
  {
    return false ;
  }
  // gui folder
  const std::string gui_folder = folder_append_separator( input_folder ) + "gui" ;
  if( ! folder_exists( gui_folder ) )
  {
    return false ;
  }

  // gui/thumbnails folder
  const std::string thumbnail_folder = folder_append_separator( gui_folder ) + "thumbnails" ;
  if( ! folder_exists( thumbnail_folder ) )
  {
    return false ;
  }

  // sfm folder
  const std::string sfm_folder = folder_append_separator( input_folder ) + "sfm" ;
  if( ! folder_exists( sfm_folder ) )
  {
    return false ;
  }

  return true ;
}

/**
* @brief Test if
*/
bool SfMProject::HasUnsavedChanges( void ) const
{
  return m_has_unsaved_changes ;
}



} // namespace SfMGui
} // namespace openMVG