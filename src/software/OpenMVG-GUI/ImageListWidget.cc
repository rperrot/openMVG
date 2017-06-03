#include "ImageListWidget.hh"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QVBoxLayout>

#include <iostream>

namespace openMVG_gui
{

ImageListWidgetItem::ImageListWidgetItem( const std::string & name , QListWidget * parent , const int id )
  : QListWidgetItem( name.c_str() , parent ) ,
    m_id( id )
{

}

int ImageListWidgetItem::id( void ) const
{
  return m_id ;
}


/**
* @brief ctr
* @param parent parent widget
*/
ImageListWidget::ImageListWidget( QWidget * parent )
  : QWidget( parent )
{
  buildInterface() ;
  makeConnections() ; 
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
  m_images = paths ;
  m_image_list_view->clear() ;
  for( size_t id_image = 0 ; id_image < paths.size() ; ++id_image )
  {
    const std::string base_name = stlplus::filename_part( paths[id_image].second ) ;
    QImage img( paths[id_image].second.c_str() );
    ImageListWidgetItem * item = new ImageListWidgetItem( base_name , m_image_list_view , paths[id_image].first ) ;
    item->setData( Qt::DecorationRole ,  QPixmap::fromImage( img ) );
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled ) ;
  }
}

/**
* @brief remove all images of the widget
*/
void ImageListWidget::clear( void )
{
  m_images.clear() ;
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

void ImageListWidget::makeConnections( void )
{
  connect( m_image_list_view , SIGNAL( itemSelectionChanged() ) , this , SLOT( onSelectionChanged() ) ) ;
}

void ImageListWidget::onSelectionChanged( void )
{
  QList<QListWidgetItem *> selectedItems = m_image_list_view->selectedItems() ;
  if( selectedItems.count() == 1 )
  {
    ImageListWidgetItem * item = dynamic_cast<ImageListWidgetItem*>( selectedItems.at( 0 ) ) ;
    if( item )
    {
      emit hasSelectedAnImage( item->id() ) ;
    }
  }
}



} // namespace openMVG_gui
