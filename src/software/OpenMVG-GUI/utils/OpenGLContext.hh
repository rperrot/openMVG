// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_OPENGL_CONTEXT_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_OPENGL_CONTEXT_HH_

#include <QOpenGLWidget>

namespace openMVG_gui
{

// Generic class holding an openGL Context
// In the future it should be a transparent class for various contexts 
class OpenGLContext
{
  public:
    /**
     * @brief Ctr 
     * @param widget The widget that contains the context 
     */
    OpenGLContext( QOpenGLWidget * widget ) ;

    /**
     * @brief Make the current context active 
     */
    void makeCurrent( void ) ;

    /**
     * @brief Unactivate the context 
     */
    void doneCurrent( void ) ;

  private:

    QOpenGLWidget * m_widget ;
} ;

} // namespace openMVG_gui

#endif