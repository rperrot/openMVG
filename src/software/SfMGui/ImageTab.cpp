#include "software/SfMGui/ImageTab.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

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
}


/**
* @brief Build interface
*/
void ImageTab::BuildInterface( void )
{
  QHBoxLayout * mainLayout = new QHBoxLayout ;
  QVBoxLayout * btnLayout = new QVBoxLayout ;

  m_tbl_view = new QTableView( this ) ;
  m_btn_add_image = new QPushButton( "Add image" ) ;
  m_btn_add_folder = new QPushButton( "Add folder" ) ;

  btnLayout->addWidget( m_btn_add_image ) ;
  btnLayout->addWidget( m_btn_add_folder ) ;
  btnLayout->addStretch( ) ;

  mainLayout->addWidget( m_tbl_view ) ;
  mainLayout->addLayout( btnLayout ) ;

  setLayout( mainLayout ) ;
}

} // namespace SfMGui
} // namespace openMVG