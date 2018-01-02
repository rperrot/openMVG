// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "WorkerMatchesComputation.hh"

#include "WorkerProgressInterface.hh"

#include "utils/MatchingStats.hh"

#include "openMVG/matching/indMatch.hpp"
#include "openMVG/matching/indMatch_utils.hpp"
#include "openMVG/matching_image_collection/Matcher.hpp"
#include "openMVG/matching_image_collection/Pair_Builder.hpp"
#include "openMVG/sfm/pipelines/sfm_regions_provider.hpp"
#include "openMVG/sfm/sfm_data.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QCoreApplication>

#include <chrono>

namespace openMVG_gui
{
/**
 * @brief Ctr
 * @param proj Project
 */
WorkerMatchesComputation::WorkerMatchesComputation( std::shared_ptr<Project> &proj, std::shared_ptr<openMVG::sfm::Regions_Provider> &regions_provider, const WorkerNextAction &na )
  : WorkerInterface( na )
  , m_project( proj )
  , m_regions_provider( regions_provider )
{
  m_progress_value = 0;
}

/**
 * @brief get progress range
 */
void WorkerMatchesComputation::progressRange( int &min, int &max ) const
{
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData();
  min                                              = 0;
  max                                              = openMVG::exhaustivePairs( sfm_data->GetViews().size() ).size() + 1;

  std::cerr << "WorkerMatchesComputation"
            << " min : " << min << " - max : " << max << std::endl;
}

/**
 * @brief get putatives matches after blind matching
 * @note only valid after success of process
 */
std::shared_ptr<openMVG::matching::PairWiseMatches> WorkerMatchesComputation::putativeMatches( void ) const
{
  return m_map_PutativesMatches;
}

/**
 * @brief Do the computation
 */
void WorkerMatchesComputation::process( void )
{
  const std::string sFeaturePath = m_project->projectPaths().featuresPath( m_project->featureParams() );

  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data                     = m_project->SfMData();
  std::shared_ptr<openMVG::matching_image_collection::Matcher> matcher = m_project->matchingParams().matcher();
  openMVG::Pair_Set pairs                                              = openMVG::exhaustivePairs( sfm_data->GetViews().size() );
  const int nb_pair                                                    = pairs.size();

  m_progress_value = 0;
  sendProgress();

  WorkerProgressInterface *progressInterface = new WorkerProgressInterface();

  connect( progressInterface, SIGNAL( increment( int ) ), this, SLOT( hasIncremented( int ) ), Qt::DirectConnection );

  // Compute matching for all pairs
  m_map_PutativesMatches = std::make_shared<openMVG::matching::PairWiseMatches>();

  const auto start = std::chrono::high_resolution_clock::now();
  matcher->Match( m_regions_provider, pairs, *m_map_PutativesMatches, progressInterface );
  const auto end                                  = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double> elapsed_sec = std::chrono::duration_cast<std::chrono::duration<double>>( end - start );

  // Save the putative matches file
  if ( !openMVG::matching::Save( *m_map_PutativesMatches, stlplus::create_filespec( sFeaturePath, "matches.putative.bin" ) ) )
  {
    std::cerr << "Could not save result" << std::endl;
    // TODO : find a way to inform the user of the failure

    m_progress_value = nb_pair + 1;
    sendProgress();
    emit finished( NEXT_ACTION_ERROR );

    delete progressInterface;
    QCoreApplication::processEvents();
    return;
  }

  // Save the statistics file
  MatchingStats stat_file( elapsed_sec.count() );
  stat_file.save( stlplus::create_filespec( sFeaturePath, "matches.putative.stat" ) );

  m_progress_value = nb_pair + 1;
  sendProgress();
  emit finished( nextAction() );
  QCoreApplication::processEvents();
  delete progressInterface;
}

void WorkerMatchesComputation::sendProgress( void )
{
  int progress_value = m_progress_value;
  emit progress( progress_value );
}

/**
 * @brief internal progress bar has been incremented, now signal it to the external progress dialog
 */
void WorkerMatchesComputation::hasIncremented( int nb )
{
  m_progress_value += nb;
  sendProgress();
}

} // namespace openMVG_gui