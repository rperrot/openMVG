// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "WorkerAutomaticReconstruction.hh"

#include "ApplicationSettings.hh"

#include "WorkerColorComputation.hh"
#include "WorkerFeaturesComputation.hh"
#include "WorkerFeaturesProviderLoad.hh"
#include "WorkerGeometricFiltering.hh"
#include "WorkerIncrementalSfMComputation.hh"
#include "WorkerMatchesComputation.hh"
#include "WorkerMatchesProviderLoad.hh"
#include "WorkerProjectCreation.hh"
#include "WorkerRegionsProviderLoad.hh"
#include "WorkerThumbnailGeneration.hh"

#include "graphics/hierarchies/LinearHierarchy.hh"
#include "graphics/objects/CameraGizmo.hh"
#include "graphics/objects/SphericalGizmo.hh"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QApplication>

namespace openMVG_gui
{

/**
 * @brief Ctr
 * @param input_folder Input (image) folder
 * @param output_folder Output (project) folder
 * @param preset Preset to set
 */
WorkerAutomaticReconstruction::WorkerAutomaticReconstruction( const std::string &                  input_folder,
                                                              const std::string &                  output_folder,
                                                              const AutomaticReconstructionPreset &preset,
                                                              std::shared_ptr<SceneManager> &      scn_mgr,
                                                              const WorkerNextAction &             na ) :
    WorkerInterface( na ),
    m_input_image_folder( input_folder ), m_output_project_folder( output_folder ), m_preset( preset ),
    m_scn_mgr( scn_mgr )
{
}

/**
 * @brief Dtr
 */
WorkerAutomaticReconstruction::~WorkerAutomaticReconstruction()
{
  if ( !m_threadColorComputation->wait( 10 ) )
  {
    m_threadColorComputation->terminate();
    m_threadColorComputation->wait();
    delete m_threadColorComputation;
  }
  if ( !m_threadFeatureProviderLoad->wait( 10 ) )
  {
    m_threadFeatureProviderLoad->terminate();
    m_threadFeatureProviderLoad->wait();
    delete m_threadFeatureProviderLoad;
  }
  if ( !m_threadFeatureComputation->wait( 10 ) )
  {
    m_threadFeatureComputation->terminate();
    m_threadFeatureComputation->wait();
    delete m_threadFeatureComputation;
  }
  if ( !m_threadFeatureProviderLoad->wait( 10 ) )
  {
    m_threadFeatureProviderLoad->terminate();
    m_threadFeatureProviderLoad->wait();
    delete m_threadFeatureProviderLoad;
  }
  if ( !m_threadGeometricFiltering->wait( 10 ) )
  {
    m_threadGeometricFiltering->terminate();
    m_threadGeometricFiltering->wait();
    delete m_threadGeometricFiltering;
  }
  if ( !m_threadIncrementalSfMComputation->wait( 10 ) )
  {
    m_threadIncrementalSfMComputation->terminate();
    m_threadIncrementalSfMComputation->wait();
    delete m_threadIncrementalSfMComputation;
  }
  if ( !m_threadMatchesComputation->wait( 10 ) )
  {
    m_threadMatchesComputation->terminate();
    m_threadMatchesComputation->wait();
    delete m_threadMatchesComputation;
  }
  if ( !m_threadMatchesProviderLoad->wait( 10 ) )
  {
    m_threadMatchesProviderLoad->terminate();
    m_threadMatchesProviderLoad->wait();
    delete m_threadMatchesProviderLoad;
  }
  if ( !m_threadProjectCreation->wait( 10 ) )
  {
    m_threadProjectCreation->terminate();
    m_threadProjectCreation->wait();
    delete m_threadProjectCreation;
  }
  if ( !m_threadRegionsProviderLoad->wait( 10 ) )
  {
    m_threadRegionsProviderLoad->terminate();
    m_threadRegionsProviderLoad->wait();
    delete m_threadRegionsProviderLoad;
  }
  if ( !m_threadThumbnailGeneration->wait( 10 ) )
  {
    m_threadThumbnailGeneration->terminate();
    m_threadThumbnailGeneration->wait();
    delete m_threadThumbnailGeneration;
  }
}

/**
 * @brief get progress range of current stage
 */
void WorkerAutomaticReconstruction::progressRangeCurrentStage( int &min, int &max ) const
{
}

/**
 * @brief get progress range overall (ie: number of stage)
 */
void WorkerAutomaticReconstruction::progressRangeOverall( int &min, int &max ) const
{
  // 1 -> Project creation
  // 2 -> ThumbnailCreation
  // 3 -> feature computation
  // 4 -> region provider load
  // 5 -> matching
  // 6 -> geometric filtering
  // 7 -> feature provider load
  // 8 -> matching provider load
  // 9 -> sfm
  // 10 -> color
  min = 0;
  max = 10;
}

/**
 * @brief Get computed project
 * @return The project
 */
std::shared_ptr<Project> WorkerAutomaticReconstruction::project( void ) const
{
  return m_project;
}

/**
 * @brief Do the computation
 */
void WorkerAutomaticReconstruction::process( void )
{
  m_progress_value   = 0;
  m_progress_overall = 0;
  sendProgressOverall();
  sendProgressCurrentStage();

  doProjectCreation();
  m_mutex.lock();
}

/**
 * @brief internal progress bar has been incremented, now signal it to the external progress dialog
 */
void WorkerAutomaticReconstruction::hasIncrementedCurrentStage( int nb )
{
  m_progress_value += nb;
  sendProgressCurrentStage();
}

/**
 * @brief
 */
void WorkerAutomaticReconstruction::hasIncrementedStage( void )
{
  m_progress_overall += 1;
  sendProgressOverall();
}

/**
 * @brief set progress value to the main thread
 */
void WorkerAutomaticReconstruction::sendProgressCurrentStage( void )
{
  int  progress_value = m_progress_value;
  emit progressCurrentStage( progress_value );
}

/**
 * @brief Set progress value to the main thread
 */
void WorkerAutomaticReconstruction::sendProgressOverall( void )
{
  int  progress_value = m_progress_overall;
  emit progressOverall( progress_value );
}

void WorkerAutomaticReconstruction::doProjectCreation( void )
{
  // Default scene manager

  // Choose the sensor width database :
  // - Use the one in the application settings directory if it exists.
  // - If it does not exists, use the one bundled with the application
  std::string camera_sensor_width_database_file = ApplicationSettings::defaultSensorWidthDatabasePath();
  if ( stlplus::file_exists( ApplicationSettings::applicationWideSensorWidthDatabasePath() ) )
  {
    camera_sensor_width_database_file = ApplicationSettings::applicationWideSensorWidthDatabasePath();
  }
  const IntrinsicParams intrin_params;
  m_worker_project_creation = std::make_shared<WorkerProjectCreation>(
      m_output_project_folder, m_input_image_folder, intrin_params, camera_sensor_width_database_file, m_scn_mgr );
  int min, max;
  m_worker_project_creation->progressRange( min, max );
  emit( progressRangeCurrentStage( min, max ) );
  emit( messageCurrentStage( std::string( "[1/10] Project creation" ) ) );

  m_threadProjectCreation = new QThread( this );
  QThread *thread         = m_threadProjectCreation;
  m_worker_project_creation->moveToThread( thread );

  // Connections
  //  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() )  ) ;
  connect( thread, SIGNAL( finished() ), this, SLOT( hasDoneProjectCreation() ) );
  connect( m_worker_project_creation.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_project_creation.get(), SIGNAL( progress( int ) ), this, SIGNAL( progressCurrentStage( int ) ) );
  connect( thread, SIGNAL( started() ), m_worker_project_creation.get(), SLOT( process() ) );

  thread->start();
}

void WorkerAutomaticReconstruction::doThumbnailCreation( void )
{
  m_worker_thumbnail_generation = std::make_shared<WorkerThumbnailGeneration>( m_project );
  int min, max;
  m_worker_thumbnail_generation->progressRange( min, max );
  emit( progressRangeCurrentStage( min, max ) );
  emit( messageCurrentStage( std::string( "[2/10] Thumbnail creation" ) ) );

  m_threadThumbnailGeneration = new QThread( this );
  QThread *thread             = m_threadThumbnailGeneration;
  m_worker_thumbnail_generation->moveToThread( thread );

  // Connections
  //  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() )   ) ;
  connect( thread, SIGNAL( finished() ), this, SLOT( hasDoneThumbnailCreation() ) );
  connect( m_worker_thumbnail_generation.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread,
           SLOT( quit() ) );
  connect( m_worker_thumbnail_generation.get(), SIGNAL( progress( int ) ), this,
           SIGNAL( progressCurrentStage( int ) ) );
  connect( thread, SIGNAL( started() ), m_worker_thumbnail_generation.get(), SLOT( process() ) );

  thread->start();
}

void WorkerAutomaticReconstruction::doFeatureComputation( void )
{
  FeatureParams f_params;
  if ( m_preset == AUTOMATIC_RECONSTRUCTION_PRESET_NORMAL )
  {
    f_params.setPreset( FEATURE_PRESET_NORMAL );
  }
  else if ( m_preset == AUTOMATIC_RECONSTRUCTION_PRESET_HIGH )
  {
    f_params.setPreset( FEATURE_PRESET_HIGH );
  }
  else if ( m_preset == AUTOMATIC_RECONSTRUCTION_PRESET_ULTRA )
  {
    f_params.setPreset( FEATURE_PRESET_ULTRA );
  }
  m_project->setFeatureParams( f_params );

  m_worker_feature_computation = std::make_shared<WorkerFeaturesComputation>( m_project, true );
  int min, max;
  m_worker_feature_computation->progressRange( min, max );
  emit( progressRangeCurrentStage( min, max ) );
  emit( messageCurrentStage( std::string( "[3/10] Features computation" ) ) );

  m_threadFeatureComputation = new QThread( this );
  QThread *thread            = m_threadFeatureComputation;
  m_worker_feature_computation->moveToThread( thread );

  // Connections
  //  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() )  ) ;
  connect( thread, SIGNAL( started() ), m_worker_feature_computation.get(), SLOT( process() ) );
  connect( m_worker_feature_computation.get(), SIGNAL( progress( int ) ), this, SIGNAL( progressCurrentStage( int ) ) );
  connect( m_worker_feature_computation.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_feature_computation.get(), SIGNAL( finished( const WorkerNextAction & ) ), this,
           SLOT( hasDoneFeatureComputation() ) );

  thread->start();
}

void WorkerAutomaticReconstruction::doFeatureMatching( void )
{
  // 1 region provider load
  // 2 matching
  // 3 geometric filtering
  m_worker_regions_provider_load = std::make_shared<WorkerRegionsProviderLoad>( m_project );
  int min, max;
  m_worker_regions_provider_load->progressRange( min, max );
  emit( progressRangeCurrentStage( min, max ) );
  emit( messageCurrentStage( std::string( "[4/10] Feature regions preparation" ) ) );

  m_threadRegionsProviderLoad = new QThread( this );
  QThread *thread             = m_threadRegionsProviderLoad;
  m_worker_regions_provider_load->moveToThread( thread );

  // Connections
  //  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread, SIGNAL( started() ), m_worker_regions_provider_load.get(), SLOT( process() ) );
  connect( m_worker_regions_provider_load.get(), SIGNAL( progress( int ) ), this,
           SIGNAL( progressCurrentStage( int ) ) );
  connect( m_worker_regions_provider_load.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread,
           SLOT( quit() ) );
  connect( m_worker_regions_provider_load.get(), SIGNAL( finished( const WorkerNextAction & ) ), this,
           SLOT( hasDoneRegionProviderLoad() ) );

  thread->start();
}

void WorkerAutomaticReconstruction::doSfMReconstruction( void )
{
  // feature provider
  // matches provider
  // incremental SfM

  m_worker_feature_provider_load = std::make_shared<WorkerFeaturesProviderLoad>( m_project );
  int min, max;
  m_worker_feature_provider_load->progressRange( min, max );
  emit( progressRangeCurrentStage( min, max ) );
  emit( messageCurrentStage( std::string( "[8/10] Features preparation" ) ) );

  m_threadFeatureProviderLoad = new QThread( this );
  QThread *thread             = m_threadFeatureProviderLoad;
  m_worker_feature_provider_load->moveToThread( thread );

  // Connections
  //  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread, SIGNAL( started() ), m_worker_feature_provider_load.get(), SLOT( process() ) );
  connect( m_worker_feature_provider_load.get(), SIGNAL( progress( int ) ), this,
           SIGNAL( progressCurrentStage( int ) ) );
  connect( m_worker_feature_provider_load.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread,
           SLOT( quit() ) );
  connect( m_worker_feature_provider_load.get(), SIGNAL( finished( const WorkerNextAction & ) ), this,
           SLOT( hasDoneFeatureProviderLoad() ) );

  thread->start();
}

void WorkerAutomaticReconstruction::doColorization( void )
{
  m_worker_color_computation = std::make_shared<WorkerColorComputation>( m_project );
  int min, max;
  m_progress_value = 0;
  m_worker_color_computation->progressRange( min, max );
  emit( progressRangeCurrentStage( min, max ) );
  emit( messageCurrentStage( std::string( "[10/10] Color computation" ) ) );

  m_threadColorComputation = new QThread( this );
  QThread *thread          = m_threadColorComputation;
  m_worker_color_computation->moveToThread( thread );

  // Connections
  //  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread, SIGNAL( started() ), m_worker_color_computation.get(), SLOT( process() ) );
  connect( m_worker_color_computation.get(), SIGNAL( progress( int ) ), this, SIGNAL( progressCurrentStage( int ) ) );
  connect( m_worker_color_computation.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_color_computation.get(), SIGNAL( finished( const WorkerNextAction & ) ), this,
           SLOT( hasDoneColorization() ) );

  thread->start();
}

void WorkerAutomaticReconstruction::hasDoneProjectCreation( void )
{
  m_project = m_worker_project_creation->project();
  // delete project creation
  m_worker_project_creation.reset();

  hasIncrementedStage();
  doThumbnailCreation();
}

void WorkerAutomaticReconstruction::hasDoneThumbnailCreation( void )
{
  // destroy thumbnail creation
  m_worker_thumbnail_generation.reset();

  hasIncrementedStage();
  doFeatureComputation();
}

void WorkerAutomaticReconstruction::hasDoneFeatureComputation( void )
{
  // destroy worker feature computation
  m_worker_feature_computation.reset();

  hasIncrementedStage();
  doFeatureMatching();
}

void WorkerAutomaticReconstruction::hasDoneRegionProviderLoad( void )
{
  hasIncrementedStage();

  std::shared_ptr<openMVG::sfm::Regions_Provider> region = m_worker_regions_provider_load->regionsProvider();
  m_worker_matches_computation = std::make_shared<WorkerMatchesComputation>( m_project, region );
  int min, max;
  m_worker_matches_computation->progressRange( min, max );
  emit( progressRangeCurrentStage( min, max ) );
  emit( messageCurrentStage( std::string( "[5/10] Matches computation" ) ) );

  m_threadMatchesComputation = new QThread( this );
  QThread *thread            = m_threadMatchesComputation;
  m_worker_matches_computation->moveToThread( thread );

  // Connections
  //  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread, SIGNAL( started() ), m_worker_matches_computation.get(), SLOT( process() ) );
  connect( m_worker_matches_computation.get(), SIGNAL( progress( int ) ), this, SIGNAL( progressCurrentStage( int ) ) );
  connect( m_worker_matches_computation.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_matches_computation.get(), SIGNAL( finished( const WorkerNextAction & ) ), this,
           SLOT( hasDoneFeatureMatching() ) );

  thread->start();
}

void WorkerAutomaticReconstruction::hasDoneFeatureMatching( void )
{
  hasIncrementedStage();

  // destroy worker putative matches ; worker region provider
  auto putatives               = m_worker_matches_computation->putativeMatches();
  auto regions                 = m_worker_regions_provider_load->regionsProvider();
  m_worker_geometric_filtering = std::make_shared<WorkerGeometricFiltering>( m_project, regions, putatives );
  int min, max;
  m_worker_geometric_filtering->progressRange( min, max );
  emit( progressRangeCurrentStage( min, max ) );
  emit( messageCurrentStage( std::string( "[6/10] Geometric filtering" ) ) );

  m_threadGeometricFiltering = new QThread( this );
  QThread *thread            = m_threadGeometricFiltering;
  m_worker_geometric_filtering->moveToThread( thread );

  // Connections
  //  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread, SIGNAL( started() ), m_worker_geometric_filtering.get(), SLOT( process() ) );
  connect( m_worker_geometric_filtering.get(), SIGNAL( progress( int ) ), this, SIGNAL( progressCurrentStage( int ) ) );
  connect( m_worker_geometric_filtering.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_geometric_filtering.get(), SIGNAL( finished( const WorkerNextAction & ) ), this,
           SLOT( hasDoneGeometricFiltering() ) );

  thread->start();
}

void WorkerAutomaticReconstruction::hasDoneGeometricFiltering( void )
{
  // destroy : worker matches ; worker regions provider ; worker geometric filtering
  m_worker_matches_computation.reset();
  m_worker_regions_provider_load.reset();
  m_worker_geometric_filtering.reset();

  hasIncrementedStage();
  doSfMReconstruction();
}

void WorkerAutomaticReconstruction::hasDoneFeatureProviderLoad( void )
{
  hasIncrementedStage();

  std::string matches_name       = "matches.f.bin";
  m_worker_matches_provider_load = std::make_shared<WorkerMatchesProviderLoad>( m_project, matches_name );
  int min, max;
  m_worker_matches_provider_load->progressRange( min, max );
  emit( progressRangeCurrentStage( min, max ) );
  emit( messageCurrentStage( std::string( "[7/10] Feature matches preparation" ) ) );

  m_threadMatchesProviderLoad = new QThread( this );
  QThread *thread             = m_threadMatchesProviderLoad;
  m_worker_matches_provider_load->moveToThread( thread );

  // Connections
  //  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread, SIGNAL( started() ), m_worker_matches_provider_load.get(), SLOT( process() ) );
  connect( m_worker_matches_provider_load.get(), SIGNAL( progress( int ) ), this,
           SIGNAL( progressCurrentStage( int ) ) );
  connect( m_worker_matches_provider_load.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread,
           SLOT( quit() ) );
  connect( m_worker_matches_provider_load.get(), SIGNAL( finished( const WorkerNextAction & ) ), this,
           SLOT( hasDoneMatchesProviderLoad() ) );

  thread->start();
}

void WorkerAutomaticReconstruction::hasDoneMatchesProviderLoad( void )
{
  hasIncrementedStage();

  auto features = m_worker_feature_provider_load->featuresProvider();
  auto matches  = m_worker_matches_provider_load->matchesProvider();
  m_worker_incremental_sfm_computation =
      std::make_shared<WorkerIncrementalSfMComputation>( m_project, features, matches );
  int min, max;
  m_worker_incremental_sfm_computation->progressRange( min, max );
  emit( progressRangeCurrentStage( min, max ) );
  emit( messageCurrentStage( std::string( "[9/10] 3d reconstruction" ) ) );

  m_threadIncrementalSfMComputation = new QThread( this );
  QThread *thread                   = m_threadIncrementalSfMComputation;
  m_worker_incremental_sfm_computation->moveToThread( thread );

  // Connections
  //  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread, SIGNAL( started() ), m_worker_incremental_sfm_computation.get(), SLOT( process() ) );
  connect( m_worker_incremental_sfm_computation.get(), SIGNAL( progress( int ) ), this,
           SIGNAL( progressCurrentStage( int ) ) );
  connect( m_worker_incremental_sfm_computation.get(), SIGNAL( finished( const WorkerNextAction & ) ), thread,
           SLOT( quit() ) );
  connect( m_worker_incremental_sfm_computation.get(), SIGNAL( finished( const WorkerNextAction & ) ), this,
           SLOT( hasDoneSfMReconstruction() ) );

  thread->start();
}

void WorkerAutomaticReconstruction::hasDoneSfMReconstruction( void )
{
  m_worker_matches_provider_load.reset();
  m_worker_feature_provider_load.reset();
  m_worker_incremental_sfm_computation.reset();

  hasIncrementedStage();
  doColorization();
}

void WorkerAutomaticReconstruction::hasDoneColorization( void )
{
  m_worker_color_computation.reset();

  hasIncrementedStage();
  m_mutex.unlock();

  emit finished( NEXT_ACTION_NONE );
}

} // namespace openMVG_gui