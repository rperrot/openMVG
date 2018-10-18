#include "AddSensorEntryDialog.hh"

#include <QDoubleValidator>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace openMVG_gui
{
AddSensorEntryDialog::AddSensorEntryDialog( QWidget *parent ) :
    QDialog( parent )
{
  buildInterface();
  makeConnections();
}

std::string AddSensorEntryDialog::brand( void ) const
{
  return m_brandTextEdit->text().toStdString();
}

std::string AddSensorEntryDialog::model( void ) const
{
  return m_modelTextEdit->text().toStdString();
}

double AddSensorEntryDialog::sensorWidth( void ) const
{
  return std::stod( m_sensorWidthTextEdit->text().toStdString() );
}

/**
   * @brief action to be executed when user click on cancel button
   */
void AddSensorEntryDialog::onCancel( void )
{
  done( QDialog::Rejected );
}

/**
   * @brief action to be executed when user click on OK button
   */
void AddSensorEntryDialog::onOk( void )
{
  done( QDialog::Accepted );
}

void AddSensorEntryDialog::buildInterface( void )
{
  QGridLayout *cmdLayout = new QGridLayout;

  QVBoxLayout *mainLayout = new QVBoxLayout;
  QHBoxLayout *btnLayout  = new QHBoxLayout;

  QLabel *brand         = new QLabel( "Brand" );
  m_brandTextEdit       = new QLineEdit;
  QLabel *model         = new QLabel( "Model" );
  m_modelTextEdit       = new QLineEdit;
  QLabel *sensorWidth   = new QLabel( "Sensor width" );
  m_sensorWidthTextEdit = new QLineEdit;
  m_sensorWidthTextEdit->setValidator( new QDoubleValidator );

  cmdLayout->addWidget( brand, 0, 0 );
  cmdLayout->addWidget( m_brandTextEdit, 0, 1 );
  cmdLayout->addWidget( model, 1, 0 );
  cmdLayout->addWidget( m_modelTextEdit, 1, 1 );
  cmdLayout->addWidget( sensorWidth, 2, 0 );
  cmdLayout->addWidget( m_sensorWidthTextEdit, 2, 1 );

  m_ok = new QPushButton( "Ok" );
  m_ok->setDefault( true );
  m_cancel = new QPushButton( "Cancel" );
  m_cancel->setDefault( false );

  btnLayout->addStretch();
  btnLayout->addWidget( m_cancel );
  btnLayout->addWidget( m_ok );

  mainLayout->addLayout( cmdLayout );
  mainLayout->addLayout( btnLayout );

  setLayout( mainLayout );
}
void AddSensorEntryDialog::makeConnections( void )
{
  connect( m_ok, SIGNAL( clicked() ), this, SLOT( onOk() ) );
  connect( m_cancel, SIGNAL( clicked() ), this, SLOT( onCancel() ) );
}

} // namespace openMVG_gui