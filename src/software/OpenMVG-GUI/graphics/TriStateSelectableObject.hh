// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_TRI_STATE_SELECTABLE_OBJECT_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_TRI_STATE_SELECTABLE_OBJECT_HH_

namespace openMVG_gui
{

/**
 * @brief The three possible level of selection
 * -> No selection
 * -> first selected
 * - second selected
 */
enum TriStateSelection
{
  TRI_STATE_SELECTION_NONE ,
  TRI_STATE_SELECTION_FIRST ,
  TRI_STATE_SELECTION_SECOND
} ;

/**
 * @brief Handle a selectable object with three possible state :
 * - Not selected
 * - First state selected
 * - Second state selected
 */
class TriStateSelectableObject
{
  public:

    /**
     * @brief Ctr
     * @param state The current selection
     */
    TriStateSelectableObject( const TriStateSelection state = TRI_STATE_SELECTION_NONE ) ;

    /**
     * @brief Get current selection
     * @return current selection
     */
    TriStateSelection selectionState( void ) const ;

    /**
     * @brief Set current selection
     * @param state The new state
     */
    void setSelectionState( const TriStateSelection state ) ;

  private:

    /// Current selection state
    TriStateSelection m_state ;
} ;

} // namespace openMVG_gui

#endif