#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_IMAGE_LIST_WIDGET_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_IMAGE_LIST_WIDGET_HH_

#include <QListWidget>
#include <QWidget>

namespace openMVG_gui
{
/**
* @brief A widget used to display a list of images in thumbnails
* @note The widget may in a next step resize the images
*/
class ImageListWidget : public QWidget
{
  public:

    /**
    * @brief ctr
    * @param parent parent widget
    */
    ImageListWidget( QWidget * parent ) ;


  public slots :

    /**
    * @brief Set the list of images to display
    * @param paths list of path of the images to display
    * @note if there are existing images, it replace everything
    */
    void setImages( const std::vector< std::pair< int , std::string > > & paths ) ;

    /**
    * @brief remove all images of the widget
    */
    void clear( void ) ;

  private:

    /**
    * @brief Build interface widgets 
    */
    void buildInterface( void ) ;

    QListWidget * m_image_list_view ;
} ;

} // namespace openMVG_gui

#endif