#include "ColorIndicatorWidget.hh"

#include <QPalette>

namespace openMVG_gui
{

ColorIndicatorWidget::ColorIndicatorWidget( QWidget *parent )
    : QFrame( parent )
{
  setFrameStyle( QFrame::Box );
  setMinimumSize( 50, 20 );
}

void ColorIndicatorWidget::setColor( const QColor &col )
{
  QPalette palette = this->palette();
  palette.setColor( backgroundRole(), col );
  setAutoFillBackground( true );
  setPalette( palette );
}

void ColorIndicatorWidget::mousePressEvent( QMouseEvent *event ) 
{
  emit clicked();
}

} // namespace openMVG_gui
