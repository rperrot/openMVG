#include "WorkerGeometricFiltering.hh"

#include "WorkerProgressInterface.hh"
#include "utils/MatchingStats.hh"

#include "openMVG/sfm/pipelines/sfm_regions_provider.hpp"

#include "openMVG/matching/indMatch_utils.hpp"
#include "openMVG/matching_image_collection/GeometricFilter.hpp"
#include "openMVG/matching_image_collection/F_ACRobust.hpp"
#include "openMVG/matching_image_collection/E_ACRobust.hpp"
#include "openMVG/matching_image_collection/H_ACRobust.hpp"

#include <QCoreApplication>

#include <chrono>

using namespace openMVG::matching_image_collection ;

namespace openMVG_gui
{

WorkerGeometricFiltering::WorkerGeometricFiltering( std::shared_ptr<Project> & project ,
    std::shared_ptr<openMVG::sfm::Regions_Provider> & regions_provider ,
    std::shared_ptr<openMVG::matching::PairWiseMatches> & map_putative ,
    const WorkerNextAction & na )
  :
  WorkerInterface( na ) ,
  m_project( project ) ,
  m_regions_provider( regions_provider ) ,
  m_map_putative( map_putative )
{

}

/**
* @brief get progress range
*/
void WorkerGeometricFiltering::progressRange( int & min , int & max ) const
{
  min = 0 ;
  max = m_map_putative->size() + 2 ;
}

/**
* @brief get putatives matches after geometric filtering
* @note only valid after success of process
*/
std::shared_ptr<openMVG::matching::PairWiseMatches> WorkerGeometricFiltering::filteredMatches( void ) const
{
  return m_map_filtered;
}


/**
* @brief Do the computation
*/
void WorkerGeometricFiltering::process( void )
{
  const std::string sFeaturePath = m_project->featuresPath() ;
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData() ;
  const MatchingParams m_params = m_project->matchingParams() ;
  const int imax_iteration = m_params.maxIterationFiltering() ;

  std::unique_ptr<openMVG::matching_image_collection::ImageCollectionGeometricFilter> filter_ptr(
    new openMVG::matching_image_collection::ImageCollectionGeometricFilter( sfm_data.get() , m_regions_provider ) );

  WorkerProgressInterface * progressInterface = new WorkerProgressInterface() ;

  connect( progressInterface , SIGNAL( increment( int ) ) , this , SLOT( hasIncremented( int ) ) , Qt::DirectConnection ) ;

  m_progress_value = 0 ;
  sendProgress() ;

  const auto start = std::chrono::high_resolution_clock::now() ;

  bool bGuided_matching = false ;
  std::string sGeometricMatchesFilename ;
  if ( filter_ptr )
  {
    const double d_distance_ratio = 0.6;

    switch ( m_params.geometricModel() )
    {
      case MATCHING_GEOMETRY_TYPE_HOMOGRAPHY:
      {
        const bool bGeometric_only_guided_matching = true;
        filter_ptr->Robust_model_estimation( GeometricFilter_HMatrix_AC( 4.0, imax_iteration ),
                                             *m_map_putative, bGuided_matching,
                                             bGeometric_only_guided_matching ? -1.0 : d_distance_ratio, progressInterface );
        m_map_filtered = std::make_shared<openMVG::matching::PairWiseMatches>( filter_ptr->Get_geometric_matches() ) ;
        sGeometricMatchesFilename = "matches.h.bin";
        break;
      }
      case MATCHING_GEOMETRY_TYPE_FUNDAMENTAL:
      {
        filter_ptr->Robust_model_estimation( GeometricFilter_FMatrix_AC( 4.0, imax_iteration ),
                                             *m_map_putative, bGuided_matching, d_distance_ratio, progressInterface );
        m_map_filtered = std::make_shared<openMVG::matching::PairWiseMatches>( filter_ptr->Get_geometric_matches() ) ;
        sGeometricMatchesFilename = "matches.f.bin" ;
        break;
      }
      case MATCHING_GEOMETRY_TYPE_ESSENTIAL:
      {
        filter_ptr->Robust_model_estimation( GeometricFilter_EMatrix_AC( 4.0, imax_iteration ),
                                             *m_map_putative, bGuided_matching, d_distance_ratio, progressInterface );
        m_map_filtered = std::make_shared<openMVG::matching::PairWiseMatches>( filter_ptr->Get_geometric_matches() ) ;

        //-- Perform an additional check to remove pairs with poor overlap
        std::vector<openMVG::matching::PairWiseMatches::key_type> vec_toRemove;
        for ( openMVG::matching::PairWiseMatches::const_iterator iterMap = m_map_filtered->begin();
              iterMap != m_map_filtered->end(); ++iterMap )
        {
          const size_t putativePhotometricCount = m_map_putative->find( iterMap->first )->second.size();
          const size_t putativeGeometricCount = iterMap->second.size();
          const float ratio = putativeGeometricCount / ( float )putativePhotometricCount;
          if ( putativeGeometricCount < 50 || ratio < .3f )
          {
            // the pair will be removed
            vec_toRemove.push_back( iterMap->first );
          }
        }
        //-- remove discarded pairs
        for ( std::vector< openMVG::matching::PairWiseMatches::key_type>::const_iterator
              iter =  vec_toRemove.begin(); iter != vec_toRemove.end(); ++iter )
        {
          m_map_filtered->erase( *iter );
        }

        sGeometricMatchesFilename = "matches.e.bin";
        break;
      }
    }
    const auto end = std::chrono::high_resolution_clock::now() ;
    const std::chrono::duration<double> elapsed_sec = std::chrono::duration_cast<std::chrono::duration<double>>( end - start ) ;

    m_progress_value = m_map_putative->size() + 1 ;
    sendProgress() ;

    //---------------------------------------
    //-- Export geometric filtered matches
    //---------------------------------------
    if ( !Save( *m_map_filtered,
                stlplus::create_filespec( stlplus::folder_append_separator( sFeaturePath ) , sGeometricMatchesFilename ) ) )
    {
      // std::cerr
      //     << "Cannot save computed matches in: "
      //     << std::string( sMatchesDirectory + "/" + sGeometricMatchesFilename );
    }

    // Save the statistics file
    MatchingStats stat_file = MatchingStats::load( stlplus::create_filespec( sFeaturePath , "matches.putative.stat" ) ) ;
    stat_file = MatchingStats( stat_file.putativeElapsedTime() ,  elapsed_sec.count() ) ;
    stat_file.save( stlplus::create_filespec( sFeaturePath , "matches.filtered.stat" ) ) ;
  }

  m_progress_value = m_map_putative->size() + 2 ;
  sendProgress() ;
  emit finished( nextAction() ) ;
  QCoreApplication::processEvents();

  delete progressInterface ;
}

void WorkerGeometricFiltering::sendProgress( void )
{
  int progress_value = m_progress_value ;
  emit progress( progress_value ) ;
}


/**
* @brief internal progress bar has been incremented, now signal it to the external progress dialog
*/
void WorkerGeometricFiltering::hasIncremented( int nb )
{
  m_progress_value += nb ;
  sendProgress() ;
}

} // namespace openMVG_gui