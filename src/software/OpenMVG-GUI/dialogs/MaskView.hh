// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_MASK_VIEW_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_MASK_VIEW_HH_

#include <QBrush>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QImage>
#include <QPen>
#include <QWidget>


namespace openMVG_gui
{
class MaskView : public QGraphicsView
{
  public:
    MaskView( QWidget * parent , QGraphicsScene * scn ) ;

    /**
    * @brief Set backgroundimage
    */
    void setBackgroundImage( QImage & img ) ;

    /**
    * @brief Set initial mask image
    */
    void setMaskImage( QImage & img ) ;

    /**
    * @brief Fit to view exactly the view image
    */
    void fit() ;

    /**
    * @brief get image mask
    */
    QImage getMask( void ) ;

    /**
    * @brief Get background item
    */
    QGraphicsPixmapItem * backgroundItem( void ) const ;

    /**
    * @brief clear all items
    */
    void clear( void ) ;

    /**
    * @brief
    */
    void setCurrentPenBrush( const QPen & pen , const QBrush & br ) ;

    /**
    * @brief Set current brush radius
    */
    void setCurrentRadius( const double rad )  ;


    /**
    * @brief Action to be executed when user click
    */
    void mousePressEvent( QMouseEvent * ev ) override ;

    /**
    * @brief Action to be executed when user move the mouse on the view
    */
    void mouseMoveEvent( QMouseEvent * ev ) override ;

    /**
    * @brief Action to be executed when user mouse quit the view
    */
    void leaveEvent( QEvent * ev ) override ;

    /**
    * @brief Action to be executed for zooming
    */
    void wheelEvent( QWheelEvent *event ) override ;




  private:

    QGraphicsPixmapItem * m_backgrounditem ;
    QGraphicsPixmapItem * m_maskimageitem ;
    QGraphicsEllipseItem * m_cursor ;

    QBrush m_cur_brush ;
    QPen m_cur_pen ;
    double m_radius ;

} ;

} // namespace openMVG_gui

#endif