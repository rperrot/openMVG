// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_WIDGETS_INTRINSIC_EDITOR_DIALOG_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_WIDGETS_INTRINSIC_EDITOR_DIALOG_HH_

#include "openMVG/cameras/cameras.hpp"

#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace openMVG_gui
{

/**
 * @brief Dialog used to edit an intrinsic
 */
class IntrinsicEditorDialog : public QDialog
{
  public:

    /**
     * @brief Ctr
     * @param parent parent widget
     * @param intrin Intrinsic to edit
     */
    IntrinsicEditorDialog( QWidget * parent , std::shared_ptr<openMVG::cameras::IntrinsicBase> intrin ) ;

    /**
     * @brief Get current intrinsic
     * @return current intrinsic
     */
    std::shared_ptr<openMVG::cameras::IntrinsicBase> intrinsic( void ) const ;

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
     * @brief Action to be executed when used click on reset button
     */
    void onHasChangedType( void ) ;

    /**
     * @brief Action to be executed when user has edited width line
     */
    void onHasEditedWidth( void ) ;

    /**
     * @brief Action to be executed when user has edited height  line
     */
    void onHasEditedHeight( void ) ;

    /**
     * @brief Action to be executed when user has edited focal line
     */
    void onHasEditedFocal( void ) ;

    /**
     * @brief Action to be executed when user has edited principal point X line
     */
    void onHasEditedPPX( void ) ;

    /**
    * @brief Action to be executed when user has edited principal point Y line
    */
    void onHasEditedPPY( void ) ;

    /**
    * @brief Action to be executed when user has edited param 1 line
    */
    void onHasEditedParam1( void ) ;

    /**
    * @brief Action to be executed when user has edited param 2 line
    */
    void onHasEditedParam2( void ) ;

    /**
    * @brief Action to be executed when user has edited param 3 line
    */
    void onHasEditedParam3( void ) ;

    /**
    * @brief Action to be executed when user has edited param 4 line
    */
    void onHasEditedParam4( void ) ;

    /**
    * @brief Action to be executed when user has edited param 5 line
    */
    void onHasEditedParam5( void ) ;


  private:

    /**
     * @brief Fill values according to the current intrinsic data
     * @param set_index Indicate if the combo box should be modified accordingly
     */
    void fillValues( const bool set_index = true ) ;
    /**
     * @brief Update interface according to the current intrinsic data
     */
    void updateInterface( void ) ;
    /**
     * @brief Build interface widgets
     */
    void buildInterface( void ) ;
    /**
     * @brief Make connections between widgets
     */
    void makeConnections( void ) ;


    // The original intrinsic
    std::shared_ptr<openMVG::cameras::IntrinsicBase> m_original_intrinsic ;

    // The current intrinsic
    std::shared_ptr<openMVG::cameras::IntrinsicBase> m_intrinsic ;

    QLabel * m_type_lbl ;
    QComboBox * m_type ;

    QGroupBox * m_basic_params_grp ;
    QLabel * m_width_lbl ;
    QLineEdit * m_width ;
    QLabel * m_height_lbl ;
    QLineEdit * m_height ;

    QGroupBox * m_pinhole_params_grp ;
    QLabel * m_focal_lbl ;
    QLineEdit * m_focal ;
    QLabel * m_ppx_lbl ;
    QLineEdit * m_ppx ;
    QLabel * m_ppy_lbl ;
    QLineEdit * m_ppy ;

    QGroupBox * m_distortion_params_grp ;
    QLabel * m_param_1_lbl ;
    QLineEdit * m_param_1 ;
    QLabel * m_param_2_lbl ;
    QLineEdit * m_param_2 ;
    QLabel * m_param_3_lbl ;
    QLineEdit * m_param_3 ;
    QLabel * m_param_4_lbl ;
    QLineEdit * m_param_4 ;
    QLabel * m_param_5_lbl ;
    QLineEdit * m_param_5 ;

    QPushButton * m_ok_btn ;
    QPushButton * m_cancel_btn ;
    QPushButton * m_reset_btn ;


    Q_OBJECT
} ;
} // namespace openMVG_gui

#endif