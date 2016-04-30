#include "software/SfMGui/ProjectCreationWizardOutputPage.hpp"

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
ProjectCreationWizardOutputPage::ProjectCreationWizardOutputPage( QWidget * parent )
  : QWizardPage( parent )
{
  BuildInterface() ;
  MakeConnections() ;

  registerField( "outputPath*" , m_output_path ) ;
}

/**
* @brief Action to be executed when the user click on the "..." button
*/
void ProjectCreationWizardOutputPage::onClickOpenButton( void )
{
  QString dir = QFileDialog::getExistingDirectory( this , "Output project path" , QDir::homePath() ,  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ) ;

  if( ! ( dir.isEmpty() || dir.isNull() ) )
  {
    m_output_path->setText( dir );
  }
}


/**
* @brief Build interface
*/
void ProjectCreationWizardOutputPage::BuildInterface( )
{
  setTitle( "Select project path" ) ;
  setSubTitle( "Please select a folder to save project" ) ;

  m_output_path_label = new QLabel( "Project path" ) ;
  m_output_path = new QLineEdit ;
  m_output_path->setEnabled( false ) ;
  m_output_path_btn = new QPushButton( "..." ) ;

  QGridLayout * layout = new QGridLayout ;

  layout->addWidget( m_output_path_label , 0 , 0 ) ;
  layout->addWidget( m_output_path , 0 , 1 ) ;
  layout->addWidget( m_output_path_btn , 0 , 2 ) ;

  setLayout( layout ) ;
}

/**
* @brief Make connection between elements
*/
void ProjectCreationWizardOutputPage::MakeConnections()
{
  connect( m_output_path_btn , SIGNAL( clicked() ) , this , SLOT( onClickOpenButton() ) ) ;
}


}
}