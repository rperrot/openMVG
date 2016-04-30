#ifndef _OPENMVG_SFMGUI_IMAGETAB_HPP_
#define _OPENMVG_SFMGUI_IMAGETAB_HPP_

#include <QWidget>
#include <QPushButton>
#include <QTableWidget>

namespace openMVG
{
namespace SfMGui
{

/**
* @brief Class managing adding/removing image list
*/
class ImageTab : public QWidget
{
    Q_OBJECT

  public:

    /**
    * @brief Constructor
    * @param parent Parent
    */
    ImageTab( QWidget * parent = nullptr ) ;

    /**
    * @brief Add a new row to the view
    * @param imagePath Path of the image to be displayed in the table
    * @param imageName Name of the image to be displayed
    * @param image_width Width (in pixel) of the input image
    * @param image_height Height (in pixel) of the input image
    * @param focal Focal length of the input image
    * @note imagePath is idealy a path to the thumbnail
    */
    void AddRow( const std::string & imagePath , const std::string & imageName , const int image_width , const int image_height , const float focal ) ;

    /**
    * @brief Clear view
    */
    void Reset( void ) ;

  signals:

    void hasTriggeredRowDelete( const int row ) ;

  public slots:


    /**
    * @brief Action to be executed when user want to delete a row
    * @param row to be deleted
    */
    void onWantToDeleteRow( const int row ) ;


  private:

    /**
    * @brief Build interface
    */
    void BuildInterface( void ) ;

    /**
    * @brief Make connections betweens widgets elements
    */
    void MakeConnections( void ) ;

    /// List of all images
    QTableWidget * m_tbl_widget ;


    /// Btn used to add a single image to the list
    QPushButton * m_btn_add_image ;

    /// Btn used to add all image in a folder
    QPushButton * m_btn_add_folder ;
} ;

} // namespace SfMGui
} // namespace openMVG

#endif