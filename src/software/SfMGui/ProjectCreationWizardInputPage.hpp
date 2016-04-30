#ifndef _OPENMVG_SFMGUI_PROJECT_CREATION_WIZARD_INPUT_PAGE_HPP_
#define _OPENMVG_SFMGUI_PROJECT_CREATION_WIZARD_INPUT_PAGE_HPP_

#include <QWizardPage>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

namespace openMVG
{
namespace SfMGui
{

/**
* @brief Wizard page used to help user to select an input image folder
* @note: we should not use directly this class, prefer ProjectCreationWizard class instead
*/
class ProjectCreationWizardInputPage : public QWizardPage
{
    Q_OBJECT

  public:

    /**
    * @brief Ctr
    * @param parent parent
    */
    ProjectCreationWizardInputPage( QWidget * parent = nullptr ) ;

  public slots:

    /**
    * @brief Action executed when user click the "..." button
    */
    void onClickOpenButton( void ) ;

  private:

    /**
    * @brief build input page
    */
    void BuildInterface( ) ;

    /**
    * @brief make connection between elements of the page
    */
    void MakeConnections() ;

    QLabel * m_input_path_label ;
    QLineEdit * m_input_path ;
    QPushButton * m_input_path_btn ;

} ;
}
}

#endif