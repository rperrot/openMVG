// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_APPLICATION_SETTINGS_DIALOG_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_APPLICATION_SETTINGS_DIALOG_HH_

#include "ApplicationSettings.hh"

#include "ColorIndicatorWidget.hh"

#include <QDialog>
#include <QFrame>
#include <QPushButton>

namespace openMVG_gui
{

/**
 * @brief Dialog used to provide the user a way to set settings of the application
 */
class ApplicationSettingsDialog : public QDialog
{
public:
  ApplicationSettingsDialog( QWidget *parent, const ApplicationSettings &setting );

  /**
   * @brief Get the settings in the dialog
   * @return current settings of the dialog
   */
  ApplicationSettings settings( void ) const;

public slots:

  /**
   * @brief action to be executed when user click on cancel button
   */
  void onCancel( void );

  /**
   * @brief action to be executed when user click on OK button
   */
  void onOk( void );

private slots:

  /**
   * @brief When user click on the background color indicator
   */
  void onWantToSetBackgroundColor( void );

  /**
   * @brief Get back to the initial settings
   */
  void reinitSettings( void );

  /**
   * @brief Reset to the application default settings
   */
  void resetDefaultSettings( void );

private:
  void buildInterface( void );
  void makeConnections( void );

  void update( void );

  ApplicationSettings m_currentSettings;
  ApplicationSettings m_initialSettings;

  QPushButton *m_btn_reset_default;
  QPushButton *m_btn_cancel;
  QPushButton *m_btn_ok;

  ColorIndicatorWidget *m_view_background_color_indicator;

  Q_OBJECT
};

} // namespace openMVG_gui

#endif