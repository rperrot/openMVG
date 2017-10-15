#include "WorkerMatchesProviderLoad.hh"

#include "openMVG/sfm/pipelines/sfm_matches_provider.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

namespace openMVG_gui
{
WorkerMatchesProviderLoad::WorkerMatchesProviderLoad( std::shared_ptr<Project> project, const std::string &filename, const WorkerNextAction &na )
  : WorkerInterface( na )
  , m_project( project )
  , m_matches_filename( filename )
{
}

/**
 * @brief get progress range
 */
void WorkerMatchesProviderLoad::progressRange( int &min, int &max ) const
{
  min = 0;
  max = 1;
}

/**
 * @brief get the matches provider after loading
 * @note result is only valid after success of the process function
 */
std::shared_ptr<openMVG::sfm::Matches_Provider> WorkerMatchesProviderLoad::matchesProvider( void ) const
{
  return m_matches_provider;
}

/**
 * @brief Do the computation
 */
void WorkerMatchesProviderLoad::process( void )
{
  emit progress( 0 );

  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData();
  const std::string sFeaturePath                   = m_project->projectPaths().featuresPath( m_project->featureParams() );

  // Matches reading
  m_matches_provider = std::make_shared<openMVG::sfm::Matches_Provider>();
  if // Try to read the two matches file formats
  ( !( m_matches_provider->load( *sfm_data, stlplus::create_filespec( sFeaturePath, m_matches_filename ) ) ) )
  {
    std::cerr << std::endl << "Invalid matches file." << std::endl;

    // TODO find a way to provide an error return
    emit progress( 1 );
    emit finished( NEXT_ACTION_ERROR );

    return;
  }

  emit progress( 1 );
  emit finished( nextAction() );
}
}