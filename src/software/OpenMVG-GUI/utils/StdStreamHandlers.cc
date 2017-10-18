// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "StdStreamHandlers.hh"

namespace openMVG_gui
{

/**
 * @brief stream Stream to redirect
 * @param text_edit QTextEdit control used to transfer stream to
 */
StdStreamHandlers::StdStreamHandlers( std::ostream &stream, QTextEdit* text_edit ) : m_stream( stream )
{
  log_window = text_edit;
  m_old_buf = stream.rdbuf();
  // Take ownership of the stream (ie: it's the main function call that make the link between the stream and the handler)
  stream.rdbuf( this );
}

/**
 * @brief Dtr
 */
StdStreamHandlers::~StdStreamHandlers()
{
  m_stream.rdbuf( m_old_buf );
}

std::basic_streambuf<char>::int_type StdStreamHandlers::overflow( int_type v )
{
  if ( v == '\n' )
  {
    QMetaObject::invokeMethod( log_window , "append" , Qt::QueuedConnection , Q_ARG( QString, QString::fromStdString( m_string ) ) );
    //    log_window->append( m_string.c_str() );
    m_string.erase( m_string.begin(), m_string.end() );
  }
  else
  {
    m_string += v;
  }

  return v;
}

/**
 * @brief Triggered when stream has been augmented by a given stream data
 * @param p Message to be written to the stream
 * @param n Number of character to get from the stream
 * @return Number of character written
 */
std::streamsize StdStreamHandlers::xsputn( const char *p, std::streamsize n )
{
  m_string.append( p, p + n );

  int pos = 0;
  while ( pos != std::string::npos )
  {
    pos = m_string.find( '\n' );
    if ( pos != std::string::npos )
    {
      std::string tmp( m_string.begin(), m_string.begin() + pos );
      QMetaObject::invokeMethod( log_window , "append" , Qt::QueuedConnection , Q_ARG( QString, QString( tmp.c_str() ) ) );
      m_string.erase( m_string.begin(), m_string.begin() + pos + 1 );
    }
  }

  return n;
}

} // namespace openMVG_gui