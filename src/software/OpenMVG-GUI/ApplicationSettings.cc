// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ApplicationSettings.hh"

#include "Version.hh"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <cereal/archives/xml.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

#include <QStandardPaths>

#include <clocale>
#include <fstream>
#include <vector>

namespace openMVG_gui
{

const openMVG::Vec4 ApplicationSettings::DEFAULT_VIEW_BACKGROUND_COLOR = {0.251, 0.251, 0.251, 1.0};

ApplicationSettings &ApplicationSettings::instance()
{
  // It's thread safe in C++11
  static ApplicationSettings instance;
  return instance;
}

// Get the path of the application configuration file
std::string ApplicationSettings::configPath( void )
{
  QString dir = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
  dir += "/application_settings.xml";
  return dir.toUtf8().constData();
}

/**
 * Default settings
 */
ApplicationSettings::ApplicationSettings( void )
    : m_view_background_color( ApplicationSettings::DEFAULT_VIEW_BACKGROUND_COLOR )
{
}

/**
 * @brief Settings from a file
 * @param file File to load
 */
ApplicationSettings::ApplicationSettings( const std::string &file )
{
  if ( !load( file ) )
  {
    *this = ApplicationSettings();
  }
}

/**
 * @brief Get background color of the 3d view
 * @return background color
 */
openMVG::Vec4 ApplicationSettings::viewBackgroundColor( void ) const
{
  return m_view_background_color;
}

/**
 * @brief Set background color of the 3d view
 * @param col New background color
 */
void ApplicationSettings::setViewBackgroundColor( const openMVG::Vec4 &col )
{
  m_view_background_color = col;
}

/**
 * @brief Save application settings to a file
 * @param file The output file
 * @retval true if save is OK
 * @retval false if save fails
 */
bool ApplicationSettings::save( const std::string &file )
{
  std::ofstream o_file( file );
  if ( !o_file )
  {
    return false;
  }

  setlocale( LC_ALL, "C" );
  setlocale( LC_NUMERIC, "C" );

  cereal::XMLOutputArchive archive( o_file );

  try
  {
    archive( cereal::make_nvp( "major_version", GuiVersion::CURRENT_VERSION_MAJOR ) );
    archive( cereal::make_nvp( "minor_version", GuiVersion::CURRENT_VERSION_MINOR ) );
    archive( cereal::make_nvp( "revision_version", GuiVersion::CURRENT_VERSION_REVISION ) );

    // Background
    {
      const std::vector<double> data = {m_view_background_color[ 0 ], m_view_background_color[ 1 ],
                                        m_view_background_color[ 2 ], m_view_background_color[ 3 ]};
      archive( cereal::make_nvp( "view_background_color", data ) );
    }
  }
  catch ( ... )
  {
    return false;
  }

  return true;
}

/**
 *  Load a settings file
 * @param file File to load
 * @retval true if load is OK
 * @retval false if load fails
 */
bool ApplicationSettings::load( const std::string &file )
{
  if ( !stlplus::file_exists( file ) )
  {
    return false;
  }

  // Load global project state
  setlocale( LC_ALL, "C" );
  setlocale( LC_NUMERIC, "C" );

  std::ifstream i_file( file );
  if ( !i_file )
  {
    return false;
  }

  cereal::XMLInputArchive archive( i_file );

  int major_version;
  int minor_version;
  int revision_version;
  try
  {
    archive( cereal::make_nvp( "major_version", major_version ) );
    archive( cereal::make_nvp( "minor_version", minor_version ) );
    archive( cereal::make_nvp( "revision_version", revision_version ) );

    // Available in version 0.1.0 :
    if ( GuiVersion::CURRENT_VERSION >= GuiVersion( 0, 1, 0 ) )
    {
      std::vector<double> data( 4 );
      archive( cereal::make_nvp( "view_background_color", data ) );
      m_view_background_color = {data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ]};
    }
    else
    {
      m_view_background_color = ApplicationSettings::DEFAULT_VIEW_BACKGROUND_COLOR;
    }
  }
  catch ( ... )
  {
    return false;
  }

  return true;
}

  void ApplicationSettings::set( const ApplicationSettings & src)
  {
    (*this) = src ; 
  }


} // namespace openMVG_gui
