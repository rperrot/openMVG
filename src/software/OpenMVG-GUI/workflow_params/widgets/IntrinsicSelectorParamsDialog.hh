// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_WIDGETS_INTRINSIC_SELECTOR_PARAMS_DIALOG_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_WIDGETS_INTRINSIC_SELECTOR_PARAMS_DIALOG_HH_

#include "openMVG/cameras/cameras.hpp"
#include "openMVG/types.hpp"

#include <QDialog>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>

#include <map>

namespace openMVG_gui
{
class Project ;

/**
 * View used to define intrinsics for the cameras
 */
class IntrinsicSelectorParamsDialog : public QDialog
{
  public:

    /**
     * @brief Constructor 
     * @param parent widget 
     * @param proj project in which intrinsics are found (then edited)
     */
    IntrinsicSelectorParamsDialog( QWidget * parent , std::shared_ptr<Project> proj ) ;

    /**
     * @brief Get list of intrinsics
     */
    std::map < openMVG::IndexT , std::shared_ptr< openMVG::cameras::IntrinsicBase> > intrinsics( void ) const ;

    /**
     * @brief Get list of intrinsic_id for all cameras
     */
    std::map< openMVG::IndexT , openMVG::IndexT > indexes( void ) const ;

  public slots:

    /**
    * @brief action to be executed when user click on cancel button
    */
    void onCancel( void ) ;

    /**
    * @brief action to be executed when user click on ok button
    */
    void onOk( void ) ;

    /**
    * @brief Action to be executed when user click on reset button
    */
    void onReset( void ) ;

    /**
     * @brief Action to be executed when user right click on the intrinsic view
     */
    void onRightClickIntrinsics( const QPoint &pos ) ;

    /**
     * @brief Action to be executed when user wants to delete an intrinsic
     * @param intrinsic_id Id of the intrinsic to delete
     */
    void onDeleteIntrinsic( const openMVG::IndexT intrinsic_id ) ;

    /**
     * @brief Action to be executed when user wants to edit an intrinsic
     * @param intrinsic_id Id of the intrinsic to edit
     */
    void onEditIntrinsic( const openMVG::IndexT intrinsic_id ) ;

    /**
     * @brief Action to be executed when user wants to assign selected intrinsic to all compatible views
     * @param intrinsic_id Id of the intrinsic to assign
     * @note Compatible view is a view with same width and height
     */
    void onAssignIntrinsicToCompatibleViews( const openMVG::IndexT intrinsic_id ) ;

    /**
     * @brief Action to be executed when user wants to assign selected intrinsic to all compatible views and view with undefined intrinsic
     * @param intrinsic_id Id of the intrinsic to assign
     * @note Compatible view is a view with same width and height
     */
    void onAssignIntrinsicToCompatibleUndefinedViews( const openMVG::IndexT intrinsic_id ) ;

    /**
     * @brief Action to be executed when user right click on the views view
     */
    void onRightClickViews( const QPoint &pos ) ;

    /**
     * @brief Create intrinsic for a given view
     * @param view_id Id of the view
     */
    void onCreateIntrinsicForView( const openMVG::IndexT view_id ) ;

    /**
     * @brief Delete intrinsic (reference only) for a given view
     * @param view_id If of the view to modify
     * @note Removal set intrinsic id of the view to undefined
     */
    void onDeleteIntrinsicForView( const openMVG::IndexT view_id ) ;

    /**
     * @brief Assign intrinsic for the given view
     * @param view_id Id of the view to modify
     * @param intrinsic_id Id of the Intrinsic to set to the given view
     */
    void onAssignIntrinsicForView( const openMVG::IndexT view_id , const openMVG::IndexT intrinsic_id ) ;


  private:


    /**
     * @brief Given a project, fill all values inside internal data
     */
    void populateLists( std::shared_ptr<Project> proj ) ;
    /**
     * @brief update view according to the data
     */
    void updateTableViews( void ) ;
    /**
     * @brief build interface widgets
     */
    void buildInterface( void ) ;
    /**
     * @brief Make connections between widgets
     */
    void makeConnections( void ) ;

    // View about intrinsics
    QTableView * m_intrinsic_view ;
    QStandardItemModel * m_intrinsic_model ;

    // View about each view and their associated intrinsics
    QTableView * m_views_view ;
    QStandardItemModel * m_views_model ;

    QPushButton * m_ok_btn ;
    QPushButton * m_cancel_btn ;
    QPushButton * m_reset_btn ;

    std::shared_ptr<Project> m_project ;

    // Handle data
    // ID intrinsic -> Intrinsic
    std::map< openMVG::IndexT , std::shared_ptr<openMVG::cameras::IntrinsicBase> > m_intrinsics ;
    // ID View -> ID Intrinsic
    std::map< openMVG::IndexT , openMVG::IndexT > m_map_view_intrinsic ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif