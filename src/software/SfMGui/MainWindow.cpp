#include "software/SfMGui/MainWindow.hpp"

#include <iostream>

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
  m_tabwidget = new QTabWidget( this ) ;

  m_image_tab = new ImageTab( this ) ;
  m_setting_tab = new SettingTab( this ) ;
  m_result_tab = new ResultTab( this ) ;

  m_tabwidget->addTab( m_image_tab , "Images" ) ;
  m_tabwidget->addTab( m_setting_tab , "Settings" ) ;
  m_tabwidget->addTab( m_result_tab , "Result" ) ;

  setCentralWidget( m_tabwidget ) ;

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
}


} // namespace SfMGui
} // namespace openMVG