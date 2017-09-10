#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_FEATURES_PROVIDER_LOAD_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_FEATURES_PROVIDER_LOAD_HH_

#include "Project.hh"
#include "WorkerInterface.hh"

#include <QObject>

// Fwrd decl
namespace openMVG
{
namespace sfm
{
struct Features_Provider;
} // namespace sfm
} // namespace openMVG

namespace openMVG_gui
{

/**
 * @brief Worker used to load features
 * @todo : provide a finer feedback by using a splitted progress
 */
class WorkerFeaturesProviderLoad : public QObject, public WorkerInterface
{
  public:
    /**
     * @param pro Project
     */
    WorkerFeaturesProviderLoad( std::shared_ptr<Project> &pro, const WorkerNextAction &na = NEXT_ACTION_NONE );

    /**
     * @brief get progress range
     */
    void progressRange( int &min, int &max ) const;

    /**
     * @brief Get features provider
     * @note only valid after success of progress function
     */
    std::shared_ptr<openMVG::sfm::Features_Provider> featuresProvider( void ) const;

  public slots:

    /**
     * @brief Do the computation
     */
    void process( void );

  signals:

    // 0 -> nothing done
    // 1 -> all done
    void progress( int );

    // After computation of all matches (ie: signal to clear memory)
    void finished( const WorkerNextAction &na );

  private:
    /// The project
    std::shared_ptr<Project> m_project;
    /// The output (note: only valid after process finished with success )
    std::shared_ptr<openMVG::sfm::Features_Provider> m_feature_provider;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif