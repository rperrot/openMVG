#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_CLUSTER_COMPUTATION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_CLUSTER_COMPUTATION_HH_

#include "WorkerInterface.hh"

#include "Project.hh"

#include <QObject>

#include <atomic>

namespace openMVG_gui
{

/**
 * @brief Worker compute clusters
 */
class WorkerClusterComputation : public QObject, public WorkerInterface
{
  public:
    WorkerClusterComputation( std::shared_ptr<Project> &proj, const int cluster_lower_bound, const int cluster_upper_bound, const float voxelSize = 10.0 );

    /**
     * @brief get progress range
     */
    void progressRange( int &min, int &max ) const;

  public slots:
    /**
     * @brief create the project
     */
    void process( void );

    /**
     * @brief internal progress bar has been incremented, now signal it to the external progress dialog
     */
    void hasIncremented( int );

    /**
     * @brief set progress value to the main thread
     */
    void sendProgress( void );

  signals:

    // 0 -> nothing done
    // 1 -> all done
    void progress( int );

    // After computation
    void finished( const WorkerNextAction &na );

  private:
    std::atomic<unsigned int> m_progress_value;

    std::shared_ptr<Project> m_project;
    int m_cluster_lower_bound;
    int m_cluster_upper_bound;
    float m_cluster_voxel_grid_size;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif