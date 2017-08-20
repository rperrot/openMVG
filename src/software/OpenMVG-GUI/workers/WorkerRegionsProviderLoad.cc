#include "WorkerRegionsProviderLoad.hh"

#include "WorkerProgressInterface.hh"

#include "openMVG/features/regions_factory_io.hpp"
#include "openMVG/matching/indMatch_utils.hpp"
#include "openMVG/sfm/pipelines/sfm_regions_provider.hpp"

#include <QCoreApplication>

namespace openMVG_gui
{

/**
* @brief ctr
*/
WorkerRegionsProviderLoad::WorkerRegionsProviderLoad( std::shared_ptr<Project> & pro ,
    const WorkerNextAction & na )
  : WorkerInterface( na ) ,
    m_project( pro )
{

}

/**
* @brief get progress range
*/
void WorkerRegionsProviderLoad::progressRange( int & min , int & max ) const
{
  min = 0 ;
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData() ;
  max = sfm_data->GetViews().size() + 1 ;
}

/**
* @brief get the region loaded (only valid after process has ended with success)
*/
std::shared_ptr<openMVG::sfm::Regions_Provider> WorkerRegionsProviderLoad::regionsProvider( void ) const
{
  return m_regions_provider ;
}

/**
* @brief Do the computation
*/
void WorkerRegionsProviderLoad::process( void )
{

  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData() ;
  const int nb_region = sfm_data->GetViews().size() ;
  const std::string featurePath = m_project->featuresPath() ;

  // Get region type
  const std::string sImage_describer = stlplus::create_filespec( featurePath, "image_describer", "json" );
  std::unique_ptr<openMVG::features::Regions> regions_type = openMVG::features::Init_region_type_from_file( sImage_describer );
  if ( !regions_type )
  {
    std::cerr << "could not load region_type" << std::endl ;
    m_progress_value = nb_region + 1 ;
    sendProgress() ;
    // TODO emit a failure option in finished ?
    emit finished( NEXT_ACTION_ERROR ) ;
    return ;
  }


  WorkerProgressInterface * progressInterface = new WorkerProgressInterface() ;

  connect( progressInterface , SIGNAL( increment( int ) ) , this , SLOT( hasIncremented( int ) ) ,  Qt::DirectConnection ) ;

  m_progress_value = 0 ;
  sendProgress();

  // Load regions
  m_regions_provider = std::make_shared<openMVG::sfm::Regions_Provider>();
  if ( !m_regions_provider->load( *sfm_data, featurePath, regions_type, progressInterface ) )
  {
    std::cerr << "could not load regions" << std::endl ;
    m_progress_value = nb_region + 1 ;
    sendProgress() ;
    // TODO emit a failure option in finished ?
    emit finished( NEXT_ACTION_ERROR ) ;
    return ;
  }

  delete progressInterface ;

  m_progress_value = nb_region + 1 ;
  sendProgress() ;
  emit finished( nextAction() ) ;
}

/**
* @brief internal progress bar has been incremented, now signal it to the external progress dialog
*/
void WorkerRegionsProviderLoad::hasIncremented( int nb )
{
  m_progress_value += nb ;
  sendProgress() ;
}

/**
* @brief set progress value to the main thread
*/
void WorkerRegionsProviderLoad::sendProgress( void )
{
  int progress_value = m_progress_value ;
  emit progress( progress_value ) ;
}


} // namespace openMVG_gui
