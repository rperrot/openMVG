#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_GEOMETRIC_FILTERING_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_GEOMETRIC_FILTERING_HH_

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
* @brief class used to perform geometric filtering after som matching
*/
class WorkerGeometricFiltering : public QObject, public WorkerInterface
{
  public:
    WorkerGeometricFiltering( std::shared_ptr<Project> & project ,
                              std::shared_ptr<openMVG::sfm::Regions_Provider> & regions_provider ,
                              std::shared_ptr<openMVG::matching::PairWiseMatches> & map_putative ,
                              const WorkerNextAction & na = NEXT_ACTION_NONE ) ;

    /**
    * @brief get progress range
    */
    void progressRange( int & min , int & max ) const ;

    /**
    * @brief get putatives matches after geometric filtering
    * @note only valid after success of process
    */
    std::shared_ptr<openMVG::matching::PairWiseMatches> filteredMatches( void ) const ;

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

    // Internal counter for the number of step already done
    std::atomic<unsigned int> m_progress_value ;
    
    /// in
    std::shared_ptr<Project> m_project ;
    std::shared_ptr<openMVG::sfm::Regions_Provider> m_regions_provider ;
    std::shared_ptr<openMVG::matching::PairWiseMatches> m_map_putative ;
    /// out
    std::shared_ptr<openMVG::matching::PairWiseMatches> m_map_filtered ;
; 

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif