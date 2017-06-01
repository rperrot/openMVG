#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_NEW_PROJECT_DIALOG_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_NEW_PROJECT_DIALOG_HH_

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace openMVG_gui
{
class NewProjectDialog : public QDialog
{
  public:

    /**
    * @brief Ctr
    */
    NewProjectDialog( QWidget * parent ) ;

    /**
    * @brief Path of the output project
    */
    std::string outputProjectPath( void ) const ;

    /**
    * @brief Path of the input image
    */
    std::string inputImagePath( void ) const ;


  public slots:

    /**
    * @brief action to be executed when user want to select output folder
    */
    void onWantToSelectProjectPath( void ) ;

    /**
    * @brief action to be executed when user want to select input image folder
    */
    void onWantToSelectImagePath( void ) ;

    /**
    * @brief action to be executed when user click on cancel button
    */
    void onCancel( void ) ;

    /**
    * @brief action to be executed when user click on ok button
    */
    void onOk( void ) ;

  private:

    void buildInterface( void ) ;
    void makeConnections( void ) ; 

    /// The labels
    QLabel * m_lbl_input_image ;
    QLabel * m_lbl_output_project ;

    /// The line edit
    QLineEdit * m_line_input ;
    QLineEdit * m_line_project ;

    /// The buttons
    QPushButton * m_btn_project ;
    QPushButton * m_btn_image ;

    /// Validation btn
    QPushButton * m_btn_cancel ;
    QPushButton * m_btn_ok ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif