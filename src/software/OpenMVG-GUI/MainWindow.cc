#include "MainWindow.hh"

// Dialogs
#include "NewProjectDialog.hh"
#include "workflow_params/widgets/FeatureParamsDialog.hh"
#include "workflow_params/widgets/MatchingParamsDialog.hh"
#include "workflow_params/widgets/SfMParamsDialog.hh"


#include "graphics/hierarchies/LinearHierarchy.hh"
#include "graphics/objects/PointCloud.hh"
#include "graphics/objects/SphericalGizmo.hh"
#include "utils/BoundingSphere.hh"
#include "utils/PlyLoader.hh"


#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QApplication>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QThread>
#include <QToolBar>

namespace openMVG_gui
{

/**
* @brief Main window
*/
MainWindow::MainWindow()
{
  setWindowTitle( "OpenMVG-GUI" );
  showMaximized() ;

  buildInterface() ;
  buildMenus() ;
  buildToolbar() ;
  makeConnections() ;

  m_state = STATE_EMPTY ;
  updateInterface() ;

  // Set all workers to nullptr
  m_worker_project_creation            = nullptr ;
  m_worker_thumbnail_generation        = nullptr ;
  m_worker_features_computation        = nullptr ;
  m_worker_matches_computation         = nullptr ;
  m_worker_regions_provide_load        = nullptr ;
  m_worker_geometric_filtering         = nullptr ;
  m_worker_features_provider_load      = nullptr ;
  m_worker_matches_provider_load       = nullptr ;
  m_worker_incremental_sfm_computation = nullptr ;
  m_worker_global_sfm_computation      = nullptr ;
  m_worker_color_computation           = nullptr ;
}

/**
* @brief Action to be executed when user want to create a new project
*/
void MainWindow::onNewProject( void )
{
  qInfo( "New Project" ) ;
  NewProjectDialog dlg( this ) ;

  int res = dlg.exec() ;
  if( res == QDialog::Accepted )
  {
    // Default scene manager
    std::shared_ptr<Camera> cam = std::make_shared<Camera>( openMVG::Vec3( 0.0 , -3.0 , 3.0 ) ,
                                  openMVG::Vec3( 0.0 , 0.0 , 0.0 ) ,
                                  openMVG::Vec3( 0.0 , 0.0 , 1.0 ) ,
                                  openMVG::D2R( 90 ) ,
                                  0.1 ,
                                  100.0 ) ;
    std::shared_ptr<SceneHierarchy> s_hier = std::make_shared<LinearHierarchy>() ;
    std::shared_ptr<SceneManager> default_scene_manager = std::make_shared<SceneManager>( cam , s_hier ) ;
    default_scene_manager->addObject( m_result_view->grid() ) ;
    default_scene_manager->addObject( m_result_view->sphericalGizmo() ) ;

    // 1 - project creation (exif, ...)
    const std::string base_path = dlg.outputProjectPath() ;
    const std::string image_path = dlg.inputImagePath() ;
    const IntrinsicParams intrin_params ;
    const std::string camera_sensor_width_database_file = stlplus::create_filespec( stlplus::folder_append_separator( stlplus::folder_append_separator( QCoreApplication::applicationDirPath().toStdString() ) + "ressources" ) + "sensor_database" , "sensor_width_camera_database.txt" ) ;

    m_worker_project_creation = new WorkerProjectCreation( base_path , image_path , intrin_params , camera_sensor_width_database_file , default_scene_manager ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_project_creation->moveToThread( thread ) ;

    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( thread , SIGNAL( started() ) , m_worker_project_creation , SLOT( process() ) ) ;
    connect( m_worker_project_creation , SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_project_creation , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasCreatedProject( const WorkerNextAction & ) ) ) ;
    connect( m_worker_project_creation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) ) ;

    int progress_min = 0 ;
    int progress_max = 0 ;
    m_worker_project_creation->progressRange( progress_min , progress_max ) ;
    m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
    m_progress_dialog->setCancelButton( nullptr ) ;
    m_progress_dialog->setLabelText( "Project Creation, please wait ..." ) ;
    const int width = QFontMetrics( m_progress_dialog->font() ).width( m_progress_dialog->labelText() ) + 100;
    m_progress_dialog->resize( width , m_progress_dialog->height() ) ;
    m_progress_dialog->setRange( progress_min , progress_max ) ;
    m_progress_dialog->setMinimumDuration( 100 ) ;
    m_progress_dialog->setValue( 0 ) ;

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
      m_project->save() ;
    }
    else if( btn == QMessageBox::Cancel )
    {
      return ;
    }
  }
  resetInterface() ;

  // Now open project and set scene according to the specified project
  QString path = QFileDialog::getOpenFileName( this , "Open project file" ,  QDir::homePath() , "OpenMVG project file (*.omvg)" ) ;
  if( path.isEmpty() || path.isNull() )
  {
    return ;
  }
  const std::string projectPath = path.toStdString() ;

  // TODO : save camera inside project (because a specific point a view may be user defined ?)
  std::shared_ptr<Camera> cam = std::make_shared<Camera>( openMVG::Vec3( 0.0 , -3.0 , 3.0 ) ,
                                openMVG::Vec3( 0.0 , 0.0 , 0.0 ) ,
                                openMVG::Vec3( 0.0 , 0.0 , 1.0 ) ,
                                openMVG::D2R( 90 ) ,
                                0.1 ,
                                10000.0 ) ;
  std::shared_ptr<SceneHierarchy> s_hier = std::make_shared<LinearHierarchy>() ;
  std::shared_ptr<SceneManager> default_scene_manager = std::make_shared<SceneManager>( cam , s_hier ) ;
  default_scene_manager->addObject( m_result_view->grid() ) ;
  std::shared_ptr<SphericalGizmo> sph_giz = std::dynamic_pointer_cast<SphericalGizmo>( m_result_view->sphericalGizmo() );
  default_scene_manager->addObject( sph_giz ) ;

  m_project = nullptr ;

  m_project = std::make_shared<Project>( projectPath , default_scene_manager ) ;
  m_result_view->setScene( m_project->sceneManager() );
  m_result_view->prepareObjects() ;
  m_result_view->updateTrackballSize() ;

  m_result_view->update() ;


  // Load thumbnails
  onUpdateImageList() ;

  // Update scene state
  m_state = STATE_PROJECT_OPENED ;
  if( m_project->hasColorComputed() )
  {
    m_state = STATE_COLOR_COMPUTED ;

    // Load the color file
    std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
    const std::string sparse = m_project->colorizedSfMPlyPath() ;

    // Load from file
    std::vector< openMVG::Vec3 > pts ;
    std::vector< openMVG::Vec3 > col ;
    LoadPly( sparse , pts , col ) ;

    // Fit camera to the point cloud
    if( pts.size() > 0 )
    {
      openMVG::Vec3 bsCenter ;
      double bsRad ;
      computeBoundingSphere( pts , bsCenter , bsRad ) ;
      std::shared_ptr<Camera> cam = mgr->camera() ;
      cam->fitBoundingSphere( bsCenter , bsRad ) ;

      sph_giz->setCenter( bsCenter ) ;
    }

    // Add to the scene, to the project and to the result view
    std::shared_ptr<RenderableObject> sprs  = std::make_shared<PointCloud>( m_result_view->pointShader() , pts , col ) ;
    mgr->addObject( sprs ) ;
    m_project->setSparsePointCloud( sprs ) ;
    m_result_view->prepareObjects() ;
    m_result_view->updateTrackballSize() ;

    m_result_view->update() ;
  }
  else if( m_project->hasSfMComputed() )
  {
    m_state = STATE_SFM_COMPUTED ;

    // Load the cloud file
    std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
    const std::string sparse = m_project->sfMDataPlyPath() ;

    // Load from file
    std::vector< openMVG::Vec3 > pts ;
    std::vector< openMVG::Vec3 > col ;
    LoadPly( sparse , pts , col ) ;

    // Fit camera to the point cloud
    if( pts.size() > 0 )
    {
      openMVG::Vec3 bsCenter ;
      double bsRad ;
      computeBoundingSphere( pts , bsCenter , bsRad ) ;
      std::shared_ptr<Camera> cam = mgr->camera() ;
      cam->fitBoundingSphere( bsCenter , bsRad ) ;

      sph_giz->setCenter( bsCenter ) ;
    }


    // Add to the scene, to the project and to the result view
    std::shared_ptr<RenderableObject> sprs  = std::make_shared<PointCloud>( m_result_view->pointShader() , pts , col ) ;
    mgr->addObject( sprs ) ;
    m_project->setSparsePointCloud( sprs ) ;
    m_result_view->prepareObjects() ;
    m_result_view->updateTrackballSize() ;

    m_result_view->update() ;
  }
  else if( m_project->hasAllMatchesComputed() )
  {
    m_state = STATE_MATCHES_COMPUTED ;
  }
  else if( m_project->hasAllFeaturesComputed() )
  {
    m_state = STATE_FEATURES_COMPUTED ;
  }
  updateInterface() ;
}

/**
* @brief Action to be executed when user want to save a project
*/
void MainWindow::onSaveProject( void )
{
  qInfo( "Save Project" ) ;
  if( m_project )
  {
    m_project->save() ;
  }
}

/**
* @brief Action to be executed when user want to save a project to another folder
*/
void MainWindow::onSaveAsProject( void )
{
  // TODO :
  qInfo( "Save as Project" ) ;
}

/**
* @brief Action to be executed when user want to close current project
*/
void MainWindow::onCloseProject( void )
{
  qInfo( "Close Project" ) ;
  if( hasUnsavedChange() && m_project )
  {
    QMessageBox::StandardButton btn = QMessageBox::question( this , "Project unsaved" , "Project unsaved, save now ?" , QMessageBox::Cancel | QMessageBox::No | QMessageBox::Save , QMessageBox::Save ) ;
    if( btn == QMessageBox::Save )
    {
      m_project->save() ;
    }
    else if( btn == QMessageBox::Cancel )
    {
      return ;
    }
  }

  // Reset interface (project/scene)
  m_project = nullptr ;
  resetInterface() ;
}

/**
* @brief Action to be executed when user want to quit the project
*/
void MainWindow::onQuit( void )
{
  qInfo( "Quit" ) ;
  QApplication::quit() ;
}

/**
* @brief Action to be executed when user want to compute features
*/
void MainWindow::onComputeFeatures( void )
{
  qInfo( "Compute features" ) ;

  m_worker_features_computation = new WorkerFeaturesComputation( m_project ) ;
  QThread * thread = new QThread( this ) ;
  m_worker_features_computation->moveToThread( thread ) ;

  connect( thread , SIGNAL( started() ) , m_worker_features_computation , SLOT( process() ) ) ;
  connect( m_worker_features_computation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_features_computation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedFeatures( const WorkerNextAction & ) ) ) ;
  connect( m_worker_features_computation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) ) ;

  int progress_min = 0 ;
  int progress_max = 0 ;
  m_worker_features_computation->progressRange( progress_min , progress_max ) ;
  m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
  m_progress_dialog->setCancelButton( nullptr ) ;
  m_progress_dialog->setLabelText( "Features computation, please wait ..." ) ;
  const int width = QFontMetrics( m_progress_dialog->font() ).width( m_progress_dialog->labelText() ) + 100;
  m_progress_dialog->resize( width , m_progress_dialog->height() ) ;
  m_progress_dialog->setRange( progress_min , progress_max ) ;
  m_progress_dialog->setMinimumDuration( 100 ) ;
  m_progress_dialog->setValue( 0 ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to compute matches
*/
void MainWindow::onComputeMatches( void )
{
  // First load then what's next
  WorkerNextAction act = NEXT_ACTION_COMPUTE_MATCHES | NEXT_ACTION_COMPUTE_GEOMETRIC_FILTERING ;

  // Load regions
  qInfo( "Load regions" ) ;
  m_worker_regions_provide_load = new WorkerRegionsProviderLoad( m_project , act ) ;
  QThread * thread = new QThread( this ) ;
  m_worker_regions_provide_load->moveToThread( thread ) ;

  connect( thread , SIGNAL( started() ) , m_worker_regions_provide_load , SLOT( process() ) ) ;
  connect( m_worker_regions_provide_load, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_regions_provide_load, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasLoadedRegions( const WorkerNextAction & ) ) ) ;
  connect( m_worker_regions_provide_load , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) ) ;

  int progress_min = 0 ;
  int progress_max = 0 ;
  m_worker_regions_provide_load->progressRange( progress_min , progress_max ) ;
  m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
  m_progress_dialog->setCancelButton( nullptr ) ;
  m_progress_dialog->setLabelText( "Loading regions, please wait ..." ) ;
  const int width = QFontMetrics( m_progress_dialog->font() ).width( m_progress_dialog->labelText() ) + 100;
  m_progress_dialog->resize( width , m_progress_dialog->height() ) ;
  m_progress_dialog->setRange( progress_min , progress_max ) ;
  m_progress_dialog->setMinimumDuration( 100 ) ;
  m_progress_dialog->setValue( 0 ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to compute structure
*/
void MainWindow::onComputeSfM( void )
{
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
  m_worker_features_provider_load = new WorkerFeaturesProviderLoad( m_project , act ) ;
  QThread * thread = new QThread( this ) ;
  m_worker_features_provider_load->moveToThread( thread ) ;

  connect( thread , SIGNAL( started() ) , m_worker_features_provider_load , SLOT( process() ) ) ;
  connect( m_worker_features_provider_load, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_features_provider_load, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasLoadedFeatures( const WorkerNextAction & ) ) ) ;
  connect( m_worker_features_provider_load , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) ) ;

  int progress_min = 0 ;
  int progress_max = 0 ;
  m_worker_features_provider_load->progressRange( progress_min , progress_max ) ;
  m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
  m_progress_dialog->setCancelButton( nullptr ) ;
  m_progress_dialog->setLabelText( "Loading features, please wait ..." ) ;
  const int width = QFontMetrics( m_progress_dialog->font() ).width( m_progress_dialog->labelText() ) + 100;
  m_progress_dialog->resize( width , m_progress_dialog->height() ) ;
  m_progress_dialog->setRange( progress_min , progress_max ) ;
  m_progress_dialog->setMinimumDuration( 100 ) ;
  m_progress_dialog->setValue( 0 ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to compute color
*/
void MainWindow::onComputeColor( void )
{
  qInfo( "Compute Color" ) ;

  m_worker_color_computation = new WorkerColorComputation( m_project );
  QThread * thread = new QThread( this ) ;
  m_worker_color_computation->moveToThread( thread ) ;

  connect( thread , SIGNAL( started() ) , m_worker_color_computation , SLOT( process() ) ) ;
  connect( m_worker_color_computation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_color_computation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedColor( const WorkerNextAction & ) ) ) ;
  connect( m_worker_color_computation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) ) ;

  int progress_min = 0 ;
  int progress_max = 0 ;
  m_worker_color_computation->progressRange( progress_min , progress_max ) ;
  m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
  m_progress_dialog->setCancelButton( nullptr ) ;
  m_progress_dialog->setLabelText( "Computing scene color, please wait ..." ) ;
  const int width = QFontMetrics( m_progress_dialog->font() ).width( m_progress_dialog->labelText() ) + 100;
  m_progress_dialog->resize( width , m_progress_dialog->height() ) ;
  m_progress_dialog->setRange( progress_min , progress_max ) ;
  m_progress_dialog->setMinimumDuration( 100 ) ;
  m_progress_dialog->setValue( 0 ) ;

  thread->start() ;
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
  m_progress_dialog->reset() ;

  // Set the project
  m_project = m_worker_project_creation->project() ;
  delete m_worker_project_creation ;
  m_worker_project_creation = nullptr ;

  // Initialize the 3d view
  m_result_view->setScene( m_project->sceneManager() );
  m_result_view->prepareObjects() ;
  m_result_view->updateTrackballSize() ;

  m_result_view->update() ;

  // Generate the thumbnails
  m_worker_thumbnail_generation = new WorkerThumbnailGeneration( m_project ) ;
  QThread * thread = new QThread( this ) ;
  m_worker_thumbnail_generation->moveToThread( thread ) ;

  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread , SIGNAL( started() ) , m_worker_thumbnail_generation , SLOT( process() ) ) ;
  connect( m_worker_thumbnail_generation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_thumbnail_generation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onUpdateImageList() ) ) ;
  connect( m_worker_thumbnail_generation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) ) ;

  int progress_min = 0 ;
  int progress_max = 0 ;
  m_worker_thumbnail_generation->progressRange( progress_min , progress_max ) ;
  m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
  m_progress_dialog->setCancelButton( nullptr ) ;
  m_progress_dialog->setLabelText( "Thumbnails Creation, please wait ..." ) ;
  const int width = QFontMetrics( m_progress_dialog->font() ).width( m_progress_dialog->labelText() ) + 100;
  m_progress_dialog->resize( width , m_progress_dialog->height() ) ;
  m_progress_dialog->setRange( progress_min , progress_max ) ;
  m_progress_dialog->setMinimumDuration( 100 ) ;
  m_progress_dialog->setValue( 0 ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to update image list widget
*/
void MainWindow::onUpdateImageList( void )
{
  delete m_worker_thumbnail_generation ;
  m_worker_thumbnail_generation = nullptr ;

  const std::vector< std::pair< int , std::string > > images_path = m_project->GetImageNames() ;
  const std::string thumb_path = m_project->thumbnailsPath() ;

  std::vector< std::pair<int, std::string>> images_full_path ;
  for( size_t id_image = 0 ; id_image < images_path.size() ; ++id_image )
  {
    const std::string path = stlplus::create_filespec( stlplus::folder_append_separator( thumb_path ) , images_path[ id_image ].second ) ;
    images_full_path.emplace_back( std::make_pair( images_path[ id_image ].first , path ) ) ;
  }

  m_image_list->setImages( images_full_path ) ;

  m_state = STATE_PROJECT_OPENED ;
  updateInterface() ;
}

/**
* @brief Action to be executed when features have been computed
*/
void MainWindow::onHasComputedFeatures( const WorkerNextAction & next_action  )
{
  m_progress_dialog->reset() ;
  delete m_worker_features_computation ;
  m_worker_features_computation = nullptr ;

  m_state = STATE_FEATURES_COMPUTED ;
  updateInterface() ;
}

/**
* @brief Action to be executed when features_provider has been loaded
* @note this is before incremental SfM
*/
void MainWindow::onHasLoadedFeatures( const WorkerNextAction & next_action )
{
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
    m_worker_matches_provider_load = new WorkerMatchesProviderLoad( m_project , match_name , remove( next_action , NEXT_ACTION_LOAD_MATCHES ) ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_matches_provider_load->moveToThread( thread ) ;

    connect( thread , SIGNAL( started() ) , m_worker_matches_provider_load , SLOT( process() ) ) ;
    connect( m_worker_matches_provider_load, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_matches_provider_load, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasLoadedMatches( const WorkerNextAction & ) ) ) ;
    connect( m_worker_matches_provider_load , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) ) ;
    connect( m_worker_matches_provider_load , SIGNAL( finished( const WorkerNextAction & ) ) , m_progress_dialog , SLOT( reset() ) );

    int progress_min = 0 ;
    int progress_max = 0 ;
    m_worker_matches_provider_load->progressRange( progress_min , progress_max ) ;
    m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
    m_progress_dialog->setCancelButton( nullptr ) ;
    m_progress_dialog->setLabelText( "Loading matches, please wait ..." ) ;
    const int width = QFontMetrics( m_progress_dialog->font() ).width( m_progress_dialog->labelText() ) + 100;
    m_progress_dialog->resize( width , m_progress_dialog->height() ) ;
    m_progress_dialog->setRange( progress_min , progress_max ) ;
    m_progress_dialog->setMinimumDuration( 100 ) ;
    m_progress_dialog->setValue( 0 ) ;

    thread->start() ;
  }
}

/**
* @brief Action to be executed when matches_provider has been loaded
*/
void MainWindow::onHasLoadedMatches( const WorkerNextAction & next_action )
{
  m_progress_dialog->reset() ;

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

    m_worker_incremental_sfm_computation = new WorkerIncrementalSfMComputation( proj , features_provider , matches_provider , reload_initial_intrinsic , remove( next_action , NEXT_ACTION_COMPUTE_INCREMENTAL_SFM ) ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_incremental_sfm_computation->moveToThread( thread ) ;

    connect( thread , SIGNAL( started() ) , m_worker_incremental_sfm_computation , SLOT( process() ) ) ;
    connect( m_worker_incremental_sfm_computation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_incremental_sfm_computation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedSfM( const WorkerNextAction & ) ) ) ;
    connect( m_worker_incremental_sfm_computation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) ) ;

    int progress_min = 0 ;
    int progress_max = 0 ;
    m_worker_incremental_sfm_computation->progressRange( progress_min , progress_max ) ;
    m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
    m_progress_dialog->setCancelButton( nullptr ) ;
    m_progress_dialog->setLabelText( "Incremental SfM computation, please wait ..." ) ;
    const int width = QFontMetrics( m_progress_dialog->font() ).width( m_progress_dialog->labelText() ) + 100;
    m_progress_dialog->resize( width , m_progress_dialog->height() ) ;
    m_progress_dialog->setRange( progress_min , progress_max ) ;
    m_progress_dialog->setMinimumDuration( 100 ) ;
    m_progress_dialog->setValue( 0 ) ;

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

    m_worker_global_sfm_computation = new WorkerGlobalSfMComputation( proj , features_provider , matches_provider , reload_initial_intrinsic , remove( next_action , NEXT_ACTION_COMPUTE_GLOBAL_SFM ) ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_global_sfm_computation->moveToThread( thread ) ;

    connect( thread , SIGNAL( started() ) , m_worker_global_sfm_computation , SLOT( process() ) ) ;
    connect( m_worker_global_sfm_computation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_global_sfm_computation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedSfM( const WorkerNextAction & ) ) ) ;
    connect( m_worker_global_sfm_computation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) ) ;

    int progress_min = 0 ;
    int progress_max = 0 ;
    m_worker_global_sfm_computation->progressRange( progress_min , progress_max ) ;
    m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
    m_progress_dialog->setCancelButton( nullptr ) ;
    m_progress_dialog->setLabelText( "Global SfM computation, please wait ..." ) ;
    const int width = QFontMetrics( m_progress_dialog->font() ).width( m_progress_dialog->labelText() ) + 100;
    m_progress_dialog->resize( width , m_progress_dialog->height() ) ;
    m_progress_dialog->setRange( progress_min , progress_max ) ;
    m_progress_dialog->setMinimumDuration( 100 ) ;
    m_progress_dialog->setValue( 0 ) ;

    thread->start() ;
  }
}

/**
* @brief Action to be executed when regions are loaded
*/
void MainWindow::onHasLoadedRegions( const WorkerNextAction & next_action )
{
  m_progress_dialog->reset() ;

  if( contains( next_action , NEXT_ACTION_COMPUTE_MATCHES ) )
  {
    // Now launch matches computation
    std::shared_ptr<openMVG::sfm::Regions_Provider> regions_provider = m_worker_regions_provide_load->regionsProvider() ;
    m_worker_matches_computation = new WorkerMatchesComputation( m_project , regions_provider , remove( next_action , NEXT_ACTION_COMPUTE_MATCHES ) ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_matches_computation->moveToThread( thread ) ;

    connect( thread , SIGNAL( started() ) , m_worker_matches_computation , SLOT( process( ) ) ) ;
    connect( m_worker_matches_computation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_matches_computation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedMatches( const WorkerNextAction & ) ) ) ;
    connect( m_worker_matches_computation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) ) ;
    connect( m_worker_matches_computation, SIGNAL( finished( const WorkerNextAction & ) )  , m_progress_dialog , SLOT( reset() ) ) ;

    int progress_min = 0 ;
    int progress_max = 0 ;
    m_worker_matches_computation->progressRange( progress_min , progress_max ) ;
    m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
    m_progress_dialog->setCancelButton( nullptr ) ;
    m_progress_dialog->setLabelText( "Matches computation, please wait ..." ) ;
    const int width = QFontMetrics( m_progress_dialog->font() ).width( m_progress_dialog->labelText() ) + 100;
    m_progress_dialog->resize( width , m_progress_dialog->height() ) ;
    m_progress_dialog->setRange( progress_min , progress_max ) ;
    m_progress_dialog->setMinimumDuration( 100 ) ;
    m_progress_dialog->setValue( 0 ) ;

    thread->start() ;
  }
}


/**
* @brief Action to be executed when matches have been computed
*/
void MainWindow::onHasComputedMatches( const WorkerNextAction & next_action )
{
  if( contains( next_action , NEXT_ACTION_COMPUTE_GEOMETRIC_FILTERING ) )
  {
    std::shared_ptr<openMVG::sfm::Regions_Provider> regions_provider = m_worker_regions_provide_load->regionsProvider() ;
    std::shared_ptr<openMVG::matching::PairWiseMatches> putative_matches = m_worker_matches_computation->putativeMatches() ;

    m_worker_geometric_filtering = new WorkerGeometricFiltering( m_project , regions_provider , putative_matches , remove( next_action , NEXT_ACTION_COMPUTE_GEOMETRIC_FILTERING ) ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_geometric_filtering->moveToThread( thread ) ;

    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( thread , SIGNAL( started() ) , m_worker_geometric_filtering , SLOT( process() ) ) ;
    connect( m_worker_geometric_filtering, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_geometric_filtering, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasDoneGeometricFiltering( const WorkerNextAction & ) ) ) ;
    connect( m_worker_geometric_filtering , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) ) ;
    connect( m_worker_geometric_filtering, SIGNAL( finished( const WorkerNextAction & ) ), m_progress_dialog, SLOT( reset() ) );

    int progress_min = 0 ;
    int progress_max = 0 ;
    m_worker_geometric_filtering->progressRange( progress_min , progress_max ) ;
    m_progress_dialog->setWindowModality( Qt::WindowModal ) ;
    m_progress_dialog->setCancelButton( nullptr ) ;
    m_progress_dialog->setLabelText( "Geometric filtering, please wait ..." ) ;
    const int width = QFontMetrics( m_progress_dialog->font() ).width( m_progress_dialog->labelText() ) + 150;
    m_progress_dialog->resize( width , m_progress_dialog->height() ) ;
    m_progress_dialog->setRange( progress_min , progress_max ) ;
    m_progress_dialog->setMinimumDuration( 100 ) ;
    m_progress_dialog->setValue( 0 ) ;

    thread->start() ;
  }
}

/**
* @brief Action to be executed when geometric filtering has been computed
*/
void MainWindow::onHasDoneGeometricFiltering( const WorkerNextAction & next_action )
{
  m_progress_dialog->reset() ;

  m_state = STATE_MATCHES_COMPUTED ;
  updateInterface() ;

  delete m_worker_regions_provide_load ;
  m_worker_regions_provide_load = nullptr ;
  delete m_worker_geometric_filtering ;
  m_worker_geometric_filtering = nullptr ;
  delete m_worker_matches_computation ;
  m_worker_matches_computation = nullptr ;
}


/**
* @brief Action to be executed when SfM has been computed
*/
void MainWindow::onHasComputedSfM( const WorkerNextAction & next_action )
{
  if( m_project->sfMMethod() == SFM_METHOD_INCREMENTAL )
  {
    delete m_worker_features_provider_load ;
    m_worker_features_provider_load = nullptr ;
    delete m_worker_matches_provider_load ;
    m_worker_matches_provider_load = nullptr ;
    delete m_worker_incremental_sfm_computation ;
    m_worker_incremental_sfm_computation = nullptr ;
  }
  else
  {
    // Global
    delete m_worker_features_provider_load ;
    m_worker_features_provider_load = nullptr ;
    delete m_worker_matches_provider_load ;
    m_worker_matches_provider_load = nullptr ;
    delete m_worker_global_sfm_computation ;
    m_worker_global_sfm_computation = nullptr ;
  }

  // Remove old object
  std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
  std::shared_ptr<RenderableObject> sprs = m_project->sparsePointCloud() ;
  if( sprs )
  {
    mgr->removeObject( sprs ) ;
  }

  // Load sparse point cloud
  const std::string sparse = m_project->sfMDataPlyPath() ;

  // Load from file
  std::vector< openMVG::Vec3 > pts ;
  std::vector< openMVG::Vec3 > col ;
  LoadPly( sparse , pts , col ) ;

  // Add to the scene, to the project and to the result view
  sprs = std::make_shared<PointCloud>( m_result_view->pointShader() , pts , col ) ;
  mgr->addObject( sprs ) ;
  m_project->setSparsePointCloud( sprs ) ;
  m_result_view->prepareObjects() ;
  m_result_view->updateTrackballSize() ;
  m_result_view->update() ;

  m_state = STATE_SFM_COMPUTED ;
  updateInterface() ;
}

void MainWindow::onHasComputedColor( const WorkerNextAction & next_action  )
{
  delete m_worker_color_computation ;
  m_worker_color_computation = nullptr ;

  // Remove old object
  std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
  std::shared_ptr<RenderableObject> sprs = m_project->sparsePointCloud() ;
  mgr->removeObject( sprs ) ;
  const std::string colorized = m_project->colorizedSfMPlyPath() ;

  // Add colorized object
  std::vector< openMVG::Vec3 > pts ;
  std::vector< openMVG::Vec3 > col ;
  LoadPly( colorized , pts , col ) ;

  sprs = std::make_shared<PointCloud>( m_result_view->pointShader() , pts , col ) ;
  mgr->addObject( sprs ) ;
  m_project->setSparsePointCloud( sprs ) ;
  m_result_view->prepareObjects() ;
  m_result_view->updateTrackballSize() ;
  m_result_view->update() ;

  m_state = STATE_COLOR_COMPUTED ;
  updateInterface() ;
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
* @brief Set the interface exactly the same as after launching the application
* -> make the interface into a clean step
*/
void MainWindow::resetInterface( void )
{
  m_result_view->setScene( nullptr ) ;
  m_result_view->update() ;

  m_image_list->clear() ;

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

  m_result_view = new ResultViewWidget( this ) ;

  // Add everything to the main window
  QWidget * mainWidget = new QWidget ;
  QHBoxLayout * mainLayout = new QHBoxLayout ;
  mainLayout->setContentsMargins( 0 , 0 , 0 , 0 ) ;
  mainLayout->addWidget( m_image_list , 1 ) ;
  mainLayout->addWidget( m_result_view , 4 ) ;

  mainWidget->setLayout( mainLayout ) ;

  m_progress_dialog = new QProgressDialog( this ) ;
  m_progress_dialog->setRange( 0 , 1 ) ;
  m_progress_dialog->setValue( 1 ) ;
  m_progress_dialog->reset() ;

  setCentralWidget( mainWidget ) ;
}

/**
* @brief Build Menus items
*/
void MainWindow::buildMenus( void )
{
  QMenuBar * mbar = menuBar() ;

  m_file_menu = mbar->addMenu( "File" ) ;
  m_workflow_menu = mbar->addMenu( "Workflow" ) ;
  m_settings_menu = mbar->addMenu( "Settings" ) ;

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

  m_compute_features_act = m_workflow_menu->addAction( "Compute features" ) ;
  m_compute_matches_act = m_workflow_menu->addAction( "Compute matches" ) ;
  m_compute_sfm_act = m_workflow_menu->addAction( "Compute SfM" ) ;
  m_compute_color_act = m_workflow_menu->addAction( "Compute color" ) ;

  m_setting_features_act = m_settings_menu->addAction( "Features" ) ;
  m_setting_matches_act = m_settings_menu->addAction( "Matching" ) ;
  m_setting_sfm_act = m_settings_menu->addAction( "SfM" ) ;
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
  connect( m_compute_features_act , SIGNAL( triggered() ) , this , SLOT( onComputeFeatures() ) ) ;
  connect( m_compute_matches_act , SIGNAL( triggered() ) , this , SLOT( onComputeMatches() ) ) ;
  connect( m_compute_sfm_act , SIGNAL( triggered() ) , this , SLOT( onComputeSfM() ) ) ;
  connect( m_compute_color_act , SIGNAL( triggered() ) , this , SLOT( onComputeColor() ) ) ;
  connect( m_setting_features_act , SIGNAL( triggered() ) , this , SLOT( onChangeFeatureSettings() ) ) ;
  connect( m_setting_matches_act , SIGNAL( triggered() ) , this , SLOT( onChangeMatchesSettings() ) ) ;
  connect( m_setting_sfm_act , SIGNAL( triggered() ) , this , SLOT( onChangeSfMSettings() ) ) ;
}

} // namespace openMVG_gui