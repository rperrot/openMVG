// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
