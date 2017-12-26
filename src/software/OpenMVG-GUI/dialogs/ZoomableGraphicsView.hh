// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_ZOOMABLE_GRAPHICS_VIEW_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_ZOOMABLE_GRAPHICS_VIEW_HH_

#include <QGraphicsView>

namespace openMVG_gui
{

/**
 * @brief Utility class that add zoom using wheel in a QGraphicsView
 */
class ZoomableGraphicsView : public QGraphicsView
{
  public:

    /**
     * @brief Ctr
     * @param scn The scene to show
     */
    ZoomableGraphicsView( QGraphicsScene * scn ) ;

    /**
     * @brief Handle wheel event (implement zoom in/out)
     * @param event Details about wheel event
     * @note If possible, try to zoom in/out using mouse position as anchor
     */
    void wheelEvent( QWheelEvent *event ) ;

    /**
     * @brief Zoom In/Out
     * @param factor Zoom factor
     * @note zoom in if factor > 1 ; zoom out if factor < 1
     * @note Zoom is made at the center of the view
     */
    void zoom( const double factor ) ;

  private:

} ;

} // namespace openMVG_gui

#endif