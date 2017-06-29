#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_DETAIL_LIST_WIDGET_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_DETAIL_LIST_WIDGET_HH_

#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace openMVG_gui
{


/**
* @brief Widget used to get informations about the project (images, matching, ...)
*/
class DetailListWidget : public QTreeWidget
{
  public:

    /**
    * @brief Ctr
    * @param parent Parent widget
    */
    DetailListWidget( QWidget * parent ) ;

    /**
    * @brief Set Images information
    * @param names Name of the images
    * @param resolution (width,height) of the images
    */
    void setImagesInfos( std::map< int , std::string > & names ,
                         std::map<int, std::pair<int, int> > & resolution ) ;

    void drawRow( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override ;

  private:

    void BuildInterface( void ) ;

    QTreeWidgetItem * m_item_images ;
    QTreeWidgetItem * m_item_features ;
    QTreeWidgetItem * m_item_matches ;
    QTreeWidgetItem * m_item_reconstruction ;

} ;

} // namespace openMVG_gui

#endif