#include "WorkerExportClustersToMVS.hh"

#include "WorkerExportToMVE.hh"
#include "WorkerExportToOpenMVS.hh"
#include "WorkerExportToPMVS.hh"
#include "WorkerSfMDataLoad.hh"

#include "openMVG/sfm/sfm_data.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <algorithm>

namespace openMVG_gui
{
/**
 * @brief Ctr
 * @param clusters_path Path containing the clusters
 * @param output_base_folder Base folder for exporting data
 * @param method The exporter to use
 */
WorkerExportClustersToMVS::WorkerExportClustersToMVS( const std::string &clusters_path, const std::string &output_base_folder,
    const MVS_exporter &method )
  : WorkerInterface( NEXT_ACTION_NONE ) ,
    m_output_path( output_base_folder ) ,
    m_method( method )
{
  // Build list of clusters
  const std::vector<std::string> files = stlplus::folder_files( clusters_path ) ;
  for( const auto & it : files )
  {
    const std::string ext = stlplus::extension_part( it ) ;
    if( ext == "bin" )
    {
      // This should be enough.
      m_clusters_path.emplace_back( stlplus::folder_append_separator( clusters_path ) + it ) ;
    }
  }
  //                             name    pos
  // Ensure the paths are sorted 0000 ->  0
  //                             0001 ->  1
  std::sort( m_clusters_path.begin() , m_clusters_path.end() ) ;
}

/**
 * @brief get progress range of current stage
 */
void WorkerExportClustersToMVS::progressRangeCurrentStage( int &min, int &max ) const
{
}

/**
 * @brief get progress range overall (ie: number of stage)
 */
void WorkerExportClustersToMVS::progressRangeOverall( int &min, int &max ) const
{
  min = 0 ;
  max = m_clusters_path.size() ;
}

/**
* @brief Get current method used for export
*/
MVS_exporter WorkerExportClustersToMVS::method( void ) const
{
  return m_method ;
}


/**
 * @brief Do the computation
 */
void WorkerExportClustersToMVS::process( void )
{
  m_progress_value   = 0;
  m_progress_overall = 0;
  sendProgressOverall();
  sendProgressCurrentStage();

  // Launch first loading
  processLoadingCurrentCluster() ;
}

/**
 * @brief internal progress bar has been incremented, now signal it to the external progress dialog
 */
void WorkerExportClustersToMVS::hasIncrementedCurrentStage( int nb )
{
  m_progress_value += nb;
  sendProgressCurrentStage();
}

/**
 * @brief
 */
void WorkerExportClustersToMVS::hasIncrementedStage( void )
{
  m_progress_overall += 1;
  sendProgressOverall();
}

/**
 * @brief set progress value to the main thread
 */
void WorkerExportClustersToMVS::sendProgressCurrentStage( void )
{
  int progress_value = m_progress_value;
  emit progressCurrentStage( progress_value );
}

/**
 * @brief Set progress value to the main thread
 */
void WorkerExportClustersToMVS::sendProgressOverall( void )
{
  int progress_value = m_progress_overall;
  emit progressOverall( progress_value );
}

/**
 * @brief Action to be executed when a sfm data has been loaded
 */
void WorkerExportClustersToMVS::hasLoadedClusterData( void )
{
  // 1 - Get the sfm_data
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_worker_sfm_data_load->SfMData();
  m_worker_sfm_data_load.reset() ;

  // Clean delete of the thread
  if ( !m_thread_sfm_data_load->wait( 10 ) )
  {
    m_thread_sfm_data_load->terminate();
    m_thread_sfm_data_load->wait();
    delete m_thread_sfm_data_load;
    m_thread_sfm_data_load = nullptr ;
  }

  // 2 - Launch the computation of the corresponding exporter
  const std::string output_folder =
    stlplus::folder_append_separator( m_output_path ) + "cluster_" + std::to_string( m_progress_overall );

  // If something exists, remove the folder
  if( stlplus::folder_exists( output_folder ) )
  {
    stlplus::folder_delete( output_folder ) ;
  }

  // Create folder
  if( ! stlplus::folder_exists( output_folder ) )
  {
    if( ! stlplus::folder_create( output_folder ) )
    {
      m_progress_overall = m_clusters_path.size() ;
      sendProgressOverall() ;
      emit finished( NEXT_ACTION_ERROR ) ;

      return ;
    }
    if( ! stlplus::folder_exists( output_folder ) )
    {
      m_progress_overall = m_clusters_path.size() ;
      sendProgressOverall() ;
      emit finished( NEXT_ACTION_ERROR ) ;

      return ;
    }
  }

  const std::string progress_overall_msg = std::string( "[" ) + std::to_string( m_progress_overall + 1 ) + "/" + std::to_string( m_clusters_path.size() ) + "]" ;

  QThread *thread ;

  switch( m_method )
  {
    case MVS_EXPORTER_MVE:
    {
      m_worker_export_to_MVE = std::make_shared<WorkerExportToMVE>( sfm_data , output_folder ) ;

      int min, max;
      m_worker_export_to_MVE->progressRange( min, max );
      emit( progressRangeCurrentStage( min, max ) );
      emit( messageCurrentStage( std::string( progress_overall_msg + " Export cluster" ) ) );

      m_thread_mvs_exporter = new QThread( this );
      thread         = m_thread_mvs_exporter;
      m_worker_export_to_MVE->moveToThread( thread );

      connect( m_worker_export_to_MVE.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
      connect( m_worker_export_to_MVE.get(), SIGNAL( progress( int ) ), this, SIGNAL( progressCurrentStage( int ) ) );
      connect( thread, SIGNAL( started() ), m_worker_export_to_MVE.get(), SLOT( process() ) );

      break ;
    }
    case MVS_EXPORTER_OPENMVS:
    {
      const std::string output_undist_folder = stlplus::folder_append_separator( output_folder ) + "undist" ;
      const std::string output_file   = stlplus::create_filespec( output_folder , "scene.mvs" );

      m_worker_export_to_OpenMVS = std::make_shared<WorkerExportToOpenMVS>( sfm_data , output_file , output_undist_folder ) ;

      int min, max;
      m_worker_export_to_OpenMVS->progressRange( min, max );
      emit( progressRangeCurrentStage( min, max ) );
      emit( messageCurrentStage( std::string( progress_overall_msg + " Export cluster" ) ) );

      m_thread_mvs_exporter = new QThread( this );
      thread         = m_thread_mvs_exporter;
      m_worker_export_to_OpenMVS->moveToThread( thread );

      connect( m_worker_export_to_OpenMVS.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
      connect( m_worker_export_to_OpenMVS.get(), SIGNAL( progress( int ) ), this, SIGNAL( progressCurrentStage( int ) ) );
      connect( thread, SIGNAL( started() ), m_worker_export_to_OpenMVS.get(), SLOT( process() ) );

      break ;
    }
    case MVS_EXPORTER_PMVS:
    {
      m_worker_export_to_PMVS = std::make_shared<WorkerExportToPMVS>( sfm_data , output_folder ) ;

      int min, max;
      m_worker_export_to_PMVS->progressRange( min, max );
      emit( progressRangeCurrentStage( min, max ) );
      emit( messageCurrentStage( std::string( progress_overall_msg + " Export cluster" ) ) );

      m_thread_mvs_exporter = new QThread( this );
      thread         = m_thread_mvs_exporter;
      m_worker_export_to_PMVS->moveToThread( thread );

      connect( m_worker_export_to_PMVS.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
      connect( m_worker_export_to_PMVS.get(), SIGNAL( progress( int ) ), this, SIGNAL( progressCurrentStage( int ) ) );
      connect( thread, SIGNAL( started() ), m_worker_export_to_PMVS.get(), SLOT( process() ) );

      break ;
    }
  }

  connect( thread, SIGNAL( finished() ), this, SLOT( hasExportedToMVS() ) );

  thread->start();
}

/**
 * @brief Action to be executed when a mvs export has been done
 */
void WorkerExportClustersToMVS::hasExportedToMVS( void )
{
  m_worker_export_to_MVE.reset() ;
  m_worker_export_to_OpenMVS.reset() ;
  m_worker_export_to_PMVS.reset() ;

  // Clean delete of the thread
  if ( !m_thread_mvs_exporter->wait( 10 ) )
  {
    m_thread_mvs_exporter->terminate();
    m_thread_mvs_exporter->wait();
    delete m_thread_mvs_exporter;
  }

  hasIncrementedStage() ;
  // Start computation of the next cluster
  processLoadingCurrentCluster() ;
}

void WorkerExportClustersToMVS::processLoadingCurrentCluster( void )
{
  if( m_progress_overall < m_clusters_path.size() )
  {
    // Process current cluster
    const std::string path_current_data = m_clusters_path[ m_progress_overall ] ;

    m_worker_sfm_data_load = std::make_shared<WorkerSfMDataLoad>( path_current_data ) ;

    const std::string progress_overall_msg = std::string( "[" ) + std::to_string( m_progress_overall + 1 ) + "/" + std::to_string( m_clusters_path.size() ) + "]" ;

    int min, max;
    m_worker_sfm_data_load->progressRange( min, max );
    emit( progressRangeCurrentStage( min, max ) );
    emit( messageCurrentStage( std::string( progress_overall_msg + " Loading cluster data" ) ) );

    m_thread_sfm_data_load = new QThread( this );
    QThread * thread         = m_thread_sfm_data_load;
    m_worker_sfm_data_load->moveToThread( thread );

    connect( m_worker_sfm_data_load.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( thread , SIGNAL( finished() ), this , SLOT( hasLoadedClusterData() ) );
    connect( m_worker_sfm_data_load.get(), SIGNAL( progress( int ) ), this, SIGNAL( progressCurrentStage( int ) ) );
    connect( thread, SIGNAL( started() ), m_worker_sfm_data_load.get(), SLOT( process() ) );

    thread->start() ;
  }
  else
  {
    // No more cluster to compute, that's the end !
    m_progress_overall = m_clusters_path.size() ;
    sendProgressOverall() ;
    emit finished( nextAction() ) ;
  }
}

} // namespace openMVG_gui