// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "TriStateSelectableObject.hh"

namespace openMVG_gui
{

/**
* @brief Ctr
* @param state The current selection
*/
TriStateSelectableObject::TriStateSelectableObject( const TriStateSelection state )
  : m_state( state )
{

}

/**
 * @brief Get current selection
 * @return current selection
 */
TriStateSelection TriStateSelectableObject::selectionState( void ) const
{
  return m_state ;
}

/**
 * @brief Set current selection
 * @param state The new state
 */
void TriStateSelectableObject::setSelectionState( const TriStateSelection state )
{
  m_state = state ;
}

} // namespace openMVG_gui