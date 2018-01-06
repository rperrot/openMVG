// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_RECONSTRUCTION_SUMMARY_WIDGET_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_RECONSTRUCTION_SUMMARY_WIDGET_HH_

#include <QWebEngineView>
#include <QWidget>

namespace openMVG_gui
{

/**
 * @brief Class used to show results of the reconstruction process
 */
class ReconstructionSummaryWidget : public QWidget
{
  public:

    /**
     * @brief Ctr
     * @param path Reconstruction summary path
     * @param parent parent widget
     */
    ReconstructionSummaryWidget( QWidget * parent = nullptr ) ;

    /**
     * @brief Set path where the summary is
     * @param path Path
     */
    void setPath( const std::string & path ) ;


  signals:

    void hasBeenClosed( void ) ;

  protected:

    void closeEvent( QCloseEvent * event ) override ;


  private slots:

    void hasLoadedURL( bool ok ) ;

  private:
    /**
     * @brief build interface widgets
     */
    void buildInterface( void ) ;

    QWebEngineView * m_view ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif