#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_DETAIL_LIST_WIDGET_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_DETAIL_LIST_WIDGET_HH_

#include "utils/FeaturesStats.hh"

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

    /**
    * @brief Set features statistics
    * @param show_names Hierarchy for name information (ex : SIFT / SIFT / ULTRA )
    * @param stats Statistics (image name, stats)
    */
    void setFeaturesInfos( const std::vector<std::string> & show_names ,
                           const std::map< std::string , FeaturesStats > & stats ) ;

    void drawRow( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override ;

    // Clear all item except structural items 
    void clear( ) ;

  private:

    void BuildInterface( void ) ;

    QTreeWidgetItem * m_item_images ;
    QTreeWidgetItem * m_item_features ;
    QTreeWidgetItem * m_item_matches ;
    QTreeWidgetItem * m_item_reconstruction ;

} ;

} // namespace openMVG_gui

#endif