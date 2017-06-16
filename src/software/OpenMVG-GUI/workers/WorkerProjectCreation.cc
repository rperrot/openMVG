#include "WorkerProjectCreation.hh"

#include "WorkerProgressInterface.hh"


#include "openMVG/sfm/sfm_data_io.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QCoreApplication>

#include <iostream>

namespace openMVG_gui
{

/**
* @brief Constructor
* @param base_path Project output path
* @param image_path Input image path
* @param intrin Intrisic params
* @param database_file Camera database file
*/
WorkerProjectCreation::WorkerProjectCreation( const std::string &base_path ,
    const std::string &image_path ,
    const IntrinsicParams & intrin ,
    const std::string &database_file ,
    std::shared_ptr<SceneManager> s_mgr ,
    const WorkerNextAction & na )
  :
  WorkerInterface( na ) ,
  m_project_base_path( base_path ) ,
  m_input_image_path( image_path ) ,
  m_intrin_params( intrin ) ,
  m_database_path( database_file ) ,
  m_scn_manager( s_mgr )
{

}

/**
* @brief get progress range
*/
void WorkerProjectCreation::progressRange( int & min , int & max ) const
{
  const std::vector<std::string> vec_image = stlplus::folder_files( m_input_image_path );
  min = 0 ;
  max = vec_image.size() + 2 ;
}

/**
* @brief Function used to retrieve the project after creation
* @return the newly created project (if success only )
*/
std::shared_ptr<Project> WorkerProjectCreation::project( void )
{
  return m_project ;
}

/**
* @brief create the project
*/
void WorkerProjectCreation::process( void )
{
  const std::vector<std::string> vec_image = stlplus::folder_files( m_input_image_path );


  WorkerProgressInterface * progressInterface = new WorkerProgressInterface() ;

  connect( progressInterface , SIGNAL( increment( int ) ) , this , SLOT( hasIncremented( int ) ) , Qt::DirectConnection ) ;

  m_progress_value = 0 ;
  sendProgress() ;

  m_project = std::make_shared<Project>( m_project_base_path , m_input_image_path , m_intrin_params , m_database_path , m_scn_manager , progressInterface ) ;

  m_progress_value = vec_image.size() + 1 ;
  sendProgress() ;

  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData() ;

  const std::string matchesPath = m_project->matchesPath() ;


  if ( !Save(
         *sfm_data,
         stlplus::create_filespec( matchesPath, "sfm_data.json" ).c_str(),
         openMVG::sfm::ESfM_Data( openMVG::sfm::ESfM_Data::VIEWS | openMVG::sfm::ESfM_Data::INTRINSICS ) ) )
  {
    emit progress( vec_image.size() + 2 ) ;
    emit finished( NEXT_ACTION_ERROR ) ;
    return ;
  }


  m_progress_value = vec_image.size() + 2 ;
  sendProgress() ;
  emit finished( nextAction() ) ;
  QCoreApplication::processEvents();

  delete progressInterface ;
}

/**
* @brief internal progress bar has been incremented, now signal it to the external progress dialog
*/
void WorkerProjectCreation::hasIncremented( int nb )
{
  m_progress_value += nb ;
  sendProgress() ;
}

/**
* @brief set progress value to the main thread
*/
void WorkerProjectCreation::sendProgress( void )
{
  int progress_value = m_progress_value ;
  emit progress( progress_value ) ;
}

} // namespace openMVG_gui