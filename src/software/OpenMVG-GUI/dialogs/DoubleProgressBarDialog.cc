#include "DoubleProgressBarDialog.hh"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace openMVG_gui
{
/**
* @brief Ctr
* @param parent Parent widget
*/
DoubleProgressBarDialog::DoubleProgressBarDialog( QWidget * parent )
  : QDialog( parent ) ,
    m_canceled( false )
{
  buildInterface() ;
  makeConnections() ;
}

/**
* @brief Indicate if progress was canceled (either by cancel button or by cancel slot)
*/
bool DoubleProgressBarDialog::wasCanceled( void )
{
  return m_canceled ;
}

/**
* @brief Set progress bar range for first progress bar
* @param min Minimum value
* @param max Maximum value
*/
void DoubleProgressBarDialog::setRange1( int min , int max )
{
  m_bar_1->setMinimum( min ) ;
  m_bar_1->setMaximum( max ) ;
}

/**
* @brief Set progress bar range for second progress bar
* @param min Minimum value
* @param max Maximum value
*/
void DoubleProgressBarDialog::setRange2( int min , int max )
{
  m_bar_2->setMinimum( min ) ;
  m_bar_2->setMaximum( max ) ;
}

/**
* @brief Set value of first progress bar
* @param value Value to set
*/
void DoubleProgressBarDialog::setValue1( int value )
{
  m_bar_1->setValue( value ) ;
}

/**
* @brief Set value of second progress bar
* @param value Value to set
*/
void DoubleProgressBarDialog::setValue2( int value )
{
  m_bar_2->setValue( value ) ;
}

/**
* @brief Set label for first progress bar
* @param value New label
*/
void DoubleProgressBarDialog::setLabelText1( const std::string & value )
{
  m_label_1->setText( value.c_str() ) ;
}

/**
* @brief Set label for second progress bar
* @param value New label
*/
void DoubleProgressBarDialog::setLabelText2( const std::string & value )
{
  m_label_2->setText( value.c_str() ) ;
}

/**
* @brief Cancel (ie: hide) progress bar
*/
void DoubleProgressBarDialog::cancel( void )
{
  m_canceled = true ;
  setVisible( false ) ;
}

/**
* @brief Build interface
*/
void DoubleProgressBarDialog::buildInterface( void )
{
  // Progress layout
  m_label_1 = new QLabel ;
  m_bar_1 = new QProgressBar ;
  m_label_2 = new QLabel ;
  m_bar_2 = new QProgressBar ;

  QVBoxLayout * progressLayout = new QVBoxLayout ;
  progressLayout->addWidget( m_label_1 ) ;
  progressLayout->addWidget( m_bar_1 ) ;
  progressLayout->addWidget( m_label_2 ) ;
  progressLayout->addWidget( m_bar_2 ) ;

  // Cancel layout
  /*
  m_push_1 = new QPushButton( "Cancel" ) ;
  QHBoxLayout * cancelLayout = new QHBoxLayout ;
  cancelLayout->addStretch() ;
  cancelLayout->addWidget( m_push_1 ) ;
  cancelLayout->addStretch() ;
  */

  // Final layout
  QVBoxLayout * mainLayout = new QVBoxLayout ;
  mainLayout->addLayout( progressLayout ) ;
//  mainLayout->addLayout( cancelLayout ) ;

  setLayout( mainLayout ) ;
}

/**
* @brief Make connections between widgets
*/
void DoubleProgressBarDialog::makeConnections( void )
{
  // TODO connect push button 
}

} // namespace openMVG_gui