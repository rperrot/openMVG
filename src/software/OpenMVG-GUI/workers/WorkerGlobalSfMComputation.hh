#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_GLOBAL_SFM_COMPUTATION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_GLOBAL_SFM_COMPUTATION_HH_

#include "Project.hh"
#include "WorkerInterface.hh"

#include <QObject>

namespace openMVG_gui
{

/**
 * @brief Worker used to compute global SfM
 */
class WorkerGlobalSfMComputation : public QObject, public WorkerInterface
{
  public:
    /**
     * @brief Ctr
     * @param project The global project
     * @param features_provider The features
     * @param matches_provider The matches
     * @param reload_initial_intrinsics Decide (true) to restart from a clean sfm_data (from matches dir) or reuse already refined intrinsics (false)
     * @param na Next action to execute after process
     */
    WorkerGlobalSfMComputation( std::shared_ptr<Project> project, std::shared_ptr<openMVG::sfm::Features_Provider> &features_provider, std::shared_ptr<openMVG::sfm::Matches_Provider> &matches_provider, const bool reload_initial_intrinsics = true, const WorkerNextAction &na = NEXT_ACTION_NONE );

    /**
     * @brief get progress range
     */
    void progressRange( int &min, int &max ) const;

  public slots:

    /**
     * @brief Do the computation
     */
    void process( void );

  signals:

    // 0 -> nothing done
    // n -> all done (n : number of match pair)
    void progress( int );

    // After computation of all matches (ie: signal to clear memory)
    void finished( const WorkerNextAction &na );

  private:
    std::shared_ptr<Project> m_project;
    std::shared_ptr<openMVG::sfm::Features_Provider> m_feature_provider;
    std::shared_ptr<openMVG::sfm::Matches_Provider> m_matches_provider;

    bool m_reload_initial_intrinsics;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif