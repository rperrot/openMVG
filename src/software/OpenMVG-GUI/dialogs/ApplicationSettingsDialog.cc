// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ApplicationSettingsDialog.hh"

#include "SensorDatabaseDialog.hh"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QColorDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace openMVG_gui
{

ApplicationSettingsDialog::ApplicationSettingsDialog( QWidget *parent, const ApplicationSettings &setting ) :
    QDialog( parent ), m_initialSettings( setting ), m_currentSettings( setting )
{
  buildInterface();
  makeConnections();

  update();
}

/**
 * @brief Get the settings in the dialog
 * @return current settings of the dialog
 */
ApplicationSettings ApplicationSettingsDialog::settings( void ) const
{
  return m_currentSettings;
}

/**
 * @brief action to be executed when user click on cancel button
 */
void ApplicationSettingsDialog::onCancel( void )
{
  done( QDialog::Rejected );
}

/**
 * @brief action to be executed when user click on OK button
 */
void ApplicationSettingsDialog::onOk( void )
{
  done( QDialog::Accepted );
}

/**
 * @brief When user click on the background color indicator
 */
void ApplicationSettingsDialog::onWantToSetBackgroundColor( void )
{
  const openMVG::Vec4 backgroundColor = m_currentSettings.viewBackgroundColor();
  const QColor        col( backgroundColor[ 0 ] * 255, backgroundColor[ 1 ] * 255, backgroundColor[ 2 ] * 255, backgroundColor[ 3 ] * 255 );

  QColorDialog dlg( col );

  int res = dlg.exec();
  if ( res == QColorDialog::Accepted )
  {
    const QColor newColor = dlg.currentColor();
    m_view_background_color_indicator->setColor( newColor );

    const double r = newColor.redF();
    const double g = newColor.greenF();
    const double b = newColor.blueF();
    const double a = newColor.alphaF();

    m_currentSettings.setViewBackgroundColor( {r, g, b, a} );
  }
}

/**
 * @brief When user click on sensor database btn
 *
 */
void ApplicationSettingsDialog::onWantToManageSensorDatabase( void )
{
  SensorDatabaseDialog dlg( this );

  // Load the sensor width database
  // Choose the sensor width database :
  // - Use the one in the application settings directory if it exists.
  // - If it does not exists, use the one bundled with the application
  std::string camera_sensor_width_database_file = ApplicationSettings::defaultSensorWidthDatabasePath();
  if ( stlplus::file_exists( ApplicationSettings::applicationWideSensorWidthDatabasePath() ) )
  {
    camera_sensor_width_database_file = ApplicationSettings::applicationWideSensorWidthDatabasePath();
  }
  dlg.setMainDatabaseFromFile( camera_sensor_width_database_file );
  const std::string user_defined_sensor_width_database_file = ApplicationSettings::applicationWideUserDefinedSensorWidthDatabasePath();
  if ( stlplus::file_exists( user_defined_sensor_width_database_file ) )
  {
    dlg.setUserDefinedDatabaseFromFile( user_defined_sensor_width_database_file );
  }

  const int res = dlg.exec();
}

/**
 * @brief Get back to the initial settings
 */
void ApplicationSettingsDialog::reinitSettings( void )
{
  m_currentSettings = m_initialSettings;
  update();
}

/**
 * @brief Reset to the application default settings
 */
void ApplicationSettingsDialog::resetDefaultSettings( void )
{
  m_currentSettings = ApplicationSettings();
  update();
}

void ApplicationSettingsDialog::buildInterface( void )
{
  // 3d view group
  QGroupBox *  viewGrp              = new QGroupBox( "3d View" );
  QGridLayout *viewGrpLayout        = new QGridLayout;
  QLabel *     lblBackgroundColor   = new QLabel( "Background color" );
  m_view_background_color_indicator = new ColorIndicatorWidget;

  viewGrpLayout->addWidget( lblBackgroundColor, 0, 0 );
  viewGrpLayout->addWidget( m_view_background_color_indicator, 0, 1 );
  viewGrp->setLayout( viewGrpLayout );

  // Application settings group
  QGroupBox *applicationGrp         = new QGroupBox( "Application" );
  m_sensorDatabaseMgmt              = new QPushButton( "Sensor database" );
  QGridLayout *applicationGrpLayout = new QGridLayout;
  applicationGrpLayout->addWidget( m_sensorDatabaseMgmt );
  applicationGrp->setLayout( applicationGrpLayout );

  // Main dialog buttons
  m_btn_reset_default = new QPushButton( "Reset Default" );
  m_btn_cancel        = new QPushButton( "Cancel" );
  m_btn_cancel->setDefault( false );
  m_btn_ok = new QPushButton( "OK" );
  m_btn_ok->setDefault( true );

  QHBoxLayout *btnLayout = new QHBoxLayout;
  btnLayout->addWidget( m_btn_reset_default );
  btnLayout->addStretch();
  btnLayout->addWidget( m_btn_cancel );
  btnLayout->addWidget( m_btn_ok );

  QVBoxLayout *mainLayout = new QVBoxLayout;

  mainLayout->addWidget( viewGrp );
  mainLayout->addWidget( applicationGrp );
  mainLayout->addLayout( btnLayout );

  setLayout( mainLayout );
}

void ApplicationSettingsDialog::makeConnections( void )
{
  connect( m_btn_reset_default, SIGNAL( clicked() ), this, SLOT( resetDefaultSettings() ) );
  connect( m_btn_cancel, SIGNAL( clicked() ), this, SLOT( onCancel() ) );
  connect( m_btn_ok, SIGNAL( clicked() ), this, SLOT( onOk() ) );

  connect( m_view_background_color_indicator, SIGNAL( clicked() ), this, SLOT( onWantToSetBackgroundColor() ) );

  connect( m_sensorDatabaseMgmt, SIGNAL( clicked() ), this, SLOT( onWantToManageSensorDatabase() ) );
}

void ApplicationSettingsDialog::update( void )
{
  // Update interface using current settings

  // 3d view background color
  {
    const openMVG::Vec4 backgroundColor = m_currentSettings.viewBackgroundColor();

    const QColor col( backgroundColor[ 0 ] * 255, backgroundColor[ 1 ] * 255, backgroundColor[ 2 ] * 255,
                      backgroundColor[ 3 ] * 255 );
    m_view_background_color_indicator->setColor( col );
  }
}

} // namespace openMVG_gui
