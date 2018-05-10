// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_COLOR_INDICATOR_WIDGET_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_COLOR_INDICATOR_WIDGET_HH_

#include <QColor>
#include <QFrame>
#include <QWidget>

namespace openMVG_gui
{
/**
 * @brief simple widget used to show a frame with background color on it
 * User can set it's color
 * If user click on it, a signal "clicked()" is emitted
 */
class ColorIndicatorWidget : public QFrame
{
public:
  ColorIndicatorWidget( QWidget *parent = nullptr );

  void setColor( const QColor &col );

signals:

  void clicked( void );

protected:
  void mousePressEvent( QMouseEvent *event ) override;

private:
  Q_OBJECT
};

} // namespace openMVG_gui

#endif