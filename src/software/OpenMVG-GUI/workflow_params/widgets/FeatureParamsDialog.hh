// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_WIDGETS_FEATURE_PARAMS_DIALOG_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_WIDGETS_FEATURE_PARAMS_DIALOG_HH_

#include "workflow_params/FeatureParams.hh"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QPushButton>
#include <QSpinBox>

namespace openMVG_gui
{
class FeatureParamsDialog : public QDialog
{
  public:

    /**
    * @brief Ctr
    * @param parent Parent Widget
    * @param initialParams Initial Parameters passed to the dialog
    */
    FeatureParamsDialog( QWidget * parent , const FeatureParams & initialParams ) ;

    /**
    * @brief Get current parameters
    * @return Current parameters corresponding to the interface
    */
    FeatureParams params( void ) const ;

  public slots :

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

    /**
    * @brief Set params to a specified value
    * @param value New value
    */
    void setParams( const FeatureParams & value ) ;

    void buildInterface( void ) ;
    void makeConnections( void ) ;

    QComboBox * m_feature_method ;
    QComboBox * m_feature_preset ;
    QCheckBox * m_check_upright ;
    QSpinBox * m_spin_nb_parallel_job ;

    QPushButton * m_ok_btn ;
    QPushButton * m_cancel_btn ;
    QPushButton * m_reset_btn ;

    /// The parameters passed to the ctr
    FeatureParams m_initial_params ;


    Q_OBJECT
} ;
} // namespace openMVG_gui

#endif