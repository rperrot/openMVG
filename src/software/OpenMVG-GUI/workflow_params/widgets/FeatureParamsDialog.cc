// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "FeatureParamsDialog.hh"

#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <iostream>

namespace openMVG_gui
{

/**
* @brief Ctr
* @param parent Parent Widget
* @param initialParams Initial Parameters passed to the dialog
*/
FeatureParamsDialog::FeatureParamsDialog( QWidget * parent , const FeatureParams & initialParams )
  : QDialog( parent ) ,
    m_initial_params( initialParams )
{
  buildInterface() ;
  makeConnections() ;

  setParams( initialParams ) ;
  setWindowTitle( "Features params" ) ;
}

/**
* @brief Get current parameters
* @return Current parameters corresponding to the interface
*/
FeatureParams FeatureParamsDialog::params( void ) const
{
  // TODO : maybe find a way to do it more generically
  // (to facilitate addition of new feature types without rewriting 100 classes ...)

  const int idx_name   = m_feature_method->currentIndex() ;
  const int idx_preset = m_feature_preset->currentIndex() ;
  const bool upright   = m_check_upright->isChecked() ;

  FeatureType type = FEATURE_TYPE_SIFT ;
  switch( idx_name )
  {
    case 0 :
    {
      type = FEATURE_TYPE_SIFT ;
      break ;
    }
    case 1 :
    {
      type = FEATURE_TYPE_SIFT_ANATOMY ;
      break ;
    }
    case 2 :
    {
      type = FEATURE_TYPE_AKAZE_FLOAT ;
      break ;
    }
    case 3 :
    {
      type = FEATURE_TYPE_AKAZE_MLDB ;
      break ;
    }
    default :
    {
      std::cerr << "Unknown feature type" << std::endl ;
      std::cerr << "Defaulting to SIFT" << std::endl ;
      // TODO throw something ?
    }
  }

  FeaturePreset preset = FEATURE_PRESET_NORMAL ;
  switch( idx_preset )
  {
    case 0 :
    {
      preset = FEATURE_PRESET_NORMAL ;
      break ;
    }
    case 1 :
    {
      preset = FEATURE_PRESET_HIGH ;
      break ;
    }
    case 2 :
    {
      preset = FEATURE_PRESET_ULTRA ;
      break ;
    }
    default:
    {
      std::cerr << "Unknown feature preset" << std::endl ;
      std::cerr << "Defaulting to NORMAL" << std::endl ;
      // TODO throw something ?
    }
  }

  return FeatureParams( type , preset , upright ) ;
}

/**
* @brief action to be executed when user click on cancel button
*/
void FeatureParamsDialog::onCancel( void )
{
  done( QDialog::Rejected ) ;
}

/**
* @brief action to be executed when user click on ok button
*/
void FeatureParamsDialog::onOk( void )
{
  // TODO : is it mandatory to check if combo boxes have something selected ?
  // (ie: can it happened ? if true, this function should check if dialog is valid)
  done( QDialog::Accepted ) ;
}

/**
* @brief Action to be executed when user click on reset button
*/
void FeatureParamsDialog::onReset( void )
{
  setParams( m_initial_params ) ;
}

/**
* @brief Set params to a specified value
* @param value New value
*/
void FeatureParamsDialog::setParams( const FeatureParams & value )
{
  const FeatureType type = value.type() ;
  const FeaturePreset preset = value.preset() ;
  const bool upright = value.upright() ;

  switch( type )
  {
    case FEATURE_TYPE_SIFT:
    {
      m_feature_method->setCurrentIndex( 0 ) ;
      break ;
    }
    case FEATURE_TYPE_SIFT_ANATOMY :
    {
      m_feature_method->setCurrentIndex( 1 ) ;
      break ;
    }
    case FEATURE_TYPE_AKAZE_FLOAT :
    {
      m_feature_method->setCurrentIndex( 2 ) ;
      break ;
    }
    case FEATURE_TYPE_AKAZE_MLDB :
    {
      m_feature_method->setCurrentIndex( 3 ) ;
      break ;
    }
  }

  switch( preset )
  {
    case FEATURE_PRESET_NORMAL :
    {
      m_feature_preset->setCurrentIndex( 0 ) ;
      break ;
    }
    case FEATURE_PRESET_HIGH :
    {
      m_feature_preset->setCurrentIndex( 1 ) ;
      break ;
    }
    case FEATURE_PRESET_ULTRA :
    {
      m_feature_preset->setCurrentIndex( 2 ) ;
      break ;
    }
  }

  m_check_upright->setCheckState( upright ? Qt::Checked : Qt::Unchecked ) ;
}

void FeatureParamsDialog::buildInterface( void )
{
  QGridLayout * glayout = new QGridLayout ;

  // Interface
  QLabel * labelFeatType = new QLabel( "Type" ) ;
  QLabel * labelFeatPreset = new QLabel( "Preset" ) ;
  QLabel * labelUpRight = new QLabel( "Upright" ) ;

  m_feature_method = new QComboBox( this ) ;
  m_feature_method->addItem( "SIFT" ) ;
  m_feature_method->addItem( "SIFT Anatomy" ) ;
  m_feature_method->addItem( "AKAZE FLOAT" ) ;
  m_feature_method->addItem( "AKAZE MLDB" ) ;

  m_feature_preset = new QComboBox( this ) ;
  m_feature_preset->addItem( "NORMAL" ) ;
  m_feature_preset->addItem( "HIGH" ) ;
  m_feature_preset->addItem( "ULTRA" ) ;

  m_check_upright = new QCheckBox( this ) ;
  m_check_upright->setTristate( false ) ;

  glayout->addWidget( labelFeatType , 0 , 0 ) ;
  glayout->addWidget( m_feature_method , 0 , 1 ) ;
  glayout->addWidget( labelFeatPreset , 1 , 0 ) ;
  glayout->addWidget( m_feature_preset , 1 , 1 ) ;
  glayout->addWidget( labelUpRight , 2 , 0 ) ;
  glayout->addWidget( m_check_upright , 2 , 1 ) ;

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

  mainLayout->addLayout( glayout ) ;
  mainLayout->addLayout( btnLayout ) ;

  setLayout( mainLayout ) ;
  adjustSize();
  mainLayout->setSizeConstraint( QLayout::SetFixedSize );
}

void FeatureParamsDialog::makeConnections( void )
{
  connect( m_cancel_btn , SIGNAL( clicked() ) , this , SLOT( onCancel() ) );
  connect( m_reset_btn , SIGNAL( clicked() ) , this , SLOT( onReset() ) ) ;
  connect( m_ok_btn , SIGNAL( clicked() ) , this , SLOT( onOk() ) );
}



} // namespace openMVG_gui