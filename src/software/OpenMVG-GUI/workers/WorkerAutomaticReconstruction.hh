#ifndef _WORKERS_WORKER_AUTOMATIC_RECONSTRUCTION_HH_
#define _WORKERS_WORKER_AUTOMATIC_RECONSTRUCTION_HH_

#include "WorkerInterface.hh"

#include "AutomaticReconstructionDialog.hh"
#include "Project.hh"

#include <QObject>
#include <QThread>

#include <mutex>
#include <string>

namespace openMVG_gui
{

class WorkerColorComputation ;
class WorkerFeaturesComputation ;
class WorkerFeaturesProviderLoad ;
class WorkerGeometricFiltering ;
class WorkerIncrementalSfMComputation ;
class WorkerMatchesComputation ;
class WorkerMatchesProviderLoad ;
class WorkerProjectCreation ;
class WorkerRegionsProviderLoad ;
class WorkerThumbnailGeneration ;

/**
* @brief Worker that do all : from project creation to colorization of sfm point cloud
*/
class WorkerAutomaticReconstruction : public QObject , public WorkerInterface
{
  public:

    /**
    * @brief Ctr
    * @param input_folder Input (image) folder
    * @param output_folder Output (project) folder
    * @param preset Preset to set
    */
    WorkerAutomaticReconstruction( const std::string & input_folder ,
                                   const std::string & output_folder ,
                                   const AutomaticReconstructionPreset & preset ,
                                   std::shared_ptr<SceneManager> & scn_mgr ,
                                   const WorkerNextAction & na = NEXT_ACTION_NONE ) ;

    /**
    * @brief Dtr
    */
    ~WorkerAutomaticReconstruction() ;

    /**
    * @brief get progress range of current stage
    */
    void progressRangeCurrentStage( int & min , int & max ) const ;

    /**
    * @brief get progress range overall (ie: number of stage)
    */
    void progressRangeOverall( int & min , int & max ) const ;

    /**
    * @brief Get computed project
    * @return The project
    */
    std::shared_ptr<Project> project( void ) const ;

  signals :

    // 0 -> nothing done
    // n -> all done (n : number of step in current stage)
    void progressCurrentStage( int ) ;

    // 0 -> nothing done
    void progressOverall( int ) ;

    // After computation of all features (ie: signal to clear memory)
    void finished( const WorkerNextAction & na );

    // Set progress range for current stage
    void progressRangeCurrentStage( int min , int max ) ;

    // Set message of current stage
    void messageCurrentStage( const std::string & msg ) ;

  public slots:

    /**
    * @brief Do the computation
    */
    void process( void ) ;

  private slots:

    /**
    * @brief internal progress bar has been incremented, now signal it to the external progress dialog
    */
    void hasIncrementedCurrentStage( int ) ;

    /**
    * @brief
    */
    void hasIncrementedStage( void ) ;

    /**
    * @brief set progress value to the main thread
    */
    void sendProgressCurrentStage( void ) ;

    /**
    * @brief Set progress value to the main thread
    */
    void sendProgressOverall( void ) ;

    // end of the computation
    void hasDoneProjectCreation(  ) ;
    void hasDoneThumbnailCreation(  ) ;
    void hasDoneFeatureComputation(  ) ;
    void hasDoneRegionProviderLoad(  ) ;
    void hasDoneFeatureMatching(  ) ;
    void hasDoneGeometricFiltering(  ) ;
    void hasDoneFeatureProviderLoad(  ) ;
    void hasDoneMatchesProviderLoad(  ) ;
    void hasDoneSfMReconstruction(  ) ;
    void hasDoneColorization(  ) ;


  private:

    // begining of the computation
    void doProjectCreation( void ) ;
    void doThumbnailCreation( void ) ;
    void doFeatureComputation( void ) ;
    void doFeatureMatching( void ) ;
    void doSfMReconstruction( void ) ;
    void doColorization( void ) ;


    // Internal counter for the number of step already done
    std::atomic<unsigned int> m_progress_value ;
    std::atomic<unsigned int> m_progress_overall ;

    // Variable used to count the number of step
    int m_nb_images ;  // Number of input images
    int m_nb_matches ; // Number of matches (ie: number of pair of images) to compute

    std::shared_ptr<SceneManager> m_scn_mgr ;

    std::string m_input_image_folder ;
    std::string m_output_project_folder ;

    AutomaticReconstructionPreset m_preset ;
    std::shared_ptr<Project> m_project ;

    // The workers
    WorkerColorComputation          * m_worker_color_computation ;
    WorkerFeaturesComputation       * m_worker_feature_computation ;
    WorkerFeaturesProviderLoad      * m_worker_feature_provider_load ;
    WorkerGeometricFiltering        * m_worker_geometric_filtering ;
    WorkerIncrementalSfMComputation * m_worker_incremental_sfm_computation ;
    WorkerMatchesComputation        * m_worker_matches_computation ;
    WorkerMatchesProviderLoad       * m_worker_matches_provider_load ;
    WorkerProjectCreation           * m_worker_project_creation ;
    WorkerRegionsProviderLoad       * m_worker_regions_provider_load ;
    WorkerThumbnailGeneration       * m_worker_thumbnail_generation ;

    QThread * m_threadColorComputation ;
    QThread * m_threadFeatureComputation ;
    QThread * m_threadFeatureProviderLoad ; 
    QThread * m_threadGeometricFiltering ; 
    QThread * m_threadIncrementalSfMComputation ; 
    QThread * m_threadMatchesComputation ; 
    QThread * m_threadMatchesProviderLoad ; 
    QThread * m_threadProjectCreation ; 
    QThread * m_threadRegionsProviderLoad ; 
    QThread * m_threadThumbnailGeneration ;

    std::mutex m_mutex ;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif