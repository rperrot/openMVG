#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_EXPORT_TO_PMVS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_EXPORT_TO_PMVS_HH_

#include "WorkerInterface.hh"

#include <QObject>

#include <atomic>
#include <string>

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

class WorkerExportToPMVS : public QObject, public WorkerInterface
{
  public:
    WorkerExportToPMVS( std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data ,
                        const std::string & output_directory ,
                        const int downsampling_factor = 1 ,
                        const int cpu_count = 8 ,
                        const bool export_vis = true ) ;
    /**
    * @brief get progress range
    */
    void progressRange( int & min , int & max ) const ;

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
    * @brief Send progress signal
    */
    void sendProgress( void ) ;

  signals :

    // 0 -> nothing done
    // n -> all done (n : number of images)
    void progress( int ) ;

    // After computation of all features (ie: signal to clear memory)
    void finished( const WorkerNextAction & na );

  private:

    // Internal counter for the number of step already done
    std::atomic<unsigned int> m_progress_value ;

    std::shared_ptr<openMVG::sfm::SfM_Data> m_sfm_data ;
    std::string m_output_directory ;
    int m_downsampling_factor ;
    int m_cpu_count ;
    bool m_export_vis_file ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif