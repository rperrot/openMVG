#ifndef _OPENMVG_SFMGUI_IMAGETAB_HPP_
#define _OPENMVG_SFMGUI_IMAGETAB_HPP_

#include <QWidget>
#include <QPushButton>
#include <QTableView>

namespace openMVG
{
namespace SfMGui
{

/**
* @brief Class managing adding/removing image list
*/
class ImageTab : public QWidget
{
  public:

    /**
    * @brief Constructor
    * @param parent Parent
    */
    ImageTab( QWidget * parent = nullptr ) ;

  private:

    /**
    * @brief Build interface
    */
    void BuildInterface( void ) ;

    /// List of all images
    QTableView * m_tbl_view ;

    /// Btn used to add a single image to the list
    QPushButton * m_btn_add_image ;

    /// Btn used to add all image in a folder
    QPushButton * m_btn_add_folder ;
} ;

} // namespace SfMGui
} // namespace openMVG

#endif