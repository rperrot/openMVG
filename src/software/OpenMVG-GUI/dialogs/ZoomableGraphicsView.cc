// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ZoomableGraphicsView.hh"

#include <QScrollBar>
#include <QWheelEvent>


#include <cmath>

namespace openMVG_gui
{

/**
 * @brief Ctr
 * @param scn The scene to show
 */
ZoomableGraphicsView::ZoomableGraphicsView( QGraphicsScene * scn )
  : QGraphicsView( scn )
{

}


/**
 * @brief Handle wheel event (implement zoom in/out)
 * @param event Details about wheel event
 * @note If possible, try to zoom in/out using mouse position as anchor
 */
void ZoomableGraphicsView::wheelEvent( QWheelEvent *event )
{
  const QPointF oldPos = mapToScene( event->pos() );

  const double zoomInFactor = 1.25 ;
  const double zoomOutFactor = 1.0 / zoomInFactor ;

  setTransformationAnchor( QGraphicsView::NoAnchor ) ;
  setResizeAnchor( QGraphicsView::NoAnchor ) ;

  double zoomFactor = std::pow( 1.2, event->delta() / 240.0 );
  scale( zoomFactor , zoomFactor ) ;

  const QPointF newPos = mapToScene( event->pos() ) ;
  const QPointF delta = newPos - oldPos ;

  translate( delta.x() , delta.y() ) ;
}

/**
 * @brief Zoom In/Out
 * @param factor Zoom factor
 * @note zoom in if factor > 1 ; zoom out if factor < 1
 * @note Zoom is made at the center of the view
 */
void ZoomableGraphicsView::zoom( const double factor )
{
  scale( factor , factor ) ;
}


} // namespace openMVG_gui