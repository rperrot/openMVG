// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ReconstructionSummaryWidget.hh"

#include <QGridLayout>
#include <QUrl>

#include <iostream>

namespace openMVG_gui
{

/**
 * @brief Ctr
 * @param path Reconstruction summary path
 * @param parent parent widget
 */
ReconstructionSummaryWidget::ReconstructionSummaryWidget( QWidget * parent )
  : QWidget( parent )
{
  buildInterface() ;
  resize( 1024, 600 ) ;
  setWindowTitle( "Reconstuction Summary" ) ;
}

/**
 * @brief Set path where the summary is
 * @param path Path
 */
void ReconstructionSummaryWidget::setPath( const std::string & path )
{
  m_view->load( QUrl::fromLocalFile( path.c_str() ) ) ;
  m_view->show() ;
}

/**
 * @brief build interface widgets
 */
void ReconstructionSummaryWidget::buildInterface( void )
{
  m_view = new QWebEngineView( this ) ;
  m_view->show() ;
  connect( m_view , SIGNAL( loadFinished( bool ) ) , this , SLOT( hasLoadedURL( bool ) ) ) ;

  QGridLayout * layout = new QGridLayout ;

  layout->addWidget( m_view ) ;

  setLayout( layout ) ;
}

void ReconstructionSummaryWidget::closeEvent( QCloseEvent * event )
{
  emit hasBeenClosed() ;
}


void ReconstructionSummaryWidget::hasLoadedURL( bool ok )
{
  if( ! ok )
  {
    std::cerr << "Reconstruction summary : load failure" << std::endl ;
  }
}

} // namespace openMVG_gui
