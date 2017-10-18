// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ConsoleWidget.hh"

#include <QGridLayout>

namespace openMVG_gui
{

/**
* @brief Ctr
* @param parent Parent widget
*/
ConsoleWidget::ConsoleWidget( QWidget * parent )
  : QWidget( parent )
{
  buildInterface( );
  setWindowTitle( "Console" );

  resize( 1024, 600 ) ;

  // Handle std::cout
  handler_cout = new StdStreamHandlers( std::cout , m_text_element ) ;
  // Handle std::cerr
  handler_cerr = new StdStreamHandlers( std::cerr , m_text_element ) ;
}

/**
 * @brief Build interface
 */
void ConsoleWidget::buildInterface( void )
{
  m_text_element = new QTextEdit( this ) ;
  m_text_element->setReadOnly( true ) ;
  QGridLayout * layout = new QGridLayout ;

  layout->addWidget( m_text_element ) ;

  setLayout( layout ) ;
}

} // namespace openMVG_gui