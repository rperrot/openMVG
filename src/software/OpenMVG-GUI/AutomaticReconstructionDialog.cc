#include "AutomaticReconstructionDialog.hh"

#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QVBoxLayout>

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"


namespace openMVG_gui
{

AutomaticReconstructionDialog::AutomaticReconstructionDialog( QWidget * parent )
  : QDialog( parent )
{
  BuildInterface() ;
  MakeConnections() ;

  setWindowTitle( "Automatic Reconstruction" ) ;
}

/**
* @brief Build interface widgets
*/
void AutomaticReconstructionDialog::BuildInterface( void )
{
  // Main parameters : the in/out paths
  QGroupBox * grpMainParams = new QGroupBox( "Main parameters" ) ;

  QGridLayout * layoutMainParams = new QGridLayout ;

  m_input_folder_lbl = new QLabel( "Input image folder" ) ;
  m_input_folder_line = new QLineEdit( this ) ;
  m_input_folder_line->setEnabled( false ) ;
  m_input_folder_btn = new QPushButton(  "..." , this ) ;

  m_output_project_folder_lbl = new QLabel( "Output project folder" ) ;
  m_output_project_folder_line = new QLineEdit( this ) ;
  m_output_project_folder_line->setEnabled( false ) ;
  m_output_project_folder_btn = new QPushButton( "..." , this ) ;


  layoutMainParams->addWidget( m_input_folder_lbl , 0 , 0 ) ;
  layoutMainParams->addWidget( m_input_folder_line , 0 , 1 ) ;
  layoutMainParams->addWidget( m_input_folder_btn , 0 , 2 ) ;

  layoutMainParams->addWidget( m_output_project_folder_lbl , 1 , 0 ) ;
  layoutMainParams->addWidget( m_output_project_folder_line , 1 , 1 ) ;
  layoutMainParams->addWidget( m_output_project_folder_btn , 1 , 2 ) ;

  grpMainParams->setLayout( layoutMainParams ) ;

  // Options
  QGroupBox * grpOptions = new QGroupBox( "Options" ) ;

  QGridLayout * layoutOptions = new QGridLayout ;

  m_feature_preset_lbl = new QLabel( "Preset" ) ;
  m_feature_preset_combo = new QComboBox ;
  m_feature_preset_combo->addItem( "Normal" ) ;
  m_feature_preset_combo->addItem( "High" ) ;
  m_feature_preset_combo->addItem( "Ultra" ) ;

  layoutOptions->addWidget( m_feature_preset_lbl , 0 , 0 ) ;
  layoutOptions->addWidget( m_feature_preset_combo , 0 , 1 ) ;

  grpOptions->setLayout( layoutOptions ) ;

  // Buttons
  m_btn_cancel = new QPushButton( "Cancel" ) ;
  m_btn_cancel->setDefault( false ) ;
  m_btn_ok = new QPushButton( "OK" ) ;
  m_btn_ok->setDefault( true ) ;

  QHBoxLayout * btnLayout = new QHBoxLayout ;
  btnLayout->addStretch() ;
  btnLayout->addWidget( m_btn_cancel ) ;
  btnLayout->addWidget( m_btn_ok ) ;

  // Add to the final layout
  QVBoxLayout * mainLayout = new QVBoxLayout ;
  mainLayout->addWidget( grpMainParams ) ;
  mainLayout->addWidget( grpOptions ) ;
  mainLayout->addLayout( btnLayout ) ;

  setLayout( mainLayout ) ;
  adjustSize();
  mainLayout->setSizeConstraint( QLayout::SetFixedSize );
}

/**
* @brief Path of the input image
*/
std::string AutomaticReconstructionDialog::inputImagePath( void ) const
{
  QString value = m_input_folder_line->text( ) ;
  if( value.isEmpty() || value.isNull() )
  {
    return std::string() ;
  }
  else
  {
    return value.toStdString() ;
  }
}

/**
* @brief Path of the output project
*/
std::string AutomaticReconstructionDialog::outputProjectPath( void ) const
{
  QString value = m_output_project_folder_line->text( ) ;
  if( value.isEmpty() || value.isNull() )
  {
    return std::string() ;
  }
  else
  {
    return value.toStdString() ;
  }
}

/**
* @brief Reconstruction preset selected
*/
AutomaticReconstructionPreset AutomaticReconstructionDialog::preset( void ) const
{
  const int index = m_feature_preset_combo->currentIndex() ;
  switch( index )
  {
    case 0 :
    {
      return AUTOMATIC_RECONSTRUCTION_PRESET_NORMAL ;
    }
    case 1 :
    {
      return AUTOMATIC_RECONSTRUCTION_PRESET_HIGH ;
    }
    case 2 :
    {
      return AUTOMATIC_RECONSTRUCTION_PRESET_ULTRA ;
    }
  }
  QMessageBox::critical( ( QWidget* )this , "Error" , "No preset selected, using Normal" ) ;
  return AUTOMATIC_RECONSTRUCTION_PRESET_NORMAL ;
}


/**
* @brief action to be executed when user click on cancel button
*/
void AutomaticReconstructionDialog::onCancel( void )
{
  done( QDialog::Rejected ) ;
}

/**
* @brief action to be executed when user click on OK button
*/
void AutomaticReconstructionDialog::onOk( void )
{
  // Check if paths are valid
  const std::string p_path = outputProjectPath() ;
  const std::string i_path = inputImagePath() ;

  bool projectPathValid = stlplus::folder_exists( p_path ) ;
  bool inputPathValid = stlplus::folder_exists( i_path ) ;
  bool valid = projectPathValid && inputPathValid ;
  if( valid )
  {
    done( QDialog::Accepted ) ;
  }
  else
  {
    if( ! projectPathValid )
    {
      QMessageBox::critical( this , "Error" , "Project path is invalid" ) ;
    }
    else
    {
      QMessageBox::critical( this , "Error" , "Input image path is invalid" ) ;
    }
  }
}

/**
* @brief action to be executed when user want to select output folder
*/
void AutomaticReconstructionDialog::onWantToSelectProjectPath( void )
{
  QString dir =  QFileDialog::getExistingDirectory( this, tr( "Select project directory" ),
                 QDir::homePath() ,
                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
  if( ! ( dir.isEmpty() || dir.isNull() ) )
  {
    m_output_project_folder_line->setText( dir ) ;
  }
}

/**
* @brief action to be executed when user want to select input image folder
*/
void AutomaticReconstructionDialog::onWantToSelectImagePath( void )
{
  QString dir =  QFileDialog::getExistingDirectory( this, tr( "Select input image directory" ),
                 QDir::homePath() ,
                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
  if( ! ( dir.isEmpty() || dir.isNull() ) )
  {
    m_input_folder_line->setText( dir ) ;
  }
}


/**
* @brief Make connections between elements
*/
void AutomaticReconstructionDialog::MakeConnections( void )
{
  connect( m_btn_cancel , SIGNAL( clicked() ) , this , SLOT( onCancel() ) )  ;
  connect( m_btn_ok , SIGNAL( clicked() ) , this , SLOT( onOk() ) ) ;
  connect( m_input_folder_btn , SIGNAL( clicked() ) , this , SLOT( onWantToSelectImagePath() ) ) ;
  connect( m_output_project_folder_btn , SIGNAL( clicked() ) , this , SLOT( onWantToSelectProjectPath() ) ) ;
}

} // namespace openMVG_gui