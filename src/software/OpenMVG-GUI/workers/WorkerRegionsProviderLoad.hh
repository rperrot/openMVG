#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_REGIONS_PROVIDER_LOAD_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_REGIONS_PROVIDER_LOAD_HH_

#include "Project.hh"
#include "WorkerInterface.hh"

#include <QObject>

#include <atomic>
#include <mutex>

namespace openMVG
{
namespace sfm
{
class Regions_Provider ;
}
}

namespace openMVG_gui
{

/**
* @brief Class used to load the features from disk
*/
class WorkerRegionsProviderLoad : public QObject, public WorkerInterface
{
  public:

    /**
    * @brief ctr
    */
    WorkerRegionsProviderLoad( std::shared_ptr<Project> & pro ,
                               const WorkerNextAction & na = NEXT_ACTION_NONE ) ;

    /**
    * @brief get progress range
    */
    void progressRange( int & min , int & max ) const ;

    /**
    * @brief get the region loaded (only valid after process has ended with success)
    */
    std::shared_ptr<openMVG::sfm::Regions_Provider> regionsProvider( void ) const ;

  public slots:

    /**
    * @brief Do the computation
    * @param next_action Action to execute after matching
    */
    void process( void ) ;

    /**
    * @brief internal progress bar has been incremented, now signal it to the external progress dialog
    */
    void hasIncremented( int ) ;

    /**
    * @brief set progress value to the main thread
    */
    void sendProgress( void ) ;


  signals :

    // 0 -> nothing done
    // n -> all done (n : number of match pair)
    void progress( int ) ;

    /**
    * @brief After computation of all matches (ie: signal to clear memory)
    * @param next_action Action to execute after matching done (passed to process slot)
    * @note if next_action == -1 -> something failed
    */
    void finished( const WorkerNextAction & na );

  private:

    std::shared_ptr<Project> m_project ;

    // Internal counter for the number of step already done
    std::atomic<unsigned int> m_progress_value ;
    std::mutex m_mutex ; 

    std::shared_ptr<openMVG::sfm::Regions_Provider> m_regions_provider ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif