// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "FeaturesViewerWidget.hh"

#include "Project.hh"

#include <QHBoxLayout>
#include <QToolBar>
#include <QVBoxLayout>

#include <random>

namespace openMVG_gui
{
/**
 * @brief Constructor
 * @param parent Parent widget
 */
FeaturesViewerWidget::FeaturesViewerWidget( QWidget * parent )
  : QWidget( parent )
{
  buildInterface() ;
  makeConnections() ;

  resize( 1024 , 768 ) ;
  setWindowTitle( "Feature viewer" ) ;
}

/**
 * @brief Set project
 */
void FeaturesViewerWidget::setProject( std::shared_ptr<Project> proj )
{
  m_project = proj ;

  m_image_list->clear() ;
  m_feature_set->clear() ;

  // Update image list
  const std::vector< std::pair< int , std::string > > & images_names = m_project->GetImageNames() ;
  for( const auto & it : images_names )
  {
    m_image_list->addItem( it.second.c_str() , QVariant( it.first ) ) ;
  }
}

void FeaturesViewerWidget::closeEvent( QCloseEvent * event ) 
{
  emit hasBeenClosed() ; 
}



/**
 * @brief Set the current image to show
 * @param path Path of the image to show
 */
void FeaturesViewerWidget::setImage( const std::string & path )
{
  m_graphics_scn->clear() ;

  QImage img( path.c_str() ) ;
  m_cur_image = QPixmap::fromImage( img ) ;
  m_graphics_scn->addPixmap( m_cur_image.copy() ) ;

  m_image_view->fitInView( m_graphics_scn->sceneRect(), Qt::KeepAspectRatio );
}

/**
 * @brief Set the current image to show
 * @param id Id of the image (wrt to the combo box, not to the project)
 */
void FeaturesViewerWidget::setImage( const int id )
{
  if( id != -1 )
  {
    const int project_id = m_image_list->itemData( id ).toInt() ;
    size_t img_id = 0 ;
    const std::vector< std::pair< int , std::string > > & images_names = m_project->GetImageNames() ;
    for( const auto & it : images_names )
    {
      if( it.first == project_id )
      {
        break ;
      }
      ++img_id ;
    }

    const std::string real_path = m_project->getImagePath( img_id ) ;
    setImage( real_path ) ;

    // Load features already computed for this image
    clearFeatureSet() ;
    const std::map < std::string , std::vector< openMVG::Vec2 > > feats = m_project->getFeaturesPositions( img_id ) ;
    for( auto & it : feats )
    {
      addFeaturesSet( it.first , it.second ) ;
    }
  }
  else
  {
    // Clear the image
    m_graphics_scn->clear() ;
  }
}


/**
 * @brief Action to be executed when user select a feature set
 */
void FeaturesViewerWidget::onSelectFeatureSet( void )
{
  const int id_feat = m_feature_set->currentIndex() ;
  const int id_image = m_image_list->currentIndex() ;
  if( ( id_feat != -1 ) &&
      ( id_image != -1 ) &&
      ! m_cur_image.isNull() )
  {
    m_graphics_scn->clear() ;
    m_graphics_scn->addPixmap( m_cur_image.copy() ) ;

    if( id_feat != -1 )
    {
      if( m_index_name.count( id_feat ) > 0 )
      {
        const std::string feat_name = m_index_name.at( id_feat ) ;
        const std::vector< openMVG::Vec2 > & pts = m_pts_list.at( feat_name ) ;
        // Draw points
        const double size = 5.0 ;
        std::uniform_int_distribution<int> distrib_rgb( 0 , 255 ) ;
        std::default_random_engine rng ;
        for( auto & pt : pts )
        {
          QColor color( distrib_rgb( rng ) , distrib_rgb( rng ) , distrib_rgb( rng ) ) ;
          QPen pen( color ) ;

          m_graphics_scn->addEllipse( pt.x() - size / 2.0 , pt.y() - size / 2.0 , size , size , pen ) ;
        }
      }
    }

  }
}

/**
 * @brief Action to be executed when user wants to zoom images in order to fit the window
 */
void FeaturesViewerWidget::onZoomToFit( void )
{
  m_image_view->fitInView( m_graphics_scn->sceneRect(), Qt::KeepAspectRatio );
}

/**
 * @brief Action to be executed when user wants to zoom images in order to be real size
 */
void FeaturesViewerWidget::onZoom11( void )
{
  m_image_view->resetMatrix() ;
  m_image_view->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  m_image_view->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
}

/**
 * @brief Action to be executed when user wants to zoom in
 */
void FeaturesViewerWidget::onZoomIn( void )
{
  m_image_view->zoom( 1.1 ) ;
}

/**
 * @brief Action to be executed when user wants to zoom out
 */
void FeaturesViewerWidget::onZoomOut( void )
{
  m_image_view->zoom( 0.9 ) ;
}


/**
 * @brief Add a feature set to the current set
 * @param Name to be displayed for this set
 * @param pts a set of point
 */
void FeaturesViewerWidget::addFeaturesSet( const std::string name ,
    const std::vector< openMVG::Vec2 > & pts )
{
  const int nb_elt = m_feature_set->count() ;
  m_pts_list.insert( { name , pts } ) ;
  m_name_index.insert( { name , nb_elt } ) ;
  m_index_name.insert( { nb_elt , name } ) ;

  // Make sure this is the last line or the others vectors are 0 when selecting only one image
  m_feature_set->addItem( name.c_str() ) ;
}

/**
 * @brief Remove all features from the set of displayable features
 */
void FeaturesViewerWidget::clearFeatureSet( void )
{
  m_feature_set->clear() ;
  m_pts_list.clear() ;
  m_name_index.clear() ;
  m_index_name.clear() ;
}

/**
 * @brief build interface widgets
 */
void FeaturesViewerWidget::buildInterface( void )
{
  m_graphics_scn = new QGraphicsScene ;
  m_image_view = new ZoomableGraphicsView( m_graphics_scn ) ;

  QVBoxLayout * mainLayout = new QVBoxLayout ;
  QHBoxLayout * imageLayout = new QHBoxLayout ;
  QVBoxLayout * zoomLayout = new QVBoxLayout ;
  QHBoxLayout * selectLayout = new QHBoxLayout ;

  m_image_lbl = new QLabel( "Image" ) ;
  m_image_list = new QComboBox ;
  m_featureset_lbl = new QLabel( "Features type" ) ;
  m_feature_set = new QComboBox ;

  selectLayout->addStretch() ;
  selectLayout->addWidget( m_image_lbl ) ;
  selectLayout->addWidget( m_image_list ) ;
  selectLayout->addWidget( m_featureset_lbl ) ;
  selectLayout->addWidget( m_feature_set ) ;
  selectLayout->addStretch() ;

  QToolBar * toolbar = new QToolBar ;
  toolbar->setContentsMargins( 0 , 0 , 0 , 0 ) ;
  toolbar->setOrientation( Qt::Vertical ) ;

  m_zoom_to_fill_act = toolbar->addAction( "Fit" ) ;
  m_zoom_1_1_act = toolbar->addAction( "1:1" ) ;
  m_zoom_in_act = toolbar->addAction( "+" ) ;
  m_zoom_out_act = toolbar->addAction( "-" ) ;

  zoomLayout->setContentsMargins( 0 , 0 , 0 , 0 ) ;
  zoomLayout->addWidget( toolbar ) ;
  zoomLayout->addStretch();

  imageLayout->addWidget( m_image_view ) ;
  imageLayout->addLayout( zoomLayout ) ;

  mainLayout->setContentsMargins( 5 , 5 , 5 , 5 ) ;
  mainLayout->addLayout( selectLayout ) ;
  mainLayout->addLayout( imageLayout ) ;

  setLayout( mainLayout ) ;
}

/**
 * @brief Make connections between widgets
 */
void FeaturesViewerWidget::makeConnections( void )
{
  connect( m_image_list , SIGNAL( currentIndexChanged( int ) ) , this , SLOT( setImage( const int ) ) ) ;
  connect( m_feature_set , SIGNAL( currentIndexChanged( int ) ) , this , SLOT( onSelectFeatureSet() ) ) ;
  connect( m_zoom_1_1_act , SIGNAL( triggered() ) , this , SLOT( onZoom11() ) ) ;
  connect( m_zoom_to_fill_act , SIGNAL( triggered() ) , this , SLOT( onZoomToFit() ) ) ;
  connect( m_zoom_in_act , SIGNAL( triggered() ) , this , SLOT( onZoomIn() ) ) ;
  connect( m_zoom_out_act , SIGNAL( triggered() ) , this , SLOT( onZoomOut() ) ) ;
}

} // namespace openMVG_gui
