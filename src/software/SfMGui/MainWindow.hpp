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

#include <QMenu>
#include <QAction>

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


    /**
    * @brief Action executed when user create a new project
    */
    void onMenuNewProject( void ) ;

    /**
    * @brief Action executed when user want to open a project
    */
    void onMenuOpenProject( void ) ;


    /**
    * @brief Action executed when user want to save a project
    */
    void onMenuSaveProject( void ) ;


    /**
    * @brief Action executed when user want to save a project to a new directory
    */
    void onMenuSaveAsProject( void ) ;


    /**
    * @brief Action executed when user create want to close a project
    */
    void onMenuCloseProject( void ) ;


    /**
    * @brief Action executed when user want to quit application
    */
    void onMenuQuit( void ) ;


    /**
    * @brief Action executed when user want to load default settings
    */
    void onMenuSettingLoadDefault( void ) ;


    /**
    * @brief Action executed when user want to Load an image dir
    */
    void onMenuProjectLoadImageDir( void ) ;


    /**
    * @brief Action executed when user want to ReLoad an image dir
    */
    void onMenuProjectReloadImageDir( void ) ;


    /**
    * @brief Action executed when user want to compute SfM
    */
    void onMenuProjectComputeSfM( void ) ;


    /**
    * @brief Action executed when user want to export project to MVE format
    */
    void onMenuProjectExportToMVE( void ) ;

    /**
    * @brief Action executed when user want some help (but why ?)
    */
    void onMenuHelp( void ) ;


  private:

    /**
    * @brief Save or close an unsaved project
    * @retval true If project saved or closed
    * @retval false If project unsaved or user give up close/save
    * @retval true If project has not unsaved changes
    */
    bool SaveOrClose( void ) ;

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


    /// Menus
    QMenu * m_file_menu ;
    QAction * m_new_project ;
    QAction * m_open_project ;
    QAction * m_save_project ;
    QAction * m_save_project_as ;
    QAction * m_close_project ;
    QAction * m_quit ;

    QMenu * m_configuration_menu ;
    QAction * m_load_default_configuration ;

    QMenu * m_project_menu ;
    QAction * m_select_input_images_directory ;
    QAction * m_reload_input_directory ;
    QAction * m_compute_sfm ;
    QAction * m_export_to_mve ;

    QMenu * m_help_menu ;
    QAction * m_help_rtfm ;

};
} // namespace SfMGui
} // namespace openMVG

#endif