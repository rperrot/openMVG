// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "MainWindow.hh"

// Dialogs
#include "AutomaticReconstructionDialog.hh"
#include "ClusterComputationSettingsDialog.hh"
#include "MaskDefinitionDialog.hh"
#include "NewProjectDialog.hh"

#include "workflow_params/widgets/IntrinsicSelectorParamsDialog.hh"
#include "workflow_params/widgets/FeatureParamsDialog.hh"
#include "workflow_params/widgets/MatchingParamsDialog.hh"
#include "workflow_params/widgets/SfMParamsDialog.hh"


#include "graphics/hierarchies/LinearHierarchy.hh"
#include "graphics/objects/CameraGizmo.hh"
#include "graphics/objects/PointCloud.hh"
#include "graphics/objects/SphericalGizmo.hh"
#include "utils/BoundingSphere.hh"
#include "utils/ImageDescriberInfo.hh"
#include "utils/PlyLoader.hh"

#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QThread>
#include <QToolBar>
#include <QVBoxLayout>

#include <exception>
#include <thread>

namespace openMVG_gui
{
/**
* @brief Main window
*/
MainWindow::MainWindow()
  : m_project( nullptr )
{
  setWindowTitle( "OpenMVG-GUI" );
  showMaximized() ;

  buildInterface() ;
  buildMenus() ;
  buildToolbar() ;
  makeConnections() ;

  m_state = STATE_EMPTY ;
  updateInterface() ;

  m_progress_dialog = nullptr ;

  m_detail_list->setVisible( false ) ;
}

/**
* @brief Action to be executed when user want to create a new project
*/
void MainWindow::onNewProject( void )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  qInfo( "New Project" ) ;
  NewProjectDialog dlg( this ) ;

  int res = dlg.exec() ;
  if( res == QDialog::Accepted )
  {
    // Default scene manager
    std::shared_ptr<SceneHierarchy> s_hier = std::make_shared<LinearHierarchy>() ;
    std::shared_ptr<SceneManager> default_scene_manager = std::make_shared<SceneManager>( nullptr , s_hier ) ;
    default_scene_manager->addObject( m_result_view->grid() ) ;
    default_scene_manager->addObject( m_result_view->sphericalGizmo() ) ;

    // 1 - project creation (exif, ...)
    const std::string base_path = dlg.outputProjectPath() ;
    const std::string image_path = dlg.inputImagePath() ;
    const IntrinsicParams intrin_params ;
    const std::string camera_sensor_width_database_file = stlplus::create_filespec( stlplus::folder_append_separator( stlplus::folder_append_separator( QCoreApplication::applicationDirPath().toStdString() ) + "ressources" ) + "sensor_database" , "sensor_width_camera_database.txt" ) ;

    m_worker_project_creation = std::make_shared<WorkerProjectCreation>( base_path , image_path , intrin_params , camera_sensor_width_database_file , default_scene_manager ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_project_creation->moveToThread( thread ) ;

    int progress_min = 0 , progress_max = 0 ;
    m_worker_project_creation->progressRange( progress_min , progress_max ) ;
    createProgress( "Project Creation, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( thread , SIGNAL( started() ) , m_worker_project_creation.get() , SLOT( process() ) ) ;
    connect( m_worker_project_creation.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasCreatedProject( const WorkerNextAction & ) ) ) ;
    connect( m_worker_project_creation.get() , SIGNAL( finished( const WorkerNextAction & ) ) , thread , SLOT( quit() ) ) ;
    connect( m_worker_project_creation.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

    thread->start() ;
  }
}

/**
* @brief Action to be executed when user want to open a project
*/
void MainWindow::onOpenProject( void )
{
  qInfo( "Open Project" ) ;
  if( hasUnsavedChange() && m_project )
  {
    QMessageBox::StandardButton btn = QMessageBox::question( this , "Project unsaved" , "Project unsaved, save now ?" , QMessageBox::Cancel | QMessageBox::No | QMessageBox::Save , QMessageBox::Save ) ;
    if( btn == QMessageBox::Save )
    {
      try
      {
        m_project->save() ;
      }
      catch( std::runtime_error & err )
      {
        QMessageBox::StandardButton btn = QMessageBox::critical( this , "Error" , "There was an error during save of the project" ) ;
      }
    }
    else if( btn == QMessageBox::Cancel )
    {
      return ;
    }
  }

  // Now open project and set scene according to the specified project
  QString path = QFileDialog::getOpenFileName( this , "Open project file" ,  QDir::homePath() , "OpenMVG project file (*.omvg)" ) ;
  if( path.isEmpty() || path.isNull() )
  {
    return ;
  }

  m_project.reset() ;
  resetInterface() ;

  const std::string projectPath = path.toStdString() ;

  std::shared_ptr<SceneHierarchy> s_hier = std::make_shared<LinearHierarchy>() ;
  std::shared_ptr<SceneManager> default_scene_manager = std::make_shared<SceneManager>( nullptr , s_hier ) ;
  default_scene_manager->addObject( m_result_view->grid() ) ;
  std::shared_ptr<SphericalGizmo> sph_giz = std::dynamic_pointer_cast<SphericalGizmo>( m_result_view->sphericalGizmo() );
  default_scene_manager->addObject( sph_giz ) ;

  try
  {
    m_project = std::make_shared<Project>( projectPath , default_scene_manager ) ;
  }
  catch( std::runtime_error & err )
  {
    QMessageBox::StandardButton btn = QMessageBox::critical( this , "Error" , "Could not open project" ) ;
    m_project.reset() ;
    updateInterface() ;
    return ;
  }

  m_feature_viewer_widget->setProject( m_project ) ;

  m_project->sceneManager()->setCamera( m_project->viewportCamera() ) ;
  m_result_view->setScene( m_project->sceneManager() );
  m_result_view->prepareObjects() ;
  m_result_view->updateTrackballSize() ;

  m_result_view->update() ;


  // Load thumbnails
  onUpdateImageList() ;

  for( int i = 0 ; i < m_project->nbImage() ; ++i )
  {
    if( m_project->maskEnabled( i ) )
    {
      m_image_list->setMaskEnabled( i , true ) ;
    }
  }

  /**
  * Select a matching method that is compatible with the features computed
  * Because project could have been saved before feature computation
  */

  // Update scene state
  m_state = STATE_PROJECT_OPENED ;
  if( stlplus::folder_exists( stlplus::folder_append_separator( m_project->projectPaths().exportPath() ) + "clusters" ) )
  {

    // Load the color file
    std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
    const std::string sparse = m_project->projectPaths().colorizedPlyCloud( m_project->sfMMethod() ) ;

    postFeaturesComputation() ;
    postMatchesComputation() ;
    postSfMComputation() ;
    postColorComputation() ;
    m_state = STATE_CLUSTERING_COMPUTED ;
  }
  else if( m_project->hasColorComputed() )
  {
    // Load the color file
    std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
    const std::string sparse = m_project->projectPaths().colorizedPlyCloud( m_project->sfMMethod() ) ;

    postFeaturesComputation() ;
    postMatchesComputation() ;
    postSfMComputation() ;
    postColorComputation() ;

    m_state = STATE_COLOR_COMPUTED ;
  }
  else if( m_project->hasSfMComputed() )
  {
    postFeaturesComputation() ;
    postMatchesComputation() ;
    postSfMComputation();
    m_state = STATE_SFM_COMPUTED ;
  }
  else if( m_project->hasAllMatchesComputed() )
  {
    postFeaturesComputation() ;
    postMatchesComputation() ;
    m_state = STATE_MATCHES_COMPUTED ;
  }
  else if( m_project->hasAllFeaturesComputed() )
  {
    postFeaturesComputation() ;
    m_state = STATE_FEATURES_COMPUTED ;
  }
  updateInterface() ;
}

/**
* @brief Action to be executed when user want to save a project
*/
void MainWindow::onSaveProject( void )
{
  // qInfo( "Save Project" ) ;
  if( m_project )
  {
    try
    {
      m_project->save() ;
    }
    catch( std::runtime_error & err )
    {
      QMessageBox::StandardButton btn = QMessageBox::critical( this , "Error" , "There was an error during save of the project" ) ;
    }
  }
}

/**
* @brief Action to be executed when user want to save a project to another folder
*/
void MainWindow::onSaveAsProject( void )
{
  qInfo( "Save as Project" ) ;
  QMessageBox::critical( this , "Sorry" , "This feature is not implemented yet" ) ;
}

/**
* @brief Action to be executed when user want to close current project
*/
void MainWindow::onCloseProject( void )
{
  // qInfo( "Close Project" ) ;
  if( hasUnsavedChange() && m_project )
  {
    QMessageBox::StandardButton btn = QMessageBox::question( this , "Project unsaved" , "Project unsaved, save now ?" , QMessageBox::Cancel | QMessageBox::No | QMessageBox::Save , QMessageBox::Save ) ;
    if( btn == QMessageBox::Save )
    {
      try
      {
        m_project->save() ;
      }
      catch( std::runtime_error & err )
      {
        QMessageBox::StandardButton btn = QMessageBox::critical( this , "Error" , "There was an error during save of the project" ) ;
      }
    }
    else if( btn == QMessageBox::Cancel )
    {
      return ;
    }
  }

  // Reset interface (project/scene)
  m_project.reset() ;
  m_result_view->setScene( nullptr ) ;
  m_feature_viewer_widget->setProject( nullptr ) ;
  m_detail_list->clear() ;

  resetInterface() ;
}

/**
* @brief Action to be executed when user want to quit the project
*/
void MainWindow::onQuit( void )
{
  qInfo( "Quit" ) ;
  if( m_project && hasUnsavedChange() )
  {
    QMessageBox::StandardButton btn = QMessageBox::question( this , "Project unsaved" , "Project unsaved, save now ?" , QMessageBox::Cancel | QMessageBox::No | QMessageBox::Save , QMessageBox::Save ) ;
    if( btn == QMessageBox::Save )
    {
      try
      {
        m_project->save() ;
      }
      catch( std::runtime_error & err )
      {
        QMessageBox::StandardButton btn = QMessageBox::critical( this , "Error" , "There was an error during save of the project" ) ;
      }
    }
    else if( btn == QMessageBox::Cancel )
    {
      return ;
    }
  }

  QApplication::quit() ;
}

/**
* @brief Action to be executed when user wants to compute automatic reconstruction
*/
void MainWindow::onComputeAutomaticReconstruction( void )
{
  AutomaticReconstructionDialog dlg( this ) ;

  int res = dlg.exec() ;
  if( res == QDialog::Accepted )
  {
    // Default scene manager
    std::shared_ptr<Camera> cam = std::make_shared<Camera>( ) ;
    std::shared_ptr<SceneHierarchy> s_hier = std::make_shared<LinearHierarchy>() ;
    std::shared_ptr<SceneManager> default_scene_manager = std::make_shared<SceneManager>( cam , s_hier ) ;
    default_scene_manager->addObject( m_result_view->grid() ) ;
    default_scene_manager->addObject( m_result_view->sphericalGizmo() ) ;

    const std::string input_folder  = dlg.inputImagePath() ;
    const std::string output_folder = dlg.outputProjectPath() ;
    const AutomaticReconstructionPreset preset = dlg.preset() ;
    m_worker_automatic_reconstruction = std::make_shared<WorkerAutomaticReconstruction>( input_folder ,
                                        output_folder ,
                                        preset ,
                                        default_scene_manager ) ;

    int min, max ;
    m_worker_automatic_reconstruction->progressRangeOverall( min , max ) ;
    m_double_progress_dialog = new DoubleProgressBarDialog( this ) ;
    m_double_progress_dialog->setRange1( min , max ) ;
    m_double_progress_dialog->setValue1( 0 ) ;
    m_double_progress_dialog->setLabelText2( "Current step" ) ;
    m_double_progress_dialog->setWindowModality( Qt::WindowModal ) ;
    m_double_progress_dialog->show();

    QThread * thread = new QThread( this ) ;
    m_worker_automatic_reconstruction->moveToThread( thread ) ;

    connect( thread , SIGNAL( started() ) , m_worker_automatic_reconstruction.get() , SLOT( process() ) ) ;
    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( m_worker_automatic_reconstruction.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasDoneAutomaticReconstruction( const WorkerNextAction & ) ) ) ;
    connect( m_worker_automatic_reconstruction.get() , SIGNAL( finished( const WorkerNextAction & ) ) , thread , SLOT( quit() ) ) ;
    connect( m_worker_automatic_reconstruction.get() , SIGNAL( progressOverall( int ) ) , m_double_progress_dialog , SLOT( setValue1( int ) ) ) ;
    connect( m_worker_automatic_reconstruction.get() , SIGNAL( progressCurrentStage( int ) ) , m_double_progress_dialog , SLOT( setValue2( int ) ) ) ;
    connect( m_worker_automatic_reconstruction.get() , SIGNAL( progressRangeCurrentStage( int , int ) ) , m_double_progress_dialog , SLOT( setRange2( int , int ) ) ) ;
    connect( m_worker_automatic_reconstruction.get() , SIGNAL( messageCurrentStage( const std::string & ) ) , m_double_progress_dialog , SLOT( setLabelText1( const std::string & ) ) ) ;

    thread->start() ;
  }
}

void MainWindow::onOpenPipelineEditor()
{
  /*  PipelineEditor * editor = new PipelineEditor( nullptr ) ;
    editor->show() ;
    */
}


/**
* @brief Action to be executed when user want to compute features
*/
void MainWindow::onComputeFeatures( void )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  bool overwrite = false ;
  if( m_project->hasAllFeaturesComputed() || m_project->hasPartialFeaturesComputed() )
  {
    // Ask user to overwrite ?
    QMessageBox::StandardButton btn = QMessageBox::question( this , "File exists" , "Some images have already a description, overwrite it ?" , QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel ) ;
    if( btn == QMessageBox::Cancel )
    {
      return ;
    }
    else if( btn == QMessageBox::Yes )
    {
      overwrite = true ;
    }
    else
    {
      overwrite = false ;
    }
  }

  //  qInfo( "Compute features" ) ;


  m_worker_features_computation = std::make_shared<WorkerFeaturesComputation>( m_project , overwrite ) ;
  QThread * thread = new QThread( this ) ;
  m_worker_features_computation->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_features_computation->progressRange( progress_min , progress_max ) ;
  createProgress( "Features computation, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( started() ) , m_worker_features_computation.get() , SLOT( process() ) ) ;
  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( m_worker_features_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_features_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedFeatures( const WorkerNextAction & ) ) ) ;
  connect( m_worker_features_computation.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to compute matches
*/
void MainWindow::onComputeMatches( void )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  // First load then what's next
  WorkerNextAction act = NEXT_ACTION_COMPUTE_MATCHES | NEXT_ACTION_COMPUTE_GEOMETRIC_FILTERING ;

  // Load regions
  qInfo( "Load regions" ) ;
  m_worker_regions_provide_load = std::make_shared<WorkerRegionsProviderLoad>( m_project , act ) ;
  QThread * thread = new QThread( this ) ;
  m_worker_regions_provide_load->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_regions_provide_load->progressRange( progress_min , progress_max ) ;
  createProgress( "Loading regions, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( started() ) , m_worker_regions_provide_load.get() , SLOT( process() ) ) ;
  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( m_worker_regions_provide_load.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_regions_provide_load.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasLoadedRegions( const WorkerNextAction & ) ) ) ;
  connect( m_worker_regions_provide_load.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to compute structure
*/
void MainWindow::onComputeSfM( void )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  qInfo( "Compute SfM" ) ;

  WorkerNextAction act = NEXT_ACTION_LOAD_MATCHES ;
  if( m_project->sfMMethod() == SFM_METHOD_INCREMENTAL )
  {
    act = act | NEXT_ACTION_COMPUTE_INCREMENTAL_SFM ;
  }
  else
  {
    act = act | NEXT_ACTION_COMPUTE_GLOBAL_SFM ;
  }

  // Load features
  m_worker_features_provider_load = std::make_shared<WorkerFeaturesProviderLoad>( m_project , act ) ;
  QThread * thread = new QThread( this ) ;
  m_worker_features_provider_load->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_features_provider_load->progressRange( progress_min , progress_max ) ;
  createProgress( "Loading features, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( started() ) , m_worker_features_provider_load.get() , SLOT( process() ) ) ;
  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( m_worker_features_provider_load.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_features_provider_load.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasLoadedFeatures( const WorkerNextAction & ) ) ) ;
  connect( m_worker_features_provider_load.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to compute color
*/
void MainWindow::onComputeColor( void )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  qInfo( "Compute Color" ) ;

  m_worker_color_computation = std::make_shared<WorkerColorComputation>( m_project );
  QThread * thread = new QThread( this ) ;
  m_worker_color_computation->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_color_computation->progressRange( progress_min , progress_max ) ;
  createProgress( "Computing scene color, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( started() ) , m_worker_color_computation.get() , SLOT( process() ) ) ;
  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( m_worker_color_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_color_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedColor( const WorkerNextAction & ) ) ) ;
  connect( m_worker_color_computation.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

  thread->start() ;
}

/**
 * @brief Action to be executed when user wants to change intrinsics for view
 */
void MainWindow::onChangeIntrinsicsSettings( void )
{
  qInfo( "Change intrinsics " ) ;
  IntrinsicSelectorParamsDialog dlg( this , m_project ) ;

  int res = dlg.exec() ;
  if( res == QDialog::Accepted )
  {
    std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData() ;

    // Replace intrinsics
    sfm_data->intrinsics.clear() ;
    for( auto it : dlg.intrinsics() )
    {
      sfm_data->intrinsics.insert( { it.first , it.second } ) ;
    }
    // Replace intrinsics reference in the views
    for( auto it : dlg.indexes() )
    {
      sfm_data->views.at( it.first )->id_intrinsic = it.second ;
    }

    // Save the sfm_data.json
    const std::string matchesPath = m_project->projectPaths().matchesPath();
    if ( !Save( *sfm_data, stlplus::create_filespec( matchesPath, "sfm_data.json" ).c_str(),
                openMVG::sfm::ESfM_Data( openMVG::sfm::ESfM_Data::VIEWS | openMVG::sfm::ESfM_Data::INTRINSICS ) ) )
    {
      std::cerr << "Error while saving sfm_data.json" << std::endl;
    }
  }
}


/**
* @brief Action to be executed when user wants to change features computation settings
*/
void MainWindow::onChangeFeatureSettings( void )
{
  qInfo( "Change feature settings" ) ;
  FeatureParamsDialog dlg( this , m_project->featureParams() ) ;

  int res = dlg.exec() ;
  if( res == QDialog::Accepted )
  {
    m_project->setFeatureParams( dlg.params() ) ;
  }
}

/**
* @brief Action to be executed when user wants to change matching computation settings
*/
void MainWindow::onChangeMatchesSettings( void )
{
  qInfo( "Change matches settings" ) ;

  MatchingParamsDialog dlg( this , m_project->matchingParams() ) ;

  // Load the image describer (if it exists)
  const std::string featuresPath = m_project->projectPaths().featuresPath( m_project->featureParams() ) ;
  const std::string describerPath = stlplus::create_filespec( featuresPath , "image_describer.json" ) ;
  if( stlplus::file_exists( describerPath ) )
  {
    if( isBinaryDescriber( describerPath ) )
    {
      dlg.enableBinaryMode() ;
      dlg.disableScalarMode() ;
    }
    else
    {
      dlg.enableScalarMode() ;
      dlg.disableBinaryMode() ;
    }
  }


  int res = dlg.exec() ;
  if ( res == QDialog::Accepted )
  {
    m_project->setMatchingParams( dlg.params() ) ;
  }
}

/**
* @brief Action to be executed when user wants to change
*/
void MainWindow::onChangeSfMSettings( void )
{
  qInfo( "Change SfM settings" ) ;

  SfMParamsDialog dlg( this , m_project->sfMMethod() , m_project->incrementalSfMParams() , m_project->globalSfMParams() , m_project->imageIds() ) ;

  int res = dlg.exec() ;
  if( res == QDialog::Accepted )
  {
    m_project->setSfMMethod( dlg.method() ) ;
    m_project->setIncrementalSfMParams( dlg.incrementalParams() ) ;
    m_project->setGlobalSfMParams( dlg.globalParams() ) ;
  }
}

/**
* @brief Action to be executed when a project has been created
* -> Prepare interface (generate thumbnails, ...)
*/
void MainWindow::onHasCreatedProject( const WorkerNextAction & next_action  )
{
  qInfo( "Project created" ) ;
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    QMessageBox::critical( this , "Error" , "There was an error during project creation" ) ;
    m_project.reset() ;
    m_worker_project_creation.reset() ;
    resetInterface() ;
    return ;
  }

  // Set the project
  m_project = m_worker_project_creation->project() ;
  m_worker_project_creation.reset() ;

  m_feature_viewer_widget->setProject( m_project ) ;

  // Initialize the 3d view
  m_result_view->setScene( m_project->sceneManager() );
  m_result_view->prepareObjects() ;
  m_result_view->updateTrackballSize() ;

  m_result_view->update() ;

  // Generate the thumbnails
  m_worker_thumbnail_generation = std::make_shared<WorkerThumbnailGeneration>( m_project ) ;
  QThread * thread = new QThread( this ) ;
  m_worker_thumbnail_generation->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_thumbnail_generation->progressRange( progress_min , progress_max ) ;
  createProgress( "Thumbnails Creation, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread , SIGNAL( started() ) , m_worker_thumbnail_generation.get() , SLOT( process() ) ) ;
  connect( m_worker_thumbnail_generation.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_thumbnail_generation.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onUpdateImageList() ) ) ;
  connect( m_worker_thumbnail_generation.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to update image list widget
*/
void MainWindow::onUpdateImageList( void )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  m_worker_thumbnail_generation.reset() ;

  const std::vector< std::pair< int , std::string > > images_path = m_project->GetImageNames() ;
  const std::string thumb_path = m_project->projectPaths().thumbnailsPath() ;

  std::vector< std::pair<int, std::string>> images_full_path ;
  std::map< int , std::string > image_id_names ;
  std::map< int , std::pair<int, int> > image_resolution ;
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData() ;
  for( size_t id_image = 0 ; id_image < images_path.size() ; ++id_image )
  {
    const std::string path = stlplus::create_filespec( stlplus::folder_append_separator( thumb_path ) , images_path[ id_image ].second ) ;
    images_full_path.emplace_back( std::make_pair( images_path[ id_image ].first , path ) ) ;

    image_id_names[ images_path[ id_image ].first ] = images_path[ id_image ].second ;
    image_resolution[ images_path[ id_image ].first ] =
      std::make_pair( sfm_data->GetViews().at( images_path[ id_image ].first )->ui_width ,
                      sfm_data->GetViews().at( images_path[ id_image ].first )->ui_height ) ;

  }

  m_image_list->setImages( images_full_path ) ;
  m_detail_list->setImagesInfos( image_id_names , image_resolution );

  m_state = STATE_PROJECT_OPENED ;
  updateInterface() ;
}

/**
* @brief Action to be executed when user wants to show/hide the grid
*/
void MainWindow::onShowHideGrid( void )
{
  std::shared_ptr<RenderableObject> g = m_result_view->grid() ;
  const bool active = g->isVisible() ;
  g->setVisible( ! active ) ;
  m_result_view->update() ;
}

/**
* @brief Action to be executed when user wants to show/hide the camera gizmos
*/
void MainWindow::onShowHideCameraGizmos( void )
{
  if( m_project )
  {
    std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
    if( mgr )
    {
      // Remove selection of all previous gizmos
      std::vector< std::shared_ptr<RenderableObject> > gizmos = mgr->cameraGizmos() ;
      for( auto & it : gizmos )
      {
        std::shared_ptr<CameraGizmo> c_gizmo = std::dynamic_pointer_cast<CameraGizmo>( it ) ;
        if( c_gizmo.use_count() )
        {
          c_gizmo->switchVisibility( ) ;
        }
      }
    }
  }
  m_result_view->update() ;
}

/**
* @brief Action to be executed when user wants to show/hide image list
*/
void MainWindow::onShowImageList( void )
{
  const bool visible = m_show_hide_image_list_act->isChecked() ;
  m_image_list->setVisible( visible ) ;
  m_result_view->update() ;
}


/**
* @brief Action to be exected when user wants to show/hide detail list
*/
void MainWindow::onShowHideDetail( void )
{
  const bool visible = m_show_hide_detail_list_act->isChecked() ;
  m_detail_list->setVisible( visible ) ;
  m_result_view->update() ;
}


/**
 * @brief Action to be executed when user wants to show/hide report
 */
void MainWindow::onShowReconstructionReport( void )
{
  const bool visible = m_show_hide_reconstruction_summary_act->isChecked() ;
  if( visible )
  {
    m_result_summary_widget->show() ;
  }
  else
  {
    m_result_summary_widget->hide() ;
  }
}

/**
 * @brief Action to be executed when user close the reconstruction report window
 */
void MainWindow::onCloseReconstructionReport( void )
{
  m_show_hide_reconstruction_summary_act->setChecked( false ) ;
}


/**
 * @brief Action to be executed when user wants to show/hide console
 */
void MainWindow::onShowHideConsoleWindow( void )
{
  const bool visible = m_show_hide_console_act->isChecked() ;
  if( visible )
  {
    m_console_widget->show() ;
  }
  else
  {
    m_console_widget->hide() ;
  }
}

/**
 * @brief Action to be executed when user close the console window
 */
void MainWindow::onCloseConsoleWindow( void )
{
  m_show_hide_console_act->setChecked( false ) ;
}


/**
 * @brief Show features on a given image
 */
void MainWindow::onShowHideFeatureViewer( void )
{
  const bool visible = m_show_hide_features_viewer->isChecked() ;
  if( visible )
  {
    m_feature_viewer_widget->show() ;
  }
  else
  {
    m_feature_viewer_widget->hide() ;
  }
}

/**
 * @brief Action to be executed when user close the feature viewer
 */
void MainWindow::onCloseFeatureViewer( void )
{
  m_show_hide_features_viewer->setChecked( false ) ;
}


/**
* @brief Action to be executed when user has selected an image in the image list
* @param id Id of the selected image
*/
void MainWindow::onSelectImage( int id )
{
  if( m_project )
  {
    std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
    if( mgr )
    {
      // Remove selection of all previous gizmos
      std::vector< std::shared_ptr<RenderableObject> > gizmos = mgr->cameraGizmos() ;
      for( auto & it : gizmos )
      {
        std::shared_ptr<CameraGizmo> c_gizmo = std::dynamic_pointer_cast<CameraGizmo>( it ) ;
        if( c_gizmo.use_count() )
        {
          c_gizmo->setSelectionWeight( -1.0 ) ; // setSelectionState( TRI_STATE_SELECTION_NONE ) ;
        }
      }

      // Select the specified one
      {
        std::shared_ptr<RenderableObject> gizmo = mgr->cameraGizmo( id ) ;
        std::shared_ptr<CameraGizmo> c_gizmo = std::dynamic_pointer_cast<CameraGizmo>( gizmo ) ;
        if( c_gizmo.use_count() )
        {
          c_gizmo->setSelectionWeight( 2.0 ) ; // TRI_STATE_SELECTION_FIRST );
        }
      }

      const std::vector<std::pair<int, double> > linked = m_project->linkedViewsWithStrength( id ) ;
      for( const auto linked_id : linked )
      {
        std::shared_ptr<RenderableObject> gizmo = mgr->cameraGizmo( linked_id.first ) ;
        std::shared_ptr<CameraGizmo> c_gizmo = std::dynamic_pointer_cast<CameraGizmo>( gizmo ) ;
        if( c_gizmo.use_count() )
        {
          c_gizmo->setSelectionWeight( linked_id.second ) ; // TRI_STATE_SELECTION_SECOND );
        }

      }
    }
  }
  m_result_view->update() ;
}


/**
* @brief Action to be executed when user want to define mask of an image
* @param id Id of the image for which mask would be defined
*/
void MainWindow::onDefineMask( int id )
{
  MaskDefinitionDialog dlg( this , m_project , id ) ;
  dlg.setMaskActivated( m_project->maskEnabled( id ) );

  int res = dlg.exec() ;
  if( res == QDialog::Accepted )
  {
    if( dlg.hasMaskActivated() )
    {
      QImage img = dlg.getMask() ;

      // Save mask
      const std::string path = m_project->getMaskImagePath( id ) ;
      img.save( path.c_str() ) ;
      m_project->setMaskEnabled( id , true ) ;
      m_image_list->setMaskEnabled( id , true ) ;
    }
    else
    {
      m_project->setMaskEnabled( id , false ) ;
      m_image_list->setMaskEnabled( id , false ) ;
    }
  }
  else
  {

  }
}


/**
* @brief Action to be executed when user select perspective projection matrix
*/
void MainWindow::onSetPerspectiveProjection( void )
{
  m_view_projection_orthographic->setChecked( false );
  m_view_projection_perspective->setChecked( true ) ;

  if( m_project )
  {
    std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
    if( mgr )
    {
      std::shared_ptr<Camera> cam = mgr->camera() ;
      if( cam )
      {
        cam->setOrtho( false ) ;
        m_result_view->update();
      }
    }
  }

}


/**
* @brief Action to be executed when user select orthographic projection matrix
*/
void MainWindow::onSetOrthographicProjection( void )
{
  m_view_projection_orthographic->setChecked( true );
  m_view_projection_perspective->setChecked( false ) ;

  std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
  if( m_project )
  {
    std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
    if( mgr )
    {
      std::shared_ptr<Camera> cam = mgr->camera() ;
      if( cam )
      {
        cam->setOrtho( true ) ;
        m_result_view->update();
      }
    }
  }
}

/**
* @brief Action to be executed when user want to compute clusters
*/
void MainWindow::onComputeClusters( void )
{
  qInfo( "Compute clusters" ) ;

  ClusterComputationSettingsDialog dlg( this ) ;
  int res = dlg.exec() ;
  if( res == QDialog::Accepted )
  {
    QThread * thread = new QThread( this ) ;

    const int lower_bound = dlg.clusterLowerBound() ;
    const int upper_bound = dlg.clusterUpperBound() ;
    const float voxel_size = dlg.clusterGridSize() ;

    m_worker_cluster_computation = std::make_shared<WorkerClusterComputation>( m_project , lower_bound , upper_bound , voxel_size ) ;
    m_worker_cluster_computation->moveToThread( thread ) ;

    int progress_min = 0 , progress_max = 0 ;
    m_worker_cluster_computation->progressRange( progress_min , progress_max ) ;
    createProgress( "Cluster computation, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( thread , SIGNAL( started() ) , m_worker_cluster_computation.get() , SLOT( process() ) ) ;
    connect( m_worker_cluster_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_cluster_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedClustering( const WorkerNextAction & ) ) ) ;
    connect( m_worker_cluster_computation.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

    thread->start() ;
  }
}

/**
* @brief Action to be executed when user want to export to openMVS
*/
void MainWindow::onExportToOpenMVS( void )
{
  qInfo( "Export to OpenMVS" ) ;

  const std::string output_folder = stlplus::folder_append_separator( m_project->projectPaths().exportPath() ) + "openMVS" ;
  const std::string output_file   = stlplus::create_filespec( output_folder , "scene.mvs" );
  const std::string output_undist_folder = stlplus::folder_append_separator( output_folder ) + "undist" ;

  if( stlplus::folder_exists( output_folder ) )
  {
    // Clean all existing data
    stlplus::folder_delete( output_folder , true ) ;
  }

  // Create output folder
  if( ! stlplus::folder_exists( output_folder ) )
  {
    if( ! stlplus::folder_create( output_folder ) )
    {
      onHasExportedToOpenMVS( NEXT_ACTION_ERROR ) ;
      return ;
    }
    if( ! stlplus::folder_exists( output_folder ) )
    {
      onHasExportedToOpenMVS( NEXT_ACTION_ERROR ) ;
      return ;
    }
  }

  //
  QThread * thread = new QThread( this ) ;

  m_worker_export_to_openMVS = std::make_shared<WorkerExportToOpenMVS>( m_project->SfMData() , output_file , output_undist_folder ) ;
  m_worker_export_to_openMVS->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_export_to_openMVS->progressRange( progress_min , progress_max ) ;
  createProgress( "Export to openMVS, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread , SIGNAL( started() ) , m_worker_export_to_openMVS.get() , SLOT( process() ) ) ;
  connect( m_worker_export_to_openMVS.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_export_to_openMVS.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasExportedToOpenMVS( const WorkerNextAction & ) ) ) ;
  connect( m_worker_export_to_openMVS.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to export to MVE
*/
void MainWindow::onExportToMVE( void )
{
  qInfo( "Export to MVE" ) ;

  const std::string output_folder_path = stlplus::folder_append_separator( m_project->projectPaths().exportPath() ) + "MVE" ;

  // If something exists, remove the folder
  if( stlplus::folder_exists( output_folder_path ) )
  {
    stlplus::folder_delete( output_folder_path ) ;
  }

  // Create folder
  if( ! stlplus::folder_exists( output_folder_path ) )
  {
    if( ! stlplus::folder_create( output_folder_path ) )
    {
      onHasExportedToMVE( NEXT_ACTION_ERROR ) ;
      return ;
    }
    if( ! stlplus::folder_exists( output_folder_path ) )
    {
      onHasExportedToMVE( NEXT_ACTION_ERROR ) ;
      return ;
    }
  }

  QThread * thread = new QThread( this ) ;

  m_worker_export_to_MVE = std::make_shared<WorkerExportToMVE>( m_project->SfMData() , output_folder_path ) ;
  m_worker_export_to_MVE->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_export_to_MVE->progressRange( progress_min , progress_max ) ;
  createProgress( "Export to MVE, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread , SIGNAL( started() ) , m_worker_export_to_MVE.get() , SLOT( process() ) ) ;
  connect( m_worker_export_to_MVE.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_export_to_MVE.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasExportedToMVE( const WorkerNextAction & ) ) ) ;
  connect( m_worker_export_to_MVE.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to export to PMVS
*/
void MainWindow::onExportToPMVS( void )
{
  qInfo( "Export to PMVS" ) ;

  const std::string output_folder_path = stlplus::folder_append_separator( m_project->projectPaths().exportPath() ) + "PMVS" ;

  // If something exists, remove the folder
  if( stlplus::folder_exists( output_folder_path ) )
  {
    stlplus::folder_delete( output_folder_path ) ;
  }

  // Create folder
  if( ! stlplus::folder_exists( output_folder_path ) )
  {
    if( ! stlplus::folder_create( output_folder_path ) )
    {
      onHasExportedToPMVS( NEXT_ACTION_ERROR ) ;
      return ;
    }
    if( ! stlplus::folder_exists( output_folder_path ) )
    {
      onHasExportedToPMVS( NEXT_ACTION_ERROR ) ;
      return ;
    }
  }

  const int resize_factor = 1 ;
  const int cpu_count = std::thread::hardware_concurrency() == 0 ? 4 : std::thread::hardware_concurrency() ;

  QThread * thread = new QThread( this ) ;

  m_worker_export_to_PMVS = std::make_shared<WorkerExportToPMVS>( m_project->SfMData() , output_folder_path , resize_factor , cpu_count , true ) ;
  m_worker_export_to_PMVS->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_export_to_PMVS->progressRange( progress_min , progress_max ) ;
  createProgress( "Export to PMVS, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread , SIGNAL( started() ) , m_worker_export_to_PMVS.get() , SLOT( process() ) ) ;
  connect( m_worker_export_to_PMVS.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_export_to_PMVS.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasExportedToPMVS( const WorkerNextAction & ) ) ) ;
  connect( m_worker_export_to_PMVS.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to export each clusters to openMVS
*/
void MainWindow::onExportClustersToOpenMVS( void )
{
  qInfo( "Export clusters to OpenMVS" ) ;

  const std::string output_folder = stlplus::folder_append_separator( m_project->projectPaths().exportPath() ) + "clusters_openMVS" ;
  const std::string clusters_path  = stlplus::folder_append_separator( m_project->projectPaths().exportPath() ) + "clusters" ;

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
      onHasExportedToPMVS( NEXT_ACTION_ERROR ) ;
      return ;
    }
    if( ! stlplus::folder_exists( output_folder ) )
    {
      onHasExportedToPMVS( NEXT_ACTION_ERROR ) ;
      return ;
    }
  }

  m_worker_export_clusters_to_MVS = std::make_shared<WorkerExportClustersToMVS>( clusters_path , output_folder , MVS_EXPORTER_OPENMVS ) ;
  int min, max ;
  m_worker_export_clusters_to_MVS->progressRangeOverall( min , max ) ;

  m_double_progress_dialog = new DoubleProgressBarDialog( this ) ;
  m_double_progress_dialog->setRange1( min , max ) ;
  m_double_progress_dialog->setValue1( 0 ) ;
  m_double_progress_dialog->setLabelText2( "Current step progress" ) ;
  m_double_progress_dialog->setWindowModality( Qt::WindowModal ) ;
  m_double_progress_dialog->show();

  QThread * thread = new QThread( this ) ;
  m_worker_export_clusters_to_MVS->moveToThread( thread ) ;

  connect( thread , SIGNAL( started() ) , m_worker_export_clusters_to_MVS.get() , SLOT( process() ) ) ;
  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasExportedClustersToMVS( const WorkerNextAction & ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( finished( const WorkerNextAction & ) ) , thread , SLOT( quit() ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( progressOverall( int ) ) , m_double_progress_dialog , SLOT( setValue1( int ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( progressCurrentStage( int ) ) , m_double_progress_dialog , SLOT( setValue2( int ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( progressRangeCurrentStage( int , int ) ) , m_double_progress_dialog , SLOT( setRange2( int , int ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( messageCurrentStage( const std::string & ) ) , m_double_progress_dialog , SLOT( setLabelText1( const std::string & ) ) ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to export each clusters to MVE
*/
void MainWindow::onExportClustersToMVE( void )
{
  qInfo( "Export clusters to MVE" ) ;

  const std::string output_folder = stlplus::folder_append_separator( m_project->projectPaths().exportPath() ) + "clusters_openMVE" ;
  const std::string clusters_path  = stlplus::folder_append_separator( m_project->projectPaths().exportPath() ) + "clusters" ;

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
      onHasExportedToPMVS( NEXT_ACTION_ERROR ) ;
      return ;
    }
    if( ! stlplus::folder_exists( output_folder ) )
    {
      onHasExportedToPMVS( NEXT_ACTION_ERROR ) ;
      return ;
    }
  }

  m_worker_export_clusters_to_MVS = std::make_shared<WorkerExportClustersToMVS>( clusters_path , output_folder , MVS_EXPORTER_MVE ) ;
  int min, max ;
  m_worker_export_clusters_to_MVS->progressRangeOverall( min , max ) ;

  m_double_progress_dialog = new DoubleProgressBarDialog( this ) ;
  m_double_progress_dialog->setRange1( min , max ) ;
  m_double_progress_dialog->setValue1( 0 ) ;
  m_double_progress_dialog->setLabelText2( "Current step progress" ) ;
  m_double_progress_dialog->setWindowModality( Qt::WindowModal ) ;
  m_double_progress_dialog->show();

  QThread * thread = new QThread( this ) ;
  m_worker_export_clusters_to_MVS->moveToThread( thread ) ;

  connect( thread , SIGNAL( started() ) , m_worker_export_clusters_to_MVS.get() , SLOT( process() ) ) ;
  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasExportedClustersToMVS( const WorkerNextAction & ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( finished( const WorkerNextAction & ) ) , thread , SLOT( quit() ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( progressOverall( int ) ) , m_double_progress_dialog , SLOT( setValue1( int ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( progressCurrentStage( int ) ) , m_double_progress_dialog , SLOT( setValue2( int ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( progressRangeCurrentStage( int , int ) ) , m_double_progress_dialog , SLOT( setRange2( int , int ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( messageCurrentStage( const std::string & ) ) , m_double_progress_dialog , SLOT( setLabelText1( const std::string & ) ) ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to export each clusters to PMVS
*/
void MainWindow::onExportClustersToPMVS( void )
{
  qInfo( "Export clusters to PMVS" ) ;

  const std::string output_folder = stlplus::folder_append_separator( m_project->projectPaths().exportPath() ) + "clusters_PMVS" ;
  const std::string clusters_path  = stlplus::folder_append_separator( m_project->projectPaths().exportPath() ) + "clusters" ;

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
      onHasExportedToPMVS( NEXT_ACTION_ERROR ) ;
      return ;
    }
    if( ! stlplus::folder_exists( output_folder ) )
    {
      onHasExportedToPMVS( NEXT_ACTION_ERROR ) ;
      return ;
    }
  }

  m_worker_export_clusters_to_MVS = std::make_shared<WorkerExportClustersToMVS>( clusters_path , output_folder , MVS_EXPORTER_PMVS ) ;
  int min, max ;
  m_worker_export_clusters_to_MVS->progressRangeOverall( min , max ) ;

  m_double_progress_dialog = new DoubleProgressBarDialog( this ) ;
  m_double_progress_dialog->setRange1( min , max ) ;
  m_double_progress_dialog->setValue1( 0 ) ;
  m_double_progress_dialog->setLabelText2( "Current step progress" ) ;
  m_double_progress_dialog->setWindowModality( Qt::WindowModal ) ;
  m_double_progress_dialog->show();

  QThread * thread = new QThread( this ) ;
  m_worker_export_clusters_to_MVS->moveToThread( thread ) ;

  connect( thread , SIGNAL( started() ) , m_worker_export_clusters_to_MVS.get() , SLOT( process() ) ) ;
  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasExportedClustersToMVS( const WorkerNextAction & ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( finished( const WorkerNextAction & ) ) , thread , SLOT( quit() ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( progressOverall( int ) ) , m_double_progress_dialog , SLOT( setValue1( int ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( progressCurrentStage( int ) ) , m_double_progress_dialog , SLOT( setValue2( int ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( progressRangeCurrentStage( int , int ) ) , m_double_progress_dialog , SLOT( setRange2( int , int ) ) ) ;
  connect( m_worker_export_clusters_to_MVS.get() , SIGNAL( messageCurrentStage( const std::string & ) ) , m_double_progress_dialog , SLOT( setLabelText1( const std::string & ) ) ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when features have been computed
*/
void MainWindow::onHasComputedFeatures( const WorkerNextAction & next_action  )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  m_worker_features_computation.reset() ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    QMessageBox::critical( this , "Error" , "There was an error during feature computation" ) ;
    return ;
  }


  postFeaturesComputation() ;
}

/**
* @brief Action to be executed when features_provider has been loaded
* @note this is before incremental SfM
*/
void MainWindow::onHasLoadedFeatures( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    m_worker_features_provider_load.reset() ;
    QMessageBox::critical( this , "Error" , "There was an error during feature loading" ) ;
    return ;
  }

  if( contains( next_action , NEXT_ACTION_LOAD_MATCHES ) )
  {
    // Load matches
    std::string match_name ;
    // Get the first available matche file
    if( m_project->sfMMethod() == SFM_METHOD_INCREMENTAL )
    {
      if( m_project->hasMatchesFundamentalFiltered() )
      {
        match_name = "matches.f.bin" ;
      }
      else if( m_project->hasMatchesEssentialFiltered() )
      {
        match_name = "matches.e.bin" ;
      }
      else if( m_project->hasMatchesHomographyFiltered() )
      {
        match_name = "matches.h.bin" ;
      }
      else
      {
        // TODO : error !
      }
    }
    else // method global
    {
      if( m_project->hasMatchesEssentialFiltered() )
      {
        match_name = "matches.e.bin" ;
      }
      else if( m_project->hasMatchesFundamentalFiltered() )
      {
        match_name = "matches.f.bin" ;
      }
      else if( m_project->hasMatchesHomographyFiltered() )
      {
        match_name = "matches.h.bin" ;
      }
      else
      {
        // TODO : error !
      }
    }
    m_worker_matches_provider_load = std::make_shared<WorkerMatchesProviderLoad>( m_project , match_name , remove( next_action , NEXT_ACTION_LOAD_MATCHES ) ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_matches_provider_load->moveToThread( thread ) ;

    int progress_min = 0 , progress_max = 0 ;
    m_worker_matches_provider_load->progressRange( progress_min , progress_max ) ;
    createProgress( "Loading matches, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( started() ) , m_worker_matches_provider_load.get() , SLOT( process() ) ) ;
    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( m_worker_matches_provider_load.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_matches_provider_load.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasLoadedMatches( const WorkerNextAction & ) ) ) ;
    connect( m_worker_matches_provider_load.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;


    thread->start() ;
  }
  else
  {
    m_worker_features_provider_load.reset() ;
  }
}

/**
* @brief Action to be executed when matches_provider has been loaded
*/
void MainWindow::onHasLoadedMatches( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    m_worker_matches_provider_load.reset() ;
    QMessageBox::critical( this , "Error" , "There was an error during matches loading" ) ;
    return ;
  }


  // Compute SfM
  if( contains( next_action , NEXT_ACTION_COMPUTE_INCREMENTAL_SFM ) )
  {
    qInfo( "onHasLoadedMatches -> Compute incremental SfM" ) ;
    // TODO : Message if it has already computed something
    const bool reload_initial_intrinsic = true ;

    // Incremental
    std::shared_ptr<Project> proj = m_project ;
    std::shared_ptr<openMVG::sfm::Features_Provider> features_provider = m_worker_features_provider_load->featuresProvider() ;
    std::shared_ptr<openMVG::sfm::Matches_Provider> matches_provider = m_worker_matches_provider_load->matchesProvider() ;

    m_worker_incremental_sfm_computation = std::make_shared<WorkerIncrementalSfMComputation>( proj , features_provider , matches_provider , reload_initial_intrinsic , remove( next_action , NEXT_ACTION_COMPUTE_INCREMENTAL_SFM ) ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_incremental_sfm_computation->moveToThread( thread ) ;

    int progress_min = 0 , progress_max = 0 ;
    m_worker_incremental_sfm_computation->progressRange( progress_min , progress_max ) ;
    createProgress( "Incremental SfM computation, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( started() ) , m_worker_incremental_sfm_computation.get() , SLOT( process() ) ) ;
    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( m_worker_incremental_sfm_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_incremental_sfm_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedSfM( const WorkerNextAction & ) ) ) ;
    connect( m_worker_incremental_sfm_computation.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

    thread->start() ;
  }
  else if( contains( next_action , NEXT_ACTION_COMPUTE_GLOBAL_SFM ) )
  {
    // TODO : Message if it has already computed something
    const bool reload_initial_intrinsic = true ;

    // Incremental
    std::shared_ptr<Project> proj = m_project ;
    std::shared_ptr<openMVG::sfm::Features_Provider> features_provider = m_worker_features_provider_load->featuresProvider() ;
    std::shared_ptr<openMVG::sfm::Matches_Provider> matches_provider = m_worker_matches_provider_load->matchesProvider() ;

    m_worker_global_sfm_computation = std::make_shared<WorkerGlobalSfMComputation>( proj , features_provider , matches_provider , reload_initial_intrinsic , remove( next_action , NEXT_ACTION_COMPUTE_GLOBAL_SFM ) ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_global_sfm_computation->moveToThread( thread ) ;

    int progress_min = 0 , progress_max = 0 ;
    m_worker_global_sfm_computation->progressRange( progress_min , progress_max ) ;
    createProgress( "Global SfM computation, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( started() ) , m_worker_global_sfm_computation.get() , SLOT( process() ) ) ;
    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( m_worker_global_sfm_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_global_sfm_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedSfM( const WorkerNextAction & ) ) ) ;
    connect( m_worker_global_sfm_computation.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

    thread->start() ;
  }
  else
  {
    m_worker_matches_provider_load.reset() ;
  }
}

/**
* @brief Action to be executed when regions are loaded
*/
void MainWindow::onHasLoadedRegions( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    m_worker_regions_provide_load.reset() ;
    QMessageBox::critical( this , "Error" , "There was an error during region loading" ) ;
    return ;
  }


  if( contains( next_action , NEXT_ACTION_COMPUTE_MATCHES ) )
  {
    // Now launch matches computation
    std::shared_ptr<openMVG::sfm::Regions_Provider> regions_provider = m_worker_regions_provide_load->regionsProvider() ;
    m_worker_matches_computation = std::make_shared<WorkerMatchesComputation>( m_project , regions_provider , remove( next_action , NEXT_ACTION_COMPUTE_MATCHES ) ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_matches_computation->moveToThread( thread ) ;

    int progress_min = 0 , progress_max = 0 ;
    m_worker_matches_computation->progressRange( progress_min , progress_max ) ;
    createProgress( "Matches computation, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( started() ) , m_worker_matches_computation.get() , SLOT( process( ) ) ) ;
    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( m_worker_matches_computation.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;
    connect( m_worker_matches_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_matches_computation.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedMatches( const WorkerNextAction & ) ) ) ;

    thread->start() ;
  }
  else
  {
    m_worker_regions_provide_load.reset() ;
  }
}


/**
* @brief Action to be executed when matches have been computed
*/
void MainWindow::onHasComputedMatches( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    m_worker_matches_computation.reset() ;
    QMessageBox::critical( this , "Error" , "There was an error during matches computation" ) ;
    return ;
  }

  if( contains( next_action , NEXT_ACTION_COMPUTE_GEOMETRIC_FILTERING ) )
  {
    std::shared_ptr<openMVG::sfm::Regions_Provider> regions_provider = m_worker_regions_provide_load->regionsProvider() ;
    std::shared_ptr<openMVG::matching::PairWiseMatches> putative_matches = m_worker_matches_computation->putativeMatches() ;

    m_worker_geometric_filtering = std::make_shared<WorkerGeometricFiltering>( m_project , regions_provider , putative_matches , NEXT_ACTION_NONE ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_geometric_filtering->moveToThread( thread ) ;

    int progress_min = 0 , progress_max = 0 ;
    m_worker_geometric_filtering->progressRange( progress_min , progress_max ) ;
    createProgress( "Geometric filtering, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( thread , SIGNAL( started() ) , m_worker_geometric_filtering.get() , SLOT( process() ) ) ;
    connect( m_worker_geometric_filtering.get() , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;
    connect( m_worker_geometric_filtering.get() , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_geometric_filtering.get() , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasDoneGeometricFiltering( const WorkerNextAction & ) ) ) ;

    thread->start() ;
  }
  else
  {
    m_worker_matches_computation.reset() ;
  }
}

/**
* @brief Action to be executed when geometric filtering has been computed
*/
void MainWindow::onHasDoneGeometricFiltering( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  m_worker_regions_provide_load.reset() ;
  m_worker_geometric_filtering.reset() ;
  m_worker_matches_computation.reset() ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    QMessageBox::critical( this , "Error" , "There was an error during geometric filtering" ) ;
    return ;
  }

  m_state = STATE_MATCHES_COMPUTED ;
  updateInterface() ;

  postMatchesComputation() ;
}

/**
* @brief Action to be executed when clustering has been computed
*/
void MainWindow::onHasComputedClustering( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  m_worker_cluster_computation.reset() ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    QMessageBox::critical( this , "Error" , "There was an error during clustering" ) ;
    return ;
  }

  m_state = STATE_CLUSTERING_COMPUTED ;
  updateInterface() ;
}

/**
* @brief Action to be executed when exporting to openMVS has been computed
*/
void MainWindow::onHasExportedToOpenMVS( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  m_worker_export_to_openMVS.reset() ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    QMessageBox::critical( this , "Error" , "There was an error during export to openMVS" ) ;
    return ;
  }

  QMessageBox::information( this , "Information" , "Project exported to the \"export/openMVS\" folder inside the project folder" ) ;

}

/**
* @brief Action to be executed when exporting to MVE has been done
*/
void MainWindow::onHasExportedToMVE( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  m_worker_export_to_MVE.reset() ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    QMessageBox::critical( this , "Error" , "There was an error during export to MVE" ) ;
    return ;
  }

  QMessageBox::information( this , "Information" , "Project exported to the \"export/MVE\" folder inside the project folder" ) ;
}


/**
* @brief Action to be executed when exporting to PMVS has been done
*/
void MainWindow::onHasExportedToPMVS( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  m_worker_export_to_PMVS.reset() ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    QMessageBox::critical( this , "Error" , "There was an error during export to PMVS" ) ;
    return ;
  }

  QMessageBox::information( this , "Information" , "Project exported to the \"export/PMVS\" folder inside the project folder" ) ;
}

/**
 * @brief Action to be executed when exporting to PMVS has been done
 */
void MainWindow::onHasExportedClustersToMVS( const WorkerNextAction & next_action )
{
  m_double_progress_dialog->hide() ;
  delete m_double_progress_dialog ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    QMessageBox::critical( this , "Error" , "There was an error during export of the clusters" ) ;
    m_worker_export_clusters_to_MVS.reset() ;
    return ;
  }

  switch( m_worker_export_clusters_to_MVS->method() )
  {
    case MVS_EXPORTER_MVE :
    {
      QMessageBox::information( this , "Information" , "Project exported to the \"export/clusters_MVE\" folder inside the project folder" ) ;
      break ;
    }
    case MVS_EXPORTER_OPENMVS:
    {
      QMessageBox::information( this , "Information" , "Project exported to the \"export/clusters_openMVS\" folder inside the project folder" ) ;
      break ;
    }
    case MVS_EXPORTER_PMVS:
    {
      QMessageBox::information( this , "Information" , "Project exported to the \"export/clusters_PMVS\" folder inside the project folder" ) ;
      break ;
    }
  }

  m_worker_export_clusters_to_MVS.reset() ;
}


/**
* @brief Action to be executed when SfM has been computed
*/
void MainWindow::onHasComputedSfM( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  m_worker_features_provider_load.reset() ;
  m_worker_matches_provider_load.reset() ;

  if( m_project->sfMMethod() == SFM_METHOD_INCREMENTAL )
  {
    m_worker_incremental_sfm_computation.reset() ;
  }
  else
  {
    // Global
    m_worker_global_sfm_computation.reset() ;
  }

  if( next_action == NEXT_ACTION_ERROR )
  {
    QMessageBox::critical( this , "Error" , "There was an error during SfM computation" ) ;
    return ;
  }

  postSfMComputation() ;
}

void MainWindow::onHasComputedColor( const WorkerNextAction & next_action  )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  m_worker_color_computation.reset() ;

  if( next_action == NEXT_ACTION_ERROR )
  {
    QMessageBox::critical( this , "Error" , "There was an error during color computation" ) ;
    return ;
  }

  postColorComputation() ;
}

/**
* @brief Action to be executed when automatic reconstruction is done
*/
void MainWindow::onHasDoneAutomaticReconstruction( const WorkerNextAction & next_action )
{
  if( next_action == NEXT_ACTION_ERROR )
  {
    QMessageBox::critical( this , "Error" , "There was an error during automatic reconstruction" ) ;

    m_worker_automatic_reconstruction.reset() ;

    m_project.reset() ;
    resetInterface() ;

    return ;
  }

  m_project = m_worker_automatic_reconstruction->project() ;

  m_double_progress_dialog->hide() ;
  delete m_double_progress_dialog ;

  m_worker_automatic_reconstruction.reset() ;

  m_feature_viewer_widget->setProject( m_project ) ;

  // Initialize the 3d view
  m_result_view->setScene( m_project->sceneManager() );
  m_result_view->prepareObjects() ;
  m_result_view->updateTrackballSize() ;

  m_result_view->update() ;

  onUpdateImageList() ;
  postFeaturesComputation() ;
  postMatchesComputation() ;
  postSfMComputation() ;
  postColorComputation() ;
}


/**
* @brief indicate if some parameters in the project are not saved on disk
*/
bool MainWindow::hasUnsavedChange( void ) const
{
  if( m_project )
  {
    return m_project->hasUnsavedChange() ;
  }
  return false ;
}

/**
* @brief Post actions to be executed after feature computation (or feature computation failure)
*/
void MainWindow::postFeaturesComputation( void )
{
  /**
  * Select a matching method that is compatible with the features computed
  */
  const std::string featuresPath = m_project->projectPaths().featuresPath( m_project->featureParams() ) ;
  const std::string describerPath = stlplus::create_filespec( featuresPath , "image_describer.json" ) ;
  if( stlplus::file_exists( describerPath ) )
  {
    if( isBinaryDescriber( describerPath ) )
    {
      m_project->matchingParams().setMethod( MATCHING_METHOD_BRUTE_FORCE_HAMMING ) ;
    }
    else
    {
      if( m_project->matchingParams().method() == MATCHING_METHOD_BRUTE_FORCE_HAMMING )
      {
        m_project->matchingParams().setMethod( MATCHING_METHOD_FAST_CASCADE_HASHING_L2 ) ;
      }
    }
  }
  else
  {
    //no image_describer -> no features computed -> so we can exit now
    return ;
  }

  std::vector< std::string > valid_features_path = m_project->projectPaths().featuresPaths() ;
  for( const auto & feature_path : valid_features_path )
  {
    std::map< std::string , FeaturesStats > infos ;
    const std::vector< std::pair< int , std::string > > images_path = m_project->GetImageNames() ;
    for( const auto & cur_image_path : images_path )
    {
      const std::string sStat = stlplus::create_filespec( feature_path, stlplus::basename_part( cur_image_path.second ), "stat" ) ;
      if( stlplus::file_exists( sStat ) )
      {
        FeaturesStats cur_stat = FeaturesStats::load( sStat ) ;
        infos[cur_image_path.second] = cur_stat ;
      }
    }
    // Add to the detail path
    if( infos.size() > 0 )
    {
      std::vector<std::string> feature_hierarchy = stlplus::folder_elements( feature_path );
      while( feature_hierarchy.size() > 3 )
      {
        feature_hierarchy.erase( feature_hierarchy.begin() ) ;
      }
      m_detail_list->setFeaturesInfos( feature_hierarchy , infos ) ;
    }

    // Update feature viewer
    m_feature_viewer_widget->setProject( m_project ) ;
  }



  /**
  * @brief Update interface
  */
  m_state = STATE_FEATURES_COMPUTED ;
  updateInterface() ;
}

/**
* @brief Post actions to be executed after matches computation (or matches computation failure)
*/
void MainWindow::postMatchesComputation( void )
{
  // Load matches statistics
  std::string matching_path = m_project->projectPaths().featuresPath( m_project->featureParams() ) ;
  std::vector<std::string> matching_hierarchy = stlplus::folder_elements( matching_path );
  while( matching_hierarchy.size() > 3 )
  {
    matching_hierarchy.erase( matching_hierarchy.begin() ) ;
  }


  const std::string path_filtered = stlplus::create_filespec( matching_path , "matches.filtered.stat" ) ;
  const std::string path_matched  = stlplus::create_filespec( matching_path , "matches.putative.stat" ) ;

  if( stlplus::file_exists( path_filtered ) )
  {
    MatchingStats infos = MatchingStats::load( path_filtered ) ;
    m_detail_list->setMatchesInfos( matching_hierarchy , infos ) ;
    m_state = STATE_MATCHES_COMPUTED ;
  }
  else if( stlplus::file_exists( path_matched ) )
  {
    MatchingStats infos = MatchingStats::load( path_matched ) ;
    m_detail_list->setMatchesInfos( matching_hierarchy , infos ) ;
    m_state = STATE_MATCHES_COMPUTED ;
  }

  updateInterface() ;
}

/**
* @brief Post actions to be executed after sfm computation (or sfm computation failure)
*/
void MainWindow::postSfMComputation( void )
{
  // 1 - Load point cloud to the interface

  // Remove old object in the project
  std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
  mgr->removePointClouds() ;

  // Add the camera gizmos
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm = m_project->SfMData() ;
  if( sfm )
  {
    std::shared_ptr<RenderableObject> sprs = std::make_shared<PointCloud>( m_result_view->getContext() , m_result_view->pointShader() , sfm ) ;
    mgr->addObject( sprs ) ;
    m_project->setSparsePointCloud( sprs ) ;

    mgr->removeCameraGizmos() ;
    std::map<int, std::shared_ptr<RenderableObject>> cam_gizmos ;

    // Get all spherical cameras
    std::map<int, bool> map_is_spherical ;
    for( auto & cur_view : sfm->GetViews() )
    {
      const auto & cur_intrin = sfm->GetIntrinsics().at( cur_view.second->id_intrinsic ) ;
      const bool is_spherical = std::dynamic_pointer_cast<openMVG::cameras::Intrinsic_Spherical>( cur_intrin ) != nullptr ;
      map_is_spherical.insert( { cur_view.second->id_pose , is_spherical } );
    }

    for( auto & cur_pose : sfm->GetPoses() )
    {
      const bool is_spherical = map_is_spherical[ cur_pose.first ] ;
      cam_gizmos[ cur_pose.first ] = std::make_shared<CameraGizmo>( m_result_view->getContext() , m_result_view->pointShader() , cur_pose.second , is_spherical , 0.1 ) ;
    }
    mgr->setCameraGizmos( cam_gizmos ) ;
  }

  m_result_view->prepareObjects() ;
  m_result_view->updateTrackballSize() ;
  m_result_view->update() ;

  // 2 - Load statistics from file
  const std::string summaryPath = m_project->projectPaths().htmlReportPath( m_project->sfMMethod() );
  m_result_summary_widget->setPath( summaryPath ) ;

  // 3 - Update interface
  m_state = STATE_SFM_COMPUTED ;
  updateInterface() ;
}

/**
* @brief Post actions to be executed after color computation (or color computation failure)
*/
void MainWindow::postColorComputation( void )
{
  // Remove old object
  std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
  mgr->removePointClouds() ;

  // Load the colorized one
  const std::string colorized = m_project->projectPaths().colorizedPlyCloud( m_project->sfMMethod() ) ;
  if( stlplus::file_exists( colorized ) )
  {
    // Add colorized object
    std::vector< openMVG::Vec3 > pts ;
    std::vector< openMVG::Vec3 > col ;
    LoadPly( colorized , pts , col ) ;

    std::shared_ptr<RenderableObject> sprs = std::make_shared<PointCloud>( m_result_view->getContext() , m_result_view->pointShader() , pts , col ) ;
    mgr->addObject( sprs ) ;
    m_project->setSparsePointCloud( sprs ) ;
    m_result_view->prepareObjects() ;
    m_result_view->updateTrackballSize() ;
    m_result_view->update() ;

    m_state = STATE_COLOR_COMPUTED ;
    // Load statistics
  }

  updateInterface() ;
}

/**
* @brief Set the interface exactly the same as after launching the application
* -> make the interface into a clean step
*/
void MainWindow::resetInterface( void )
{
  m_feature_viewer_widget->setProject( nullptr ) ;

  m_result_view->setScene( nullptr ) ;
  m_result_view->update() ;

  m_image_list->clear() ;
  m_detail_list->clear() ;

  m_state = STATE_EMPTY ;
  updateInterface() ;
}


/**
* @brief Update interface based on the state of the application
*/
void MainWindow::updateInterface( void )
{
  if( ! m_project )
  {
    // No project -> clear everything in a startup state
    m_state = STATE_EMPTY ;
  }

  switch( m_state )
  {
    case STATE_EMPTY :
    {
      // Can do nothing except Opening or Creating a new project
      m_file_save_act->setEnabled( false ) ;
      m_file_save_act_tb->setEnabled( false ) ;
      m_file_close_act->setEnabled( false ) ;

      m_compute_features_act->setEnabled( false ) ;
      m_compute_features_act_tb->setEnabled( false ) ;
      m_compute_matches_act->setEnabled( false ) ;
      m_compute_matches_act_tb->setEnabled( false ) ;
      m_compute_sfm_act->setEnabled( false ) ;
      m_compute_sfm_act_tb->setEnabled( false ) ;
      m_compute_color_act->setEnabled( false ) ;
      m_compute_color_act_tb->setEnabled( false ) ;

      m_export_to_clusters_act->setEnabled( false ) ;
      m_export_to_openMVS_act->setEnabled( false ) ;
      m_export_to_MVE_act->setEnabled( false ) ;
      m_export_to_PMVS_act->setEnabled( false ) ;
      m_clustered_exports_menu->setEnabled( false ) ;
      m_export_to_clusters_openMVS_act->setEnabled( false ) ;
      m_export_to_clusters_MVE_act->setEnabled( false ) ;
      m_export_to_clusters_PMVS_act->setEnabled( false ) ;

      m_setting_intrinsics_act->setEnabled( false ) ;
      m_setting_features_act->setEnabled( false );
      m_setting_matches_act->setEnabled( false ) ;
      m_setting_sfm_act->setEnabled( false ) ;

      break ;
    }
    case STATE_PROJECT_OPENED :
    {
      // Can compute features
      m_file_save_act->setEnabled( true ) ;
      m_file_save_act_tb->setEnabled( true ) ;
      m_file_close_act->setEnabled( true ) ;

      m_compute_features_act->setEnabled( true ) ;
      m_compute_features_act_tb->setEnabled( true ) ;
      m_compute_matches_act->setEnabled( false ) ;
      m_compute_matches_act_tb->setEnabled( false ) ;
      m_compute_sfm_act->setEnabled( false ) ;
      m_compute_sfm_act_tb->setEnabled( false ) ;
      m_compute_color_act->setEnabled( false ) ;
      m_compute_color_act_tb->setEnabled( false ) ;

      m_export_to_clusters_act->setEnabled( false ) ;
      m_export_to_openMVS_act->setEnabled( false ) ;
      m_export_to_MVE_act->setEnabled( false ) ;
      m_export_to_PMVS_act->setEnabled( false ) ;
      m_clustered_exports_menu->setEnabled( false ) ;
      m_export_to_clusters_openMVS_act->setEnabled( false ) ;
      m_export_to_clusters_MVE_act->setEnabled( false ) ;
      m_export_to_clusters_PMVS_act->setEnabled( false ) ;

      m_setting_intrinsics_act->setEnabled( true ) ;
      m_setting_features_act->setEnabled( true );
      m_setting_matches_act->setEnabled( true ) ;
      m_setting_sfm_act->setEnabled( true ) ;

      break ;
    }
    case STATE_FEATURES_COMPUTED :
    {
      // Can compute matches
      m_file_save_act->setEnabled( true ) ;
      m_file_save_act_tb->setEnabled( true ) ;
      m_file_close_act->setEnabled( true ) ;

      m_compute_features_act->setEnabled( true ) ;
      m_compute_features_act_tb->setEnabled( true ) ;
      m_compute_matches_act->setEnabled( true ) ;
      m_compute_matches_act_tb->setEnabled( true ) ;
      m_compute_sfm_act->setEnabled( false ) ;
      m_compute_sfm_act_tb->setEnabled( false ) ;
      m_compute_color_act->setEnabled( false ) ;
      m_compute_color_act_tb->setEnabled( false ) ;

      m_export_to_clusters_act->setEnabled( false ) ;
      m_export_to_openMVS_act->setEnabled( false ) ;
      m_export_to_MVE_act->setEnabled( false ) ;
      m_export_to_PMVS_act->setEnabled( false ) ;
      m_clustered_exports_menu->setEnabled( false ) ;
      m_export_to_clusters_openMVS_act->setEnabled( false ) ;
      m_export_to_clusters_MVE_act->setEnabled( false ) ;
      m_export_to_clusters_PMVS_act->setEnabled( false ) ;

      m_setting_intrinsics_act->setEnabled( true ) ;
      m_setting_features_act->setEnabled( true );
      m_setting_matches_act->setEnabled( true ) ;
      m_setting_sfm_act->setEnabled( true ) ;


      break ;
    }
    case STATE_MATCHES_COMPUTED :
    {
      // Can compute SfM
      m_file_save_act->setEnabled( true ) ;
      m_file_save_act_tb->setEnabled( true ) ;
      m_file_close_act->setEnabled( true ) ;

      m_compute_features_act->setEnabled( true ) ;
      m_compute_features_act_tb->setEnabled( true ) ;
      m_compute_matches_act->setEnabled( true ) ;
      m_compute_matches_act_tb->setEnabled( true ) ;
      m_compute_sfm_act->setEnabled( true ) ;
      m_compute_sfm_act_tb->setEnabled( true ) ;
      m_compute_color_act->setEnabled( false ) ;
      m_compute_color_act_tb->setEnabled( false ) ;

      m_export_to_clusters_act->setEnabled( false ) ;
      m_export_to_openMVS_act->setEnabled( false ) ;
      m_export_to_MVE_act->setEnabled( false ) ;
      m_export_to_PMVS_act->setEnabled( false ) ;
      m_clustered_exports_menu->setEnabled( false ) ;
      m_export_to_clusters_openMVS_act->setEnabled( false ) ;
      m_export_to_clusters_MVE_act->setEnabled( false ) ;
      m_export_to_clusters_PMVS_act->setEnabled( false ) ;

      m_setting_intrinsics_act->setEnabled( true ) ;
      m_setting_features_act->setEnabled( true );
      m_setting_matches_act->setEnabled( true ) ;
      m_setting_sfm_act->setEnabled( true ) ;

      break ;
    }
    case STATE_SFM_COMPUTED :
    {
      // Can compute color
      m_file_save_act->setEnabled( true ) ;
      m_file_save_act_tb->setEnabled( true ) ;
      m_file_close_act->setEnabled( true ) ;

      m_compute_features_act->setEnabled( true ) ;
      m_compute_features_act_tb->setEnabled( true ) ;
      m_compute_matches_act->setEnabled( true ) ;
      m_compute_matches_act_tb->setEnabled( true ) ;
      m_compute_sfm_act->setEnabled( true ) ;
      m_compute_sfm_act_tb->setEnabled( true ) ;
      m_compute_color_act->setEnabled( true ) ;
      m_compute_color_act_tb->setEnabled( true ) ;

      m_export_to_clusters_act->setEnabled( true ) ;
      m_export_to_openMVS_act->setEnabled( true ) ;
      m_export_to_MVE_act->setEnabled( true ) ;
      m_export_to_PMVS_act->setEnabled( true ) ;
      m_clustered_exports_menu->setEnabled( false ) ;
      m_export_to_clusters_openMVS_act->setEnabled( false ) ;
      m_export_to_clusters_MVE_act->setEnabled( false ) ;
      m_export_to_clusters_PMVS_act->setEnabled( false ) ;

      m_setting_intrinsics_act->setEnabled( true ) ;
      m_setting_features_act->setEnabled( true );
      m_setting_matches_act->setEnabled( true ) ;
      m_setting_sfm_act->setEnabled( true ) ;

      break ;
    }
    case STATE_COLOR_COMPUTED :
    {
      // Have done everything
      m_file_save_act->setEnabled( true ) ;
      m_file_save_act_tb->setEnabled( true ) ;
      m_file_close_act->setEnabled( true ) ;

      m_compute_features_act->setEnabled( true ) ;
      m_compute_features_act_tb->setEnabled( true ) ;
      m_compute_matches_act->setEnabled( true ) ;
      m_compute_matches_act_tb->setEnabled( true ) ;
      m_compute_sfm_act->setEnabled( true ) ;
      m_compute_sfm_act_tb->setEnabled( true ) ;
      m_compute_color_act->setEnabled( true ) ;
      m_compute_color_act_tb->setEnabled( true ) ;

      m_export_to_clusters_act->setEnabled( true ) ;
      m_export_to_openMVS_act->setEnabled( true ) ;
      m_export_to_MVE_act->setEnabled( true ) ;
      m_export_to_PMVS_act->setEnabled( true ) ;
      m_clustered_exports_menu->setEnabled( false ) ;
      m_export_to_clusters_openMVS_act->setEnabled( false ) ;
      m_export_to_clusters_MVE_act->setEnabled( false ) ;
      m_export_to_clusters_PMVS_act->setEnabled( false ) ;

      m_setting_intrinsics_act->setEnabled( true ) ;
      m_setting_features_act->setEnabled( true );
      m_setting_matches_act->setEnabled( true ) ;
      m_setting_sfm_act->setEnabled( true ) ;

      break ;
    }
    case STATE_CLUSTERING_COMPUTED :
    {
      // Have done everything
      m_file_save_act->setEnabled( true ) ;
      m_file_save_act_tb->setEnabled( true ) ;
      m_file_close_act->setEnabled( true ) ;

      m_compute_features_act->setEnabled( true ) ;
      m_compute_features_act_tb->setEnabled( true ) ;
      m_compute_matches_act->setEnabled( true ) ;
      m_compute_matches_act_tb->setEnabled( true ) ;
      m_compute_sfm_act->setEnabled( true ) ;
      m_compute_sfm_act_tb->setEnabled( true ) ;
      m_compute_color_act->setEnabled( true ) ;
      m_compute_color_act_tb->setEnabled( true ) ;

      m_export_to_clusters_act->setEnabled( true ) ;
      m_export_to_openMVS_act->setEnabled( true ) ;
      m_export_to_MVE_act->setEnabled( true ) ;
      m_export_to_PMVS_act->setEnabled( true ) ;
      m_clustered_exports_menu->setEnabled( true ) ;
      m_export_to_clusters_openMVS_act->setEnabled( true ) ;
      m_export_to_clusters_MVE_act->setEnabled( true ) ;
      m_export_to_clusters_PMVS_act->setEnabled( true ) ;

      m_setting_intrinsics_act->setEnabled( true ) ;
      m_setting_features_act->setEnabled( true );
      m_setting_matches_act->setEnabled( true ) ;
      m_setting_sfm_act->setEnabled( true ) ;

      break ;
    }
  }
}

/**
* @brief Build interface elements
*/
void MainWindow::buildInterface( void )
{
  m_image_list = new ImageListWidget( this ) ;
  m_detail_list = new DetailListWidget( this ) ;

  m_result_view = new ResultViewWidget( this ) ;


  // Add everything to the main window
  QWidget * mainWidget = new QWidget ;
  QHBoxLayout * mainLayout = new QHBoxLayout ;
  mainLayout->setContentsMargins( 0 , 0 , 0 , 0 ) ;
  mainLayout->addWidget( m_image_list , 1 ) ;
  mainLayout->addWidget( m_detail_list , 1 ) ;


  // Result part
  mainLayout->addWidget( m_result_view, 5 ) ;

  m_result_summary_widget = new ReconstructionSummaryWidget( nullptr ) ;
  m_result_summary_widget->hide() ;

  m_feature_viewer_widget = new FeaturesViewerWidget( nullptr ) ;
  m_feature_viewer_widget->hide() ;

  m_console_widget = new ConsoleWidget( nullptr ) ;

  mainWidget->setLayout( mainLayout ) ;
  setCentralWidget( mainWidget ) ;
}

/**
* @brief Build Menus items
*/
void MainWindow::buildMenus( void )
{
  QMenuBar * mbar = menuBar() ;

  // All menus
  m_file_menu = mbar->addMenu( "File" ) ;
  m_workflow_menu = mbar->addMenu( "Workflow" ) ;
  m_settings_menu = mbar->addMenu( "Settings" ) ;
  m_view_menu = mbar->addMenu( "View" ) ;

  // File actions
  m_file_new_act = m_file_menu->addAction( "New" ) ;
  m_file_new_act->setShortcuts( QKeySequence::New ) ;
  m_file_menu->addSeparator() ;
  m_file_open_act = m_file_menu->addAction( "Open" ) ;
  m_file_open_act->setShortcuts( QKeySequence::Open ) ;
  m_file_menu->addSeparator() ;
  m_file_save_act = m_file_menu->addAction( "Save"  ) ;
  m_file_save_act->setShortcuts( QKeySequence::Save ) ;
  m_file_save_as_act = m_file_menu->addAction( "Save as ..." ) ;
  m_file_menu->addSeparator() ;
  m_file_close_act = m_file_menu->addAction( "Close Project" ) ;
  m_file_close_act->setShortcuts( QKeySequence::Close ) ;
  m_file_quit_act = m_file_menu->addAction( "Quit" ) ;
  m_file_quit_act->setShortcuts( QKeySequence::Quit ) ;

  // Workflow actions
  m_automatic_workflow_act = m_workflow_menu->addAction( "Automatic reconstruction" ) ;
  //  m_pipeline_editor_act = m_workflow_menu->addAction( "Pipeline editor" ) ;
  m_workflow_menu->addSeparator() ;
  m_compute_features_act = m_workflow_menu->addAction( "Compute features" ) ;
  m_compute_matches_act = m_workflow_menu->addAction( "Compute matches" ) ;
  m_compute_sfm_act = m_workflow_menu->addAction( "Compute SfM" ) ;
  m_compute_color_act = m_workflow_menu->addAction( "Compute color" ) ;
  m_workflow_menu->addSeparator() ;
  m_export_to_openMVS_act = m_workflow_menu->addAction( "export to openMVS" ) ;
  m_export_to_MVE_act = m_workflow_menu->addAction( "export to MVE" ) ;
  m_export_to_PMVS_act = m_workflow_menu->addAction( "export to PMVS" ) ;
  m_workflow_menu->addSeparator();
  m_export_to_clusters_act = m_workflow_menu->addAction( "Clustering" ) ;
  m_clustered_exports_menu = m_workflow_menu->addMenu( "Clustered exports" ) ;
  m_export_to_clusters_openMVS_act = m_clustered_exports_menu->addAction( "clusters to openMVS" );
  m_export_to_clusters_MVE_act = m_clustered_exports_menu->addAction( "clusters to MVE" ) ;
  m_export_to_clusters_PMVS_act = m_clustered_exports_menu->addAction( "clusters to PMVS" ) ;

  // Settings actions
  m_setting_intrinsics_act = m_settings_menu->addAction( "Intrinsics" ) ;
  m_setting_features_act = m_settings_menu->addAction( "Features" ) ;
  m_setting_matches_act = m_settings_menu->addAction( "Matching" ) ;
  m_setting_sfm_act = m_settings_menu->addAction( "SfM" ) ;

  // View actions
  m_show_hide_image_list_act = m_view_menu->addAction( "Image list" );
  m_show_hide_image_list_act->setCheckable( true ) ;
  m_show_hide_image_list_act->setChecked( true ) ;
  m_show_hide_detail_list_act = m_view_menu->addAction( "Detail list" ) ;
  m_show_hide_detail_list_act->setCheckable( true ) ;
  m_show_hide_detail_list_act->setChecked( false ) ;
  m_show_hide_features_viewer = m_view_menu->addAction( "Features Viewer" ) ;
  m_show_hide_features_viewer->setCheckable( true ) ;
  m_show_hide_features_viewer->setChecked( false ) ;
  m_show_hide_reconstruction_summary_act = m_view_menu->addAction( "Reconstruction summary" ) ;
  m_show_hide_reconstruction_summary_act->setCheckable( true ) ;
  m_show_hide_reconstruction_summary_act->setChecked( false ) ;
  m_show_hide_console_act = m_view_menu->addAction( "Console" ) ;
  m_show_hide_console_act->setCheckable( true ) ;
  m_show_hide_console_act->setChecked( false ) ;

  m_view_menu->addSeparator() ;
  m_show_hide_grid_act = m_view_menu->addAction( "Grid" ) ;
  m_show_hide_grid_act->setCheckable( true ) ;
  m_show_hide_grid_act->setChecked( false ) ;
  m_show_hide_camera_gizmos_act = m_view_menu->addAction( "Camera gizmos" ) ;
  m_show_hide_camera_gizmos_act->setCheckable( true ) ;
  m_show_hide_camera_gizmos_act->setChecked( true ) ;


  m_view_projection_menu = m_view_menu->addMenu( "View Projection" ) ;

  m_view_projection_perspective = m_view_projection_menu->addAction( "Perspective" ) ;
  m_view_projection_perspective->setCheckable( true ) ;
  m_view_projection_perspective->setChecked( true ) ;
  m_view_projection_orthographic = m_view_projection_menu->addAction( "Orthographic" ) ;
  m_view_projection_orthographic->setCheckable( true ) ;
  m_view_projection_orthographic->setChecked( false ) ;

}

/**
* @brief Build the toolbar
*/
void MainWindow::buildToolbar( void )
{
  QToolBar * toolbarProject = new QToolBar( "Project" , this ) ;
  toolbarProject->setAllowedAreas( Qt::TopToolBarArea ) ;
  m_file_new_act_tb = toolbarProject->addAction( "New" ) ;
  m_file_open_act_tb = toolbarProject->addAction( "Open" ) ;
  m_file_save_act_tb = toolbarProject->addAction( "Save" ) ;

  QToolBar * toolbarWorkflow = new QToolBar( "Workflow" , this ) ;
  toolbarWorkflow->setAllowedAreas( Qt::TopToolBarArea ) ;
  m_compute_features_act_tb = toolbarWorkflow->addAction( "Compute features" ) ;
  m_compute_matches_act_tb = toolbarWorkflow->addAction( "Compute Matches" ) ;
  m_compute_sfm_act_tb = toolbarWorkflow->addAction( "Compute SfM" ) ;
  m_compute_color_act_tb = toolbarWorkflow->addAction( "Compute Color" ) ;

  addToolBar( toolbarProject ) ;
  addToolBar( toolbarWorkflow ) ;
}

/**
* @brief Make connections between elements and slots
*/
void MainWindow::makeConnections( void )
{
  // Toolbar
  connect( m_file_new_act_tb , SIGNAL( triggered() ) , this , SLOT( onNewProject() ) );
  connect( m_file_open_act_tb , SIGNAL( triggered() ) , this , SLOT( onOpenProject() ) ) ;
  connect( m_file_save_act_tb , SIGNAL( triggered() ) , this , SLOT( onSaveProject() ) ) ;
  connect( m_compute_features_act_tb , SIGNAL( triggered() ), this , SLOT( onComputeFeatures() ) ) ;
  connect( m_compute_matches_act_tb , SIGNAL( triggered() ) , this , SLOT( onComputeMatches() ) ) ;
  connect( m_compute_sfm_act_tb , SIGNAL( triggered() ) , this , SLOT( onComputeSfM() ) ) ;
  connect( m_compute_color_act_tb , SIGNAL( triggered() ) , this , SLOT( onComputeColor() ) ) ;

  // Menus
  connect( m_file_new_act , SIGNAL( triggered() ) , this , SLOT( onNewProject() ) ) ;
  connect( m_file_open_act , SIGNAL( triggered() ) , this , SLOT( onOpenProject() ) ) ;
  connect( m_file_save_act , SIGNAL( triggered() ) , this , SLOT( onSaveProject() ) ) ;
  connect( m_file_save_as_act , SIGNAL( triggered() ) , this , SLOT( onSaveAsProject() ) ) ;
  connect( m_file_close_act , SIGNAL( triggered() ) , this , SLOT( onCloseProject() ) ) ;
  connect( m_file_quit_act , SIGNAL( triggered() ) , this , SLOT( onQuit() ) ) ;
  connect( m_automatic_workflow_act , SIGNAL( triggered() ) , this , SLOT( onComputeAutomaticReconstruction() ) ) ;
  //  connect( m_pipeline_editor_act , SIGNAL( triggered() ) , this , SLOT( onOpenPipelineEditor() ) );
  connect( m_compute_features_act , SIGNAL( triggered() ) , this , SLOT( onComputeFeatures() ) ) ;
  connect( m_compute_matches_act , SIGNAL( triggered() ) , this , SLOT( onComputeMatches() ) ) ;
  connect( m_compute_sfm_act , SIGNAL( triggered() ) , this , SLOT( onComputeSfM() ) ) ;
  connect( m_compute_color_act , SIGNAL( triggered() ) , this , SLOT( onComputeColor() ) ) ;
  connect( m_export_to_clusters_act , SIGNAL( triggered() ) , this , SLOT( onComputeClusters() ) ) ;
  connect( m_export_to_openMVS_act , SIGNAL( triggered() ) , this , SLOT( onExportToOpenMVS() ) ) ;
  connect( m_export_to_MVE_act , SIGNAL( triggered() ) , this , SLOT( onExportToMVE() ) ) ;
  connect( m_export_to_PMVS_act , SIGNAL( triggered() ) , this , SLOT( onExportToPMVS() ) ) ;
  connect( m_export_to_clusters_openMVS_act , SIGNAL( triggered() ) , this , SLOT( onExportClustersToOpenMVS() ) ) ;
  connect( m_export_to_clusters_MVE_act , SIGNAL( triggered() ) , this , SLOT( onExportClustersToMVE() ) ) ;
  connect( m_export_to_clusters_PMVS_act , SIGNAL( triggered() ) , this , SLOT( onExportClustersToPMVS() ) ) ;

  connect( m_setting_intrinsics_act , SIGNAL( triggered() ) , this , SLOT( onChangeIntrinsicsSettings() ) ) ;
  connect( m_setting_features_act , SIGNAL( triggered() ) , this , SLOT( onChangeFeatureSettings() ) ) ;
  connect( m_setting_matches_act , SIGNAL( triggered() ) , this , SLOT( onChangeMatchesSettings() ) ) ;
  connect( m_setting_sfm_act , SIGNAL( triggered() ) , this , SLOT( onChangeSfMSettings() ) ) ;
  connect( m_show_hide_grid_act , SIGNAL( triggered() ) , this , SLOT( onShowHideGrid() ) ) ;
  connect( m_show_hide_camera_gizmos_act , SIGNAL( triggered() ) , this , SLOT( onShowHideCameraGizmos() ) ) ;
  connect( m_show_hide_image_list_act , SIGNAL( triggered() ) , this , SLOT( onShowImageList() ) );
  connect( m_show_hide_detail_list_act , SIGNAL( triggered() ) , this , SLOT( onShowHideDetail() ) );
  connect( m_view_projection_orthographic , SIGNAL( triggered() ) , this , SLOT( onSetOrthographicProjection() ) ) ;
  connect( m_view_projection_perspective , SIGNAL( triggered() ) , this , SLOT( onSetPerspectiveProjection() ) ) ;
  connect( m_show_hide_features_viewer , SIGNAL( triggered() ) , this , SLOT( onShowHideFeatureViewer() ) ) ;
  connect( m_show_hide_reconstruction_summary_act , SIGNAL( triggered() ) , this , SLOT( onShowReconstructionReport() ) ) ;
  connect( m_show_hide_console_act , SIGNAL( triggered() ) , this , SLOT( onShowHideConsoleWindow() ) ) ;

  // The close events on the various floating windows
  connect( m_result_summary_widget , SIGNAL( hasBeenClosed() ) , this , SLOT( onCloseReconstructionReport() ) );
  connect( m_console_widget , SIGNAL( hasBeenClosed() ) , this , SLOT( onCloseConsoleWindow() ) ) ;
  connect( m_feature_viewer_widget , SIGNAL( hasBeenClosed() ) , this , SLOT( onCloseFeatureViewer() ) ) ;

  // Interface
  connect( m_image_list , SIGNAL( hasSelectedAnImage( int ) ) , this , SLOT( onSelectImage( int ) ) );
  connect( m_image_list , SIGNAL( hasRequestedMaskDefinition( int ) ) , this , SLOT( onDefineMask( int ) ) ) ;

}

void MainWindow::createProgress( const std::string & message , const int minvalue , const int maxvalue )
{
  m_progress_dialog = new QProgressDialog( this ) ;
  m_progress_dialog->setRange( 0 , 1 ) ;
  m_progress_dialog->setValue( 1 ) ;
  m_progress_dialog->setAutoClose( true ) ;
  m_progress_dialog->setAutoReset( true ) ;
  m_progress_dialog->reset() ;
  m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
  m_progress_dialog->setCancelButton( nullptr ) ;
  m_progress_dialog->setLabelText( message.c_str() ) ;
  m_progress_dialog->setRange( minvalue , maxvalue ) ;
  m_progress_dialog->setMinimumDuration( 100 ) ;
  m_progress_dialog->setValue( 0 ) ;

  QVBoxLayout *layout = new QVBoxLayout;
  foreach ( QObject *obj, m_progress_dialog->children() )
  {
    QWidget *widget = qobject_cast<QWidget *>( obj );
    if ( widget )
    {
      layout->addWidget( widget );
    }
  }
  m_progress_dialog->setLayout( layout );
}


} // namespace openMVG_gui