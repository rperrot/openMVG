#include "ImageListWidget.hh"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QVBoxLayout>

namespace openMVG_gui
{

/**
* @brief ctr
* @param parent parent widget
*/
ImageListWidget::ImageListWidget( QWidget * parent )
  : QWidget( parent )
{
  buildInterface() ;
  // TODO : find an automatic way to do it ? 
  // note : thumbnails are 256pix in width 
  setMinimumSize( 300 , 10 ) ;
}

/**
* @brief Set the list of images to display
* @param paths list of path of the images to display
* @note if there are existing images, it replace everything
*/
void ImageListWidget::setImages( const std::vector< std::pair< int , std::string > > & paths )
{
  m_image_list_view->clear() ; 
  for( size_t id_image = 0 ; id_image < paths.size() ; ++id_image )
  {
    const std::string base_name = stlplus::filename_part( paths[id_image].second ) ;
    QImage img( paths[id_image].second.c_str() );
    QListWidgetItem * item = new QListWidgetItem( base_name.c_str() , m_image_list_view ) ;
    item->setData( Qt::DecorationRole ,  QPixmap::fromImage( img ) );
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled ) ;
  }
}

/**
* @brief remove all images of the widget
*/
void ImageListWidget::clear( void )
{
  m_image_list_view->clear() ; 
}

/**
* @brief Build interface widgets
*/
void ImageListWidget::buildInterface( void )
{
  // Set image list to an icon view of size (128,128)
  m_image_list_view = new QListWidget( this ) ;
  m_image_list_view->setViewMode( QListWidget::IconMode ) ;
  m_image_list_view->setIconSize( QSize( 128 , 128 ) ) ;
  m_image_list_view->setResizeMode( QListWidget::Adjust );


  QVBoxLayout * mainLayout = new QVBoxLayout ;
  mainLayout->addWidget( m_image_list_view ) ;
  setLayout( mainLayout ) ;
}


} // namespace openMVG_gui
