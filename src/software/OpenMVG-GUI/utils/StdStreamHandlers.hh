// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_STD_STREAM_HANDLERS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_STD_STREAM_HANDLERS_HH_

#include <QTextEdit>

#include <iostream>
#include <streambuf>
#include <string>

namespace openMVG_gui
{

/**
 * @brief Transfer a c++ stream to a QTextEdit
 * @note Original implementation found here :
 * https://stackoverflow.com/questions/10308425/redirect-stdcout-to-a-qtextedit
 * It's an adaptation to handle multithread support with Qt
 */
class StdStreamHandlers : public std::basic_streambuf<char>
{
  public:

    /**
     * @brief stream Stream to redirect
     * @param text_edit QTextEdit control used to transfer stream to
     */
    StdStreamHandlers( std::ostream &stream, QTextEdit* text_edit ) ;

    /**
     * @brief Dtr
     */
    ~StdStreamHandlers();


  protected:

    virtual std::basic_streambuf<char>::int_type overflow( int_type v );

    /**
     * @brief Triggered when stream has been augmented by a given stream data
     * @param p Message to be written to the stream
     * @param n Number of character to get from the stream
     * @return Number of character written
     */
    virtual std::streamsize xsputn( const char *p, std::streamsize n ) ;

  private:

    std::ostream &m_stream;
    std::streambuf *m_old_buf;
    std::string m_string;

    QTextEdit* log_window;
};

} // namespace openMVG_gui

#endif