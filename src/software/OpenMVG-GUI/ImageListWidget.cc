#include "ImageListWidget.hh"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QBrush>
#include <QFontMetrics>
#include <QMenu>
#include <QPainter>
#include <QVBoxLayout>

#include <iostream>

namespace openMVG_gui
{

//// DELEGATE
ImageListDrawingDelegate::ImageListDrawingDelegate( QWidget * parent )
  : QStyledItemDelegate( parent )
{

}

void ImageListDrawingDelegate::paint( QPainter * painter ,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index ) const
{
  painter->save() ;
  QStyledItemDelegate::paint( painter, option, index );

  const int id = index.data( Qt::UserRole ).toInt() ;

  const bool hasMask = index.data( Qt::UserRole + 1 ).toBool() ;

  QPixmap img = index.data( Qt::DecorationRole ).value<QPixmap>() ;

  QBrush white_background( Qt::SolidPattern ) ;
  QColor white_a200( 255 , 255 , 255 , 200 ) ;
  white_background.setColor( white_a200 ) ;

  painter->setBrush( white_background ) ;

  QFontMetrics metric( painter->font() ) ;
  const int text_height = metric.height( ) ;
  const int text_width = metric.width( std::to_string( id ).c_str() ) ;
  // TODO : need to check how to get 5 and 3 automatically
  const int mask_text_width = metric.width( "M" ) ;

  painter->drawRect( option.rect.x() + 5 , option.rect.y() + 3 , text_width + 10 , text_height + 10 ) ;
  painter->drawText( option.rect.x() + 10 , option.rect.y() + 5 + text_height , std::to_string( id ).c_str() ) ;

  if( hasMask )
  {
    painter->drawRect( option.rect.x() + option.rect.width( ) - 15 - mask_text_width , option.rect.y() + 3 , mask_text_width + 10 , text_height + 10 ) ;
    painter->drawText( option.rect.x() + option.rect.width() - 10 - mask_text_width , option.rect.y() + 5 + text_height , "M" ) ;
  }
  painter->restore() ;
}


///// ITEM
ImageListWidgetItem::ImageListWidgetItem( const std::string & name , QListWidget * parent , const int id , const bool has_mask )
  : QListWidgetItem( name.c_str() , parent ) ,
    m_id( id ) ,
    m_has_mask( has_mask )
{

}

int ImageListWidgetItem::id( void ) const
{
  return m_id ;
}

bool ImageListWidgetItem::hasMask( void ) const
{
  return m_has_mask ;
}

void ImageListWidgetItem::setHasMask( const bool has )
{
  m_has_mask = has ;
  setData( Qt::UserRole + 1 , m_has_mask ) ;
}



//// CONTAINER

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
  setMinimumSize( 310 , 10 ) ;
}


/**
* @brief Handle right click
*/
void ImageListWidget::contextMenuEvent( QContextMenuEvent *e )
{
  if( e->reason() != QContextMenuEvent::Mouse )
  {
    return ;
  }

  QListWidgetItem * it = m_image_list_view->itemAt( e->pos() ) ;
  if( it )
  {
    ImageListWidgetItem * item = dynamic_cast<ImageListWidgetItem*>( it ) ;
    if( item )
    {
      int id = item->id() ;

      QMenu ctxMenu( "Param" , this ) ;
      QAction * maskAct = ctxMenu.addAction( "Mask" ) ;
      //      QAction * intrinAct = ctxMenu.addAction( "Intrinsic" ) ;

      connect( maskAct ,  &QAction::triggered , [ = ]()
      {
        this->onMaskDefinition( id ) ;
      } ) ;
      /*
      connect( intrinAct , &QAction::triggered , [ = ]()
      {
        this->onIntrinsicSelection( id ) ;
      } ) ;
      */

      ctxMenu.exec( mapToGlobal( e->pos() ) );
    }
  }
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
    item->setData( Qt::UserRole , paths[id_image].first ) ;
    item->setData( Qt::UserRole + 1 , false ) ;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled ) ;
  }
}

/**
* @brief Set enable/disable mask on selected image
* @param id_image Id of the image to enable
* @param enable Enable status
*/
void ImageListWidget::setMaskEnabled( const int id_image , const bool enable )
{
  for( int i = 0 ; i < m_image_list_view->count() ; ++i )
  {
    QListWidgetItem * item = m_image_list_view->item( i ) ;
    const int id = item->data( Qt::UserRole ).toInt() ;
    if( id == id_image )
    {
      item->setData( Qt::UserRole + 1 , enable ) ;
    }
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

  m_image_list_view->setItemDelegate( new ImageListDrawingDelegate( this ) ) ;


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

/**
* @brief Launch widget to define image mask
* @param id Id of the image to define
*/
void ImageListWidget::onMaskDefinition( int id )
{
  emit hasRequestedMaskDefinition( id ) ;
}

/**
* @brief Launch widget to define image mask
* @param id Id of the image to define
*/
void ImageListWidget::onIntrinsicSelection( int id )
{
  emit hasRequestedIntrinsicSelection( id ) ;
}



} // namespace openMVG_gui
