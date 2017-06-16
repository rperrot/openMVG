#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_PROJECT_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_PROJECT_HH_

#include "workflow_params/FeatureParams.hh"
#include "workflow_params/GlobalSfMParams.hh"
#include "workflow_params/IncrementalSfMParams.hh"
#include "workflow_params/IntrinsicParams.hh"
#include "workflow_params/MatchingParams.hh"

#include "graphics/SceneManager.hh"

#include "third_party/progress/progress.hpp"

#include <string>

// fwrd declaration
namespace openMVG
{
namespace sfm
{
class SfM_Data ;
}
}

namespace openMVG_gui
{

enum SfMMethod
{
  SFM_METHOD_INCREMENTAL ,
  SFM_METHOD_GLOBAL
} ;

/**
* @brief This class manage a whole openMVG project
*/
class Project
{
  public:

    /**
    * @brief Constructor from an existing project
    * @param projectFile The path of the project file
    */
    Project( const std::string & projectFile ,
             std::shared_ptr<SceneManager> mgr ) ;

    /**
    * @brief Constructor for a new project
    * @param base_path The path of the project
    * @param img_dir The path containing images of the project
    * @param intrin_params Intrinsic params used for the initialisation
    * @param camera_sensor_width_database_file File containing the sensor database
    * @todo : need to add a third parameter for intrinsic data ?
    */
    Project( const std::string & base_path ,
             const std::string & image_path ,
             const IntrinsicParams & intrin_params ,
             const std::string camera_sensor_width_database_file ,
             std::shared_ptr<SceneManager> mgr ,
             C_Progress * progress = nullptr ) ;

    /**
    * @brief The number of image in the project
    */
    size_t nbImage( void ) const ;

    /**
    * @brief Save project in base path
    */
    void save( void ) ;

    /**
    * @brief Open project file and associated data
    * @param project File Project file
    */
    void open( const std::string & projectFile ) ;


    /**
    * @brief Get path of a given image
    * @param id_image Id of the image to get
    * @node indice start at 0
    * @note if id_image >= nbImage() return an empty string
    * @return image path
    */
    std::string getImagePath( const size_t id_image ) const ;

    /**
    * @brief Get mask image path of a specified image
    * @param id_image Id of the image to get
    * @return mask image path
    */
    std::string getMaskImagePath( const size_t id_image ) const ;

    /**
    * @brief Indicate if all image have features computed
    */
    bool hasAllFeaturesComputed( void ) const ;

    /**
    * @brief Indicate if some of the images have features computed (but not all)
    * @retval true if nb image with features > 0 && nb image with features < nb images
    * @retval false if no image have features computed or if all images have features computed
    */
    bool hasPartialFeaturesComputed( void ) const ;

    /**
    * @brief Indicate if all matches are computed
    */
    bool hasAllMatchesComputed( void ) const ;

    /**
    * @brief Indicate if SfM is computed
    */
    bool hasSfMComputed( void ) const ;

    /**
    * @brief Indicate if Color is computed
    */
    bool hasColorComputed( void ) const ;

    /**
    * @brief Indicate if matches.f.bin is present
    * @retval true if file exists
    * @retval false if file does not exist
    */
    bool hasMatchesFundamentalFiltered( void ) const ;

    /**
    * @brief Indicate if matches.e.bin is present
    * @retval true if file exists
    * @retval false if file does not exist
    */
    bool hasMatchesEssentialFiltered( void ) const ;

    /**
    * @brief Indicate if matches.h.bin is present
    * @param true if file exists
    * @retval false if file does not exist
    */
    bool hasMatchesHomographyFiltered( void ) const ;

    /**
    * @brief Get feature parameters
    * @return current feature computation parameters
    */
    FeatureParams featureParams( void ) const ;

    /**
    * @brief Set feature computation parameters
    * @param f_params New params
    */
    void setFeatureParams( const FeatureParams & f_params ) ;

    /**
    * @brief get matching parameters
    * @return curent matching parameters
    */
    MatchingParams& matchingParams( void ) ;

    /**
    * @brief get matching parameters
    * @return curent matching parameters
    */
    MatchingParams matchingParams( void ) const ;

    /**
    * @brief Set Matching parameters
    * @param m_params New params
    */
    void setMatchingParams( const MatchingParams & m_params ) ;

    /**
    * @brief Get incremental SfM Parameters
    * @return incremental SfM parameters
    */
    IncrementalSfMParams incrementalSfMParams( void ) const ;

    /**
    * @brief Set incremental SfM Parameters
    * @param i_sfm_params New params
    */
    void setIncrementalSfMParams( const IncrementalSfMParams & i_sfm_params ) ;

    /**
    * @brief Get global SfM Parameters
    * @return incremental SfM params
    */
    GlobalSfMParams globalSfMParams( void ) const ;

    /**
    * @brief Set global SfM Parameters
    * @param g_sfm_params New params
    */
    void setGlobalSfMParams( const GlobalSfMParams & g_sfm_params ) ;

    /**
    * @brief Get images IDs
    * @return get current image ids
    */
    std::vector< int > imageIds( void ) const ;

    /**
    * @brief Given a base path, get thumbnail base path
    * @param return thumbnail base path
    */
    std::string thumbnailsPath( void ) const ;

    /**
    * @brief Given a base path, get matches base path
    * @param return matches base path
    */
    std::string matchesPath( void ) const ;


