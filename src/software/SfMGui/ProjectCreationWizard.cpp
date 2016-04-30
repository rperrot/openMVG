#include "software/SfMGui/ProjectCreationWizard.hpp"
#include "software/SfMGui/ProjectCreationWizardInputPage.hpp"
#include "software/SfMGui/ProjectCreationWizardOutputPage.hpp"

namespace openMVG
{
namespace SfMGui
{

/**
* @brief Constructor
* @param parent parent
*/
ProjectCreationWizard::ProjectCreationWizard( QWidget * parent )
  : QWizard( parent )
{
  addPage( new ProjectCreationWizardInputPage ) ;
  addPage( new ProjectCreationWizardOutputPage ) ;

  setWindowTitle( "Project creation" ) ;
}

/**
* @brief Get Input image path
* @return valid image path
* @note If Wizard has not been run or if it was returned with QDialog::Rejected, result is undefined
*/
std::string ProjectCreationWizard::GetInputImageFolder( void )
{
  return m_input_folder ;
}

/**
* @brief Get Input image path
* @return valid project path
* @note If Wizard has not been run or if it was returned with QDialog::Rejected, result is undefined
*/
std::string ProjectCreationWizard::GetOutputProjectFolder( void )
{
  return m_output_folder ;
}


/**
* @brief redefinition of accept function
*/
void ProjectCreationWizard::accept()
{
  m_input_folder = field( "inputPath" ).toString().toStdString() ;
  m_output_folder = field( "outputPath" ).toString().toStdString() ;

  QDialog::accept() ;
}



}
}