#ifndef _OPENMVG_SFMGUI_MAINWINDOW_HPP_
#define _OPENMVG_SFMGUI_MAINWINDOW_HPP_

#include "software/SfMGui/ImageTab.hpp"
#include "software/SfMGui/ResultTab.hpp"
#include "software/SfMGui/SettingTab.hpp"
#include "software/SfMGui/SfMProject.hpp"

#include <QMainWindow>
#include <QTabWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>


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

    /**
    * @brief Reset interface to default settings
    */
    void Reset( void ) ;

  public slots:

    /**
    * @brief Action to be executed when user wants to add an image
    */
    void onAddImage( void ) ;

    /**
    * @brief Action to be executed when user wants to add a folder of images
    */
    void onAddFolder( void ) ;

    /**
    * @brief Action to be executed when user select input images
    */
    void onSelectInputImages( void ) ;

    /**
    * @brief Action to be executed when user select project folder
    */
    void onSelectOutputProjectFolder( void ) ;

  private:

    void DoProjectCreation( void ) ;

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

    /// The project
    std::shared_ptr< SfMProject > m_project ;

    /// Input folder
    QPushButton * m_input_folder_button ;
    QLabel * m_input_folder_label ;
    QLineEdit * m_input_folder_text ;


    /// Project folder
    QPushButton * m_project_folder_button ;
    QLabel * m_project_folder_label ;
    QLineEdit * m_project_folder_text ;


    /// Container for everything
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