#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_IMAGE_LIST_WIDGET_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_IMAGE_LIST_WIDGET_HH_

#include <QContextMenuEvent>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStyledItemDelegate>
#include <QWidget>

#include <string>

namespace openMVG_gui
{

/**
 * @brief Custom drawing of the image items
 * Allow to draw id and indication if it has a mask
 */
class ImageListDrawingDelegate : public QStyledItemDelegate
{
  public:
    ImageListDrawingDelegate( QWidget *parent );

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

/**
 * @brief items that are in imagelistwidgets
 * @note this is a convenient class, it should not be used by the user
 */
class ImageListWidgetItem : public QListWidgetItem
{
  public:
    ImageListWidgetItem( const std::string &name = "", QListWidget *parent = nullptr, const int id = -1,
                         const bool has_mask = false );

    int id( void ) const;

    bool hasMask( void ) const;

    void setHasMask( const bool has );

  private:
    int m_id;
    bool m_has_mask;
};

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
    ImageListWidget( QWidget *parent );

    /**
     * @brief Handle right click
     */
    void contextMenuEvent( QContextMenuEvent *e ) override;

  public slots:

    /**
     * @brief Set the list of images to display
     * @param paths list of path of the images to display
     * @note if there are existing images, it replace everything
     */
    void setImages( const std::vector<std::pair<int, std::string>> &paths );

    /**
     * @brief Set enable/disable mask on selected image
     * @param id_image Id of the image to enable
     * @param enable Enable status
     */
    void setMaskEnabled( const int id_image, const bool enable );

    /**
     * @brief remove all images of the widget
     */
    void clear( void );

    /**
     * @brief Launch widget to define image mask
     * @param id Id of the image to define
     */
    void onMaskDefinition( int id );

    /**
     * @brief Launch widget to define image mask
     * @param id Id of the image to define
     */
    void onIntrinsicSelection( int id );

  signals:

    /**
     * @brief Action to be executed when the user has selected an image
     * @param the selected image ID
     */
    void hasSelectedAnImage( int id );

    /**
     * @brief Signal to inform the main window that user would like to define mask for an image
     * @param id Id of the image which requested the definition
     */
    void hasRequestedMaskDefinition( int id );

    /**
     * @brief Signal to inform the main window that user would like to define intrinsic for an image
     * @param id Id of the image which requested the definition
     */
    void hasRequestedIntrinsicSelection( int id );

  private slots:

    void onSelectionChanged( void );

  private:
    /**
     * @brief Build interface widgets
     */
    void buildInterface( void );
    void makeConnections( void );

    std::vector<std::pair<int, std::string>> m_images;

    QListWidget *m_image_list_view;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif