#ifndef _OPENMVG_SFMGUI_PROJECT_CREATION_WIZARD_HPP_
#define _OPENMVG_SFMGUI_PROJECT_CREATION_WIZARD_HPP_

#include <QWidget>
#include <QWizard>
#include <QWizardPage>

namespace openMVG
{
namespace SfMGui
{

/**
* @brief Simple dialog used to create a new project
* usage is to create the wizard, run exec() and if result of exec() is QDialog::Accepted
* get the valid paths (image path and project path)
*/
class ProjectCreationWizard : public QWizard
{
  public:

    /**
    * @brief Constructor
    * @param parent parent
    */
    ProjectCreationWizard( QWidget * parent ) ;

    /**
    * @brief Get Input image path
    * @return valid image path
    * @note If Wizard has not been run or if it was returned with QDialog::Rejected, result is undefined
    */
    std::string GetInputImageFolder( void ) ;

    /**
    * @brief Get Input image path
    * @return valid project path
    * @note If Wizard has not been run or if it was returned with QDialog::Rejected, result is undefined
    */
    std::string GetOutputProjectFolder( void ) ;

    /**
    * @brief redefinition of accept function
    */
    void accept() override ;

  private:

    std::string m_input_folder ;
    std::string m_output_folder ;
} ;
}
}

#endif