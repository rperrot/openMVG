// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "IntrinsicEditorDialog.hh"

#include <QDoubleValidator>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QVBoxLayout>

namespace openMVG_gui
{

/**
 * @brief Ctr
 * @param parent parent widget
 * @param intrin Intrinsic to edit
 */
IntrinsicEditorDialog::IntrinsicEditorDialog( QWidget * parent , std::shared_ptr<openMVG::cameras::IntrinsicBase> intrin )
  : QDialog( parent ) ,
    m_original_intrinsic( intrin ) ,
    m_intrinsic( std::shared_ptr<openMVG::cameras::IntrinsicBase>( intrin->clone() ) )
{
  buildInterface() ;
  makeConnections() ;

  fillValues() ;
  updateInterface() ;
}

/**
 * @brief Get current intrinsic
 * @return current intrinsic
 */
std::shared_ptr<openMVG::cameras::IntrinsicBase> IntrinsicEditorDialog::intrinsic( void ) const
{
  return m_intrinsic ;
}

/**
* @brief action to be executed when user click on cancel button
*/
void IntrinsicEditorDialog::onCancel( void )
{
  done( QDialog::Rejected ) ;
}

/**
* @brief action to be executed when user click on ok button
*/
void IntrinsicEditorDialog::onOk( void )
{
  done( QDialog::Accepted ) ;
}

/**
* @brief Action to be executed when user click on reset button
*/
void IntrinsicEditorDialog::onReset( void )
{
  m_intrinsic = std::shared_ptr<openMVG::cameras::IntrinsicBase>( m_original_intrinsic->clone() ) ;
  fillValues() ;
  updateInterface() ;
}

/**
  * @brief Action to be executed when used click on reset button
  */
void IntrinsicEditorDialog::onHasChangedType( void )
{
  // Can we reuse data
  const int index = m_type->currentIndex() ;

  std::shared_ptr<openMVG::cameras::IntrinsicBase> newIntrinsic ;

  // Prepare params
  const int w = m_intrinsic->w_ ;
  const int h = m_intrinsic->h_ ;
  std::vector< double > params = m_intrinsic->getParams() ;
  const bool prev_is_pinhole = std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic>( m_intrinsic ) != nullptr ;

  if( ! prev_is_pinhole )
  {
    // use default parameters
    params.emplace_back( 1.2 * std::max( w , h ) );
    params.emplace_back( w / 2.0 ) ;
    params.emplace_back( h / 2.0 ) ;
  }

  switch( index )
  {
    case 0: // pinhole without intrinsic
    {
      newIntrinsic = std::make_shared<openMVG::cameras::Pinhole_Intrinsic>( w , h , params[0] , params[1] , params[2] ) ;
      break ;
    }
    case 1: // pinhole + radial 1
    {
      newIntrinsic = std::make_shared<openMVG::cameras::Pinhole_Intrinsic_Radial_K1>( w , h , params[0] , params[1] , params[2] ) ;
      break;
    }
    case 2: // pinhole + radial 3
    {
      newIntrinsic = std::make_shared<openMVG::cameras::Pinhole_Intrinsic_Radial_K3>( w , h , params[0] , params[1] , params[2] ) ;
      break ;
    }
    case 3: // pinhole + brown
    {
      newIntrinsic = std::make_shared<openMVG::cameras::Pinhole_Intrinsic_Brown_T2>( w , h , params[0] , params[1] , params[2] ) ;
      break ;
    }
    case 4: // pinhole + fisheye
    {
      newIntrinsic = std::make_shared<openMVG::cameras::Pinhole_Intrinsic_Fisheye>( w , h , params[0] , params[1] , params[2] ) ;
      break ;
    }
    case 5: // spherical
    {
      newIntrinsic = std::make_shared<openMVG::cameras::Intrinsic_Spherical>( w , h ) ;
      break ;
    }
  }

  m_intrinsic = newIntrinsic ;

  fillValues( false ) ;
  updateInterface() ;
}


/**
 * @brief Action to be executed when user has edited width line
 */
void IntrinsicEditorDialog::onHasEditedWidth( void )
{
  const int w = m_width->text().toInt() ;

  m_intrinsic->w_ = w ;
}

/**
 * @brief Action to be executed when user has edited height  line
 */
void IntrinsicEditorDialog::onHasEditedHeight( void )
{
  const int h = m_height->text().toInt() ;

  m_intrinsic->h_ = h ;
}

/**
 * @brief Action to be executed when user has edited focal line
 */
void IntrinsicEditorDialog::onHasEditedFocal( void )
{
  const double focal = m_focal->text().toDouble() ;

  // If focal is edited -> it's a pinhole (focal,ppx,ppy, [params])
  std::vector< double > params = m_intrinsic->getParams() ;
  params[0] = focal ;

  m_intrinsic->updateFromParams( params ) ;
}

/**
 * @brief Action to be executed when user has edited principal point X line
 */
void IntrinsicEditorDialog::onHasEditedPPX( void )
{
  const double ppx = m_ppx->text().toDouble() ;

  // If ppx is edited -> it's a pinhole (focal,ppx,ppy, [params])
  std::vector< double > params = m_intrinsic->getParams() ;
  params[1] = ppx ;

  m_intrinsic->updateFromParams( params ) ;
}

/**
* @brief Action to be executed when user has edited principal point Y line
*/
void IntrinsicEditorDialog::onHasEditedPPY( void )
{
  const double ppy = m_ppy->text().toDouble() ;

  // If ppy is edited -> it's a pinhole (focal,ppx,ppy, [params])
  std::vector< double > params = m_intrinsic->getParams() ;
  params[2] = ppy ;

  m_intrinsic->updateFromParams( params ) ;
}

/**
* @brief Action to be executed when user has edited param 1 line
*/
void IntrinsicEditorDialog::onHasEditedParam1( void )
{
  const double param_1 = m_param_1->text().toDouble() ;

  // If params_1 is edited -> it's a pinhole ( focal, ppx, ppy, params_1 , [params] )
  std::vector< double > params = m_intrinsic->getParams() ;
  params[3] = param_1 ;

  m_intrinsic->updateFromParams( params ) ;
}

/**
* @brief Action to be executed when user has edited param 2 line
*/
void IntrinsicEditorDialog::onHasEditedParam2( void )
{
  const double param_2 = m_param_2->text().toDouble() ;

  // If params_1 is edited -> it's a pinhole ( focal, ppx, ppy, params_1 , param_2 , params_3 , [params] )
  std::vector< double > params = m_intrinsic->getParams() ;
  params[4] = param_2 ;

  m_intrinsic->updateFromParams( params ) ;
}

/**
* @brief Action to be executed when user has edited param 3 line
*/
void IntrinsicEditorDialog::onHasEditedParam3( void )
{
  const double param_3 = m_param_3->text().toDouble() ;

  // If params_1 is edited -> it's a pinhole ( focal, ppx, ppy, params_1 , param_2 , params_3 , [params] )
  std::vector< double > params = m_intrinsic->getParams() ;
  params[5] = param_3 ;

  m_intrinsic->updateFromParams( params ) ;
}

/**
* @brief Action to be executed when user has edited param 4 line
*/
void IntrinsicEditorDialog::onHasEditedParam4( void )
{
  const double param_4 = m_param_4->text().toDouble() ;

  // If params_1 is edited -> it's a pinhole ( focal, ppx, ppy, params_1 , param_2 , params_3 , params_4 , [params] )
  std::vector< double > params = m_intrinsic->getParams() ;
  params[6] = param_4 ;

  m_intrinsic->updateFromParams( params ) ;
}

/**
* @brief Action to be executed when user has edited param 5 line
*/
void IntrinsicEditorDialog::onHasEditedParam5( void )
{
  const double param_5 = m_param_5->text().toDouble() ;

  // If params_1 is edited -> it's a pinhole Brown ( focal, ppx, ppy, params_1 , param_2 , params_3 , params_4 , params_5 )
  std::vector< double > params = m_intrinsic->getParams() ;
  params[7] = param_5 ;

  m_intrinsic->updateFromParams( params ) ;
}

/**
 * @brief Fill values according to the current intrinsic data
 * @param set_index Indicate if the combo box should be modified accordingly
 */
void IntrinsicEditorDialog::fillValues( const bool set_index )
{
  const unsigned int w = m_intrinsic->w() ;
  const unsigned int h = m_intrinsic->h() ;

  m_width->setText( QString( "%1" ).arg( w ) ) ;
  m_height->setText( QString( "%1" ).arg( h ) ) ;

  const std::vector< double > params = m_intrinsic->getParams() ;

  if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Radial_K1>( m_intrinsic ) )
  {
    const double focal = params[ 0 ] ;
    const double ppx   = params[ 1 ] ;
    const double ppy   = params[ 2 ] ;

    const double k1    = params[ 3 ] ;

    if( set_index )
    {
      m_type->setCurrentIndex( 1 ) ;
    }

    m_focal->setText( QString( "%1" ).arg( focal ) ) ;
    m_ppx->setText( QString( "%1" ).arg( ppx ) ) ;
    m_ppy->setText( QString( "%1" ).arg( ppy ) ) ;

    m_param_1->setText( QString( "%1" ).arg( k1 ) ) ;
  }
  else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Radial_K3>( m_intrinsic ) )
  {
    const double focal = params[ 0 ] ;
    const double ppx   = params[ 1 ] ;
    const double ppy   = params[ 2 ] ;

    const double k1    = params[ 3 ] ;
    const double k2    = params[ 4 ] ;
    const double k3    = params[ 5 ] ;

    if( set_index )
    {
      m_type->setCurrentIndex( 2 ) ;
    }

    m_focal->setText( QString( "%1" ).arg( focal ) ) ;
    m_ppx->setText( QString( "%1" ).arg( ppx ) ) ;
    m_ppy->setText( QString( "%1" ).arg( ppy ) ) ;

    m_param_1->setText( QString( "%1" ).arg( k1 ) ) ;
    m_param_2->setText( QString( "%1" ).arg( k2 ) ) ;
    m_param_3->setText( QString( "%1" ).arg( k3 ) ) ;
  }
  else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Brown_T2>( m_intrinsic ) )
  {
    const double focal = params[ 0 ] ;
    const double ppx   = params[ 1 ] ;
    const double ppy   = params[ 2 ] ;

    const double k1    = params[ 3 ] ;
    const double k2    = params[ 4 ] ;
    const double k3    = params[ 5 ] ;
    const double t1    = params[ 6 ] ;
    const double t2    = params[ 7 ] ;

    if( set_index )
    {
      m_type->setCurrentIndex( 3 ) ;
    }

    m_focal->setText( QString( "%1" ).arg( focal ) ) ;
    m_ppx->setText( QString( "%1" ).arg( ppx ) ) ;
    m_ppy->setText( QString( "%1" ).arg( ppy ) ) ;

    m_param_1->setText( QString( "%1" ).arg( k1 ) ) ;
    m_param_2->setText( QString( "%1" ).arg( k2 ) ) ;
    m_param_3->setText( QString( "%1" ).arg( k3 ) ) ;

    m_param_4->setText( QString( "%1" ).arg( t1 ) ) ;
    m_param_5->setText( QString( "%1" ).arg( t2 ) ) ;
  }
  else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Fisheye>( m_intrinsic ) )
  {
    const double focal = params[ 0 ] ;
    const double ppx   = params[ 1 ] ;
    const double ppy   = params[ 2 ] ;

    const double k1    = params[ 3 ] ;
    const double k2    = params[ 4 ] ;
    const double k3    = params[ 5 ] ;
    const double k4    = params[ 6 ] ;

    if( set_index )
    {
      m_type->setCurrentIndex( 4 ) ;
    }

    m_focal->setText( QString( "%1" ).arg( focal ) ) ;
    m_ppx->setText( QString( "%1" ).arg( ppx ) ) ;
    m_ppy->setText( QString( "%1" ).arg( ppy ) ) ;

    m_param_1->setText( QString( "%1" ).arg( k1 ) ) ;
    m_param_2->setText( QString( "%1" ).arg( k2 ) ) ;
    m_param_3->setText( QString( "%1" ).arg( k3 ) ) ;
    m_param_4->setText( QString( "%1" ).arg( k4 ) ) ;
  }
  else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic>( m_intrinsic ) )
  {
    const double focal = params[ 0 ] ;
    const double ppx   = params[ 1 ] ;
    const double ppy   = params[ 2 ] ;

    if( set_index )
    {
      m_type->setCurrentIndex( 0 ) ;
    }

    m_focal->setText( QString( "%1" ).arg( focal ) ) ;
    m_ppx->setText( QString( "%1" ).arg( ppx ) ) ;
    m_ppy->setText( QString( "%1" ).arg( ppy ) ) ;
  }
  else if( std::dynamic_pointer_cast<openMVG::cameras::Intrinsic_Spherical>( m_intrinsic ) )
  {
    if( set_index )
    {
      m_type->setCurrentIndex( 5 ) ;
    }
  }
}


/**
 * @brief Update interface according to the current intrinsic data
 */
void IntrinsicEditorDialog::updateInterface( void )
{
  switch( m_type->currentIndex() )
  {
    case 0: // pinhole without disto
    {
      m_pinhole_params_grp->setVisible( true ) ;
      m_distortion_params_grp->setVisible( false ) ;

      m_param_1_lbl->setVisible( false ) ;
      m_param_1->setVisible( false ) ;
      m_param_2_lbl->setVisible( false ) ;
      m_param_2->setVisible( false ) ;
      m_param_3_lbl->setVisible( false ) ;
      m_param_3->setVisible( false ) ;
      m_param_4_lbl->setVisible( false ) ;
      m_param_4->setVisible( false ) ;
      m_param_5_lbl->setVisible( false ) ;
      m_param_5->setVisible( false ) ;

      break ;
    }
    case 1: // pinhole + radial 1
    {
      m_pinhole_params_grp->setVisible( true ) ;
      m_distortion_params_grp->setVisible( true ) ;

      m_param_1_lbl->setText( "k1" ) ;

      m_param_1_lbl->setVisible( true ) ;
      m_param_1->setVisible( true ) ;
      m_param_2_lbl->setVisible( false ) ;
      m_param_2->setVisible( false ) ;
      m_param_3_lbl->setVisible( false ) ;
      m_param_3->setVisible( false ) ;
      m_param_4_lbl->setVisible( false ) ;
      m_param_4->setVisible( false ) ;
      m_param_5_lbl->setVisible( false ) ;
      m_param_5->setVisible( false ) ;

      break ;
    }
    case 2: // pinhole + radial 3
    {
      m_pinhole_params_grp->setVisible( true ) ;
      m_distortion_params_grp->setVisible( true ) ;

      m_param_1_lbl->setText( "k1" ) ;
      m_param_2_lbl->setText( "k2" ) ;
      m_param_3_lbl->setText( "k3" ) ;

      m_param_1_lbl->setVisible( true ) ;
      m_param_1->setVisible( true ) ;
      m_param_2_lbl->setVisible( true ) ;
      m_param_2->setVisible( true ) ;
      m_param_3_lbl->setVisible( true ) ;
      m_param_3->setVisible( true ) ;
      m_param_4_lbl->setVisible( false ) ;
      m_param_4->setVisible( false ) ;
      m_param_5_lbl->setVisible( false ) ;
      m_param_5->setVisible( false ) ;

      break ;
    }
    case 3: // pinhole + brown
    {
      m_pinhole_params_grp->setVisible( true ) ;
      m_distortion_params_grp->setVisible( true ) ;

      m_param_1_lbl->setText( "k1" ) ;
      m_param_2_lbl->setText( "k2" ) ;
      m_param_3_lbl->setText( "k3" ) ;
      m_param_4_lbl->setText( "t1" ) ;
      m_param_5_lbl->setText( "t2" ) ;

      m_param_1_lbl->setVisible( true ) ;
      m_param_1->setVisible( true ) ;
      m_param_2_lbl->setVisible( true ) ;
      m_param_2->setVisible( true ) ;
      m_param_3_lbl->setVisible( true ) ;
      m_param_3->setVisible( true ) ;
      m_param_4_lbl->setVisible( true ) ;
      m_param_4->setVisible( true ) ;
      m_param_5_lbl->setVisible( true ) ;
      m_param_5->setVisible( true ) ;

      break ;
    }
    case 4: // Fisheye
    {
      m_pinhole_params_grp->setVisible( true ) ;
      m_distortion_params_grp->setVisible( true ) ;

      m_param_1_lbl->setText( "k1" ) ;
      m_param_2_lbl->setText( "k2" ) ;
      m_param_3_lbl->setText( "k3" ) ;
      m_param_4_lbl->setText( "k4" ) ;

      m_param_1_lbl->setVisible( true ) ;
      m_param_1->setVisible( true ) ;
      m_param_2_lbl->setVisible( true ) ;
      m_param_2->setVisible( true ) ;
      m_param_3_lbl->setVisible( true ) ;
      m_param_3->setVisible( true ) ;
      m_param_4_lbl->setVisible( true ) ;
      m_param_4->setVisible( true ) ;
      m_param_5_lbl->setVisible( false ) ;
      m_param_5->setVisible( false ) ;

      break ;
    }
    case 5: // Spherical
    {
      m_pinhole_params_grp->setVisible( false ) ;
      m_distortion_params_grp->setVisible( false ) ;

      m_param_1_lbl->setText( "k1" ) ;
      m_param_2_lbl->setText( "k2" ) ;
      m_param_3_lbl->setText( "k3" ) ;
      m_param_4_lbl->setText( "k4" ) ;
      m_param_5_lbl->setText( "k5" ) ;

      m_param_1_lbl->setVisible( false ) ;
      m_param_1->setVisible( false ) ;
      m_param_2_lbl->setVisible( false ) ;
      m_param_2->setVisible( false ) ;
      m_param_3_lbl->setVisible( false ) ;
      m_param_3->setVisible( false ) ;
      m_param_4_lbl->setVisible( false ) ;
      m_param_4->setVisible( false ) ;
      m_param_5_lbl->setVisible( false ) ;
      m_param_5->setVisible( false ) ;

      break ;
    }
  }

}

/**
 * @brief Build interface widgets
 */
void IntrinsicEditorDialog::buildInterface( void )
{
  QVBoxLayout * mainLayout = new QVBoxLayout ;

  m_type_lbl = new QLabel( "Type" ) ;
  m_type = new QComboBox ;
  m_type->addItem( "Pinhole without distortion" ) ;
  m_type->addItem( "Pinhole + Radial 1" ) ;
  m_type->addItem( "Pinhole + Radial 3" ) ;
  m_type->addItem( "Pinhole Brown" ) ;
  m_type->addItem( "Fisheye" ) ;
  m_type->addItem( "Spherical" ) ;

  QGridLayout * typeLayout = new QGridLayout ;
  typeLayout->addWidget( m_type_lbl , 0 , 0 ) ;
  typeLayout->addWidget( m_type , 0 , 1 ) ;

  // Standard params
  m_basic_params_grp = new QGroupBox( "Basic" ) ;
  m_width_lbl = new QLabel( "Width" ) ;
  m_width = new QLineEdit;
  m_width->setValidator( new QIntValidator( 0 , 100000 ) ) ;

  m_height_lbl = new QLabel( "Height" ) ;
  m_height = new QLineEdit ;
  m_height->setValidator( new QIntValidator( 0 , 100000 ) ) ;

  QGridLayout * basicLayout = new QGridLayout ;
  basicLayout->addWidget( m_width_lbl , 0 , 0 ) ;
  basicLayout->addWidget( m_width , 0 , 1 ) ;
  basicLayout->addWidget( m_height_lbl , 1 , 0 ) ;
  basicLayout->addWidget( m_height , 1 , 1 ) ;
  m_basic_params_grp->setLayout( basicLayout ) ;

  // Pinhole common params
  m_pinhole_params_grp = new QGroupBox( "Pinhole" ) ;

  m_focal_lbl = new QLabel( "Focal" ) ;
  m_focal = new QLineEdit ;
  m_focal->setValidator( new QDoubleValidator ) ;

  m_ppx_lbl = new QLabel( "Principal Point X" ) ;
  m_ppx = new QLineEdit ;
  m_ppx->setValidator( new QDoubleValidator ) ;

  m_ppy_lbl = new QLabel( "Principal Point Y" ) ;
  m_ppy = new QLineEdit ;
  m_ppy->setValidator( new QDoubleValidator ) ;

  QGridLayout * pinholeLayout = new QGridLayout ;
  pinholeLayout->addWidget( m_focal_lbl , 0 , 0 ) ;
  pinholeLayout->addWidget( m_focal , 0 , 1 ) ;
  pinholeLayout->addWidget( m_ppx_lbl , 1 , 0 ) ;
  pinholeLayout->addWidget( m_ppx , 1 , 1 ) ;
  pinholeLayout->addWidget( m_ppy_lbl , 2 , 0 ) ;
  pinholeLayout->addWidget( m_ppy , 2 , 1 ) ;
  m_pinhole_params_grp->setLayout( pinholeLayout ) ;

  // Distorsion params
  m_distortion_params_grp = new QGroupBox( "Distortion" ) ;

  m_param_1_lbl = new QLabel( "k1" ) ;
  m_param_1 = new QLineEdit ;
  m_param_1->setValidator( new QDoubleValidator ) ;

  m_param_2_lbl = new QLabel( "k2" ) ;
  m_param_2 = new QLineEdit ;
  m_param_2->setValidator( new QDoubleValidator ) ;

  m_param_3_lbl = new QLabel( "k3" ) ;
  m_param_3 = new QLineEdit ;
  m_param_3->setValidator( new QDoubleValidator ) ;

  m_param_4_lbl = new QLabel( "k4" ) ;
  m_param_4 = new QLineEdit ;
  m_param_4->setValidator( new QDoubleValidator ) ;

  m_param_5_lbl = new QLabel( "k5" ) ;
  m_param_5 = new QLineEdit ;
  m_param_5->setValidator( new QDoubleValidator ) ;

  QGridLayout * distoLayout = new QGridLayout ;
  distoLayout->addWidget( m_param_1_lbl , 0 , 0 ) ;
  distoLayout->addWidget( m_param_1 , 0 , 1 ) ;
  distoLayout->addWidget( m_param_2_lbl , 1 , 0 ) ;
  distoLayout->addWidget( m_param_2 , 1 , 1 ) ;
  distoLayout->addWidget( m_param_3_lbl , 2 , 0 ) ;
  distoLayout->addWidget( m_param_3 , 2 , 1 ) ;
  distoLayout->addWidget( m_param_4_lbl , 3 , 0 ) ;
  distoLayout->addWidget( m_param_4 , 3 , 1 ) ;
  distoLayout->addWidget( m_param_5_lbl , 4 , 0 ) ;
  distoLayout->addWidget( m_param_5 , 4 , 1 ) ;
  m_distortion_params_grp->setLayout( distoLayout ) ;

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

  mainLayout->addLayout( typeLayout ) ;
  mainLayout->addWidget( m_basic_params_grp ) ;
  mainLayout->addWidget( m_pinhole_params_grp ) ;
  mainLayout->addWidget( m_distortion_params_grp ) ;
  mainLayout->addStretch() ;
  mainLayout->addLayout( btnLayout ) ;

  setLayout( mainLayout ) ;
}

/**
 * @brief Make connections between widgets
 */
void IntrinsicEditorDialog::makeConnections( void )
{
  connect( m_cancel_btn , SIGNAL( clicked() ) , this , SLOT( onCancel() ) );
  connect( m_reset_btn , SIGNAL( clicked() ) , this , SLOT( onReset() ) ) ;
  connect( m_ok_btn , SIGNAL( clicked() ) , this , SLOT( onOk() ) );

  connect( m_type , SIGNAL( currentIndexChanged( int ) ) , this , SLOT( onHasChangedType() ) ) ;
  connect( m_width , SIGNAL( editingFinished() ) , this , SLOT( onHasEditedWidth() ) ) ;
  connect( m_height , SIGNAL( editingFinished() ) , this , SLOT( onHasEditedHeight() ) );
  connect( m_focal , SIGNAL( editingFinished() ) , this , SLOT( onHasEditedFocal() ) ) ;
  connect( m_ppx , SIGNAL( editingFinished() ) , this , SLOT( onHasEditedPPX() ) ) ;
  connect( m_ppy , SIGNAL( editingFinished() ) , this , SLOT( onHasEditedPPY() ) ) ;
  connect( m_param_1 , SIGNAL( editingFinished() ) , this , SLOT( onHasEditedParam1() ) ) ;
  connect( m_param_2 , SIGNAL( editingFinished() ) , this , SLOT( onHasEditedParam2() ) ) ;
  connect( m_param_3 , SIGNAL( editingFinished() ) , this , SLOT( onHasEditedParam3() ) ) ;
  connect( m_param_4 , SIGNAL( editingFinished() ) , this , SLOT( onHasEditedParam4() ) ) ;
  connect( m_param_5 , SIGNAL( editingFinished() ) , this , SLOT( onHasEditedParam5() ) ) ;
}

} // namespace openMVG_gui