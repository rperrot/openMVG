#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_IMAGE_LIST_WIDGET_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_IMAGE_LIST_WIDGET_HH_

#include <QListWidget>
#include <QListWidgetItem>
#include <QWidget>

#include <string>

namespace openMVG_gui
{

/**
* @brief items that are in imagelistwidgets 
* @note this is a convenient class, it should not be used by the user 
*/ 
class ImageListWidgetItem : public QListWidgetItem
{
  public:
    ImageListWidgetItem( const std::string & name , QListWidget * parent , const int id ) ;

    int id( void ) const ;

  private:
    int m_id ;
} ;

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

  signals :

    /**
    * @brief Action to be executed when the user has selected an image
    * @param the selected image ID
    */
    void hasSelectedAnImage( int id ) ;

  private slots :

    void onSelectionChanged( void ) ;

  private:

    /**
    * @brief Build interface widgets
    */
    void buildInterface( void ) ;
    void makeConnections( void ) ;

    std::vector< std::pair< int , std::string > > m_images ;

    QListWidget * m_image_list_view ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif