#include "WorkerFeaturesProviderLoad.hh"


#include "openMVG/features/io_regions_type.hpp"
#include "openMVG/features/regions.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"


namespace openMVG_gui
{


/**
* @param pro Project
*/
WorkerFeaturesProviderLoad::WorkerFeaturesProviderLoad( std::shared_ptr<Project> & pro ,
    const WorkerNextAction & na )
  :
  WorkerInterface( na ) ,
  m_project( pro )
{

}


/**
* @brief get progress range
*/
void WorkerFeaturesProviderLoad::progressRange( int & min , int & max ) const
{
  min = 0 ;
  max = 1 ;
}


/**
* @brief Get features provider
* @note only valid after success of progress function
*/
std::shared_ptr<openMVG::sfm::Features_Provider> WorkerFeaturesProviderLoad::featuresProvider( void ) const
{
  return m_feature_provider ;
}


/**
* @brief Do the computation
*/
void WorkerFeaturesProviderLoad::process( void )
{
  emit progress( 0 ) ;

  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData() ;
  const std::string sMatchesDir = m_project->matchesPath( ) ;

  const std::string sImage_describer = stlplus::create_filespec( sMatchesDir, "image_describer", "json" );
  std::unique_ptr<openMVG::features::Regions> regions_type = openMVG::features::Init_region_type_from_file( sImage_describer );
  if ( !regions_type )
  {
    std::cerr << "Invalid: " << sImage_describer << " regions type file." << std::endl;

    // TODO : get an error return
    emit progress( 1 ) ;
    emit finished( NEXT_ACTION_ERROR ) ;

    return;
  }

  // Features reading
  m_feature_provider = std::make_shared<openMVG::sfm::Features_Provider>();
  if ( !m_feature_provider->load( *sfm_data, sMatchesDir, regions_type ) )
  {
    std::cerr << std::endl << "Invalid features." << std::endl;

    // TODO : get an error return
    emit progress( 1 ) ;
    emit finished( NEXT_ACTION_ERROR ) ;

    return ;
  }


  emit progress( 1 ) ;
  emit finished( nextAction() ) ;
}

} // namespace openMVG_gui