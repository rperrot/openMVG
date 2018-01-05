// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "IntrinsicSelectorParamsDialog.hh"

#include "IntrinsicEditorDialog.hh"
#include "Project.hh"


#include "openMVG/cameras/cameras.hpp"
#include "openMVG/sfm/sfm_data.hpp"

#include <QAction>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>

#include <sstream>

namespace openMVG_gui
{

/**
 * @brief Constructor
 * @param parent widget
 * @param proj project in which intrinsics are found (then edited)
 */
IntrinsicSelectorParamsDialog::IntrinsicSelectorParamsDialog( QWidget * parent , std::shared_ptr<Project> proj )
  : QDialog( parent ) ,
    m_project( proj )
{
  buildInterface() ;
  makeConnections() ;

  populateLists( proj ) ;
  updateTableViews() ;

  setWindowTitle( "Intrinsic params" ) ;
  adjustSize();

  resize( 640 , height() ) ;
}

/**
 * @brief Get list of intrinsics
 */
std::map < openMVG::IndexT , std::shared_ptr< openMVG::cameras::IntrinsicBase> > IntrinsicSelectorParamsDialog::intrinsics( void ) const
{
  return m_intrinsics ;
}

/**
 * @brief Get list of intrinsic_id for all cameras
 */
std::map< openMVG::IndexT , openMVG::IndexT > IntrinsicSelectorParamsDialog::indexes( void ) const
{
  return m_map_view_intrinsic ;
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
  updateTableViews() ;
}

/**
 * @brief Action to be executed when user right click on the intrinsic view
 */
void IntrinsicSelectorParamsDialog::onRightClickIntrinsics( const QPoint &pos )
{
  QModelIndex index = m_intrinsic_view->indexAt( pos ) ;
  if( index.isValid() )
  {
    QMenu * rClickIntrinsicMenu = new QMenu( this ) ;

    QAction * deleteCurrentIntrinsic = rClickIntrinsicMenu->addAction( "Delete" ) ;
    QAction * editCurrentIntrinsic = rClickIntrinsicMenu->addAction( "Edit" ) ;
    rClickIntrinsicMenu->addSeparator() ;
    QAction * applyToCompatibleView = rClickIntrinsicMenu->addAction( "Assign to all compatible views" ) ;
    QAction * applyToUndefinedCompatibleView = rClickIntrinsicMenu->addAction( "Assign to all undefined compatible views" ) ;

    // Get id
    QStandardItem * itemAtPosition = ( ( QStandardItemModel* ) m_intrinsic_view->model() ) ->item( index.row() , 0 ) ;
    const openMVG::IndexT intrinsic_id = itemAtPosition->text().toInt() ;

    // Make connections
    connect( deleteCurrentIntrinsic , &QAction::triggered , this , [this, intrinsic_id] {  onDeleteIntrinsic( intrinsic_id ) ; } ) ;
    connect( editCurrentIntrinsic , &QAction::triggered , this , [this, intrinsic_id] { onEditIntrinsic( intrinsic_id ) ; } ) ;
    connect( applyToCompatibleView , &QAction::triggered , this , [this, intrinsic_id] { onAssignIntrinsicToCompatibleViews( intrinsic_id ) ; } ) ;
    connect( applyToUndefinedCompatibleView , &QAction::triggered , this , [this, intrinsic_id] { onAssignIntrinsicToCompatibleUndefinedViews( intrinsic_id ) ; } ) ;

    rClickIntrinsicMenu->exec( m_intrinsic_view->mapToGlobal( pos ) ) ;
  }
}

void IntrinsicSelectorParamsDialog::onDeleteIntrinsic( const openMVG::IndexT intrinsic_id  )
{
  // Really ?
  QMessageBox::StandardButton btn = QMessageBox::question( this , "Confirm" , "Do you really want to delete this intrinsic ? \n (camera using this intrinsic will have undefined intrinsics)" ) ;

  if( btn == QMessageBox::Yes )
  {
    // Delete references to this intrinsic in the views
    for( auto & it : m_map_view_intrinsic )
    {
      if( it.second == intrinsic_id )
      {
        it.second = openMVG::UndefinedIndexT ;
      }
    }

    // Delete the intrinsic
    m_intrinsics.erase( intrinsic_id ) ;

    updateTableViews() ;
  }
}

void IntrinsicSelectorParamsDialog::onEditIntrinsic( const openMVG::IndexT intrinsic_id  )
{
  IntrinsicEditorDialog dlg( this , m_intrinsics.at( intrinsic_id ) );

  int res = dlg.exec() ;
  if( res == QDialog::Accepted )
  {
    m_intrinsics.at( intrinsic_id ) = dlg.intrinsic() ;
    updateTableViews() ;
  }
}

void IntrinsicSelectorParamsDialog::onAssignIntrinsicToCompatibleViews( const openMVG::IndexT intrinsic_id )
{
  const int intrin_w = m_intrinsics.at( intrinsic_id )->w() ;
  const int intrin_h = m_intrinsics.at( intrinsic_id )->h() ;

  for( auto it : m_project->SfMData()->GetViews() )
  {
    const int w = it.second->ui_width ;
    const int h = it.second->ui_height ;

    if( intrin_w == w && intrin_h == h )
    {
      m_map_view_intrinsic[ it.first ] = intrinsic_id ;
    }
  }

  updateTableViews() ;
}

void IntrinsicSelectorParamsDialog::onAssignIntrinsicToCompatibleUndefinedViews( const openMVG::IndexT intrinsic_id )
{
  const int intrin_w = m_intrinsics.at( intrinsic_id )->w() ;
  const int intrin_h = m_intrinsics.at( intrinsic_id )->h() ;

  for( auto it : m_project->SfMData()->GetViews() )
  {
    const int w = it.second->ui_width ;
    const int h = it.second->ui_height ;

    if( ( intrin_w == w ) &&
        ( intrin_h == h ) &&
        ( m_map_view_intrinsic.at( it.first ) == openMVG::UndefinedIndexT ) )
    {
      m_map_view_intrinsic[ it.first ] = intrinsic_id ;
    }
  }

  updateTableViews() ;
}


/**
 * @brief Action to be executed when user right click on the views view
 */
void IntrinsicSelectorParamsDialog::onRightClickViews( const QPoint &pos )
{
  QModelIndex index = m_views_view->indexAt( pos ) ;
  if( index.isValid() )
  {
    QMenu * rClickViewMenu = new QMenu ;

    QAction * createIndividualIntrinsic = rClickViewMenu->addAction( "Create intrinsic for view" ) ;
    QAction * removeIntrinsicReference = rClickViewMenu->addAction( "Delete intrinsic reference" ) ;
    QMenu * setIntrinsic = rClickViewMenu->addMenu( "Set intrinsic" ) ;

    std::vector< openMVG::IndexT > intrinsics_ids ;
    for( auto it : m_intrinsics )
    {
      intrinsics_ids.emplace_back( it.first ) ;
    }

    // Get id of the view
    QStandardItem * itemAtPosition = ( ( QStandardItemModel* ) m_views_view->model() ) ->item( index.row() , 0 ) ;
    const openMVG::IndexT view_id = itemAtPosition->text().toInt() ;

    for( auto intrin_id : intrinsics_ids )
    {
      std::stringstream str ;
      str << intrin_id ;
      QAction * intrinact = setIntrinsic->addAction( str.str().c_str() ) ;

      connect( intrinact , &QAction::triggered , this , [this, view_id, intrin_id] { onAssignIntrinsicForView( view_id , intrin_id ) ; } ) ;
    }

    // Make connections
    connect( createIndividualIntrinsic , &QAction::triggered , this , [this, view_id] { onCreateIntrinsicForView( view_id ) ; } ) ;
    connect( removeIntrinsicReference , &QAction::triggered , this , [this, view_id ] { onDeleteIntrinsicForView( view_id ) ; } ) ;


    rClickViewMenu->exec( m_views_view->mapToGlobal( pos ) ) ;

  }
}

/**
 * @brief Create intrinsic for a given view
 * @param view_id Id of the view
 */
void IntrinsicSelectorParamsDialog::onCreateIntrinsicForView( const openMVG::IndexT view_id )
{
  // Look of a valid id
  std::vector< openMVG::IndexT > ids ;
  for( auto it : m_intrinsics )
  {
    ids.emplace_back( it.first ) ;
  }
  std::sort( ids.begin() , ids.end() ) ;
  openMVG::IndexT valid_id = 0 ;
  for( auto id : ids )
  {
    if( valid_id != id )
    {
      break ;
    }
    ++valid_id ;
  }

  const auto view = m_project->SfMData()->GetViews().at( view_id ) ;
  const int w = view->ui_width ;
  const int h = view->ui_height ;

  const double focal = 1.2 * std::max( w , h ) ;
  const double ppx = w / 2.0 ;
  const double ppy = h / 2.0 ;

  m_intrinsics.insert( { valid_id , std::make_shared<openMVG::cameras::Pinhole_Intrinsic_Radial_K3>( w , h , focal , ppx , ppy ) } ) ;
  m_map_view_intrinsic.at( view_id ) = valid_id ;
  updateTableViews() ;
}

/**
 * @brief Delete intrinsic (reference only) for a given view
 * @param view_id If of the view to modify
 * @note Removal set intrinsic id of the view to undefined
 */
void IntrinsicSelectorParamsDialog::onDeleteIntrinsicForView( const openMVG::IndexT view_id )
{
  m_map_view_intrinsic.at( view_id ) = openMVG::UndefinedIndexT ;
  updateTableViews() ;
}

/**
 * @brief Assign intrinsic for the given view
 * @param view_id Id of the view to modify
 * @param intrinsic_id Id of the Intrinsic to set to the given view
 */
void IntrinsicSelectorParamsDialog::onAssignIntrinsicForView( const openMVG::IndexT view_id , const openMVG::IndexT intrinsic_id )
{
  m_map_view_intrinsic.at( view_id ) = intrinsic_id ;
  updateTableViews() ;
}

/**
 * @brief action to be executed when selection changed on the intrinsic table view
 */
void IntrinsicSelectorParamsDialog::onHasChangedIntrinsicSeletion( void )
{
  // Enable/Disable buttons depending on selection
  QItemSelectionModel *selected_views = m_views_view->selectionModel() ;
  QItemSelectionModel *selected_intrinsics = m_intrinsic_view->selectionModel() ;

  const int nb_intrin_selected = selected_intrinsics->selectedRows().size() ;
  const int nb_views_selected = selected_views->selectedRows().size() ;

  const bool views_btn_enabled = selected_views->hasSelection() ;
  const bool intrinsics_btn_enabled = selected_intrinsics->hasSelection() ;

  m_delete_current_intrinsic_btn->setEnabled( intrinsics_btn_enabled ) ;
  if( nb_intrin_selected == 1 )
  {
    m_edit_current_intrinsic_btn->setEnabled( intrinsics_btn_enabled ) ;
    m_assign_current_intrinsic_to_compatible_views_btn->setEnabled( intrinsics_btn_enabled ) ;
    m_assign_current_intrinsic_to_undefined_views_btn->setEnabled( intrinsics_btn_enabled ) ;
  }
  else
  {
    m_edit_current_intrinsic_btn->setEnabled( false ) ;
    m_assign_current_intrinsic_to_compatible_views_btn->setEnabled( false ) ;
    m_assign_current_intrinsic_to_undefined_views_btn->setEnabled( false ) ;
  }

  if( ! intrinsics_btn_enabled )
  {
    m_set_intrinsic_for_view_btn->setEnabled( false ) ;
  }
  else if( views_btn_enabled )
  {
    if( nb_intrin_selected == 1 )
    {
      m_set_intrinsic_for_view_btn->setEnabled( true ) ;
    }
    else
    {
      m_set_intrinsic_for_view_btn->setEnabled( false ) ;
    }
  }
}

/**
 * @brief action to be executed when selection changed on the views table view
 */
void IntrinsicSelectorParamsDialog::onHasChangedViewsSelection( void )
{
  // Enable/Disable buttons depending on selection
  QItemSelectionModel *selected_views = m_views_view->selectionModel() ;
  QItemSelectionModel *selected_intrinsics = m_intrinsic_view->selectionModel() ;

  const bool views_btn_enabled = selected_views->hasSelection() ;
  const bool intrinsics_btn_enabled = selected_intrinsics->hasSelection() ;

  m_create_for_view_btn->setEnabled( views_btn_enabled ) ;
  m_delete_reference_for_view_btn->setEnabled( views_btn_enabled ) ;

  if( intrinsics_btn_enabled )
  {
    m_set_intrinsic_for_view_btn->setEnabled( views_btn_enabled ) ;
  }
  else
  {
    m_set_intrinsic_for_view_btn->setEnabled( false ) ;
  }

}

/**
 * @brief Action to be executed when user click the intrinsic/delete
 */
void IntrinsicSelectorParamsDialog::onHasClickedIntrinsicDelete( void )
{
  QItemSelectionModel *selected_intrinsics = m_intrinsic_view->selectionModel() ;
  QList<QModelIndex> selected_rows = selected_intrinsics->selectedRows() ;

  for( int i = 0 ; i < selected_rows.size() ; ++i )
  {
    // Get ID of the intrinsic
    QStandardItem * itemAtPosition = ( ( QStandardItemModel* ) m_intrinsic_view->model() ) ->item( selected_rows[ i ].row() , 0 ) ;
    openMVG::IndexT intrinsic_id = itemAtPosition->text().toInt() ;

    // Delete it !
    onDeleteIntrinsic( intrinsic_id ) ;
  }
}

/**
 * @brief Action to be executed when user click the intrinsic/edit
 */
void IntrinsicSelectorParamsDialog::onHasClickedIntrinsicEdit( void )
{
  QItemSelectionModel *selected_intrinsics = m_intrinsic_view->selectionModel() ;
  QList<QModelIndex> selected_rows = selected_intrinsics->selectedRows() ;

  QStandardItem * itemAtPosition = ( ( QStandardItemModel* ) m_intrinsic_view->model() ) ->item( selected_rows[ 0 ].row() , 0 ) ;
  openMVG::IndexT intrinsic_id = itemAtPosition->text().toInt() ;

  onEditIntrinsic( intrinsic_id ) ;
}

/**
 * @brief Action to be executed when user click the intrinsic/assign to compatible
 */
void IntrinsicSelectorParamsDialog::onHasClickedIntrinsicAssignToCompatible( void )
{
  QItemSelectionModel *selected_intrinsics = m_intrinsic_view->selectionModel() ;
  QList<QModelIndex> selected_rows = selected_intrinsics->selectedRows() ;

  QStandardItem * itemAtPosition = ( ( QStandardItemModel* ) m_intrinsic_view->model() ) ->item( selected_rows[ 0 ].row() , 0 ) ;
  openMVG::IndexT intrinsic_id = itemAtPosition->text().toInt() ;

  onAssignIntrinsicToCompatibleViews( intrinsic_id ) ;
}

/**
 * @brief Action to be executed when user click the intrinsic/assign to undefined
 */
void IntrinsicSelectorParamsDialog::onHasClickedIntrinsicAssignToUndefined( void )
{
  QItemSelectionModel *selected_intrinsics = m_intrinsic_view->selectionModel() ;
  QList<QModelIndex> selected_rows = selected_intrinsics->selectedRows() ;

  QStandardItem * itemAtPosition = ( ( QStandardItemModel* ) m_intrinsic_view->model() ) ->item( selected_rows[ 0 ].row() , 0 ) ;
  openMVG::IndexT intrinsic_id = itemAtPosition->text().toInt() ;

  onAssignIntrinsicToCompatibleUndefinedViews( intrinsic_id ) ;
}

/**
 * @brief Action to be executed when user click the views/create
 */
void IntrinsicSelectorParamsDialog::onHasClickedViewsCreate( void )
{
  QItemSelectionModel *selected_views = m_views_view->selectionModel() ;
  QList<QModelIndex> selected_rows = selected_views->selectedRows() ;

  for( int i = 0 ; i < selected_rows.size() ; ++i )
  {
    QStandardItem * itemAtPosition = ( ( QStandardItemModel* ) m_views_view->model() ) ->item( selected_rows[ i ].row() , 0 ) ;
    openMVG::IndexT view_id = itemAtPosition->text().toInt() ;

    onCreateIntrinsicForView( view_id ) ;
  }
}

/**
 * @brief Action to be executed when user click the view/delete
 */
void IntrinsicSelectorParamsDialog::onHasClickedViewsDelete( void )
{
  QItemSelectionModel *selected_views = m_views_view->selectionModel() ;
  QList<QModelIndex> selected_rows = selected_views->selectedRows() ;

  for( int i = 0 ; i < selected_rows.size() ; ++i )
  {
    QStandardItem * itemAtPosition = ( ( QStandardItemModel* ) m_views_view->model() ) ->item( selected_rows[ i ].row() , 0 ) ;
    openMVG::IndexT view_id = itemAtPosition->text().toInt() ;

    onDeleteIntrinsicForView( view_id ) ;
  }
}

/**
 * @brief Action to be executed when user click the view/assign
 */
void IntrinsicSelectorParamsDialog::onHasClickedViewsAssign( void )
{
  QItemSelectionModel *selected_views = m_views_view->selectionModel() ;
  QList<QModelIndex> selected_views_rows = selected_views->selectedRows() ;

  QItemSelectionModel *selected_intrinsics = m_intrinsic_view->selectionModel() ;
  QList<QModelIndex> selected_intrinsics_rows = selected_intrinsics->selectedRows() ;

  QStandardItem * itemAtPosition = ( ( QStandardItemModel* ) m_intrinsic_view->model() ) ->item( selected_intrinsics_rows[ 0 ].row() , 0 ) ;
  openMVG::IndexT intrinsic_id = itemAtPosition->text().toInt() ;

  for( int i = 0 ; i < selected_views_rows.size() ; ++i )
  {
    QStandardItem * itemAtPosition = ( ( QStandardItemModel* ) m_views_view->model() ) ->item( selected_views_rows[ i ].row() , 0 ) ;
    openMVG::IndexT view_id = itemAtPosition->text().toInt() ;

    onAssignIntrinsicForView( view_id , intrinsic_id ) ;
  }
}


/**
 * @brief Given a project, fill all values inside internal data
 */
void IntrinsicSelectorParamsDialog::populateLists( std::shared_ptr<Project> proj )
{
  const std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = proj->SfMData() ;

  m_intrinsics.clear() ;
  m_map_view_intrinsic.clear() ;

  // Fill intrinsics
  for( auto it : sfm_data->GetIntrinsics() )
  {
    m_intrinsics.insert( { it.first , std::shared_ptr<openMVG::cameras::IntrinsicBase>( it.second->clone() ) } ) ;
  }

  // Fill views
  for( auto it : sfm_data->GetViews() )
  {
    m_map_view_intrinsic.insert( { it.first , it.second->id_intrinsic } ) ;
  }
}

/**
 * @brief update view according to the data
 */
void IntrinsicSelectorParamsDialog::updateTableViews( void )
{
  const std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData() ;

  // Clear data and reform ids
  m_intrinsic_model->clear() ;
  m_intrinsic_model->setHorizontalHeaderItem( 0 , new QStandardItem( "Intrinsic ID" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 1 , new QStandardItem( "Type" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 2 , new QStandardItem( "Width" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 3 , new QStandardItem( "Height" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 4 , new QStandardItem( "Focal" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 5 , new QStandardItem( "Princ. Point X" ) ) ;
  m_intrinsic_model->setHorizontalHeaderItem( 6 , new QStandardItem( "Princ. Point Y" ) ) ;
  m_intrinsic_view->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

  m_views_model->clear() ;
  m_views_model->setHorizontalHeaderItem( 0 , new QStandardItem( "View ID" ) ) ;
  m_views_model->setHorizontalHeaderItem( 1 , new QStandardItem( "Name" ) ) ;
  m_views_model->setHorizontalHeaderItem( 2 , new QStandardItem( "Width" ) ) ;
  m_views_model->setHorizontalHeaderItem( 3 , new QStandardItem( "Height" ) ) ;
  m_views_model->setHorizontalHeaderItem( 4 , new QStandardItem( "Intrinsic ID" ) ) ;
  m_views_view->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

  const int nb_intrinsic = m_intrinsics.size() ;
  const int nb_view = sfm_data->GetViews().size() ;

  // Insert intrinsic elements
  std::vector< openMVG::IndexT > intrinsics_ids ;
  for( auto it = m_intrinsics.begin() ; it != m_intrinsics.end() ; ++it )
  {
    intrinsics_ids.emplace_back( it->first ) ;
  }

  // Insert intrinsics
  for( int id_intrinsic = 0 ; id_intrinsic < nb_intrinsic ; ++id_intrinsic )
  {
    const auto real_id = intrinsics_ids[ id_intrinsic ] ;
    auto cur_intrin = m_intrinsics.at( real_id ) ;

    QList<QStandardItem*> rowItems ;

    // 0 - ID
    rowItems.append( new QStandardItem( QString( "%1" ).arg( real_id ) ) ) ;

    // 1 - Type
    if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Radial_K1>( cur_intrin ) )
    {
      rowItems.append( new QStandardItem( "Radial K1" ) ) ;
    }
    else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Radial_K3>( cur_intrin ) )
    {
      rowItems.append( new QStandardItem( "Radial K3" ) ) ;
    }
    else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Brown_T2>( cur_intrin ) )
    {
      rowItems.append( new QStandardItem( "Brown T2" ) ) ;
    }
    else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic_Fisheye>( cur_intrin ) )
    {
      rowItems.append( new QStandardItem( "Fisheye" ) ) ;
    }
    else if( std::dynamic_pointer_cast<openMVG::cameras::Pinhole_Intrinsic>( cur_intrin ) )
    {
      rowItems.append( new QStandardItem( "Pinhole" ) ) ;
    }
    else if( std::dynamic_pointer_cast<openMVG::cameras::Intrinsic_Spherical>( cur_intrin ) )
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

    const auto intrinsic_id = m_map_view_intrinsic[ real_view_id ] ;
    const std::string path = cur_view->s_Img_path ;
    const int w = cur_view->ui_width ;
    const int h = cur_view->ui_height ;

    QList<QStandardItem*> rowItems ;

    // ID
    rowItems.append( new QStandardItem( QString( "%1" ).arg( real_view_id ) ) ) ;
    // Name
    rowItems.append( new QStandardItem( path.c_str() ) ) ;
    // Width
    rowItems.append( new QStandardItem( QString( "%1" ).arg( w ) ) ) ;
    // Height
    rowItems.append( new QStandardItem( QString( "%1" ).arg( h ) ) ) ;
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

  // Enable/Disable buttons depending on selection
  QItemSelectionModel *selected_intrinsics = m_intrinsic_view->selectionModel() ;
  QItemSelectionModel *selected_views = m_views_view->selectionModel() ;

  const bool intrinsics_btn_enabled = selected_intrinsics->hasSelection() ;
  const bool views_btn_enabled = selected_views->hasSelection() ;

  m_delete_current_intrinsic_btn->setEnabled( intrinsics_btn_enabled ) ;
  m_edit_current_intrinsic_btn->setEnabled( intrinsics_btn_enabled ) ;
  m_assign_current_intrinsic_to_compatible_views_btn->setEnabled( intrinsics_btn_enabled ) ;
  m_assign_current_intrinsic_to_undefined_views_btn->setEnabled( intrinsics_btn_enabled ) ;

  m_create_for_view_btn->setEnabled( views_btn_enabled ) ;
  m_delete_reference_for_view_btn->setEnabled( views_btn_enabled ) ;
  m_set_intrinsic_for_view_btn->setEnabled( views_btn_enabled ) ;
}

/**
 * @brief build interface widgets
 */
void IntrinsicSelectorParamsDialog::buildInterface( void )
{
  // Intrinsics
  QGroupBox * intrinsicsBox = new QGroupBox( "Intrinsics" ) ;
  QVBoxLayout * intrinsicBoxLayout = new QVBoxLayout ;

  m_intrinsic_view = new QTableView ;
  m_intrinsic_model = new QStandardItemModel ;
  m_intrinsic_view->setModel( m_intrinsic_model ) ;
  m_intrinsic_view->setContextMenuPolicy( Qt::CustomContextMenu );
  m_intrinsic_view->setSelectionBehavior( QAbstractItemView::SelectRows );


  m_delete_current_intrinsic_btn = new QPushButton( "Delete" ) ;
  m_edit_current_intrinsic_btn = new QPushButton( "Edit" ) ;
  m_assign_current_intrinsic_to_compatible_views_btn = new QPushButton( "Assign to compatible views" ) ;
  m_assign_current_intrinsic_to_undefined_views_btn = new QPushButton( "Assign to undefined views" ) ;

  QHBoxLayout * btnIntrinsics = new QHBoxLayout ;
  btnIntrinsics->addWidget( m_delete_current_intrinsic_btn ) ;
  btnIntrinsics->addWidget( m_edit_current_intrinsic_btn ) ;
  btnIntrinsics->addWidget( m_assign_current_intrinsic_to_compatible_views_btn ) ;
  btnIntrinsics->addWidget( m_assign_current_intrinsic_to_undefined_views_btn ) ;

  intrinsicBoxLayout->addWidget( m_intrinsic_view ) ;
  intrinsicBoxLayout->addLayout( btnIntrinsics ) ;
  intrinsicsBox->setLayout( intrinsicBoxLayout ) ;


  // Views
  QGroupBox * cameraBox = new QGroupBox( "Views" ) ;
  QVBoxLayout * cameraBoxLayout = new QVBoxLayout ;

  m_views_view = new QTableView ;
  m_views_model = new QStandardItemModel ;
  m_views_view->setModel( m_views_model ) ;
  m_views_view->setContextMenuPolicy( Qt::CustomContextMenu );
  m_views_view->setSelectionBehavior( QAbstractItemView::SelectRows );

  QHBoxLayout * btnViews = new QHBoxLayout ;

  m_create_for_view_btn = new QPushButton( "New" );
  m_delete_reference_for_view_btn = new QPushButton( "Delete" ) ;
  m_set_intrinsic_for_view_btn = new QPushButton( "Set intrinsic ID" ) ;

  btnViews->addWidget( m_create_for_view_btn );
  btnViews->addWidget( m_delete_reference_for_view_btn ) ;
  btnViews->addWidget( m_set_intrinsic_for_view_btn ) ;

  cameraBoxLayout->addWidget( m_views_view ) ;
  cameraBoxLayout->addLayout( btnViews ) ;
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
}

/**
 * @brief Make connections between widgets
 */
void IntrinsicSelectorParamsDialog::makeConnections( void )
{
  connect( m_cancel_btn , SIGNAL( clicked() ) , this , SLOT( onCancel() ) );
  connect( m_reset_btn , SIGNAL( clicked() ) , this , SLOT( onReset() ) ) ;
  connect( m_ok_btn , SIGNAL( clicked() ) , this , SLOT( onOk() ) );

  connect( m_intrinsic_view , SIGNAL( customContextMenuRequested( const QPoint & ) ) , this , SLOT( onRightClickIntrinsics( const QPoint & ) ) );
  connect( m_views_view , SIGNAL( customContextMenuRequested( const QPoint & ) ) , this , SLOT( onRightClickViews( const QPoint & ) ) ) ;

  connect( m_intrinsic_view->selectionModel() , SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ) , this , SLOT( onHasChangedIntrinsicSeletion() ) ) ;
  connect( m_views_view->selectionModel() , SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ) , this , SLOT( onHasChangedViewsSelection() ) ) ;

  connect( m_delete_current_intrinsic_btn , SIGNAL( clicked() ) , this , SLOT( onHasClickedIntrinsicDelete() ) ) ;
  connect( m_edit_current_intrinsic_btn , SIGNAL( clicked() ) , this , SLOT( onHasClickedIntrinsicEdit() ) ) ;
  connect( m_assign_current_intrinsic_to_compatible_views_btn , SIGNAL( clicked() ) , this , SLOT( onHasClickedIntrinsicAssignToCompatible() ) );
  connect( m_assign_current_intrinsic_to_undefined_views_btn , SIGNAL( clicked() ) , this , SLOT( onHasClickedIntrinsicAssignToUndefined() ) ) ;
  connect( m_create_for_view_btn , SIGNAL( clicked() ) , this , SLOT( onHasClickedViewsCreate() ) ) ;
  connect( m_delete_reference_for_view_btn , SIGNAL( clicked() ) , this , SLOT( onHasClickedViewsDelete() ) );
  connect( m_set_intrinsic_for_view_btn , SIGNAL( clicked() ) , this , SLOT( onHasClickedViewsAssign() ) ) ;
}


} // namespace openMVG_gui