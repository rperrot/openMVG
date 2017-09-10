#ifndef _WORKER_EXPORT_CLUSTER_TO_MVS_HH_
#define _WORKER_EXPORT_CLUSTER_TO_MVS_HH_

#include "WorkerInterface.hh"

#include <QObject>
#include <QThread>

#include <memory>
#include <mutex>
#include <string>

namespace openMVG_gui
{

class WorkerSfMDataLoad;
class WorkerExportToMVE;
class WorkerExportToOpenMVS;
class WorkerExportToPMVS;

/**
 * @brief MVS method used to export clusters
 */
enum MVS_exporter
{
  MVS_EXPORTER_MVE,
  MVS_EXPORTER_OPENMVS,
  MVS_EXPORTER_PMVS,
};

class WorkerExportClustersToMVS : public QObject, public WorkerInterface
{
  public:
    /**
     * @brief Ctr
     * @param clusters_path Path containing the clusters
     * @param output_base_folder Base folder for exporting data
     * @param method The exporter to use
     */
    WorkerExportClustersToMVS( const std::string &clusters_path,
                               const std::string &output_base_folder,
                               const MVS_exporter &method );

    /**
     * @brief get progress range of current stage
     */
    void progressRangeCurrentStage( int &min, int &max ) const;

    /**
     * @brief get progress range overall (ie: number of stage)
     */
    void progressRangeOverall( int &min, int &max ) const;

    /**
    * @brief Get current method used for export
    */
    MVS_exporter method( void ) const ;

  signals:

    // 0 -> nothing done
    // n -> all done (n : number of step in current stage)
    void progressCurrentStage( int );

    // 0 -> nothing done
    void progressOverall( int );

    // After computation of all features (ie: signal to clear memory)
    void finished( const WorkerNextAction &na );

    // Set progress range for current stage
    void progressRangeCurrentStage( int min, int max );

    // Set message of current stage
    void messageCurrentStage( const std::string &msg );

  public slots:

    /**
     * @brief Do the computation
     */
    void process( void );

  private slots:

    /**
     * @brief internal progress bar has been incremented, now signal it to the external progress dialog
     */
    void hasIncrementedCurrentStage( int );

    /**
     * @brief
     */
    void hasIncrementedStage( void );

    /**
     * @brief set progress value to the main thread
     */
    void sendProgressCurrentStage( void );

    /**
     * @brief Set progress value to the main thread
     */
    void sendProgressOverall( void );

    /**
     * @brief Action to be executed when a sfm data has been loaded
     */
    void hasLoadedClusterData( void );

    /**
     * @brief Action to be executed when a mvs export has been done
     */
    void hasExportedToMVS( void );

  private:

    /**
    * @brief Launch thread to compute current cluster export
    */
    void processLoadingCurrentCluster( void ) ;

    // Internal counter for the number of step already done
    std::atomic<unsigned int> m_progress_value;
    std::atomic<unsigned int> m_progress_overall;

    // For each clusters get it's path
    std::vector<std::string> m_clusters_path;
    std::string m_output_path;
    MVS_exporter m_method;

    // The workers
    std::shared_ptr<WorkerSfMDataLoad> m_worker_sfm_data_load;
    std::shared_ptr<WorkerExportToMVE> m_worker_export_to_MVE;
    std::shared_ptr<WorkerExportToOpenMVS> m_worker_export_to_OpenMVS;
    std::shared_ptr<WorkerExportToPMVS> m_worker_export_to_PMVS;

    // The thread
    QThread *m_thread_sfm_data_load;
    QThread *m_thread_mvs_exporter;

    // The mutex (use to keep the worker alive during computation of it's childs)
    std::mutex m_mutex ;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif