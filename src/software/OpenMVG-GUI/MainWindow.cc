#include "MainWindow.hh"

// Dialogs
#include "MaskDefinitionDialog.hh"
#include "NewProjectDialog.hh"

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
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QThread>
#include <QToolBar>
#include <QVBoxLayout>

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

    int progress_min = 0 , progress_max = 0 ;
    m_worker_project_creation->progressRange( progress_min , progress_max ) ;
    createProgress( "Project Creation, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( thread , SIGNAL( started() ) , m_worker_project_creation , SLOT( process() ) ) ;
    connect( m_worker_project_creation , SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasCreatedProject( const WorkerNextAction & ) ) ) ;
    connect( m_worker_project_creation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

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
  postFeaturesComputation() ;
  /*
  const std::string featuresPath = m_project->featuresPath() ;
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
  */

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

    // Add the camera gizmos
    std::shared_ptr<openMVG::sfm::SfM_Data> sfm = m_project->SfMData() ;
    if( sfm )
    {
      mgr->removeCameraGizmos() ;
      std::map<int, std::shared_ptr<RenderableObject>> cam_gizmos ;
      for( auto & cur_pose : sfm->GetPoses() )
      {
        cam_gizmos[ cur_pose.first ] = std::make_shared<CameraGizmo>( m_result_view->pointShader() , cur_pose.second , 0.1 ) ;
      }
      mgr->setCameraGizmos( cam_gizmos ) ;
    }

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

    // Add the camera gizmos
    std::shared_ptr<openMVG::sfm::SfM_Data> sfm = m_project->SfMData() ;
    if( sfm )
    {
      mgr->removeCameraGizmos() ;
      std::map<int, std::shared_ptr<RenderableObject>> cam_gizmos ;
      for( auto & cur_pose : sfm->GetPoses() )
      {
        cam_gizmos[ cur_pose.first ] = std::make_shared<CameraGizmo>( m_result_view->pointShader() , cur_pose.second , 0.1 ) ;
      }
      mgr->setCameraGizmos( cam_gizmos ) ;
    }

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
  m_detail_list->clear() ;

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

  qInfo( "Compute features" ) ;


  m_worker_features_computation = new WorkerFeaturesComputation( m_project , overwrite ) ;
  QThread * thread = new QThread( this ) ;
  m_worker_features_computation->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_features_computation->progressRange( progress_min , progress_max ) ;
  createProgress( "Features computation, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( started() ) , m_worker_features_computation , SLOT( process() ) ) ;
  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( m_worker_features_computation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_features_computation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedFeatures( const WorkerNextAction & ) ) ) ;
  connect( m_worker_features_computation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

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
  m_worker_regions_provide_load = new WorkerRegionsProviderLoad( m_project , act ) ;
  QThread * thread = new QThread( this ) ;
  m_worker_regions_provide_load->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_regions_provide_load->progressRange( progress_min , progress_max ) ;
  createProgress( "Loading regions, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( started() ) , m_worker_regions_provide_load , SLOT( process() ) ) ;
  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( m_worker_regions_provide_load, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_regions_provide_load, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasLoadedRegions( const WorkerNextAction & ) ) ) ;
  connect( m_worker_regions_provide_load , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

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
  m_worker_features_provider_load = new WorkerFeaturesProviderLoad( m_project , act ) ;
  QThread * thread = new QThread( this ) ;
  m_worker_features_provider_load->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_features_provider_load->progressRange( progress_min , progress_max ) ;
  createProgress( "Loading features, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( started() ) , m_worker_features_provider_load , SLOT( process() ) ) ;
  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( m_worker_features_provider_load, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_features_provider_load, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasLoadedFeatures( const WorkerNextAction & ) ) ) ;
  connect( m_worker_features_provider_load , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

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

  m_worker_color_computation = new WorkerColorComputation( m_project );
  QThread * thread = new QThread( this ) ;
  m_worker_color_computation->moveToThread( thread ) ;

  int progress_min = 0 , progress_max = 0 ;
  m_worker_color_computation->progressRange( progress_min , progress_max ) ;
  createProgress( "Computing scene color, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( started() ) , m_worker_color_computation , SLOT( process() ) ) ;
  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( m_worker_color_computation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_color_computation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedColor( const WorkerNextAction & ) ) ) ;
  connect( m_worker_color_computation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

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

  // Load the image describer (if it exists)
  const std::string featuresPath = m_project->featuresPath() ;
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
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

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

  int progress_min = 0 , progress_max = 0 ;
  m_worker_thumbnail_generation->progressRange( progress_min , progress_max ) ;
  createProgress( "Thumbnails Creation, please wait ..." , progress_min , progress_max ) ;

  connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
  connect( thread , SIGNAL( started() ) , m_worker_thumbnail_generation , SLOT( process() ) ) ;
  connect( m_worker_thumbnail_generation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
  connect( m_worker_thumbnail_generation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onUpdateImageList() ) ) ;
  connect( m_worker_thumbnail_generation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

  thread->start() ;
}

/**
* @brief Action to be executed when user want to update image list widget
*/
void MainWindow::onUpdateImageList( void )
{
  delete m_worker_thumbnail_generation ;
  m_worker_thumbnail_generation = nullptr ;

  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  const std::vector< std::pair< int , std::string > > images_path = m_project->GetImageNames() ;
  const std::string thumb_path = m_project->thumbnailsPath() ;

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
          c_gizmo->setSelected( false ) ;
        }
      }

      // Select the specified one
      std::shared_ptr<RenderableObject> gizmo = mgr->cameraGizmo( id ) ;
      std::shared_ptr<CameraGizmo> c_gizmo = std::dynamic_pointer_cast<CameraGizmo>( gizmo ) ;
      if( c_gizmo.use_count() )
      {
        c_gizmo->setSelected( true );
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
* @brief Action to be executed when features have been computed
*/
void MainWindow::onHasComputedFeatures( const WorkerNextAction & next_action  )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  delete m_worker_features_computation ;
  m_worker_features_computation = nullptr ;

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

    int progress_min = 0 , progress_max = 0 ;
    m_worker_matches_provider_load->progressRange( progress_min , progress_max ) ;
    createProgress( "Loading matches, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( started() ) , m_worker_matches_provider_load , SLOT( process() ) ) ;
    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( m_worker_matches_provider_load, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_matches_provider_load, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasLoadedMatches( const WorkerNextAction & ) ) ) ;
    connect( m_worker_matches_provider_load , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;


    thread->start() ;
  }
}

/**
* @brief Action to be executed when matches_provider has been loaded
*/
void MainWindow::onHasLoadedMatches( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

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

    int progress_min = 0 , progress_max = 0 ;
    m_worker_incremental_sfm_computation->progressRange( progress_min , progress_max ) ;
    createProgress( "Incremental SfM computation, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( started() ) , m_worker_incremental_sfm_computation , SLOT( process() ) ) ;
    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( m_worker_incremental_sfm_computation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_incremental_sfm_computation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedSfM( const WorkerNextAction & ) ) ) ;
    connect( m_worker_incremental_sfm_computation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

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

    int progress_min = 0 , progress_max = 0 ;
    m_worker_global_sfm_computation->progressRange( progress_min , progress_max ) ;
    createProgress( "Global SfM computation, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( started() ) , m_worker_global_sfm_computation , SLOT( process() ) ) ;
    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( m_worker_global_sfm_computation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_global_sfm_computation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedSfM( const WorkerNextAction & ) ) ) ;
    connect( m_worker_global_sfm_computation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;

    thread->start() ;
  }
}

/**
* @brief Action to be executed when regions are loaded
*/
void MainWindow::onHasLoadedRegions( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  if( contains( next_action , NEXT_ACTION_COMPUTE_MATCHES ) )
  {
    // Now launch matches computation
    std::shared_ptr<openMVG::sfm::Regions_Provider> regions_provider = m_worker_regions_provide_load->regionsProvider() ;
    m_worker_matches_computation = new WorkerMatchesComputation( m_project , regions_provider , remove( next_action , NEXT_ACTION_COMPUTE_MATCHES ) ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_matches_computation->moveToThread( thread ) ;

    int progress_min = 0 , progress_max = 0 ;
    m_worker_matches_computation->progressRange( progress_min , progress_max ) ;
    createProgress( "Matches computation, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( started() ) , m_worker_matches_computation , SLOT( process( ) ) ) ;
    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( m_worker_matches_computation , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;
    connect( m_worker_matches_computation, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_matches_computation, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasComputedMatches( const WorkerNextAction & ) ) ) ;

    thread->start() ;
  }
}


/**
* @brief Action to be executed when matches have been computed
*/
void MainWindow::onHasComputedMatches( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  if( contains( next_action , NEXT_ACTION_COMPUTE_GEOMETRIC_FILTERING ) )
  {
    std::shared_ptr<openMVG::sfm::Regions_Provider> regions_provider = m_worker_regions_provide_load->regionsProvider() ;
    std::shared_ptr<openMVG::matching::PairWiseMatches> putative_matches = m_worker_matches_computation->putativeMatches() ;

    m_worker_geometric_filtering = new WorkerGeometricFiltering( m_project , regions_provider , putative_matches , remove( next_action , NEXT_ACTION_COMPUTE_GEOMETRIC_FILTERING ) ) ;
    QThread * thread = new QThread( this ) ;
    m_worker_geometric_filtering->moveToThread( thread ) ;

    int progress_min = 0 , progress_max = 0 ;
    m_worker_geometric_filtering->progressRange( progress_min , progress_max ) ;
    createProgress( "Geometric filtering, please wait ..." , progress_min , progress_max ) ;

    connect( thread , SIGNAL( finished() ) , thread , SLOT( deleteLater() ) ) ;
    connect( thread , SIGNAL( started() ) , m_worker_geometric_filtering , SLOT( process() ) ) ;
    connect( m_worker_geometric_filtering , SIGNAL( progress( int ) ) , m_progress_dialog , SLOT( setValue( int ) ) , Qt::BlockingQueuedConnection ) ;
    connect( m_worker_geometric_filtering, SIGNAL( finished( const WorkerNextAction & ) ), thread, SLOT( quit() ) );
    connect( m_worker_geometric_filtering, SIGNAL( finished( const WorkerNextAction & ) ) , this , SLOT( onHasDoneGeometricFiltering( const WorkerNextAction & ) ) ) ;

    thread->start() ;
  }
}

/**
* @brief Action to be executed when geometric filtering has been computed
*/
void MainWindow::onHasDoneGeometricFiltering( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

  m_state = STATE_MATCHES_COMPUTED ;
  updateInterface() ;

  delete m_worker_regions_provide_load ;
  m_worker_regions_provide_load = nullptr ;
  delete m_worker_geometric_filtering ;
  m_worker_geometric_filtering = nullptr ;
  delete m_worker_matches_computation ;
  m_worker_matches_computation = nullptr ;

  postMatchesComputation() ;
}


/**
* @brief Action to be executed when SfM has been computed
*/
void MainWindow::onHasComputedSfM( const WorkerNextAction & next_action )
{
  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

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

  postSfMComputation() ;
}

void MainWindow::onHasComputedColor( const WorkerNextAction & next_action  )
{
  delete m_worker_color_computation ;
  m_worker_color_computation = nullptr ;

  delete m_progress_dialog ;
  m_progress_dialog = nullptr ;

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
  const std::string featuresPath = m_project->featuresPath() ;
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

  // TODO : load features statistics from files if computed
  std::vector< std::string > valid_features_path = m_project->featuresPaths() ;
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

}

/**
* @brief Post actions to be executed after sfm computation (or sfm computation failure)
*/
void MainWindow::postSfMComputation( void )
{
  // 1 - Load point cloud to the interface

  // Remove old object in the project
  std::shared_ptr<SceneManager> mgr = m_project->sceneManager() ;
  std::shared_ptr<RenderableObject> sprs = m_project->sparsePointCloud() ;
  if( sprs )
  {
    mgr->removeObject( sprs ) ;
  }

  // Load sparse point cloud
  const std::string sparse = m_project->sfMDataPlyPath() ;

  if( stlplus::file_exists( sparse ) )
  {
    // Load from file
    std::vector< openMVG::Vec3 > pts , col ;
    LoadPly( sparse , pts , col ) ;

    // Add to the scene, to the project and to the result view
    sprs = std::make_shared<PointCloud>( m_result_view->pointShader() , pts , col ) ;
    mgr->addObject( sprs ) ;
    m_project->setSparsePointCloud( sprs ) ;
  }

  // Add the camera gizmos
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm = m_project->SfMData() ;
  if( sfm )
  {
    mgr->removeCameraGizmos() ;
    std::map<int, std::shared_ptr<RenderableObject>> cam_gizmos ;
    for( auto & cur_pose : sfm->GetPoses() )
    {
      cam_gizmos[ cur_pose.first ] = std::make_shared<CameraGizmo>( m_result_view->pointShader() , cur_pose.second , 0.1 ) ;
    }
    mgr->setCameraGizmos( cam_gizmos ) ;
  }

  m_result_view->prepareObjects() ;
  m_result_view->updateTrackballSize() ;
  m_result_view->update() ;

  // 2 - Load statistics from file
  // TODO :

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
  std::shared_ptr<RenderableObject> sprs = m_project->sparsePointCloud() ;
  if( sprs )
  {
    mgr->removeObject( sprs ) ;
  }
  // Load the colorized one
  const std::string colorized = m_project->colorizedSfMPlyPath() ;
  if( stlplus::file_exists( colorized ) )
  {
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
  m_compute_features_act = m_workflow_menu->addAction( "Compute features" ) ;
  m_compute_matches_act = m_workflow_menu->addAction( "Compute matches" ) ;
  m_compute_sfm_act = m_workflow_menu->addAction( "Compute SfM" ) ;
  m_compute_color_act = m_workflow_menu->addAction( "Compute color" ) ;

  // Settings actions
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

  m_view_menu->addSeparator() ;
  m_show_hide_grid_act = m_view_menu->addAction( "Grid" ) ;
  m_show_hide_grid_act->setCheckable( true ) ;
  m_show_hide_grid_act->setChecked( true ) ;
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
  connect( m_compute_features_act , SIGNAL( triggered() ) , this , SLOT( onComputeFeatures() ) ) ;
  connect( m_compute_matches_act , SIGNAL( triggered() ) , this , SLOT( onComputeMatches() ) ) ;
  connect( m_compute_sfm_act , SIGNAL( triggered() ) , this , SLOT( onComputeSfM() ) ) ;
  connect( m_compute_color_act , SIGNAL( triggered() ) , this , SLOT( onComputeColor() ) ) ;
  connect( m_setting_features_act , SIGNAL( triggered() ) , this , SLOT( onChangeFeatureSettings() ) ) ;
  connect( m_setting_matches_act , SIGNAL( triggered() ) , this , SLOT( onChangeMatchesSettings() ) ) ;
  connect( m_setting_sfm_act , SIGNAL( triggered() ) , this , SLOT( onChangeSfMSettings() ) ) ;
  connect( m_show_hide_grid_act , SIGNAL( triggered() ) , this , SLOT( onShowHideGrid() ) ) ;
  connect( m_show_hide_camera_gizmos_act , SIGNAL( triggered() ) , this , SLOT( onShowHideCameraGizmos() ) ) ;
  connect( m_show_hide_image_list_act , SIGNAL( triggered() ) , this , SLOT( onShowImageList() ) );
  connect( m_show_hide_detail_list_act , SIGNAL( triggered() ) , this , SLOT( onShowHideDetail() ) );
  connect( m_view_projection_orthographic , SIGNAL( triggered() ) , this , SLOT( onSetOrthographicProjection() ) ) ;
  connect( m_view_projection_perspective , SIGNAL( triggered() ) , this , SLOT( onSetPerspectiveProjection() ) ) ;

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