#include "software/SfMGui/ProjectCreationWizardInputPage.hpp"

#include <QGridLayout>
#include <QFileDialog>
#include <QDir>

namespace openMVG
{
namespace SfMGui
{

/**
* @brief Ctr
* @param parent parent
*/
ProjectCreationWizardInputPage::ProjectCreationWizardInputPage( QWidget * parent )
  : QWizardPage( parent )
{
  BuildInterface() ;
  MakeConnections() ;

  registerField( "inputPath*" , m_input_path ) ;
}

/**
* @brief Action executed when user click the "..." button
*/
void ProjectCreationWizardInputPage::onClickOpenButton( void )
{
  QString dir = QFileDialog::getExistingDirectory( this , "Input image directory" , QDir::homePath() ,  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ) ;

  if( ! ( dir.isEmpty() || dir.isNull() ) )
  {
    m_input_path->setText( dir );
  }
}

/**
* @brief build input page
*/
void ProjectCreationWizardInputPage::BuildInterface( )
{
  setTitle( "Select input path" ) ;
  setSubTitle( "Please select a folder containing your input images" ) ;

  m_input_path_label = new QLabel( "Image path" ) ;
  m_input_path = new QLineEdit ;
  m_input_path->setEnabled( false ) ;
  m_input_path_btn = new QPushButton( "..." ) ;

  QGridLayout * layout = new QGridLayout ;

  layout->addWidget( m_input_path_label , 0 , 0 ) ;
  layout->addWidget( m_input_path , 0 , 1 ) ;
  layout->addWidget( m_input_path_btn , 0 , 2 ) ;

  setLayout( layout ) ;
}

/**
* @brief make connection between elements of the page
*/
void ProjectCreationWizardInputPage::MakeConnections( void )
{
  connect( m_input_path_btn , SIGNAL( clicked() ) , this , SLOT( onClickOpenButton() ) ) ;
}

}
}