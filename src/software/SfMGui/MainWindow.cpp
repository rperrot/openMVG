#include "software/SfMGui/MainWindow.hpp"

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

}

} // namespace SfMGui
} // namespace openMVG