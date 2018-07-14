// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_SELECTABLE_OBJECT_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_SELECTABLE_OBJECT_HH_

namespace openMVG_gui
{

/**
 * @brief class holding object that can be selectable ·
 */
class SelectableObject
{
public:
  /**
   * @brief ctr
   * @param selected current selection
   */
  SelectableObject( const bool selected = false );

  virtual ~SelectableObject() = default;

  /**
   * @brief get current selection state
   * @retval true if object is selected
   * @retval false if object is not selected
   */
  bool selected( void ) const;

  /**
   * @brief set current selection
   * @param sel new selection
   */
  void setSelected( const bool sel );

  /**
   * @brief switch selection state
   */
  void switchSelection( void );

private:
  bool m_selected;
};

} // namespace openMVG_gui

#endif