#include "WorkerGlobalSfMComputation.hh"

#include "openMVG/sfm/pipelines/global/GlobalSfM_rotation_averaging.hpp"
#include "openMVG/sfm/pipelines/global/GlobalSfM_translation_averaging.hpp"
#include "openMVG/sfm/pipelines/global/sfm_global_engine_relative_motions.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_matches_provider.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"
#include "openMVG/sfm/sfm_report.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

namespace openMVG_gui
{

/**
 * @brief Ctr
 * @param project The global project
 * @param features_provider The features
 * @param matches_provider The matches
 * @param reload_initial_intrinsics Decide (true) to restart from a clean sfm_data (from matches dir) or reuse already refined intrinsics (false)
 * @param na Next action to execute after process
 */
WorkerGlobalSfMComputation::WorkerGlobalSfMComputation( std::shared_ptr<Project> project, std::shared_ptr<openMVG::sfm::Features_Provider> &features_provider, std::shared_ptr<openMVG::sfm::Matches_Provider> &matches_provider, const bool reload_initial_intrinsics, const WorkerNextAction &na )
  : WorkerInterface( na )
  , m_project( project )
  , m_feature_provider( features_provider )
  , m_matches_provider( matches_provider )
{
}

/**
 * @brief get progress range
 */
void WorkerGlobalSfMComputation::progressRange( int &min, int &max ) const
{
  // 0 -> start
  // 1 -> process ended
  // 2 -> export done
  min = 0;
  max = 2;
}

/**
 * @brief Do the computation
 */
void WorkerGlobalSfMComputation::process( void )
{
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData();

  emit progress( 0 );

  if ( m_reload_initial_intrinsics )
  {
    sfm_data->poses.clear();
    sfm_data->structure.clear();

    const std::string sMatches_dir       = m_project->matchesPath();
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

  const std::string globalPath = m_project->reconstructionGlobalPath();

  openMVG::sfm::GlobalSfMReconstructionEngine_RelativeMotions sfmEngine( *sfm_data, globalPath, stlplus::create_filespec( globalPath, "Reconstruction_Report.html" ) );

  // Configure the features_provider & the matches_provider
  sfmEngine.SetFeaturesProvider( m_feature_provider.get() );
  sfmEngine.SetMatchesProvider( m_matches_provider.get() );

  const GlobalSfMParams params = m_project->globalSfMParams();

  // Configure reconstruction parameters
  sfmEngine.Set_Intrinsics_Refinement_Type( convert( params.refinerOptions() ) );

  /*
  // TODO
  b_use_motion_priors = cmd.used( 'P' );
  sfmEngine.Set_Use_Motion_Prior( b_use_motion_priors );
  */

  // Configure motion averaging method
  sfmEngine.SetRotationAveragingMethod( convert( params.rotationAveraging() ) );
  sfmEngine.SetTranslationAveragingMethod( convert( params.translationAveraging() ) );

  if ( sfmEngine.Process() )
  {
    std::cout << "...Generating SfM_Report.html" << std::endl;
    Generate_SfM_Report( sfmEngine.Get_SfM_Data(), stlplus::create_filespec( globalPath, "SfMReconstruction_Report.html" ) );

    //-- Export to disk computed scene (data & visualizable results)
    std::cout << "...Export SfM_Data to disk." << std::endl;
    Save( sfmEngine.Get_SfM_Data(), stlplus::create_filespec( globalPath, "sfm_data", ".bin" ), openMVG::sfm::ESfM_Data( openMVG::sfm::ESfM_Data::ALL ) );

    Save( sfmEngine.Get_SfM_Data(), stlplus::create_filespec( globalPath, "cloud_and_poses", ".ply" ), openMVG::sfm::ESfM_Data( openMVG::sfm::ESfM_Data::ALL ) );

    *sfm_data = sfmEngine.Get_SfM_Data();
  }
  else
  {
    emit progress( 2 );
    emit finished( NEXT_ACTION_ERROR );
  }

  emit progress( 2 );
  emit finished( nextAction() );
}

} // namespace openMVG_gui