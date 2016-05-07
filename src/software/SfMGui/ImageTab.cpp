#include "software/SfMGui/ImageTab.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCoreApplication>

#include <iostream>
#include <string>

namespace openMVG
{
namespace SfMGui
{


/**
* @brief Constructor
* @param parent Parent
*/
ImageTab::ImageTab( QWidget * parent )
  : QWidget( parent )
{
  BuildInterface() ;
  MakeConnections() ;
}


/**
* @brief Build interface
*/
void ImageTab::BuildInterface( void )
{
  QHBoxLayout * mainLayout = new QHBoxLayout ;
  //  QVBoxLayout * btnLayout = new QVBoxLayout ;

  m_tbl_widget = new QTableWidget( this ) ;
  // image | width | height | focal
  m_tbl_widget->setColumnCount( 5 ) ;
  m_tbl_widget->setIconSize( QSize( 64 , 64 ) ) ;
  QStringList tableHeader ;
  tableHeader << "Image" << "Name" << "Width" << "Height" << "Focal" ; // << "Delete" ;
  m_tbl_widget->setHorizontalHeaderLabels( tableHeader ) ;

  mainLayout->addWidget( m_tbl_widget ) ;

  setLayout( mainLayout ) ;
}

/**
* @brief Make connections betweens widgets elements
*/
void ImageTab::MakeConnections( void )
{

}

/**
* @brief Add a new row to the view
* @param imagePath Path of the image to be displayed in the table
* @param imageName Name of the image to be displayed
* @param image_width Width (in pixel) of the input image
* @param image_height Height (in pixel) of the input image
* @param focal Focal length of the input image
* @note imagePath is idealy a path to the thumbnail
*/
void ImageTab::AddRow( const std::string & imagePath , const std::string & imageName , const int image_width , const int image_height , const float focal )
{
  const int cur_nb_row = m_tbl_widget->rowCount( );
  m_tbl_widget->setRowCount( cur_nb_row + 1 ) ;

  // Image thumbnail
  QTableWidgetItem * itemImageThumb = new QTableWidgetItem;
  itemImageThumb->setIcon( QIcon( imagePath.c_str() ) ) ;
  itemImageThumb->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled ) ;
  itemImageThumb->setTextAlignment( Qt::AlignCenter ) ;


  // Image Name
  QTableWidgetItem * itemImageName = new QTableWidgetItem ;
  itemImageName->setText( imageName.c_str() ) ;
  itemImageName->setFlags( Qt::ItemIsEnabled ) ;
  itemImageName->setTextAlignment( Qt::AlignCenter ) ;

  // Image Width
  QTableWidgetItem * itemImageWidth = new QTableWidgetItem ;
  itemImageWidth->setText( std::to_string( image_width ).c_str() ) ;
  itemImageWidth->setFlags( Qt::ItemIsEnabled ) ;
  itemImageWidth->setTextAlignment( Qt::AlignCenter ) ;

  // Image height
  QTableWidgetItem * itemImageHeight = new QTableWidgetItem ;
  itemImageHeight->setText( std::to_string( image_height ).c_str() ) ;
  itemImageHeight->setFlags( Qt::ItemIsEnabled ) ;
  itemImageHeight->setTextAlignment( Qt::AlignCenter ) ;

  // Image focal
  QTableWidgetItem * itemImageFocal = new QTableWidgetItem ;
  itemImageFocal->setText( std::to_string( focal ).c_str() ) ;
  itemImageFocal->setFlags( Qt::ItemIsEnabled ) ;
  itemImageFocal->setTextAlignment( Qt::AlignCenter ) ;

  // Image delete icon
  /*
  QTableWidgetItem * itemImageDelete = new QTableWidgetItem ;
  const std::string exe_path = QCoreApplication::applicationDirPath().toStdString() ;
  itemImageDelete->setIcon( QIcon( ( exe_path + "/resources/circle-x.svg" ).c_str()  ) )  ;
  itemImageDelete->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled ) ;
  itemImageDelete->setTextAlignment( Qt::AlignCenter ) ;
  */

  // Add all items to the view
  m_tbl_widget->setItem( cur_nb_row , 0 , itemImageThumb ) ;
  m_tbl_widget->setItem( cur_nb_row , 1 , itemImageName ) ;
  m_tbl_widget->setItem( cur_nb_row , 2 , itemImageWidth ) ;
  m_tbl_widget->setItem( cur_nb_row , 3 , itemImageHeight ) ;
  m_tbl_widget->setItem( cur_nb_row , 4 , itemImageFocal ) ;
  // m_tbl_widget->setItem( cur_nb_row , 5 , itemImageDelete ) ;
}

/**
 * @brief Clear view
 */
void ImageTab::Reset( void )
{
  m_tbl_widget->clear() ;

  m_tbl_widget->setColumnCount( 5 ) ;
  QStringList tableHeader ;
  tableHeader << "Image" << "Name" << "Width" << "Height" << "Focal" ; // << "Delete" ;
  m_tbl_widget->setHorizontalHeaderLabels( tableHeader ) ;
}



/**
* @brief Action to be executed when user want to delete a row
* @param row to be deleted
*/
void ImageTab::onWantToDeleteRow( const int row )
{
#ifdef SFMGUI_DEBUG_INTERFACE
  std::cerr << __func__ << std::endl ;
#endif
  // Re-emit signal to the main window
  emit hasTriggeredRowDelete( row ) ;
}



} // namespace SfMGui
} // namespace openMVG