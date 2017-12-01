#include "FeaturesViewerWidget.hh"

#include "Project.hh"

#include <QHboxLayout>
#include <QVBoxLayout>

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


/**
 * @brief Set the current image to show
 * @param path Path of the image to show
 */
void FeaturesViewerWidget::setImage( const std::string & path )
{
  m_graphics_scn->clear() ;

  QImage img( path.c_str() ) ;
  m_base_image = m_graphics_scn->addPixmap( QPixmap::fromImage( img ) ) ;

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
    std::cerr << "Looking for image : " << project_id << std::endl ;
    size_t id = 0 ;
    const std::vector< std::pair< int , std::string > > & images_names = m_project->GetImageNames() ;
    for( const auto & it : images_names )
    {
      if( it.first == project_id )
      {
        break ;
      }
      ++id ;
    }

    const std::string real_path = m_project->getImagePath( id ) ;
    setImage( real_path ) ;
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
  m_graphics_scn->clear() ;
  m_graphics_scn->addItem( m_base_image ) ;

  const int id = m_feature_set->currentIndex() ;
  if( id != -1 )
  {
    const std::string feat_name = m_index_name.at( id ) ;
    const std::vector< openMVG::Vec2 > & pts = m_pts_list.at( feat_name ) ;
    // Draw points
  }
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
  m_feature_set->addItem( name.c_str() ) ;
  m_pts_list.insert( { name , pts } ) ;
  m_name_index.insert( { name , nb_elt } ) ;
  m_index_name.insert( { nb_elt , name } ) ;
}

/**
 * @brief Remove all features from the set of displayable features
 */
void FeaturesViewerWidget::clearFeatureSet( void )
{
  m_feature_set->clear() ;
}

/**
 * @brief build interface widgets
 */
void FeaturesViewerWidget::buildInterface( void )
{
  m_graphics_scn = new QGraphicsScene ;
  m_image_view = new QGraphicsView( m_graphics_scn ) ;

  QVBoxLayout * mainLayout = new QVBoxLayout ;
  QHBoxLayout * selectLayout = new QHBoxLayout ;

  m_image_list = new QComboBox ;
  m_feature_set = new QComboBox ;

  selectLayout->addStretch() ;
  selectLayout->addWidget( m_image_list ) ;
  selectLayout->addWidget( m_feature_set ) ;
  selectLayout->addStretch() ;

  mainLayout->addLayout( selectLayout ) ;
  mainLayout->addWidget( m_image_view ) ;

  setLayout( mainLayout ) ;
}

/**
 * @brief Make connections between widgets
 */
void FeaturesViewerWidget::makeConnections( void )
{
  connect( m_image_list , SIGNAL( currentIndexChanged( int ) ) , this , SLOT( setImage( const int ) ) ) ;
  connect( m_feature_set , SIGNAL( currentIndexChanged( int ) ) , this , SLOT( onSelectFeatureSet() ) ) ;
}

} // namespace openMVG_gui