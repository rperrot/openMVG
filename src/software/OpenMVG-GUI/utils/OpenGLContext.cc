// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OpenGLContext.hh"

namespace openMVG_gui
{

OpenGLContext::OpenGLContext( QOpenGLWidget * widget )
  : m_widget( widget )
{

}

void OpenGLContext::makeCurrent( void )
{
  m_widget->makeCurrent() ;
}

void OpenGLContext::doneCurrent( void )
{
  m_widget->doneCurrent() ;
}

} // namespace openMVG_gui