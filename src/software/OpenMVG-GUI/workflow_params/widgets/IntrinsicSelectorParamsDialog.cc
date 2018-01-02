#include "IntrinsicSelectorParamsDialog.hh"

#include "Project.hh"


#include "openMVG/cameras/cameras.hpp"
#include "openMVG/sfm/sfm_data.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QList>
#include <QVBoxLayout>


namespace openMVG_gui
{

IntrinsicSelectorParamsDialog::IntrinsicSelectorParamsDialog( QWidget * parent , std::shared_ptr<Project> proj )
  : QDialog( parent ) ,
    m_project( proj )
{
  buildInterface() ;
  makeConnections() ;

  populateLists( proj ) ;

  setWindowTitle( "Intrinsic params" ) ;
  adjustSize();

  resize( 640 , height() ) ;
}

/**
* @brief action to be executed when user click on cancel button
*/
void IntrinsicSelectorParamsDialog::onCancel( void )
{
  done( QDialog::Rejected ) ;
}

/**
* @brief action to be executed when user click on ok button
*/
void IntrinsicSelectorParamsDialog::onOk( void )
{
  // TODO : check validity of the views
  done( QDialog::Accepted ) ;
}

/**
* @brief Action to be executed when user click on reset button
*/
void IntrinsicSelectorParamsDialog::onReset( void )
{
  populateLists( m_project ) ;
}

void IntrinsicSelectorParamsDialog::populateLists( std::shared_ptr<Project> proj )
{
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = proj->SfMData() ;

  m_intrinsic_model->clear() ;
  m_intrinsic_model->setHorizontalHeaderItem( 0 , new QStandardItem( "ID" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 1 , new QStandardItem( "Type" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 2 , new QStandardItem( "Width" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 3 , new QStandardItem( "Height" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 4 , new QStandardItem( "Focal" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 5 , new QStandardItem( "Princ. Point X" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 6 , new QStandardItem( "Princ. Point Y" ) ) ;
  m_intrinsic_view->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

  m_views_model->clear() ;
  m_views_model->setHorizontalHeaderItem( 0 , new QStandardItem( "ID" ) ) ;
  m_views_model->setHorizontalHeaderItem( 1 , new QStandardItem( "Name" ) ) ;
  m_views_model->setHorizontalHeaderItem( 2 , new QStandardItem( "Intrinsic ID" ) ) ;
  m_views_view->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

  const int nb_intrinsic = sfm_data->GetIntrinsics().size() ;
  const int nb_view = sfm_data->GetViews().size() ;

  std::vector< openMVG::IndexT > intrinsics_ids ;
  for( auto it = sfm_data->GetIntrinsics().begin() ; it != sfm_data->GetIntrinsics().end() ; ++it )
  {
    intrinsics_ids.emplace_back( it->first ) ;
  }

  // Insert intrinsics
  for( int id_intrinsic = 0 ; id_intrinsic < nb_intrinsic ; ++id_intrinsic )
  {
    const auto real_id = intrinsics_ids[ id_intrinsic ] ;
    auto cur_intrin = sfm_data->GetIntrinsics().at( real_id ) ;

    QList<QStandardItem*> rowItems ;

    // 0 - ID
    rowItems.append( new QStandardItem( QString( "%1" ).arg( real_id ) ) ) ;

    // 1 - Type
    if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Radial_K1>( cur_intrin ) != nullptr )
    {
      rowItems.append( new QStandardItem( "Radial K1" ) ) ;
    }
    else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Radial_K3>( cur_intrin ) != nullptr )
    {
      rowItems.append( new QStandardItem( "Radial K3" ) ) ;
    }
    else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Brown_T2>( cur_intrin ) != nullptr )
    {
      rowItems.append( new QStandardItem( "Brown T2" ) ) ;
    }
    else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Fisheye>( cur_intrin ) != nullptr )
    {
      rowItems.append( new QStandardItem( "Fisheye" ) ) ;
    }
    else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic>( cur_intrin ) != nullptr )
    {
      rowItems.append( new QStandardItem( "Pinhole" ) ) ;
    }
    else if( std::dynamic_pointer_cast<openMVG::cameras::Intrinsic_Spherical>( cur_intrin ) != nullptr )
    {
      rowItems.append( new QStandardItem( "Spherical" ) ) ;
    }

    // 2 - Width
    rowItems.append( new QStandardItem( QString( "%1" ).arg( cur_intrin->w() ) ) ) ;
    // 3 - Height
    rowItems.append( new QStandardItem( QString( "%1" ).arg( cur_intrin->h() ) ) ) ;


    // 4, 5, 6 : Focal , PP-X , PP-Y
    if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic>( cur_intrin ) )
    {
      std::shared_ptr<openMVG::cameras::Pinhole_Intrinsic> pin_intrin =  std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic>( cur_intrin ) ;

      const double focal = pin_intrin->K()( 0 , 0 ) ;
      const double ppx = pin_intrin->K()( 0 , 2 ) ;
      const double ppy = pin_intrin->K()( 1 , 2 ) ;

      rowItems.append( new QStandardItem( QString( "%1" ).arg( focal ) ) ) ;
      rowItems.append( new QStandardItem( QString( "%1" ).arg( ppx ) ) ) ;
      rowItems.append( new QStandardItem( QString( "%1" ).arg( ppy ) ) ) ;
    }
    else
    {
      rowItems.append( new QStandardItem( "-" ) ) ;
      rowItems.append( new QStandardItem( "-" ) ) ;
      rowItems.append( new QStandardItem( "-" ) ) ;
    }


    m_intrinsic_model->appendRow( rowItems ) ;
  }

  // Insert view elements
  std::vector< openMVG::IndexT > views_ids ;
  for( auto it : sfm_data->GetViews() )
  {
    views_ids.emplace_back( it.first ) ;
  }

  for( int id_view = 0 ; id_view < nb_view ; ++id_view )
  {
    const auto real_view_id = views_ids[ id_view ] ;

    auto cur_view = sfm_data->GetViews().at( real_view_id ) ;

    const auto intrinsic_id = cur_view->id_intrinsic ;
    const std::string path = cur_view->s_Img_path ;

    QList<QStandardItem*> rowItems ;

    // ID
    rowItems.append( new QStandardItem( QString( "%1" ).arg( real_view_id ) ) ) ;
    // Name
    rowItems.append( new QStandardItem( path.c_str() ) ) ;
    // Intrinsic ID
    if( intrinsic_id != openMVG::UndefinedIndexT )
    {
      rowItems.append( new QStandardItem( QString( "%1" ).arg( intrinsic_id ) ) ) ;
    }
    else
    {
      rowItems.append( new QStandardItem( QString( "Undefined" ) ) ) ;
    }

    m_views_model->appendRow( rowItems ) ;
  }
}


void IntrinsicSelectorParamsDialog::buildInterface( void )
{
  // Intrinsics
  QGroupBox * intrinsicsBox = new QGroupBox( "Intrinsics" ) ;
  QVBoxLayout * intrinsicBoxLayout = new QVBoxLayout ;

  m_intrinsic_view = new QTableView ;
  m_intrinsic_model = new QStandardItemModel ;
  m_intrinsic_view->setModel( m_intrinsic_model ) ;

  intrinsicBoxLayout->addWidget( m_intrinsic_view ) ;
  intrinsicsBox->setLayout( intrinsicBoxLayout ) ;

  // Views
  QGroupBox * cameraBox = new QGroupBox( "Views" ) ;
  QVBoxLayout * cameraBoxLayout = new QVBoxLayout ;

  m_views_view = new QTableView ;
  m_views_model = new QStandardItemModel ;
  m_views_view->setModel( m_views_model ) ;

  cameraBoxLayout->addWidget( m_views_view ) ;
  cameraBox->setLayout( cameraBoxLayout ) ;

  // Btns
  QHBoxLayout * btnLayout = new QHBoxLayout ;

  m_ok_btn = new QPushButton( "Ok" ) ;
  m_cancel_btn = new QPushButton( "Cancel" ) ;
  m_reset_btn = new QPushButton( "Reset" ) ;

  m_ok_btn->setDefault( true ) ;
  m_cancel_btn->setDefault( false ) ;
  m_reset_btn->setDefault( false ) ;

  btnLayout->addStretch() ;
  btnLayout->addWidget( m_reset_btn ) ;
  btnLayout->addWidget( m_cancel_btn ) ;
  btnLayout->addWidget( m_ok_btn ) ;

  // Main layout
  QVBoxLayout * mainLayout = new QVBoxLayout ;

  mainLayout->addWidget( intrinsicsBox ) ;
  mainLayout->addWidget( cameraBox ) ;
  mainLayout->addLayout( btnLayout ) ;

  setLayout( mainLayout ) ;
  adjustSize();
  //  mainLayout->setSizeConstraint( QLayout::SetFixedSize );
}

void IntrinsicSelectorParamsDialog::makeConnections( void )
{
  connect( m_cancel_btn , SIGNAL( clicked() ) , this , SLOT( onCancel() ) );
  connect( m_reset_btn , SIGNAL( clicked() ) , this , SLOT( onReset() ) ) ;
  connect( m_ok_btn , SIGNAL( clicked() ) , this , SLOT( onOk() ) );
}


} // namespace openMVG_gui