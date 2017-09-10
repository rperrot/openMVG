#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_COLOR_COMPUTATION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_COLOR_COMPUTATION_HH_

#include "Project.hh"
#include "WorkerInterface.hh"

#include <QObject>

#include <atomic>
#include <vector>

// Frwd
namespace openMVG
{
namespace sfm
{
class SfM_Data;
} // namespace sfm
} // namespace openMVG

namespace openMVG_gui
{

/**
 * @brief worker class used to compute color on current sfm_data
 */
class WorkerColorComputation : public QObject, public WorkerInterface
{
  public:
    /**
     * @brief Ctr Constructor
     */
    WorkerColorComputation( std::shared_ptr<Project> proj );

    /**
     * @brief get progress range
     */
    void progressRange( int &min, int &max ) const;

  public slots:

    /**
     * @brief Do the computation
     */
    void process( void );

    /**
     * @brief internal progress bar has been incremented, now signal it to the external progress dialog
     */
    void hasIncremented( int );

    void sendProgress( void );

  signals:

    // 0 -> nothing done
    // n -> all done (n : number of points)
    void progress( int );

    // After computation of all features (ie: signal to clear memory)
    void finished( const WorkerNextAction &na );

  private:
    bool ColorizeTracks( const std::shared_ptr<openMVG::sfm::SfM_Data> &sfm_data, std::vector<openMVG::Vec3> &vec_3dPoints, std::vector<openMVG::Vec3> &vec_tracksColor );

    // Internal counter for the number of step already done
    std::atomic<unsigned int> m_progress_value;

    std::shared_ptr<Project> m_project;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif