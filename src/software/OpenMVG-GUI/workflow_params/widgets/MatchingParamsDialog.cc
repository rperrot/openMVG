#include "MatchingParamsDialog.hh"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <iostream>

namespace openMVG_gui
{

/**
* @brief Ctr
* @param parent Parent Widget
* @param initParams Initial Parameters passed to the dialog
*/
MatchingParamsDialog::MatchingParamsDialog( QWidget * parent , const MatchingParams & initParams )
  : QDialog( parent ) ,
    m_initial_params( initParams )
{
  buildInterface() ;
  makeConnections() ;

  setParams( initParams ) ;
  setWindowTitle( "Matching params" ) ;
}

/**
* @brief Get current parameters
* @return Current parameters corresponding to the interface
*/
MatchingParams MatchingParamsDialog::params( void )
{
  const int matchMethId = m_params_matching_method->currentIndex() ;
  const int geomId = m_params_geometry_type->currentIndex() ;
  const int maxIter = m_max_iteration->value() ;
  const double ratio = m_ratio->value() ;

  MatchingGeometryType geom_type = MATCHING_GEOMETRY_TYPE_FUNDAMENTAL ;
  MatchingMethod match_method = MATCHING_METHOD_BRUTEFORCE_L2 ;

  switch( matchMethId )
  {
    case 0 :
    {
      match_method = MATCHING_METHOD_BRUTEFORCE_L2 ;
      break ;
    }
    case 1 :
    {
      match_method = MATCHING_METHOD_ANN_L2 ;
      break ;
    }
    case 2 :
    {
      match_method = MATCHING_METHOD_CASCADE_HASHING_L2 ;
      break ;
    }
    case 3 :
    {
      match_method = MATCHING_METHOD_FAST_CASCADE_HASHING_L2 ;
      break ;
    }
    case 4 :
    {
      match_method = MATCHING_METHOD_BRUTE_FORCE_HAMMING ;
      break ;
    }
    default:
    {
      std::cerr << "Unknown matching method" << std::endl ;
      std::cerr << "Defaulting to brute force L2" << std::endl ;
    }
  }

  switch( geomId )
  {
    case 0 :
    {
      geom_type = MATCHING_GEOMETRY_TYPE_FUNDAMENTAL ;
      break ;
    }
    case 1 :
    {
      geom_type = MATCHING_GEOMETRY_TYPE_ESSENTIAL ;
      break ;
    }
    case 2 :
    {
      geom_type = MATCHING_GEOMETRY_TYPE_HOMOGRAPHY ;
      break ;
    }
    default:
    {
      std::cerr << "Unknown geometry type" << std::endl ;
      std::cerr << "Defaulting to Fundamental matrix" << std::endl ;
    }
  }

  return MatchingParams( match_method , ratio , geom_type , maxIter ) ;
}

/**
* @brief action to be executed when user click on cancel button
*/
void MatchingParamsDialog::onCancel( void )
{
  done( QDialog::Rejected ) ;
}

/**
* @brief action to be executed when user click on ok button
*/
void MatchingParamsDialog::onOk( void )
{
  // TODO : is it mandatory to check if combo boxes have something selected ?
  // (ie: can it happened ? if true, this function should check if dialog is valid)
  done( QDialog::Accepted ) ;
}

/**
* @brief Action to be executed when user click on reset button
*/
void MatchingParamsDialog::onReset( void )
{
  setParams( m_initial_params ) ;
}

/**
* @brief Set params to a specified value
* @param value New value
*/
void MatchingParamsDialog::setParams( const MatchingParams & value )
{
  const int maxIter = value.maxIterationFiltering() ;
  const double ratio = value.distanceRatio() ;
  const MatchingGeometryType geom_type = value.geometricModel() ;
  const MatchingMethod match_method = value.method() ;

  m_ratio->setValue( ratio ) ;
  m_max_iteration->setValue( maxIter ) ;

  switch( geom_type )
  {
    case MATCHING_GEOMETRY_TYPE_FUNDAMENTAL:
    {
      m_params_geometry_type->setCurrentIndex( 0 ) ;
      break ;
    }
    case MATCHING_GEOMETRY_TYPE_ESSENTIAL:
    {
      m_params_geometry_type->setCurrentIndex( 1 ) ;
      break ;
    }
    case MATCHING_GEOMETRY_TYPE_HOMOGRAPHY:
    {
      m_params_geometry_type->setCurrentIndex( 2 ) ;
      break ;
    }
  }

  /*
    m_params_matching_method->addItem( "Brute Force L2" ) ;
  m_params_matching_method->addItem( "ANN L2" ) ;
  m_params_matching_method->addItem( "Cascade Hashing L2" ) ;
  m_params_matching_method->addItem( "Fast Cascade Hashing L2" ) ;
  m_params_matching_method->addItem( "Brute Force Hamming" ) ;
  */

  switch( match_method )
  {
    case MATCHING_METHOD_BRUTEFORCE_L2 :
    {
      m_params_matching_method->setCurrentIndex( 0 ) ;
      break ;
    }
    case MATCHING_METHOD_ANN_L2 :
    {
      m_params_matching_method->setCurrentIndex( 1 ) ;
      break ;
    }
    case MATCHING_METHOD_CASCADE_HASHING_L2 :
    {
      m_params_matching_method->setCurrentIndex( 2 ) ;
      break ;
    }
    case MATCHING_METHOD_FAST_CASCADE_HASHING_L2 :
    {
      m_params_matching_method->setCurrentIndex( 3 ) ;
      break ;
    }
    case MATCHING_METHOD_BRUTE_FORCE_HAMMING :
    {
      m_params_matching_method->setCurrentIndex( 4 ) ;
      break ;
    }
  }
}

void MatchingParamsDialog::buildInterface( void )
{

  QGroupBox * grpMatchParams = new QGroupBox( "Matching" ) ;
  QGridLayout * layoutMatching = new QGridLayout ;

  QLabel * labelMethod = new QLabel( "Method" ) ;
  QLabel * labelRatio = new QLabel( "Distance ratio" ) ;
  m_params_matching_method = new QComboBox ;
  m_params_matching_method->addItem( "Brute Force L2" ) ;
  m_params_matching_method->addItem( "ANN L2" ) ;
  m_params_matching_method->addItem( "Cascade Hashing L2" ) ;
  m_params_matching_method->addItem( "Fast Cascade Hashing L2" ) ;
  m_params_matching_method->addItem( "Brute Force Hamming" ) ;

  m_ratio = new QDoubleSpinBox ;
  m_ratio->setMinimum( 0.0 ) ;
  m_ratio->setMaximum( 2.0 ) ;
  m_ratio->setDecimals( 3 ) ;

  layoutMatching->addWidget( labelMethod , 0 , 0 ) ;
  layoutMatching->addWidget( m_params_matching_method , 0 , 1 ) ;
  layoutMatching->addWidget( labelRatio , 1 , 0 ) ;
  layoutMatching->addWidget( m_ratio , 1 , 1 ) ;

  grpMatchParams->setLayout( layoutMatching ) ;

  QGroupBox * grpFilteringParams = new QGroupBox( "Filtering" ) ;
  QGridLayout * layoutFiltering = new QGridLayout ;

  QLabel * labelGeometricModel = new QLabel ( "Model" ) ;
  QLabel * labelIteration = new QLabel ( "Max Iteration" ) ;
  m_params_geometry_type = new QComboBox ;
  m_params_geometry_type->addItem( "Fundamental" ) ;
  m_params_geometry_type->addItem( "Essential" ) ;
  m_params_geometry_type->addItem( "Homography" ) ;

  m_max_iteration = new QSpinBox ;
  m_max_iteration->setMinimum( 1 ) ;
  m_max_iteration->setMaximum( 32768 ) ;

  layoutFiltering->addWidget( labelGeometricModel , 0 , 0 ) ;
  layoutFiltering->addWidget( m_params_geometry_type , 0 , 1 ) ;
  layoutFiltering->addWidget( labelIteration , 1 , 0 ) ;
  layoutFiltering->addWidget( m_max_iteration , 1 , 1 ) ;

  grpFilteringParams->setLayout( layoutFiltering ) ;

  // Btns
  QHBoxLayout * btnLayout = new QHBoxLayout ;

  m_ok_btn = new QPushButton( "Ok" ) ;
  m_cancel_btn = new QPushButton( "Cancel" ) ;
  m_reset_btn = new QPushButton( "Reset" ) ;

  m_ok_btn->setDefault( true ) ;
  m_cancel_btn->setDefault( false ) ;
  m_reset_btn->setDefault( false ) ;

  btnLayout->addStretch() ;
  btnLayout->addWidget( m_reset_btn ) ;
  btnLayout->addWidget( m_cancel_btn ) ;
  btnLayout->addWidget( m_ok_btn ) ;

  QVBoxLayout * mainLayout = new QVBoxLayout ;

  mainLayout->addWidget( grpMatchParams ) ;
  mainLayout->addWidget( grpFilteringParams ) ;
  mainLayout->addLayout( btnLayout ) ;

  setLayout( mainLayout ) ;
  adjustSize();
  mainLayout->setSizeConstraint( QLayout::SetFixedSize );
}

void MatchingParamsDialog::makeConnections( void )
{
  connect( m_cancel_btn , SIGNAL( clicked() ) , this , SLOT( onCancel() ) );
  connect( m_reset_btn , SIGNAL( clicked() ) , this , SLOT( onReset() ) ) ;
  connect( m_ok_btn , SIGNAL( clicked() ) , this , SLOT( onOk() ) );
}


} // namespace openMVG_gui