#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_THUMBNAIL_GENERATION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_THUMBNAIL_GENERATION_HH_

#include "Project.hh"
#include "WorkerInterface.hh"

#include <QObject>

namespace openMVG_gui
{

/**
* @brief Worker used to generate thumbnails just after project creation
*/
class WorkerThumbnailGeneration : public QObject, public WorkerInterface
{
  public:

    /**
    * @brief Ctr
    * @param pro The project for which thumbnails are required
    */
    WorkerThumbnailGeneration( std::shared_ptr<Project> & pro ,  const WorkerNextAction & na = NEXT_ACTION_NONE ) ;

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

    // After computation of all thumbnails (ie: signal to clear memory)
    void finished( const WorkerNextAction & na );

  private:

    std::shared_ptr<Project> m_project ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif