#include "WorkerIncrementalSfMComputation.hh"

#include "openMVG/sfm/pipelines/sequential/sequential_SfM.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_matches_provider.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"
#include "openMVG/sfm/sfm_report.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <string>

namespace openMVG_gui
{

/**
 * @brief Ctr
 * @param proj Project
 * @param features_provider Features
 * @param matches_provider Matches
 */
WorkerIncrementalSfMComputation::WorkerIncrementalSfMComputation( std::shared_ptr<Project> &proj, std::shared_ptr<openMVG::sfm::Features_Provider> &features_provider, std::shared_ptr<openMVG::sfm::Matches_Provider> &matches_provider, const bool reload_initial_intrinsics, const WorkerNextAction &na )
  : WorkerInterface( na )
  , m_project( proj )
  , m_feature_provider( features_provider )
  , m_match_provider( matches_provider )
  , m_reload_initial_intrinsics( reload_initial_intrinsics )
{
}

/**
 * @brief get progress range
 */
void WorkerIncrementalSfMComputation::progressRange( int &min, int &max ) const
{
  // No subdivision for now, maybe in a long long long long ... time !
  // 0 -> start
  // 1 -> process ended
  // 2 -> export done
  min = 0;
  max = 2;
}

/**
 * @brief Do the computation
 */
void WorkerIncrementalSfMComputation::process( void )
{
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData();

  const std::string incremental_path = m_project->projectPaths().reconstructionSequentialPath();

  if ( m_reload_initial_intrinsics )
  {
    sfm_data->poses.clear();
    sfm_data->structure.clear();

    const std::string sMatches_dir       = m_project->projectPaths().matchesPath();
    const std::string sSfM_Data_Filename = stlplus::create_filespec( sMatches_dir, "sfm_data.json" );

    // Load from matches
    if ( !openMVG::sfm::Load( *sfm_data, sSfM_Data_Filename, openMVG::sfm::ESfM_Data( openMVG::sfm::ESfM_Data::VIEWS | openMVG::sfm::ESfM_Data::INTRINSICS ) ) )
    {
      std::cerr << std::endl << "The input SfM_Data file \"" << sSfM_Data_Filename << "\" cannot be read." << std::endl;
      emit progress( 2 );
      emit finished( NEXT_ACTION_ERROR );
      return;
    }
  }
  else
  {
    // clear but reuse intrinsics
    sfm_data->poses.clear();
    sfm_data->structure.clear();
  }

  openMVG::sfm::SequentialSfMReconstructionEngine sfmEngine( *sfm_data, incremental_path, stlplus::create_filespec( incremental_path, "Reconstruction_Report.html" ) );

  // TODO :
  //  sfmEngine.setInitialPair( initialPairIndex );

  // Configure the features_provider & the matches_provider
  sfmEngine.SetFeaturesProvider( m_feature_provider.get() );
  sfmEngine.SetMatchesProvider( m_match_provider.get() );

  // Configure reconstruction parameters
  sfmEngine.Set_Intrinsics_Refinement_Type( convert( m_project->incrementalSfMParams().refinerOptions() ) );
  sfmEngine.SetUnknownCameraType( convert( m_project->incrementalSfMParams().unknownIntrinsic().type() ) ); // EINTRINSIC( i_User_camera_model ) ) ;
  sfmEngine.Set_Use_Motion_Prior( false );                                                                  //  b_use_motion_priors );

  emit progress( 0 );

  if ( sfmEngine.Process() )
  {
    emit progress( 1 );

    openMVG::sfm::Generate_SfM_Report( sfmEngine.Get_SfM_Data(), stlplus::create_filespec( incremental_path, "SfMReconstruction_Report.html" ) );

    //-- Export to disk computed scene (data & visualizable results)
    std::cout << "...Export SfM_Data to disk." << std::endl;
    openMVG::sfm::Save( sfmEngine.Get_SfM_Data(), stlplus::create_filespec( incremental_path, "sfm_data", ".bin" ), openMVG::sfm::ESfM_Data( openMVG::sfm::ESfM_Data::ALL ) );

    openMVG::sfm::Save( sfmEngine.Get_SfM_Data(), stlplus::create_filespec( incremental_path, "cloud_and_poses", ".ply" ), openMVG::sfm::ESfM_Data( openMVG::sfm::ESfM_Data::ALL ) );
    *sfm_data = sfmEngine.Get_SfM_Data();
  }
  else
  {
    emit progress( 1 );
    // TODO throw something or get a signal of failure
    emit progress( 2 );
    emit finished( NEXT_ACTION_ERROR );
  }

  emit progress( 2 );
  emit finished( nextAction() );
}

} // namespace openMVG_gui