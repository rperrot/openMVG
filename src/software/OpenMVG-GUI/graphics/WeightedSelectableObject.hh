// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_WEIGHTED_SELECTION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_WEIGHTED_SELECTION_HH_

#include "SelectableObject.hh"

namespace openMVG_gui
{

/**
 * @brief Class used to have a weight associated with a selection
 */
class WeightedSelectableObject : public SelectableObject
{
public:
  /**
   * @brief Ctr
   * @param selected Indicate if the object is selected
   * @param w Weight of the selection
   */
  WeightedSelectableObject( const bool selected = false, const double w = -1.0 );

  /**
   * @brief Weight of the selection
   * @return weight of the selection
   */
  double selectionWeight( void ) const;

  /**
   * @brief Set weight of the selection
   * @param w Weight of the selection
   */
  void setSelectionWeight( const double w );

private:
  double m_weight;
};

} // namespace openMVG_gui

#endif