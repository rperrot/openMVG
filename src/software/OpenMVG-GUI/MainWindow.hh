#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_MAINWINDOW_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_MAINWINDOW_HH_

#include "ApplicationState.hh"
#include "DetailListWidget.hh"
#include "ImageListWidget.hh"
#include "Project.hh"
#include "ResultViewWidget.hh"

// TODO : make forward declaration for all workers
#include "workers/WorkerAutomaticReconstruction.hh"
#include "workers/WorkerClusterComputation.hh"
#include "workers/WorkerColorComputation.hh"
#include "workers/WorkerExportClustersToMVS.hh"
#include "workers/WorkerExportToMVE.hh"
#include "workers/WorkerExportToOpenMVS.hh"
#include "workers/WorkerExportToPMVS.hh"
#include "workers/WorkerFeaturesComputation.hh"
#include "workers/WorkerFeaturesProviderLoad.hh"
#include "workers/WorkerGeometricFiltering.hh"
#include "workers/WorkerGlobalSfMComputation.hh"
#include "workers/WorkerIncrementalSfMComputation.hh"
#include "workers/WorkerMatchesComputation.hh"
#include "workers/WorkerMatchesProviderLoad.hh"
#include "workers/WorkerProjectCreation.hh"
#include "workers/WorkerRegionsProviderLoad.hh"
#include "workers/WorkerThumbnailGeneration.hh"

#include "workers/WorkerNextAction.hh"

#include "DoubleProgressBarDialog.hh"

#include <QAction>
#include <QCheckBox>
#include <QMainWindow>
#include <QMenu>
#include <QProgressDialog>
#include <QSlider>

namespace openMVG_gui
{


/**
* @brief Main application window
* this is the main entry point of the project
*/
class MainWindow : public QMainWindow
{
  public:

    /**
    * @brief Main window
    */
    MainWindow() ;

  public slots:

    /**
    * @brief Action to be executed when user wants to create a new project
    */
    void onNewProject( void ) ;

    /**
    * @brief Action to be executed when user wants to open a project
    */
    void onOpenProject( void ) ;

    /**
    * @brief Action to be executed when user wants to save a project
    */
    void onSaveProject( void ) ;

    /**
    * @brief Action to be executed when user wants to save a project to another folder
    */
    void onSaveAsProject( void ) ;

    /**
    * @brief Action to be executed when user wants to close current project
    */
    void onCloseProject( void ) ;

    /**
    * @brief Action to be executed when user wants to quit the project
    */
    void onQuit( void ) ;

    /**
    * @brief Action to be executed when user wants to compute automatic reconstruction
    */
    void onComputeAutomaticReconstruction( void ) ;

    /**
    * @brief Action to be executed when user wants to compute features
    */
    void onComputeFeatures( void ) ;

    /**
    * @brief Action to be executed when user wants to compute matches
    */
    void onComputeMatches( void ) ;

    /**
    * @brief Action to be executed when user wants to compute structure
    */
    void onComputeSfM( void ) ;

    /**
    * @brief Action to be executed when user wants to compute color
    */
    void onComputeColor( void ) ;


    /**
    * @brief Action to be executed when user wants to change features computation settings
    */
    void onChangeFeatureSettings( void ) ;

    /**
    * @brief Action to be executed when user wants to change matching computation settings
    */
    void onChangeMatchesSettings( void ) ;

    /**
    * @brief Action to be executed when user wants to change
    */
    void onChangeSfMSettings( void ) ;

    /**
    * @brief Action to be executed when user wants to update image list widget
    */
    void onUpdateImageList( void ) ;

    /**
    * @brief Action to be executed when user wants to show/hide the grid
    */
    void onShowHideGrid( void ) ;

    /**
    * @brief Action to be executed when user wants to show/hide the camera gizmos
    */
    void onShowHideCameraGizmos( void ) ;

    /**
    * @brief Action to be executed when user wants to show/hide image list
    */
    void onShowImageList( void ) ;

    /**
    * @brief Action to be exected when user wants to show/hide detail list
    */
    void onShowHideDetail( void ) ;

    /**
    * @brief Action to be executed when user has selected an image in the image list
    * @param id Id of the selected image
    */
    void onSelectImage( int id ) ;


    /**
    * @brief Action to be executed when user want to define mask of an image
    * @param id Id of the image for which mask would be defined
    */
    void onDefineMask( int id ) ;

    /**
    * @brief Action to be executed when user select perspective projection matrix
    */
    void onSetPerspectiveProjection( void ) ;

    /**
    * @brief Action to be executed when user select orthographic projection matrix
    */
    void onSetOrthographicProjection( void ) ;

