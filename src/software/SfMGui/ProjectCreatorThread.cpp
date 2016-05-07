#include "software/SfMGui/ProjectCreatorThread.hpp"

namespace openMVG
{
namespace SfMGui
{
/**
 * @brief ctr
 * @param output_project The output project path
 * @param input_image The input image path
 */
ProjectCreatorThreadWorker::ProjectCreatorThreadWorker( const std::string & output_project , const std::string & input_image )
  : m_output_project_folder_path( output_project ) ,
    m_input_image_folder_path( input_image )
{

}

/**
* @brief start creation of the project
*/
void ProjectCreatorThreadWorker::process( void )
{
  std::string camera_sensor_database_file_path ;

  std::shared_ptr< SfMProject > project = std::make_shared< SfMProject >( m_output_project_folder_path ) ;
  project->OpenImageFolder( m_input_image_folder_path , camera_sensor_database_file_path ) ;


  emit finished( project ) ;
}

/**
* @brief Constructor
* @param output_project_path The output path of the project
* @param input_image_path The input path of the project
*/
ProjectCreator::ProjectCreator( const std::string & output_project_path , const std::string input_image_path )
{
  m_worker = new ProjectCreatorThreadWorker( output_project_path , input_image_path ) ;
  m_worker->moveToThread( &m_thread ) ;

  connect( &m_thread , SIGNAL( finished() ) , m_worker , SLOT( deleteLater() ) ) ;
  connect( m_worker , SIGNAL( finished( std::shared_ptr<SfMProject> ) ) , this , SLOT( hasFinished( std::shared_ptr<SfMProject> ) ) ) ;
  connect( &m_thread , SIGNAL( started() ) , m_worker , SLOT( process() ) ) ;
}

ProjectCreator::~ProjectCreator()
{
  m_thread.quit() ;
  m_thread.wait() ;
}


/**
 * @brief Start creation of the project
 */
void ProjectCreator::start( void )
{
  m_thread.start() ;
}

void ProjectCreator::hasFinished( std::shared_ptr< SfMProject > pro )
{
  emit finished( pro ) ;
}


} // namespace SfMGui
} // namespace openMVG