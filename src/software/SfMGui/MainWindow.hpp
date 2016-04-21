#ifndef _OPENMVG_SFMGUI_MAINWINDOW_HPP_
#define _OPENMVG_SFMGUI_MAINWINDOW_HPP_

#include "software/SfMGui/ImageTab.hpp"
#include "software/SfMGui/ResultTab.hpp"
#include "software/SfMGui/SettingTab.hpp"

#include <QMainWindow>
#include <QTabWidget>


namespace openMVG
{
namespace SfMGui
{

/**
* @brief Main window of the SfMGui
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    /**
    * @brief Default constructor
    */
    MainWindow( void ) ;

  public slots:

    /**
    * @brief Action to be executed when user wants to add an image
    */
    void onAddImage( void ) ;

    /**
    * @brief Action to be executed when user wants to add a folder of images
    */
    void onAddFolder( void ) ;

  private:

    /**
    * @brief Build all interface widgets
    */
    void BuildInterface( void ) ;

    /**
    * @brief Build all menus
    */
    void BuildMenus( void ) ;

    /**
    * @brief Make connections between interface elements
    */
    void MakeConnections( void ) ;


    QTabWidget * m_tabwidget ;

    /// Tab widget used to manage the images
    ImageTab * m_image_tab ;

    /// Tab widget used to set all settings for the SfM process
    SettingTab * m_setting_tab ;

    /// Tab widget used to show the result of the SfM process
    ResultTab * m_result_tab ;

};
} // namespace SfMGui
} // namespace openMVG

#endif