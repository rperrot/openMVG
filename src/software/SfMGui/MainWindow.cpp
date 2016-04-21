#include "software/SfMGui/MainWindow.hpp"

#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>

#include <iostream>

// #define SFMGUI_DEBUG_INTERFACE 1

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

  /// Project folder
  m_project_folder_button = new QPushButton( "Select" ) ;
  m_project_folder_label = new QLabel( "Output project folder" ) ;
  m_project_folder_text = new QLineEdit ;

  QGroupBox * basicbox = new QGroupBox( "Basic settings" ) ;

  QGridLayout * basicLayout = new QGridLayout ;
  basicLayout->addWidget( m_input_folder_label , 0 , 0 ) ;
  basicLayout->addWidget( m_input_folder_text , 0 , 1 ) ;
  basicLayout->addWidget( m_input_folder_button , 0 , 2 ) ;
  basicLayout->addWidget( m_project_folder_label , 1 , 0 ) ;
  basicLayout->addWidget( m_project_folder_text , 1 , 1 ) ;
  basicLayout->addWidget( m_project_folder_button , 1 , 2 ) ;

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

}

/**
* @brief Make connections between interface elements
*/
void MainWindow::MakeConnections( void )
{
  // ImageTab -> MainWindow
  connect( m_image_tab , SIGNAL( hasTriggeredAddSingleImage() ) , this , SLOT( onAddImage() ) ) ;
  connect( m_image_tab , SIGNAL( hasTriggeredAddFolder() ) , this , SLOT( onAddFolder( ) ) ) ;

  // MainWindow -> ImageTab


  // MainWindow -> MainWindow
  connect( m_input_folder_button , SIGNAL( clicked() ) , this , SLOT( onSelectInputImages() ) ) ;
  connect( m_project_folder_button , SIGNAL( clicked() ) , this , SLOT( onSelectOutputProjectFolder() ) ) ;
}


/**
* @brief Action to be executed when user wants to add an image
*/
void MainWindow::onAddImage( void )
{

#ifdef SFMGUI_DEBUG_INTERFACE
  std::cerr << __func__ << std::endl ;
#endif
  // Open dialog to select an image
}


/**
* @brief Action to be executed when user wants to add a folder of images
*/
void MainWindow::onAddFolder( void )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  std::cerr << __func__ << std::endl ;
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
  std::cerr << __func__ << std::endl ;
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
  std::cerr << __func__ << std::endl ;
#endif

  QString folder = QFileDialog::getExistingDirectory( this , "Select output folder" , QDir::homePath() , QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks )  ;

  if( ! folder.isNull() && ! folder.isEmpty() )
  {
    m_project_folder_text->setText( folder ) ;
    DoProjectCreation() ;
  }


}

void MainWindow::DoProjectCreation( void )
{
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





} // namespace SfMGui
} // namespace openMVG