#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_MATCHES_COMPUTATION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_MATCHES_COMPUTATION_HH_

#include "Project.hh"
#include "WorkerInterface.hh"


#include <QObject>

#include <atomic>
#include <mutex>

namespace openMVG
{
namespace matching
{
struct PairWiseMatches ;
} // namespace matching
namespace sfm
{
class Regions_Provider ;
} // namespace sfm
} // namespace openMVG

namespace openMVG_gui
{

/**
* @brief Worker used to compute matches between image pairs
*/
class WorkerMatchesComputation : public QObject, public WorkerInterface
{
  public:

    /**
    * @brief Ctr
    * @param proj Project
    * @param regions_provider Features
    */
    WorkerMatchesComputation( std::shared_ptr<Project> & proj ,
                              std::shared_ptr<openMVG::sfm::Regions_Provider> &regions_provider ,
                              const WorkerNextAction & na = NEXT_ACTION_NONE ) ;

    /**
    * @brief get progress range
    */
    void progressRange( int & min , int & max ) const ;

    /**
    * @brief get putatives matches after blind matching
    * @note only valid after success of process
    */
    std::shared_ptr<openMVG::matching::PairWiseMatches> putativeMatches( void ) const ;

  public slots:

    /**
    * @brief Do the computation
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

    // After computation of all matches (ie: signal to clear memory)
    void finished( const WorkerNextAction & na );

  private:

    std::shared_ptr<openMVG::matching::PairWiseMatches> m_map_PutativesMatches;
    

    std::shared_ptr<Project> m_project ;
    std::shared_ptr<openMVG::sfm::Regions_Provider> m_regions_provider ;

    // Internal counter for the number of step already done
    std::atomic<unsigned int> m_progress_value ;
    std::mutex m_mutex ; 

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif