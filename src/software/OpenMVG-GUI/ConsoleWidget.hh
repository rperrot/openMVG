// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_CONSOLE_WIDGET_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_CONSOLE_WIDGET_HH_

#include "utils/StdStreamHandlers.hh"

#include <QTextEdit>
#include <QWidget>

#include <memory>

namespace openMVG_gui
{

/**
 * @brief Console widget used to display runtime messages (std::cout, std::cerr)
 */
class ConsoleWidget : public QWidget
{
  public:

    /**
     * @brief Ctr
     * @param parent Parent widget
     */
    ConsoleWidget( QWidget * parent ) ;

  signals:

    void hasBeenClosed( void ) ; 

  protected:

    void closeEvent( QCloseEvent * event ) override ; 

  private:


    /**
     * @brief Build interface
     */
    void buildInterface( void ) ;

    QTextEdit * m_text_element ;

    std::shared_ptr<StdStreamHandlers> m_handler_cout ;
    std::shared_ptr<StdStreamHandlers> m_handler_cerr ;

    Q_OBJECT 
} ;

} // namespace openMVG_gui

#endif