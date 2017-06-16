#include "MaskView.hh"

#include <cmath>

#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>

namespace openMVG_gui
{
MaskView::MaskView( QWidget * parent , QGraphicsScene * scn )
  : QGraphicsView( scn , parent ) ,
    m_radius( 10.0 ) ,
    m_cursor( nullptr ) ,
    m_backgrounditem( nullptr ) ,
    m_maskimageitem( nullptr )
{
  setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );

  setCursor( Qt::CrossCursor ) ;
  setMouseTracking( true ) ;
}

void MaskView::setBackgroundImage( QImage & img )
{
  m_backgrounditem = scene()->addPixmap( QPixmap::fromImage( img ) );
  m_backgrounditem->setZValue( 10e10 ) ;
  m_backgrounditem->setOpacity( 0.3 ) ;
}

/**
* @brief Set initial mask image
*/
void MaskView::setMaskImage( QImage & img )
{
  m_maskimageitem = scene()->addPixmap( QPixmap::fromImage( img ) ) ;
  m_maskimageitem->setZValue( scene()->items().count() );
}


/**
* @brief Fit to view exactly the view image
*/
void MaskView::fit()
{
  fitInView( scene()->sceneRect() , Qt::KeepAspectRatio );
}

/**
* @brief get image mask
*/
QImage MaskView::getMask( void )
{
  scene()->setSceneRect( m_backgrounditem->boundingRect() );

  QImage image( m_backgrounditem->boundingRect().size().toSize() , QImage::Format_RGB32 ) ;
  image.fill( QColor( Qt::white ) );

  // Remove background to render
  scene()->removeItem( m_backgrounditem ) ;
  if( m_cursor )
  {
    scene()->removeItem( m_cursor ) ;
  }

  QPainter painter( &image ) ;
  scene()->render( &painter );

  // Get back background
  scene()->addItem( m_backgrounditem ) ;
  if( m_cursor )
  {
    scene()->addItem( m_cursor ) ;
  }

  return image ;
}

/**
* @brief Get background item
*/
QGraphicsPixmapItem * MaskView::backgroundItem( void ) const
{
  return m_backgrounditem ;
}

/**
* @brief clear all items
*/
void MaskView::clear( void )
{
  scene()->items() ;
  QList<QGraphicsItem *> allItems = scene()->items() ;
  for( int i = 0 ; i < allItems.count() ; ++i )
  {
    scene()->removeItem( allItems[i] ) ;
  }
  scene()->addItem( m_backgrounditem ) ;
  if( m_maskimageitem )
  {
    delete m_maskimageitem ;
    m_maskimageitem = nullptr ;
  }
  if( m_cursor )
  {
    scene()->addItem( m_cursor ) ;
  }
}

/**
* @brief
*/
void MaskView::setCurrentPenBrush( const QPen & pen , const QBrush & br )
{
  m_cur_pen = pen ;
  m_cur_brush = br ;
}


void MaskView::mousePressEvent( QMouseEvent * ev )
{
  QPointF pos = mapToScene( ev->pos() ) ;
  QGraphicsEllipseItem * ellipse = scene()->addEllipse( pos.x() - m_radius / 2.0 , pos.y() - m_radius / 2.0 , m_radius , m_radius , m_cur_pen , m_cur_brush ) ;
  ellipse->setZValue( scene()->items().count() );
}

/**
* @brief action to be executed when user move the mouse on the view
*/
void MaskView::mouseMoveEvent( QMouseEvent * ev )
{
  // Draw cursor
  QPointF pos = mapToScene( ev->pos() ) ;
  QRectF oldRec = scene()->sceneRect() ;

  if( ! m_cursor )
  {
    m_cursor = new QGraphicsEllipseItem( pos.x() - m_radius / 2.0 , pos.y() - m_radius / 2.0 , m_radius , m_radius ) ;
    m_cursor->setPen( QPen( Qt::red ) ) ;
    m_cursor->setZValue( 11e10 ) ;
    scene()->addItem( m_cursor ) ;
  }
  m_cursor->setRect( pos.x() - m_radius / 2.0 , pos.y() - m_radius / 2.0 , m_radius , m_radius ) ;

  if( ev->buttons() == Qt::LeftButton )
  {
    QPointF pos = mapToScene( ev->pos() ) ;
    QGraphicsEllipseItem * ellipse = scene()->addEllipse( pos.x() - m_radius / 2.0 , pos.y() - m_radius / 2.0 , m_radius , m_radius , m_cur_pen , m_cur_brush ) ;
    ellipse->setZValue( scene()->items().count() );
  }

  scene()->setSceneRect( oldRec );

}

/**
* @brief Set current brush radius
*/
void MaskView::setCurrentRadius( const double rad )
{
  m_radius = rad ;
}

/**
  * @brief handling of the zoom effect
  * @param event Container used to answer the mouse wheel informations
  */
void MaskView::wheelEvent( QWheelEvent *event )
{
  if( event->modifiers() & Qt::ControlModifier )
  {
    QPointF oldPos = mapToScene( event->pos() );

    // Store current anchor
    const int angle    = event->angleDelta().y();
    const qreal factor = std::pow( 1.0015, event->angleDelta().y() );

    scale( factor, factor );

    QPointF newPos = mapToScene( event->pos() );
    QPointF delta = newPos - oldPos ;
    translate( delta.x() , delta.y() );
  }
  else
  {
    QGraphicsView::wheelEvent( event ) ;
  }

  // Draw cursor
  QPointF pos = mapToScene( event->pos() ) ;

  if( ! m_cursor )
  {
    m_cursor = new QGraphicsEllipseItem( pos.x() - m_radius / 2.0 , pos.y() - m_radius / 2.0 , m_radius , m_radius ) ;
    m_cursor->setZValue( 11e10 ) ;
    scene()->addItem( m_cursor ) ;
  }
  m_cursor->setRect( pos.x() - m_radius / 2.0 , pos.y() - m_radius / 2.0 , m_radius , m_radius ) ;

}

/**
* @brief Action to be executed when user mouse quit the view
*/
void MaskView::leaveEvent( QEvent * ev )
{
  if( m_cursor )
  {
    scene()->removeItem( m_cursor ) ;
    delete m_cursor ;
    m_cursor = nullptr ;
  }
}




} // namespace openMVG_gui