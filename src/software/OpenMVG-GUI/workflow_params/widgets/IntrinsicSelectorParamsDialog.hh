// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_INTRINSIC_SELECTOR_VIEW_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_INTRINSIC_SELECTOR_VIEW_HH_

#include <QDialog>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>

namespace openMVG_gui
{
class Project ;

/**
 * View used to define intrinsics for the cameras
 */
class IntrinsicSelectorParamsDialog : public QDialog
{
  public:
    IntrinsicSelectorParamsDialog( QWidget * parent , std::shared_ptr<Project> proj ) ;

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

  private:

    void populateLists( std::shared_ptr<Project> proj ) ;

    void buildInterface( void ) ;
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

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif