#include "DetailListWidget.hh"

#include <QHeaderView>

namespace openMVG_gui
{

/**
* @brief Ctr
* @param parent Parent widget
*/
DetailListWidget::DetailListWidget( QWidget * parent )
  : QTreeWidget( parent )
{
  BuildInterface() ;
  setMinimumSize( 310 , 10 ) ;
}


/**
* @brief Set Images information
* @param names Name of the images
* @param resolution (width,height) of the images
*/
void DetailListWidget::setImagesInfos( std::map< int , std::string > & names ,
                                       std::map<int, std::pair<int, int> > & resolution )
{
  // Remove existing childs
  m_item_images->takeChildren() ;

  for( auto & it : names )
  {
    const int cur_id = it.first ;
    const std::string & cur_name = it.second ;

    QTreeWidgetItem * item = new QTreeWidgetItem( m_item_images ) ;
    item->setText( 0 , std::to_string( cur_id ).c_str() ) ;

    QTreeWidgetItem * itemName = new QTreeWidgetItem( item ) ;
    itemName->setText( 0 , "Name" ) ;
    itemName->setText( 1 , cur_name.c_str() ) ;
    itemName->setData( 1 , Qt::UserRole , cur_id ) ;

    if( resolution.count( cur_id ) )
    {
      QTreeWidgetItem * itemWidth = new QTreeWidgetItem( item ) ;
      itemWidth->setText( 0 , "Width" ) ;
      itemWidth->setText( 1 , std::to_string( resolution.at( cur_id ).first ).c_str() ) ;
      itemWidth->setData( 1 , Qt::UserRole , cur_id ) ;

      QTreeWidgetItem * itemHeight = new QTreeWidgetItem( item ) ;
      itemHeight->setText( 0 , "Height" ) ;
      itemHeight->setText( 1 , std::to_string( resolution.at( cur_id ).second ).c_str() ) ;
      itemHeight->setData( 1 , Qt::UserRole , cur_id ) ;
    }

  }
}

void DetailListWidget::drawRow( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QTreeView::drawRow( painter , option , index ) ;
}


void DetailListWidget::BuildInterface( void )
{
  std::string style =
    "QTreeView {\
    background-color: rgb(64,64,64);\
    alternate-background-color: rgb(100,100,100);\
    color: white;\
    }\
    QTreeView::branch:has-children:!has-siblings:closed, \
    QTreeView::branch:closed:has-children:has-siblings { \
    border-image: none; \
    image: url(:/icons/plus.svg) \
    } \
    \
    QTreeView::branch:open:has-children:!has-siblings, \
    QTreeView::branch:open:has-children:has-siblings  { \
    border-image: none; \
    image: url(:/icons/minus.svg);\
  }";
  setStyleSheet( style.c_str() ) ;

  setAlternatingRowColors( true ) ;
  setColumnCount( 2 );
  header()->close() ;
  header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
  header()->setSectionResizeMode( 1, QHeaderView::ResizeToContents );


  m_item_images = new QTreeWidgetItem( this ) ;
  m_item_images->setFirstColumnSpanned( true ) ;
  m_item_images->setText( 0 , "Images" ) ;

  m_item_features = new QTreeWidgetItem( this ) ;
  m_item_features->setFirstColumnSpanned( true ) ;
  m_item_features->setText( 0 , "Features" ) ;

  m_item_matches = new QTreeWidgetItem( this ) ;
  m_item_matches->setFirstColumnSpanned( true ) ;
  m_item_matches->setText( 0 , "Matches" ) ;

  m_item_reconstruction = new QTreeWidgetItem( this ) ;
  m_item_reconstruction->setFirstColumnSpanned( true ) ;
  m_item_reconstruction->setText( 0 , "Reconstruction" ) ;
}


} // namespace openMVG_gui
