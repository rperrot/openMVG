// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017, 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_VERSION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_VERSION_HH_

namespace openMVG_gui
{

/**
 * @brief Get current runtime version major
 * @return major version
 */
constexpr int GUIVersionMajorNumber( void )
{
  return 0;
}

/**
 * @brief Get current runtime version major
 * @return major version
 */
constexpr int GUIVersionMinorNumber( void )
{
  return 1;
}

/**
 * @brief Get current runtime version major
 * @return major version
 */
constexpr int GUIVersionRevisionNumber( void )
{
  return 0;
}

class GuiVersion
{
public:
  static const int CURRENT_VERSION_MAJOR;
  static const int CURRENT_VERSION_MINOR;
  static const int CURRENT_VERSION_REVISION;

  static const GuiVersion CURRENT_VERSION;

  GuiVersion( const int major, const int minor, const int revision );

  friend bool operator<( const GuiVersion &v1, const GuiVersion &v2 );
  friend bool operator==( const GuiVersion &v1, const GuiVersion &v2 );

private:
  int m_major;
  int m_minor;
  int m_revision;
};

/**
 * @brief Comparison between versions
 */
bool operator<( const GuiVersion &v1, const GuiVersion &v2 );
bool operator<=( const GuiVersion &v1, const GuiVersion &v2 );
bool operator>( const GuiVersion &v1, const GuiVersion &v2 );
bool operator>=( const GuiVersion &v1, const GuiVersion &v2 );
bool operator==( const GuiVersion &v1, const GuiVersion &v2 );
bool operator!=( const GuiVersion &v1, const GuiVersion &v2 );

} // namespace openMVG_gui

#endif