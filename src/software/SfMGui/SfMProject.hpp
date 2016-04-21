#ifndef _OPENMVG_SFMGUI_SFM_PROJECT_HH_
#define _OPENMVG_SFMGUI_SFM_PROJECT_HH_

#include "openMVG/sfm/sfm_data.hpp"
#include "software/SfMGui/SfMSettings.hpp"

#include <string>

namespace openMVG
{
namespace SfMGui
{
/**
* @brief class managing a SfM project in GUI
*/
class SfMProject
{
  public:

    /**
    * @brief Open a SfM Project given it's base path
    */
    SfMProject( const std::string & input_folder ) ;

    /**
    * @brief Save all changes
    */
    bool Save( void ) ;

    /**
    * @brief Save project in a new location (move everything to the new folder )
    * @retval true if save can be done
    */
    bool SaveAs( const std::string & path ) ;

    /**
    * @brief Open an image folder (replace existing images)
    */
    bool OpenImageFolder( const std::string & filename , const std::string & sensor_width_database_path ) ;

    /**
    * @brief Remove everyting done in the project
    */
    void Reset( void ) ;

    /**
    * @brief get GUI folder path
    * @return GUI folder path for this project
    */
    std::string GetGUIFolder( void ) const ;

    /**
    * @brief Get Thumbnail folder path
    * @return thumbnail folder path for this project
    */
    std::string GetThumbailFolder( void ) const ;

    /**
    * @brief Get SFM folder path
    * @return SFM folder path for this project
    */
    std::string GetSFMFolder( void ) const ;

    /**
    * @brief Get number of input images
    */
    int NbInputImage( void ) const ;

    /**
    * @brief Set current project settings
    * @param set new settings
    */
    void SetSettings( const SfMSettings & set ) ;

    /**
    * @brief Get current project settings
    * @return Current settings
    */
    SfMSettings GetSettings( void ) const ;

  private:

    /// Root project path
    std::string m_project_root_path ;
    /// SfM Data
    openMVG::sfm::SfM_Data m_sfm_data ;
    /// SfM Settings
    SfMSettings m_settings ;

    /**
    * @brief Given input images, generate thumbnails
    */
    void GenerateThumbnails( void ) ;

    /**
    * Folder
    * Folder/gui/thumbnail
    * Folder/sfm/
    * Folder/sfm/sfm_data/...
    * @retval true on success
    */
    bool BuildProjectStructure( const std::string & input_folder ) ;

    /// Root project folder
    std::string m_project_folder ;

    /// Project file
    std::string m_project_file ;

} ;
}
}

#endif