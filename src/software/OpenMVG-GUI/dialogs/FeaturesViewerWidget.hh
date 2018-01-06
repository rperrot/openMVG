// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_FEATURES_VIEWER_WIDGET_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_FEATURES_VIEWER_WIDGET_HH_

#include "openMVG/numeric/numeric.h"

#include "ZoomableGraphicsView.hh"

#include <QAction>
#include <QComboBox>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QLabel>
#include <QWidget>

#include <map>
#include <vector>

namespace openMVG_gui
{

class Project ;

/**
 * @brief Widget used to show features computed on a given image
 */
class FeaturesViewerWidget : public QWidget
{
  public:

    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    FeaturesViewerWidget( QWidget * parent ) ;

    /**
     * @brief Set project and update interface
     */
    void setProject( std::shared_ptr<Project> proj ) ;

  signals:

    void hasBeenClosed( void ) ;

  protected:

    void closeEvent( QCloseEvent * event ) override ;


  public slots:

    /**
     * @brief Set the current image to show
     * @param path Path of the image to show
     */
    void setImage( const std::string & path ) ;

    /**
     * @brief Set the current image to show
     * @param id Id of the image (wrt to the combo box, not to the project)
     */
    void setImage( const int id ) ;

    /**
     * @brief Action to be executed when user select a feature set
     */
    void onSelectFeatureSet( void ) ;

    /**
     * @brief Action to be executed when user wants to zoom images in order to fit the window
     */
    void onZoomToFit( void ) ;

    /**
     * @brief Action to be executed when user wants to zoom images in order to be real size
     */
    void onZoom11( void ) ;

    /**
     * @brief Action to be executed when user wants to zoom in
     */
    void onZoomIn( void ) ;

    /**
     * @brief Action to be executed when user wants to zoom out
     */
    void onZoomOut( void ) ;

    /**
     * @brief Add a feature set to the current set
     * @param Name to be displayed for this set
     * @param pts a set of point
     */
    void addFeaturesSet( const std::string name , const std::vector< openMVG::Vec2 > & pts ) ;

    /**
     * @brief Remove all features from the set of displayable features
     */
    void clearFeatureSet( void ) ;

  private:

    /**
     * @brief build interface widgets
     */
    void buildInterface( void ) ;

    /**
     * @brief Make connections between widgets
     */
    void makeConnections( void ) ;

    std::shared_ptr<Project> m_project ;

    QLabel * m_image_lbl ;
    QComboBox * m_image_list ;
    QLabel * m_featureset_lbl ;
    QComboBox * m_feature_set ;
    QPixmap m_cur_image ;
    QGraphicsScene * m_graphics_scn ;
    ZoomableGraphicsView * m_image_view ;

    QAction * m_zoom_to_fill_act ;
    QAction * m_zoom_1_1_act ;
    QAction * m_zoom_in_act ;
    QAction * m_zoom_out_act ;

    /// Map between feature_name and id
    std::map< std::string , int > m_name_index ;
    std::map< int , std::string > m_index_name ;
    /// Map between feature name and it's corresponding feature set
    std::map< std::string , std::vector< openMVG::Vec2 > > m_pts_list ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif