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
    bool OpenImageFolder( const std::string & filename ,
                          const std::string & sensor_width_database_path ) ;

    /**
    * @brief Test if
    */
    bool HasUnsavedChanges( void ) const ;

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
    * @return Number of input image
    */
    unsigned int NbInputImage( void ) const ;

    /**
    * @brief Get Full image path for a given id
    * @param id Id of the image to get
    * @retval empty string if id is out of range
    * @retval full path for a given image id
    */
    std::string FullImagePath( const unsigned int id ) const ;

    /**
    * @brief Get thumbnail path for a given id
    * @param id If of the image to get
    * @retval empty string if id is out of range
    * @retval thumbnail path for a given image id
    */
    std::string ThumbnailPath( const unsigned int id ) const ;

    /**
    * @brief Get Local name of the image
    * @param id Id of the image to get
    * @retval empty string if id is out of range
    * @retval local name of the given image id
    */
    std::string ImageName( const unsigned int id ) const ;

    /**
    * @brief Get image width
    * @param id Id of the image to get
    * @retval -1 if id is out of range
    * @retval Width (in pixel) of image
    */
    int ImageWidth( const unsigned int id ) const ;

    /**
    * @brief Get image height
    * @param id Id of the image to get
    * @retval -1 if id is out of range
    * @retval Height (in pixel) of image
    */
    int ImageHeight( const unsigned int id ) const ;

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

    /**
    * @brief Compute SfM
    */
    void ComputeSfM( void ) ;

    /**
    * @brief Export to MVE subdirectory
    */
    void ExportToMVE( void ) ;

  private:

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

    bool ValidProjectStructure( const std::string & input_folder ) ;



    /// Root project path
    std::string m_project_root_path ;
    /// SfM Data
    openMVG::sfm::SfM_Data m_sfm_data ;
    /// SfM Settings
    SfMSettings m_settings ;
    /// Root project folder
    std::string m_project_folder ;
    /// Map between full image name to thumbail name
    std::map< std::string , std::string > m_map_image_to_thumbnail ;
    /// test if something has been change since last save
    bool m_has_unsaved_changes ;
} ;
}
}

#endif