    /**
    * @brief Given a base path, get global reconstruction path
    * @return reconstruction global path
    */
    std::string reconstructionGlobalPath( void ) const ;

    /**
    * @brief Given a base path, get sequential reconstruction path
    * @return reconstruction sequential path
    */
    std::string reconstructionSequentialPath( void ) const ;

    /**
    * @brief Get path of the sfm data output ply file
    */
    std::string sfMDataPlyPath( void ) const ;

    /**
    * @brief Get colorized sfm data output ply file
    */
    std::string colorizedSfMPlyPath( void ) const ;

    /**
    * @brief get access to the sfm data structure
    * @return The sfm_data structure
    */
    std::shared_ptr<openMVG::sfm::SfM_Data> SfMData( void ) const ;

    /**
    * @brief Get list of image names
    * @note this is only the base name without the full path
    * @return id of the image, filename
    */
    std::vector< std::pair< int , std::string > > GetImageNames( void ) const ;

    /**
    * @brief Get current SfM method to use
    * return current sfm method to use
    */
    SfMMethod sfMMethod( void ) const ;

    /**
    * @brief Set current SfM method to use
    * @param meth new sfm method to use
    */
    void setSfMMethod( const SfMMethod & meth ) ;

    /**
    * @brief get 3d scene manager
    * @return scene manager
    */
    std::shared_ptr<SceneManager> sceneManager( void ) const ;

    /**
    * @brief The sparse point cloud associated with the scene
    * @return the current sparse point cloud
    * @note can be nullptr
    */
    std::shared_ptr<RenderableObject> sparsePointCloud( void ) const ;

    /**
    * @brief Set the current sparse point cloud
    * @param obj New sparse point cloud
    */
    void setSparsePointCloud( std::shared_ptr<RenderableObject> obj ) ;

    /**
    * @brief Indicate if some parameters have changed since the last save on disk
    * @retval true Something has changed since the last save
    * @retval false Everything is up to date wrt to the project file
    */
    bool hasUnsavedChange( void ) const ;

    /**
    * @brief Indicate if mask is enabled for a specified image 
    * @param id Id of the image 
    */
    bool maskEnabled( const int id ) const ; 

    /**
    * @brief Enable/disable mask on selected id
    */
    void setMaskEnabled( const int id , const bool value ) ;

  private:

    /**
    * @brief Create project (structure and SfM_data)
    * @param base_path The path of the project
    * @param img_dir The path containing images of the project
    * @param intrin_params Intrinsic params used for the initialisation
    * @param camera_sensor_width_database_file File containing the sensor database
    * @todo : need to add a third parameter for intrinsic data ?
    */
    void createProject( const std::string & base_path ,
                        const std::string & image_path ,
                        const IntrinsicParams & intrin_params ,
                        const std::string camera_sensor_width_database_file ,
                        C_Progress * progres = nullptr ) ;
    /**
    * @brief Create directory structure for a new project
    * @param base_path base directory
    * @retval true if creation is OK
    * @retval false if folder creation failed
    */
    bool createDirectoryStructure( const std::string & base_path ) ;

    /**
    * @brief Given a base path, get gui base path
    * @param base_path project base path
    * @param return thumbnail base path
    */
    std::string guiPath( const std::string & base_path ) const ;

    /**
    * @brief Given a base path, get sfm base path
    * @param base_path project base path
    * @param return thumbnail base path
    */
    std::string sfmPath( const std::string & base_path ) const ;


    /**
    * @brief Given a base path, get thumbnail base path
    * @param base_path project base path
    * @param return thumbnail base path
    */
    std::string thumbnailsPath( const std::string & base_path ) const ;

    /**
    * @brief Given a base path, get matches base path
    * @param base_path project base path
    * @param return matches base path
    */
    std::string matchesPath( const std::string & base_path ) const ;

    /**
    * @brief Given a base path, get global reconstruction path
    * @param base_path project base path
    * @return reconstruction global path
    */
    std::string reconstructionGlobalPath( const std::string & base_path ) const ;

    /**
    * @brief Given a base path, get sequential reconstruction path
    * @param base_path project base path
    * @return reconstruction sequential path
    */
    std::string reconstructionSequentialPath( const std::string & base_path ) const ;

    /// Indicate if the project on disk is the last one (ie: save is up to date)
    bool m_saved ;

    /// The base path where the project reside
    std::string m_project_base_path ;
    /// The base path where the image of the project reside
    std::string m_project_image_path ;

    /// Features params used to compute features
    FeatureParams m_feature_params ;
    /// Parameters used to compute matches
    MatchingParams m_match_params ;
    /// Parameters used to compute incremental SfM
    IncrementalSfMParams m_incremental_sfm_params ;
    /// Parameters used to compute global SfM
    GlobalSfMParams m_global_sfm_params ;
    /// Sfm method to use
    enum SfMMethod m_sfm_method ;

    // The sfm data file
    std::shared_ptr<openMVG::sfm::SfM_Data> m_sfm_data ;

    // Indicate if mask is enabled
    std::map< int , bool > m_mask_enabled ;

    // The 3d scene
    std::shared_ptr<SceneManager> m_scene_mgr ;
    std::shared_ptr<RenderableObject> m_sparse_point_cloud ;
} ;

} // namespace openMVG_gui

#endif