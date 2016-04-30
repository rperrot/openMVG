#ifndef _OPENMVG_SFMGUI_PROJECT_CREATION_WIZARD_OUTPUT_PAGE_HPP_
#define _OPENMVG_SFMGUI_PROJECT_CREATION_WIZARD_OUTPUT_PAGE_HPP_

#include <QWizardPage>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>


namespace openMVG
{
namespace SfMGui
{
/**
* @brief Wizard page used to help the user to select a project path
* @note: we should not use directly this class, prefer ProjectCreationWizard class instead
*/
class ProjectCreationWizardOutputPage : public QWizardPage
{
    Q_OBJECT

  public:

    /**
    * @brief Ctr
    * @param parent parent
    */
    ProjectCreationWizardOutputPage( QWidget * parent = nullptr ) ;

  public slots:

    /**
    * @brief Action to be executed when the user click on the "..." button
    */
    void onClickOpenButton( void ) ;

  private:

    /**
    * @brief Build interface
    */
    void BuildInterface( ) ;

    /**
    * @brief Make connection between elements
    */
    void MakeConnections() ;

    QLabel * m_output_path_label ;
    QLineEdit * m_output_path ;
    QPushButton * m_output_path_btn ;

} ;

}
}

#endif