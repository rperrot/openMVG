// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_APPLICATION_SETTINGS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_APPLICATION_SETTINGS_HH_

#include "openMVG/numeric/numeric.h"

#include <memory>
#include <string>

namespace openMVG_gui
{

/**
 * @brief Class used to handle application settings
 */
class ApplicationSettings
{
public:
  static const openMVG::Vec4 DEFAULT_VIEW_BACKGROUND_COLOR;
  // Get the instance of the application settings
  static ApplicationSettings &instance();
  // Get the path of the application configuration file
  static std::string configPath( void );

  /**
   * Default settings
   */
  ApplicationSettings( void );

  /**
   * @brief Settings from a file
   * @param file File to load
   */
  ApplicationSettings( const std::string &file );

  ApplicationSettings( const ApplicationSettings &src ) = default;
  ApplicationSettings( ApplicationSettings &&src )      = default;

  ApplicationSettings &operator=( const ApplicationSettings &src ) = default;
  ApplicationSettings &operator=( ApplicationSettings &&src ) = default;

  /**
   * @brief Get background color of the 3d view
   * @return background color
   */
  openMVG::Vec4 viewBackgroundColor( void ) const;

  /**
   * @brief Set background color of the 3d view
   * @param col New background color
   */
  void setViewBackgroundColor( const openMVG::Vec4 &col );

  /**
   * @brief Save application settings to a file
   * @param file The output file
   * @retval true if save is OK
   * @retval false if save fails
   */
  bool save( const std::string &file );

  void set( const ApplicationSettings &src );

  /**
   * @brief Get sensor_width_database path situated in the application settings
   * directory
   *
   * @return sensor width database path
   */
  static std::string applicationWideSensorWidthDatabasePath( void );

  /**
   * @brief Get default sensor width database (ie: bundled with the application)
   *
   * @return sensor width database path
   */
  static std::string defaultSensorWidthDatabasePath( void );

  /**
   * @brief Get user defined sensor width database file path
   *
   * @return sensor width database path
   */
  static std::string applicationWideUserDefinedSensorWidthDatabasePath( void );

private:
  /**
   *  Load a settings file
   * @param file File to load
   * @retval true if load is OK
   * @retval false if load fails
   */
  bool load( const std::string &file );

  /// The view background color (RGBA in range [0-1])
  openMVG::Vec4 m_view_background_color;
};

} // namespace openMVG_gui

#endif