    /**
    * @brief Action to be executed when user want to compute clusters
    */
    void onComputeClusters( void ) ;

    /**
    * @brief Action to be executed when user want to export to openMVS
    */
    void onExportToOpenMVS( void ) ;

    /**
    * @brief Action to be executed when user want to export to MVE
    */
    void onExportToMVE( void ) ;

    /**
    * @brief Action to be executed when user want to export to PMVS
    */
    void onExportToPMVS( void ) ;

    /**
    * @brief Action to be executed when user want to export each clusters to openMVS
    */
    void onExportClustersToOpenMVS( void ) ;

    /**
    * @brief Action to be executed when user want to export each clusters to MVE
    */
    void onExportClustersToMVE( void ) ;

    /**
    * @brief Action to be executed when user want to export each clusters to PMVS
    */
    void onExportClustersToPMVS( void ) ;

    // After something happened
    /**
    * @brief Action to be executed when a project has been created
    * -> Prepare interface (generate thumbnails, ...)
    */
    void onHasCreatedProject( const WorkerNextAction & next_action ) ;

    /**
    * @brief Action to be executed when features have been computed
    */
    void onHasComputedFeatures( const WorkerNextAction & next_action  ) ;

    /**
    * @brief Action to be executed when features_provider has been loaded
    * @note this is before incremental SfM
    */
    void onHasLoadedFeatures( const WorkerNextAction & next_action ) ;

    /**
    * @brief Action to be executed when matches_provider has been loaded
    */
    void onHasLoadedMatches( const WorkerNextAction & next_action  ) ;

    /**
    * @brief Action to be executed when regions has been loaded
    */
    void onHasLoadedRegions( const WorkerNextAction & next_action  );

    /**
    * @brief Action to be executed when matches have been computed
    */
    void onHasComputedMatches( const WorkerNextAction & next_action  ) ;

    /**
    * @brief Action to be executed when geometric filtering has been computed
    */
    void onHasDoneGeometricFiltering( const WorkerNextAction & next_action  ) ;

    /**
    * @brief Action to be executed when SfM has been computed
    */
    void onHasComputedSfM( const WorkerNextAction & next_action  ) ;

    /**
    * @brief Action to be executed when color of SfmData has been computed
    */
    void onHasComputedColor( const WorkerNextAction & next_action  ) ;

    /**
    * @brief Action to be executed when automatic reconstruction is done
    */
    void onHasDoneAutomaticReconstruction( const WorkerNextAction & next_action ) ;

    /**
    * @brief Action to be executed when clustering has been computed
    */
    void onHasComputedClustering( const WorkerNextAction & next_action ) ;

    /**
    * @brief Action to be executed when exporting to openMVS has been computed
    */
    void onHasExportedToOpenMVS( const WorkerNextAction & next_action ) ;

    /**
    * @brief Action to be executed when exporting to MVE has been done
    */
    void onHasExportedToMVE( const WorkerNextAction & next_action ) ;

    /**
    * @brief Action to be executed when exporting to PMVS has been done
    */
    void onHasExportedToPMVS( const WorkerNextAction & next_action ) ;

    /**
    * @brief Action to be executed when exporting to PMVS has been done
    */
    void onHasExportedClustersToMVS( const WorkerNextAction & next_action ) ;

    /**
    * @brief indicate if some parameters in the project are not saved on disk
    */
    bool hasUnsavedChange( void ) const ;



  private:

    /**
    * @brief Post actions to be executed after feature computation (or feature computation failure)
    */
    void postFeaturesComputation( void ) ;

    /**
    * @brief Post actions to be executed after matches computation (or matches computation failure)
    */
    void postMatchesComputation( void ) ;

    /**
    * @brief Post actions to be executed after sfm computation (or sfm computation failure)
    */
    void postSfMComputation( void ) ;

    /**
    * @brief Post actions to be executed after color computation (or color computation failure)
    */
    void postColorComputation( void ) ;


    /**
    * @brief Set the interface exactly the same as after launching the application
    * -> make the interface into a clean step
    */
    void resetInterface( void ) ;

    /**
    * @brief Update interface based on the state of the application
    */
    void updateInterface( void ) ;

    /**
    * @brief Build interface elements
    */
    void buildInterface( void ) ;

    /**
    * @brief Build Menus items
    */
    void buildMenus( void ) ;

    /**
    * @brief Build the toolbar
    */
    void buildToolbar( void ) ;

    /**
    * @brief Make connections between elements and slots
    */
    void makeConnections( void ) ;

    void createProgress( const std::string &message , const int minvalue , const int maxvalue ) ;

    /// File menu
    QMenu * m_file_menu ;
    /// New project
    QAction * m_file_new_act ;
    QAction * m_file_new_act_tb ;
    /// Open project
    QAction * m_file_open_act ;
    QAction * m_file_open_act_tb ;
    /// Save project
    QAction * m_file_save_act ;
    QAction * m_file_save_act_tb ;
    /// Save as (at another location)
    QAction * m_file_save_as_act ;
    /// Close project
    QAction * m_file_close_act ;
    /// Quit the application
    QAction * m_file_quit_act ;
    /// Workflow
    QMenu * m_workflow_menu ;
    /// Full workflow
    QAction * m_automatic_workflow_act ;
    /// Compute features
    QAction * m_compute_features_act ;
    QAction * m_compute_features_act_tb ;
    /// Compute matches
    QAction * m_compute_matches_act ;
    QAction * m_compute_matches_act_tb ;
    /// Compute structure from motion (incr/sequ)
    QAction * m_compute_sfm_act ;
    QAction * m_compute_sfm_act_tb ;
    /// Compute color of the SfM structure
    QAction * m_compute_color_act ;
    QAction * m_compute_color_act_tb ;
    /// Export to clusters
    QAction * m_export_to_clusters_act ;
    /// Export to openMVS
    QAction * m_export_to_openMVS_act ;
    /// Export to MVE
    QAction * m_export_to_MVE_act ;
    /// Export to PMVS
    QAction * m_export_to_PMVS_act ;
    /// Clustered outputs
    QMenu * m_clustered_exports_menu ;
    /// Export to clusters + openMVS
    QAction * m_export_to_clusters_openMVS_act ;
    /// Export to clusters + MVE
    QAction * m_export_to_clusters_MVE_act ;
    /// Export to clusters + PMVS
    QAction * m_export_to_clusters_PMVS_act ;

    /// Settings
    QMenu * m_settings_menu ;
    QAction * m_setting_features_act ;
    QAction * m_setting_matches_act ;
    QAction * m_setting_sfm_act ;
    /// View
    QMenu * m_view_menu ;
    QAction * m_show_hide_grid_act ;
    QAction * m_show_hide_camera_gizmos_act ;
    QAction * m_show_hide_image_list_act ;
    QAction * m_show_hide_detail_list_act ;
    QMenu * m_view_projection_menu ;
    QAction * m_view_projection_perspective ;
    QAction * m_view_projection_orthographic ;

    /// The image listing widget
    ImageListWidget * m_image_list ;
    /// The widget that contains all details
    DetailListWidget * m_detail_list ;

    /// The result (3d view ?)
    ResultViewWidget * m_result_view ;

    ApplicationState m_state ;

    /// The project
    std::shared_ptr<Project> m_project ;

    /// The thread workers
    QProgressDialog                 * m_progress_dialog ;
    DoubleProgressBarDialog         * m_double_progress_dialog ;
    std::shared_ptr<WorkerProjectCreation>           m_worker_project_creation ;
    std::shared_ptr<WorkerThumbnailGeneration>       m_worker_thumbnail_generation ;
    std::shared_ptr<WorkerFeaturesComputation>       m_worker_features_computation ;
    std::shared_ptr<WorkerMatchesComputation>        m_worker_matches_computation ;
    std::shared_ptr<WorkerRegionsProviderLoad>       m_worker_regions_provide_load ;
    std::shared_ptr<WorkerGeometricFiltering>        m_worker_geometric_filtering ;
    std::shared_ptr<WorkerFeaturesProviderLoad>      m_worker_features_provider_load ;
    std::shared_ptr<WorkerMatchesProviderLoad>       m_worker_matches_provider_load ;
    std::shared_ptr<WorkerIncrementalSfMComputation> m_worker_incremental_sfm_computation ;
    std::shared_ptr<WorkerGlobalSfMComputation>      m_worker_global_sfm_computation ;
    std::shared_ptr<WorkerColorComputation>          m_worker_color_computation ;
    std::shared_ptr<WorkerAutomaticReconstruction>   m_worker_automatic_reconstruction ;
    std::shared_ptr<WorkerClusterComputation>        m_worker_cluster_computation ;
    std::shared_ptr<WorkerExportToOpenMVS>           m_worker_export_to_openMVS ;
    std::shared_ptr<WorkerExportToMVE>               m_worker_export_to_MVE ;
    std::shared_ptr<WorkerExportToPMVS>              m_worker_export_to_PMVS ;
    std::shared_ptr<WorkerExportClustersToMVS>       m_worker_export_clusters_to_MVS ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif