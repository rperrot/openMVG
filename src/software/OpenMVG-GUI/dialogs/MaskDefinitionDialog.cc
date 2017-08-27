#include "MaskDefinitionDialog.hh"
#include "Project.hh"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QBrush>
#include <QColor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>

namespace openMVG_gui
{

MaskDefinitionDialog::MaskDefinitionDialog( QWidget * parent ,
    std::shared_ptr<Project> project ,
    const int id_image )
  : QDialog( parent ) ,
    m_project( project ) ,
    m_image_id( id_image )
{
  resize( 800 , 600 ) ;
  buildInterface() ;
  makeConnections() ;

  setWindowTitle( "Mask definition" ) ;
  setModal( true ) ;

  onChangeActivation() ;

  show();
}

/**
* @brief Indicate if mask is activated
*/
bool MaskDefinitionDialog::hasMaskActivated( void ) const
{
  return m_activated->isChecked() ;
}

/**
* @brief Activate/Deactivate mask
* @param active Activation status
*/
void MaskDefinitionDialog::setMaskActivated( bool active )
{
  m_activated->setChecked( active ) ;
}


void MaskDefinitionDialog::showEvent( QShowEvent * )
{
  m_drawing_area->fit() ;
}

/**
* @brief Get image mask
*/
QImage MaskDefinitionDialog::getMask( void )
{
  return m_drawing_area->getMask() ;
}


/**
* @brief action to be executed when user click on cancel button
*/
void MaskDefinitionDialog::onCancel( void )
{
  done( QDialog::Rejected ) ;
}

/**
* @brief action to be executed when user click on ok button
*/
void MaskDefinitionDialog::onOk( void )
{
  done( QDialog::Accepted ) ;
}

/**
* @brief Action to be executed when user press the draw (toolbar) button
*/
void MaskDefinitionDialog::onClickDrawBtn( void )
{
  QToolButton * btnDraw = dynamic_cast<QToolButton*>( m_drawing_toolbar->widgetForAction( m_draw_act ) ) ;
  QToolButton * btnErase = dynamic_cast<QToolButton*>( m_drawing_toolbar->widgetForAction( m_erase_act ) ) ;
  btnDraw->setChecked( true ) ;
  btnErase->setChecked( false ) ;

  m_drawing_area->setCurrentPenBrush( QPen( Qt::black ) , QBrush( Qt::black , Qt::SolidPattern ) );
}

/**
* @brief Action to be executed when user press the erase (toolbar) button
*/
void MaskDefinitionDialog::onClickEraseBtn( void )
{
  QToolButton * btnDraw = dynamic_cast<QToolButton*>( m_drawing_toolbar->widgetForAction( m_draw_act ) ) ;
  QToolButton * btnErase = dynamic_cast<QToolButton*>( m_drawing_toolbar->widgetForAction( m_erase_act ) ) ;
  btnDraw->setChecked( false ) ;
  btnErase->setChecked( true ) ;

  m_drawing_area->setCurrentPenBrush( QPen( Qt::white ) , QBrush( Qt::white , Qt::SolidPattern ) );
}

/**
* @brief Action to be executed when user press the fill button
*/
void MaskDefinitionDialog::onClickFillBtn( void )
{
  // Remove all items
  QGraphicsPixmapItem * background = m_drawing_area->backgroundItem() ;
  m_drawing_area->clear() ;

  // Add a fill black rectangle
  QBrush blackbrush( Qt::black );
  m_drawing_scene->addRect( background->boundingRect() , QPen() , blackbrush );
}

/**
* @brief Action to be executed when user press the clear button
*/
void MaskDefinitionDialog::onClickClearBtn( void )
{
  // Remove all items
  m_drawing_area->clear() ;
}

/**
* @brief Action to be executed when user click on activation checkbox
*/
void MaskDefinitionDialog::onChangeActivation( void )
{
  if( m_activated->isChecked() )
  {
    m_drawing_toolbar->setEnabled( true ) ;
    m_drawing_area->setEnabled( true ) ;
  }
  else
  {
    m_drawing_toolbar->setEnabled( false ) ;
    m_drawing_area->setEnabled( false ) ;
  }
}

/**
* @brief Action to be executed when user change brush size slider
*/
void MaskDefinitionDialog::onChangeBrushSize( void )
{
  const int value = m_slider_element_size->value() ;
  m_drawing_area->setCurrentRadius( value ) ;
}


void MaskDefinitionDialog::buildInterface()
{

  QVBoxLayout * mainLayout = new QVBoxLayout ;

  QHBoxLayout * activationLayout = new QHBoxLayout ;
  QHBoxLayout * drawingLayout = new QHBoxLayout ;
  QHBoxLayout * dlgBtnsLayout = new QHBoxLayout ;

  // activation
  m_activated = new QCheckBox( "Enable" );
  m_activated->setTristate( false ) ;
  activationLayout->addStretch( ) ;
  activationLayout->addWidget( m_activated ) ;
  activationLayout->addStretch( ) ;

  // Drawing
  m_drawing_toolbar = new QToolBar ;
  m_drawing_toolbar->setOrientation( Qt::Vertical ) ;
  m_draw_act = m_drawing_toolbar->addAction( "Draw" ) ;
  m_erase_act = m_drawing_toolbar->addAction( "Erase" ) ;
  m_drawing_toolbar->addSeparator() ;
  m_fill_all_act = m_drawing_toolbar->addAction( "Fill" ) ;
  m_clear_all_act = m_drawing_toolbar->addAction( "Clear" ) ;

  QToolButton * btnDraw = dynamic_cast<QToolButton*>( m_drawing_toolbar->widgetForAction( m_draw_act ) ) ;
  QToolButton * btnErase = dynamic_cast<QToolButton*>( m_drawing_toolbar->widgetForAction( m_erase_act ) ) ;
  btnDraw->setCheckable( true ) ;
  btnDraw->setChecked( true ) ;
  btnErase->setCheckable( true ) ;
  btnErase->setChecked( false ) ;

  m_slider_element_size = new QSlider( Qt::Vertical ) ;
  m_slider_element_size->setRange( 1 , 2500 ) ;
  m_slider_element_size->setValue( 32 ) ;

  m_drawing_scene = new QGraphicsScene ;
  m_drawing_area = new MaskView( this , m_drawing_scene ) ;

  const std::string imgPath = m_project->getImagePath( m_image_id ) ;
  const std::string maskPath = m_project->getMaskImagePath( m_image_id ) ;
  if( stlplus::file_exists( maskPath ) )
  {
    QImage mask( maskPath.c_str() ) ;
    m_drawing_area->setMaskImage( mask ) ;
  }
  QImage img( imgPath.c_str() );
  m_drawing_area->setBackgroundImage( img ) ;

  m_drawing_area->setCurrentRadius( 32.0 ) ;
  m_drawing_area->setCurrentPenBrush( QPen( Qt::black ) , QBrush( Qt::black , Qt::SolidPattern ) );


  drawingLayout->addWidget( m_drawing_toolbar ) ;
  drawingLayout->addWidget( m_drawing_area ) ;
  drawingLayout->addWidget( m_slider_element_size ) ;

  // Action buttons
  m_ok_btn = new QPushButton( "Ok" ) ;
  m_cancel_btn = new QPushButton( "Cancel" ) ;

  dlgBtnsLayout->addStretch();
  dlgBtnsLayout->addWidget( m_ok_btn ) ;
  dlgBtnsLayout->addWidget( m_cancel_btn ) ;

  mainLayout->addLayout( activationLayout ) ;
  mainLayout->addLayout( drawingLayout ) ;
  mainLayout->addLayout( dlgBtnsLayout ) ;

  setLayout( mainLayout ) ;
}

void MaskDefinitionDialog::makeConnections()
{
  connect( m_ok_btn , SIGNAL( clicked() ) , this , SLOT( onOk() ) ) ;
  connect( m_cancel_btn , SIGNAL( clicked() ) , this , SLOT( onCancel() ) ) ;
  connect( m_draw_act , SIGNAL( triggered() ) , this , SLOT( onClickDrawBtn() ) ) ;
  connect( m_erase_act , SIGNAL( triggered() ) , this , SLOT( onClickEraseBtn() ) ) ;
  connect( m_fill_all_act , SIGNAL( triggered() ) , this , SLOT( onClickFillBtn() ) ) ;
  connect( m_clear_all_act , SIGNAL( triggered() ) , this , SLOT( onClickClearBtn() ) ) ;
  connect( m_activated , SIGNAL( stateChanged( int ) ) , this , SLOT( onChangeActivation() ) ) ;
  connect( m_slider_element_size , SIGNAL( valueChanged( int ) ) , this , SLOT( onChangeBrushSize() ) ) ;
}


} // namespace openMVG_gui