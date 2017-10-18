// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "SfMParamsDialog.hh"

#include <QButtonGroup>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace openMVG_gui
{

/**
* @brief Ctr
* @param method Initial SfM method
* @param incr_params Initial sequential SfM params
* @param glo_params Initial global SfM params
*/
SfMParamsDialog::SfMParamsDialog( QWidget * parent ,
                                  const SfMMethod method ,
                                  const IncrementalSfMParams & incr_params ,
                                  const GlobalSfMParams & glo_params ,
                                  const std::vector<int> & imageIds )
  : QDialog( parent ) ,
    m_initial_method( method ) ,
    m_initial_seq_params( incr_params ) ,
    m_initial_glo_params( glo_params ) ,
    m_image_ids( imageIds )
{
  buildInterface() ;
  makeConnections() ;

  setParams( method , incr_params , glo_params ) ;

  setWindowTitle( "SfM Params" ) ;
}


/**
* @brief Current SfM method
* @return Get current SfM method
*/
SfMMethod SfMParamsDialog::method( void ) const
{
  if( m_radio_incr->isChecked() )
  {
    return SFM_METHOD_INCREMENTAL ;
  }
  else
  {
    return SFM_METHOD_GLOBAL ;
  }
}

/**
* @brief Current incremental params
* @return Get current incremental SfM params
*/
IncrementalSfMParams SfMParamsDialog::incrementalParams( void ) const
{
  // Initial pair
  std::pair<int, int> initPair ;
  if( m_incr_use_automatic_pair->isChecked() )
  {
    initPair.first = -1 ;
    initPair.second = -1 ;
  }
  else
  {
    std::stringstream str_first ;
    str_first << m_incr_first_image_id->currentText().toStdString() ;

    str_first >> initPair.first ;

    std::stringstream str_second ;
    str_second << m_incr_second_image_id->currentText().toStdString() ;

    str_second >> initPair.second ;
  }

  // Unknown intrinsic type ?
  const int id_intrin = m_incr_intrinsic_type->currentIndex() ;
  const int id_refine = m_incr_refine_type->currentIndex() ;

  IntrinsicParams intrin_param ;
  switch( id_intrin )
  {
    case 0 :
    {
      intrin_param = IntrinsicParams( INTRINSIC_TYPE_PINHOLE ) ;
      break ;
    }
    case 1 :
    {
      intrin_param = IntrinsicParams( INTRINSIC_TYPE_PINHOLE_RADIAL_1 ) ;
      break ;
    }
    case 2 :
    {
      intrin_param = IntrinsicParams( INTRINSIC_TYPE_PINHOLE_RADIAL_3 ) ;
      break ;
    }
    case 3 :
    {
      intrin_param = IntrinsicParams( INTRINSIC_TYPE_PINHOLE_BROWN ) ;
      break ;
    }
    case 4 :
    {
      intrin_param = IntrinsicParams( INTRINSIC_TYPE_PINHOLE_FISHEYE ) ;
      break ;
    }
  }

  IntrinsicRefinerOptions refine_opts ;
  switch( id_refine )
  {
    case 0 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_NONE ;
      break ;
    }
    case 1 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH ;
      break ;
    }
    case 2 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_PRINCIPAL_POINT ;
      break ;
    }
    case 3 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_DISTORTION ;
      break ;
    }
    case 4 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH_PRINCIPAL_POINT ;
      break ;
    }
    case 5 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH_DISTORTION ;
      break ;
    }
    case 6 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_PRINCIPAL_POINT_DISTORTION ;
      break ;
    }
    case 7 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_ALL ;
      break ;
    }
  }

  return IncrementalSfMParams( initPair , intrin_param , refine_opts ) ;
}

/**
* @brief Current global params
* @return Get current global SfM params
*/
GlobalSfMParams SfMParamsDialog::globalParams( void ) const
{
  const int id_rot_avg = m_glo_rotation_avg_method->currentIndex() ;
  const int id_tra_avg = m_glo_translation_avg_method->currentIndex() ;
  const int id_refine = m_glo_refine_type->currentIndex() ;

  RotationAveragingMethod rot_meth ;
  switch( id_rot_avg )
  {
    case 0 :
    {
      rot_meth = ROTATION_AVERAGING_L1 ;
      break ;
    }
    case 1 :
    {
      rot_meth = ROTATION_AVERAGING_L2 ;
      break ;
    }
  }

  TranslationAveragingMethod tra_meth ;
  switch( id_tra_avg )
  {
    case 0 :
    {
      tra_meth = TRANSLATION_AVERAGING_L1 ;
      break ;
    }
    case 1 :
    {
      tra_meth = TRANSLATION_AVERAGING_L2 ;
      break ;
    }
    case 2 :
    {
      tra_meth = TRANSLATION_AVERAGING_SOFT_L1 ;
      break ;
    }
  }

  IntrinsicRefinerOptions refine_opts ;
  switch( id_refine )
  {
    case 0 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_NONE ;
      break ;
    }
    case 1 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH ;
      break ;
    }
    case 2 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_PRINCIPAL_POINT ;
      break ;
    }
    case 3 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_DISTORTION ;
      break ;
    }
    case 4 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH_PRINCIPAL_POINT ;
      break ;
    }
    case 5 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH_DISTORTION ;
      break ;
    }
    case 6 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_PRINCIPAL_POINT_DISTORTION ;
      break ;
    }
    case 7 :
    {
      refine_opts = IntrinsicRefinerOptions::INTRINSIC_REFINER_ALL ;
      break ;
    }
  }

  return GlobalSfMParams( rot_meth , tra_meth , refine_opts ) ;
}


/**
* @brief action to be executed when user click on cancel button
*/
void SfMParamsDialog::onCancel( void )
{
  done( QDialog::Rejected ) ;
}

/**
* @brief action to be executed when user click on ok button
*/
void SfMParamsDialog::onOk( void )
{
  // TODO : check if initial pair is valid for incremental (if non auto checked)
  done( QDialog::Accepted ) ;
}

/**
* @brief Action to be executed when user click on reset button
*/
void SfMParamsDialog::onReset( void )
{
  setParams( m_initial_method , m_initial_seq_params , m_initial_glo_params ) ;
}

/**
* @brief Action to be executed when used has selected a SfM method
*/
void SfMParamsDialog::onSelectMethod( void )
{
  if( m_radio_incr->isChecked() )
  {
    m_grp_incremental->setEnabled( true ) ;
    m_grp_global->setEnabled( false ) ;
  }
  else
  {
    m_grp_incremental->setEnabled( false ) ;
    m_grp_global->setEnabled( true ) ;
  }
}

/**
* @brief Action to be executed when user has checked/unchecked automatic pair for incremental SfM
*/
void SfMParamsDialog::onCheckAuto( void )
{
  if( m_incr_use_automatic_pair->isChecked() )
  {
    m_incr_first_image_id->setEnabled( false ) ;
    m_incr_second_image_id->setEnabled( false ) ;
  }
  else
  {
    m_incr_first_image_id->setEnabled( true ) ;
    m_incr_second_image_id->setEnabled( true ) ;
  }
}


/**
* @brief Set params
* @param meth SfM Method
* @param incr_params Incremental params
* @param glo_params Global params
*/
void SfMParamsDialog::setParams( const SfMMethod meth ,
                                 const IncrementalSfMParams & incr_params ,
                                 const GlobalSfMParams & glo_params )
{
  if( meth == SFM_METHOD_INCREMENTAL )
  {
    m_radio_incr->setChecked( true ) ;
    m_radio_glo->setChecked( false ) ;
  }
  else
  {
    m_radio_incr->setChecked( false ) ;
    m_radio_glo->setChecked( true ) ;
  }

  // Incremental
  switch( incr_params.unknownIntrinsic().type() )
  {
    case INTRINSIC_TYPE_PINHOLE :
    {
      m_incr_intrinsic_type->setCurrentIndex( 0 ) ;
      break ;
    }
    case INTRINSIC_TYPE_PINHOLE_RADIAL_1 :
    {
      m_incr_intrinsic_type->setCurrentIndex( 1 ) ;
      break ;
    }
    case INTRINSIC_TYPE_PINHOLE_RADIAL_3 :
    {
      m_incr_intrinsic_type->setCurrentIndex( 2 ) ;
      break ;
    }
    case INTRINSIC_TYPE_PINHOLE_BROWN :
    {
      m_incr_intrinsic_type->setCurrentIndex( 3 ) ;
      break ;
    }
    case INTRINSIC_TYPE_PINHOLE_FISHEYE :
    {
      m_incr_intrinsic_type->setCurrentIndex( 4 ) ;
      break ;
    }
  }
  switch( incr_params.refinerOptions() )
  {
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_NONE :
    {
      m_incr_refine_type->setCurrentIndex( 0 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH :
    {
      m_incr_refine_type->setCurrentIndex( 1 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_PRINCIPAL_POINT :
    {
      m_incr_refine_type->setCurrentIndex( 2 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_DISTORTION :
    {
      m_incr_refine_type->setCurrentIndex( 3 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH_PRINCIPAL_POINT :
    {
      m_incr_refine_type->setCurrentIndex( 4 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH_DISTORTION :
    {
      m_incr_refine_type->setCurrentIndex( 5 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_PRINCIPAL_POINT_DISTORTION :
    {
      m_incr_refine_type->setCurrentIndex( 6 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_ALL :
    {
      m_incr_refine_type->setCurrentIndex( 7 ) ;
      break ;
    }
  }

  // Automatic selection
  const std::pair<int, int> initPair = incr_params.initialPair() ;
  if( initPair.first == -1 ||
      initPair.second == -1 )
  {
    if( m_incr_first_image_id->count() > 2 )
    {
      m_incr_first_image_id->setCurrentIndex( 0 ) ;
      m_incr_second_image_id->setCurrentIndex( 1 ) ;
    }
    m_incr_use_automatic_pair->setChecked( true ) ;
  }
  else
  {
    m_incr_use_automatic_pair->setChecked( false ) ;

    std::stringstream str_first ;
    str_first << initPair.first ;
    std::stringstream str_second ;
    str_second << initPair.second ;

    const int id_first = m_incr_first_image_id->findText( str_first.str().c_str() );
    const int id_second = m_incr_second_image_id->findText( str_second.str().c_str() ) ;

    if( ( id_first != -1 ) && ( id_second != -1 ) )
    {
      m_incr_first_image_id->setCurrentIndex( id_first ) ;
      m_incr_second_image_id->setCurrentIndex( id_second ) ;
    }
    else
    {
      // TODO : throw an error ? (can this happen ?)
    }
  }


  // Global
  switch( glo_params.translationAveraging() )
  {
    case TRANSLATION_AVERAGING_L1 :
    {
      m_glo_translation_avg_method->setCurrentIndex( 0 ) ;
      break ;
    }
    case TRANSLATION_AVERAGING_L2 :
    {
      m_glo_translation_avg_method->setCurrentIndex( 1 ) ;
      break ;
    }
    case TRANSLATION_AVERAGING_SOFT_L1 :
    {
      m_glo_translation_avg_method->setCurrentIndex( 2 ) ;
      break;
    }
  }
  switch( glo_params.rotationAveraging() )
  {
    case ROTATION_AVERAGING_L1 :
    {
      m_glo_rotation_avg_method->setCurrentIndex( 0 ) ;
      break ;
    }
    case ROTATION_AVERAGING_L2 :
    {
      m_glo_rotation_avg_method->setCurrentIndex( 1 ) ;
      break ;
    }
  }

  switch( glo_params.refinerOptions() )
  {
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_NONE :
    {
      m_glo_refine_type->setCurrentIndex( 0 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH :
    {
      m_glo_refine_type->setCurrentIndex( 1 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_PRINCIPAL_POINT :
    {
      m_glo_refine_type->setCurrentIndex( 2 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_DISTORTION :
    {
      m_glo_refine_type->setCurrentIndex( 3 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH_PRINCIPAL_POINT :
    {
      m_glo_refine_type->setCurrentIndex( 4 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH_DISTORTION :
    {
      m_glo_refine_type->setCurrentIndex( 5 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_PRINCIPAL_POINT_DISTORTION :
    {
      m_glo_refine_type->setCurrentIndex( 6 ) ;
      break ;
    }
    case IntrinsicRefinerOptions::INTRINSIC_REFINER_ALL :
    {
      m_glo_refine_type->setCurrentIndex( 7 ) ;
      break ;
    }
  }


  // Handle visibility of elements
  onCheckAuto() ;
  onSelectMethod() ;
}

void SfMParamsDialog::buildInterface( void )
{
  // 1. Methods
  QButtonGroup * radioGrp = new QButtonGroup ;
  m_radio_incr = new QRadioButton( "Incremental" ) ;
  m_radio_glo = new QRadioButton( "Global" ) ;
  radioGrp->addButton( m_radio_incr ) ;
  radioGrp->addButton( m_radio_glo ) ;
  radioGrp->setExclusive( true ) ;
  QGroupBox * radioGrpBox = new QGroupBox( "Method" ) ;
  QHBoxLayout * radioLayout = new QHBoxLayout ;
  radioLayout->addWidget( m_radio_incr );
  radioLayout->addWidget( m_radio_glo ) ;
  radioGrpBox->setLayout( radioLayout ) ;

  // 2. Params
  // 2.1 Incremental
  QHBoxLayout * layoutParams = new QHBoxLayout ;
  m_grp_incremental = new QGroupBox( "Incremental" ) ;
  QGridLayout * layoutIncremental = new QGridLayout ;

  QLabel * labelUnknownIntrinsic = new QLabel( "Unknwon intrinsic" ) ;
  m_incr_intrinsic_type = new QComboBox ;
  m_incr_intrinsic_type->addItem( "Pinhole" ) ;
  m_incr_intrinsic_type->addItem( "Pinhole Radial 1" ) ;
  m_incr_intrinsic_type->addItem( "Pinhole Radial 3" ) ;
  m_incr_intrinsic_type->addItem( "Pinhole Brown" ) ;
  m_incr_intrinsic_type->addItem( "Pinhole Fisheye" ) ;

  QLabel * labelRefinerIncr = new QLabel( "Refine" ) ;
  m_incr_refine_type = new QComboBox ;
  m_incr_refine_type->addItem( "None" ) ;
  m_incr_refine_type->addItem( "Focal Length" ) ;
  m_incr_refine_type->addItem( "Principal Point (PP)" ) ;
  m_incr_refine_type->addItem( "Distortion" ) ;
  m_incr_refine_type->addItem( "Focal | PP" );
  m_incr_refine_type->addItem( "Focal | Distortion" );
  m_incr_refine_type->addItem( "PP | Distortion" ) ;
  m_incr_refine_type->addItem( "All" ) ;

  QGroupBox * groupInitial = new QGroupBox( "Initial pair" );
  QGridLayout * layoutInitialPair = new QGridLayout ;
  QLabel * labelInitialPairA = new QLabel( "pair A" ) ;
  m_incr_first_image_id = new QComboBox ;
  QLabel * labelInitialPairB = new QLabel( "pair B" ) ;
  m_incr_second_image_id = new QComboBox ;
  m_incr_use_automatic_pair = new QCheckBox( "Auto" ) ;


  for( const auto & id : m_image_ids )
  {
    std::stringstream str ;
    str << id ;

    m_incr_first_image_id->addItem( str.str().c_str() ) ;
    m_incr_second_image_id->addItem( str.str().c_str() ) ;
  }

  layoutInitialPair->addWidget( labelInitialPairA , 0 , 0 ) ;
  layoutInitialPair->addWidget( m_incr_first_image_id , 0 , 1 ) ;
  layoutInitialPair->addWidget( labelInitialPairB , 1 , 0 ) ;
  layoutInitialPair->addWidget( m_incr_second_image_id , 1 , 1 ) ;
  layoutInitialPair->addWidget( m_incr_use_automatic_pair , 2 , 0 , 1 , 2 ) ;
  groupInitial->setLayout( layoutInitialPair ) ;

  layoutIncremental->addWidget( labelUnknownIntrinsic , 0 , 0 ) ;
  layoutIncremental->addWidget( m_incr_intrinsic_type , 0 , 1 ) ;
  layoutIncremental->addWidget( labelRefinerIncr , 1 , 0 ) ;
  layoutIncremental->addWidget( m_incr_refine_type , 1 , 1 ) ;
  layoutIncremental->addWidget( groupInitial , 2 , 0 , 2 , 2 ) ;

  m_grp_incremental->setLayout( layoutIncremental ) ;

  // 2.2 Global
  m_grp_global = new QGroupBox( "Global" ) ;
  QGridLayout * layoutGlobal = new QGridLayout ;
  QLabel * translationAvgLabel = new QLabel( "Translation Averaging" ) ;
  m_glo_translation_avg_method = new QComboBox ;
  m_glo_translation_avg_method->addItem( "L1" ) ;
  m_glo_translation_avg_method->addItem( "L2" ) ;
  m_glo_translation_avg_method->addItem( "Soft L1" ) ;

  QLabel * rotationAvgLabel = new QLabel( "Rotation Averaging" ) ;
  m_glo_rotation_avg_method = new QComboBox ;
  m_glo_rotation_avg_method->addItem( "L1" ) ;
  m_glo_rotation_avg_method->addItem( "L2" ) ;

  QLabel * labelRefinerGlo = new QLabel( "Refine" ) ;
  m_glo_refine_type = new QComboBox ;
  m_glo_refine_type->addItem( "None" ) ;
  m_glo_refine_type->addItem( "Focal Length" ) ;
  m_glo_refine_type->addItem( "Principal Point (PP)" ) ;
  m_glo_refine_type->addItem( "Distortion" ) ;
  m_glo_refine_type->addItem( "Focal | PP" );
  m_glo_refine_type->addItem( "Focal | Distortion" );
  m_glo_refine_type->addItem( "PP | Distortion" ) ;
  m_glo_refine_type->addItem( "All" ) ;

  layoutGlobal->addWidget( translationAvgLabel , 0 , 0 ) ;
  layoutGlobal->addWidget( m_glo_translation_avg_method , 0 , 1 ) ;
  layoutGlobal->addWidget( rotationAvgLabel , 1 , 0 ) ;
  layoutGlobal->addWidget( m_glo_rotation_avg_method , 1 , 1 ) ;
  layoutGlobal->addWidget( labelRefinerGlo , 2 , 0 ) ;
  layoutGlobal->addWidget( m_glo_refine_type , 2 , 1 ) ;

  m_grp_global->setLayout( layoutGlobal ) ;

  layoutParams->addWidget( m_grp_incremental ) ;
  layoutParams->addWidget( m_grp_global ) ;

  // 3. Btns
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

  mainLayout->addWidget( radioGrpBox ) ;
  mainLayout->addLayout( layoutParams ) ;
  mainLayout->addLayout( btnLayout ) ;

  setLayout( mainLayout ) ;
  adjustSize();
  mainLayout->setSizeConstraint( QLayout::SetFixedSize );
}

void SfMParamsDialog::makeConnections( void )
{
  connect( m_cancel_btn , SIGNAL( clicked() ) , this , SLOT( onCancel() ) );
  connect( m_reset_btn , SIGNAL( clicked() ) , this , SLOT( onReset() ) ) ;
  connect( m_ok_btn , SIGNAL( clicked() ) , this , SLOT( onOk() ) );

  // No need to connect to the other radio button (toggle is on->off or off->on)
  connect( m_radio_glo , SIGNAL( toggled( bool ) ) , this , SLOT( onSelectMethod() ) );
  connect( m_incr_use_automatic_pair , SIGNAL( stateChanged( int ) ) , this , SLOT( onCheckAuto() ) ) ;
}


} // namespace openMVG_gui