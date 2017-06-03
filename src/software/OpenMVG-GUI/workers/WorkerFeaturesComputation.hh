#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_FEATURES_COMPUTATION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_FEATURES_COMPUTATION_HH_

#include "Project.hh"
#include "WorkerInterface.hh"

#include <QObject>

namespace openMVG_gui
{

/**
* @brief Worker that compute features on images
* @TODO : need a way to compute only certain features (pass a vector of ID ?)
*/
class WorkerFeaturesComputation : public QObject , public WorkerInterface
{
  public:
    WorkerFeaturesComputation( std::shared_ptr<Project> & pro ,
                               const bool overwrite_existing ,
                               const WorkerNextAction & na = NEXT_ACTION_NONE );

    /**
    * @brief get progress range
    */
    void progressRange( int & min , int & max ) const ;

  public slots:

    /**
    * @brief Do the computation
    */
    void process( void ) ;

  signals :

    // 0 -> nothing done
    // n -> all done (n : number of images)
    void progress( int ) ;

    // After computation of all features (ie: signal to clear memory)
    void finished( const WorkerNextAction & na );

  private:

    bool m_overwrite_existing ;
    std::shared_ptr<Project> m_project ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif