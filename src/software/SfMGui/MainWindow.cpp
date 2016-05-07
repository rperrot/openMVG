#include "software/SfMGui/MainWindow.hpp"

#include "software/SfMGui/ProjectCreationWizard.hpp"

#include "software/SfMGui/dependencies/waitingspinnerwidget.h"

#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QMenuBar>
#include <QApplication>
#include <QMessageBox>

#include <iostream>

#define SFMGUI_DEBUG_INTERFACE 1

#ifdef SFMGUI_DEBUG_INTERFACE
  #include <QDebug>
#endif

namespace openMVG
{
namespace SfMGui
{
/**
* @brief Default constructor
*/
MainWindow::MainWindow( void )
  : QMainWindow( )
{
  BuildInterface();
  BuildMenus() ;
  MakeConnections() ;
}

/**
* @brief Reset interface to default settings
*/
void MainWindow::Reset( void )
{
  m_image_tab->Reset() ;
  m_setting_tab->Reset() ;
  m_result_tab->Reset() ;
}

/**
* @brief Build all interface widgets
*/
void MainWindow::BuildInterface( void )
{
  // Project and input folder
  m_input_folder_button = new QPushButton( "Select" ) ;
  m_input_folder_label = new QLabel( "Input images folder" );
  m_input_folder_text = new QLineEdit ;
  m_input_folder_text->setEnabled( false ) ;

  /// Project folder
  m_project_folder_button = new QPushButton( "Select" ) ;
  m_project_folder_label = new QLabel( "Output project folder" ) ;
  m_project_folder_text = new QLineEdit ;
  m_project_folder_text->setEnabled( false ) ;

  QGroupBox * basicbox = new QGroupBox( "Basic settings" ) ;

  QGridLayout * basicLayout = new QGridLayout ;
  basicLayout->addWidget( m_input_folder_label , 0 , 0 ) ;
  basicLayout->addWidget( m_input_folder_text , 0 , 1 ) ;
  //  basicLayout->addWidget( m_input_folder_button , 0 , 2 ) ;
  basicLayout->addWidget( m_project_folder_label , 1 , 0 ) ;
  basicLayout->addWidget( m_project_folder_text , 1 , 1 ) ;
  //  basicLayout->addWidget( m_project_folder_button , 1 , 2 ) ;

  basicbox->setLayout( basicLayout ) ;

  // Tab containers
  m_tabwidget = new QTabWidget( this ) ;

  m_image_tab = new ImageTab( this ) ;
  m_setting_tab = new SettingTab( this ) ;
  m_result_tab = new ResultTab( this ) ;

  m_tabwidget->addTab( m_image_tab , "Images" ) ;
  m_tabwidget->addTab( m_setting_tab , "Settings" ) ;
  m_tabwidget->addTab( m_result_tab , "Result" ) ;

  QVBoxLayout * mainLayout = new QVBoxLayout ;

  mainLayout->addWidget( basicbox ) ;
  mainLayout->addWidget( m_tabwidget ) ;

  QWidget * dummy = new QWidget ;

  dummy->setLayout( mainLayout ) ;

  setCentralWidget( dummy ) ;

  setWindowTitle( "SfMGui" ) ;
  setMinimumSize( 1024 , 768 ) ;
}

/**
* @brief Build all menus
*/
void MainWindow::BuildMenus( void )
{
  m_file_menu = new QMenu( "File" ) ;

  m_new_project = m_file_menu->addAction( "New project" ) ;
  m_new_project->setShortcuts( QKeySequence::New ) ;
  m_new_project->setStatusTip( "Start a new project" ) ;

  m_open_project = m_file_menu->addAction( "Open project" ) ;
  m_open_project->setShortcuts( QKeySequence::Open ) ;
  m_open_project->setStatusTip( "Open a project" ) ;

  m_save_project = m_file_menu->addAction( "Save project" ) ;
  m_save_project->setShortcuts( QKeySequence::Save ) ;
  m_save_project->setStatusTip( "Save a project" ) ;

  m_save_project_as = m_file_menu->addAction( "Save project as" ) ;
  m_save_project_as->setShortcuts( QKeySequence::SaveAs ) ;
  m_save_project_as->setStatusTip( "Save a project to a new directory" ) ;

  m_close_project = m_file_menu->addAction( "Close project" ) ;
  m_close_project->setShortcuts( QKeySequence::Close ) ;
  m_close_project->setStatusTip( "Close project" ) ;

  m_quit = m_file_menu->addAction( "Quit" ) ;
  m_quit->setMenuRole( QAction::QuitRole ) ;
  m_quit->setShortcuts( QKeySequence::Quit ) ;
  m_quit->setStatusTip( "Quit application" ) ;


  m_configuration_menu = new QMenu( "Settings" ) ;
  m_load_default_configuration = m_configuration_menu->addAction( "Load default" ) ;

  m_project_menu = new QMenu( "Project" ) ;
  m_select_input_images_directory = m_project_menu->addAction( "Select input directory" ) ;
  m_reload_input_directory = m_project_menu->addAction( "Reload input directory" ) ;
  m_project_menu->addSeparator() ;
  m_compute_sfm = m_project_menu->addAction( "Compute SfM" ) ;
  m_project_menu->addSeparator() ;
  m_export_to_mve = m_project_menu->addAction( "Export to MVE" ) ;

  m_help_menu = new QMenu( "Help" ) ;
  m_help_rtfm = m_help_menu->addAction( "Help" ) ;

  QMenuBar * mbar = menuBar() ;

  mbar->addMenu( m_file_menu ) ;
  mbar->addMenu( m_configuration_menu ) ;
  mbar->addMenu( m_project_menu ) ;
  mbar->addMenu( m_help_menu ) ;
}

/**
* @brief Make connections between interface elements
*/
void MainWindow::MakeConnections( void )
{
  // ImageTab -> MainWindow
  // connect( m_image_tab , SIGNAL( hasTriggeredAddSingleImage() ) , this , SLOT( onAddImage() ) ) ;
  // connect( m_image_tab , SIGNAL( hasTriggeredAddFolder() ) , this , SLOT( onAddFolder( ) ) ) ;

  // MainWindow -> ImageTab


  // MainWindow -> MainWindow
  connect( m_input_folder_button , SIGNAL( clicked() ) , this , SLOT( onSelectInputImages() ) ) ;
  connect( m_project_folder_button , SIGNAL( clicked() ) , this , SLOT( onSelectOutputProjectFolder() ) ) ;

  // Menus
  // Menu - File
  connect( m_new_project , SIGNAL( triggered() ) , this , SLOT( onMenuNewProject() ) ) ;
  connect( m_open_project , SIGNAL( triggered() ) , this , SLOT( onMenuOpenProject() ) ) ;
  connect( m_save_project , SIGNAL( triggered() ) , this , SLOT( onMenuSaveProject() ) ) ;
  connect( m_save_project_as , SIGNAL( triggered() ) , this , SLOT( onMenuSaveAsProject() ) ) ;
  connect( m_close_project , SIGNAL( triggered() ) , this , SLOT( onMenuCloseProject() ) ) ;
  connect( m_quit , SIGNAL( triggered() ) , this , SLOT( onMenuQuit() ) ) ;

  // Menu - Settings
  connect( m_load_default_configuration , SIGNAL( triggered() ) , this , SLOT( onMenuSettingLoadDefault() ) ) ;
  // Menu - Project
  connect( m_select_input_images_directory , SIGNAL( triggered() ) , this , SLOT( onMenuProjectLoadImageDir() ) ) ;
  //  connect( m_reload_input_directory , SIGNAL( triggered() ) , this , SLOT( onMenuProjectReloadImageDir() ) ) ;
  connect( m_compute_sfm , SIGNAL( triggered() ) , this , SLOT( onMenuProjectComputeSfM() ) ) ;
  connect( m_export_to_mve , SIGNAL( triggered() ) , this , SLOT( onMenuProjectExportToMVE() ) ) ;
  // Menu - Help
  connect( m_help_rtfm , SIGNAL( triggered() ) , this , SLOT( onMenuHelp() ) ) ;
}


/**
* @brief Action to be executed when user wants to add an image
*/
void MainWindow::onAddImage( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif
  // Open dialog to select an image
}


/**
* @brief Action to be executed when user wants to add a folder of images
*/
void MainWindow::onAddFolder( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  // Open dialog to select a folder

  QString folder = QFileDialog::getExistingDirectory( this , "Open input images" , QDir::homePath() , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks )  ;

  if( ! folder.isNull() && ! folder.isEmpty() )
  {
    // Input folder
  }
}

/**
* @brief Action to be executed when user select input images
*/
void MainWindow::onSelectInputImages( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  QString folder = QFileDialog::getExistingDirectory( this , "Open input images" , QDir::homePath() , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks )  ;

  if( ! folder.isNull() && ! folder.isEmpty() )
  {
    m_input_folder_text->setText( folder ) ;
    DoProjectCreation() ;
  }
}

/**
* @brief Action to be executed when user select project folder
*/
void MainWindow::onSelectOutputProjectFolder( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  QString folder = QFileDialog::getExistingDirectory( this , "Select output folder" , QDir::homePath() , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks )  ;

  if( ! folder.isNull() && ! folder.isEmpty() )
  {
    m_project_folder_text->setText( folder ) ;
    DoProjectCreation() ;
  }
}

/**
* @brief Save or close an unsaved project
* @retval true If project saved or closed
* @retval false If project unsaved or user give up close/save
* @retval true If project has not unsaved changes
*/
bool MainWindow::SaveOrClose( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  if( ! m_project )
  {
    return true ;
  }
  else
  {
    if( m_project->HasUnsavedChanges() )
    {
      // Dialog to say if project should be saved ?
      int reply = QMessageBox::question( this , "Save changes" , "Do you want to save project changes" , QMessageBox::Save  , QMessageBox::Discard , QMessageBox::Cancel ) ;

      if( reply == QMessageBox::Save )
      {
        // Save changes
        m_project->Save() ;
        return true ;
      }
      else if( reply == QMessageBox::Discard )
      {
        // Don't save changes
        m_project = nullptr ;
        return true ;
      }
      else // == Cancel
      {
        // Cancel operation
        return false ;
      }
    }
    else
    {
      m_project = nullptr ;
      return true ;
    }
  }
}

void MainWindow::DoProjectCreation( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  const QString input_path = m_input_folder_text->text() ;
  const QString project_path = m_project_folder_text->text() ;

  QDir input_path_dir( input_path ) ;
  QDir project_path_dir( project_path ) ;

  if( input_path_dir.exists() && project_path_dir.exists( ) &&
      ! input_path.isNull() &&
      ! project_path.isNull() &&
      ! input_path.isEmpty() &&
      ! project_path.isEmpty() )
  {
    const std::string project_path_cpp = m_project_folder_text->text().toStdString() ;
    m_project = std::make_shared<SfMProject>( project_path_cpp ) ;
  }
}

/**
* @brief Acction executed when user create a new project
*/
void MainWindow::onMenuNewProject( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  if( SaveOrClose( ) )
  {
    m_project = nullptr ;
    Reset() ;
  }

  // Creation dialog
  ProjectCreationWizard creationWizard( this ) ;
  int res = creationWizard.exec() ;
  if( res == QDialog::Accepted )
  {
    // Progress
    m_spinner = new WaitingSpinnerWidget( Qt::ApplicationModal , this , true , true ) ;
    m_spinner->start() ;

    //    std::string camera_sensor_database_file_path ;
    const std::string input_image_path = creationWizard.GetInputImageFolder() ;
    const std::string output_project_path = creationWizard.GetOutputProjectFolder() ;

    m_project_creator = new ProjectCreator( output_project_path , input_image_path ) ;

    connect( m_project_creator , SIGNAL( finished( std::shared_ptr< SfMProject > ) ) , this , SLOT( onProjectCreated( std::shared_ptr< SfMProject > ) ) ) ;

    m_project_creator->start() ;

    /*
    m_project = std::make_shared<SfMProject>( output_project_path );
    m_project->OpenImageFolder( input_image_path , camera_sensor_database_file_path ) ;
    */

    m_input_folder_text->setText( QString( input_image_path.c_str() ) ) ;
    m_project_folder_text->setText( QString( output_project_path.c_str() ) ) ;
  }
}



/**
* @brief Acction executed when user want to open a project
*/
void MainWindow::onMenuOpenProject( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  if( SaveOrClose() )
  {
    // open dialog
    std::string project_path ;

    // if ok to load
    m_project = std::make_shared< SfMProject >( project_path ) ;
  }
}


/**
* @brief Acction executed when user want to save a project
*/
void MainWindow::onMenuSaveProject( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  if( m_project )
  {
    m_project->Save() ;
  }
}


/**
* @brief Acction executed when user want to save a project to a new directory
*/
void MainWindow::onMenuSaveAsProject( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  if( m_project )
  {
    // Save Folder dialog
    std::string new_project_path ;

    // If valid dialog
    m_project->SaveAs( new_project_path ) ;
  }
}


/**
* @brief Acction executed when user create want to close a project
*/
void MainWindow::onMenuCloseProject( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  if( SaveOrClose( ) )
  {
    m_project = nullptr ;
    Reset() ;
  }
}


/**
* @brief Acction executed when user want to quit application
*/
void MainWindow::onMenuQuit( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  if( SaveOrClose() )
  {
    QApplication::quit() ;
  }
}


/**
* @brief Acction executed when user want to load default settings
*/
void MainWindow::onMenuSettingLoadDefault( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  if( m_project )
  {
    SfMSettings default_settings ;
    m_project->SetSettings( default_settings ) ;
  }
}


/**
* @brief Acction executed when user want to Load an image dir
*/
void MainWindow::onMenuProjectLoadImageDir( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  if( m_project )
  {
    // Open folder dialog
    std::string input_image_path ;
    std::string camera_sensor_database_file_path ;

    // If ok
    m_project->OpenImageFolder( input_image_path , camera_sensor_database_file_path ) ;
  }
}


/**
* @brief Acction executed when user want to ReLoad an image dir
*/
void MainWindow::onMenuProjectReloadImageDir( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

}


/**
* @brief Acction executed when user want to compute SfM
*/
void MainWindow::onMenuProjectComputeSfM( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  if( m_project )
  {
    m_project->ComputeSfM( ) ;
  }
}


/**
* @brief Acction executed when user want to export project to MVE format
*/
void MainWindow::onMenuProjectExportToMVE( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

  if( m_project )
  {
    m_project->ExportToMVE( ) ;
  }
}

/**
* @brief Action executed when user want some help (but why ?)
*/
void MainWindow::onMenuHelp( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  qDebug() << __func__ ;
#endif

}

/**
* @brief Signal emmited when a project is created
*/
void MainWindow::onProjectCreated( std::shared_ptr< SfMProject > pro )
{
  m_project = pro ;
  delete m_project_creator ;

  // Set image from input list
  for( int id_image = 0 ; id_image < m_project->NbInputImage() ; ++id_image )
  {
    const std::string full_path = m_project->ThumbnailPath( id_image ) ;
    const std::string img_name = m_project->ImageName( id_image ) ;
    const int img_width = m_project->ImageWidth( id_image ) ;
    const int img_height = m_project->ImageHeight( id_image ) ;
    const float img_focal = 0.0 ;

    m_image_tab->AddRow( full_path , full_path , img_width , img_height , img_focal ) ;
  }
  m_spinner->stop() ;
  delete m_spinner ;
}


} // namespace SfMGui
} // namespace openMVG