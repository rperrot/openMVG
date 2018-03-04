// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_WEIGHTED_SELECTION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_WEIGHTED_SELECTION_HH_

namespace openMVG_gui
{

/**
 * @brief Class used to have a weight associated with a selection
 * -> Used to indicate the link between views
 * -> Link negative -> No selection
 * -> Link > 1 FUll selection
 * -> Link \in [0;1] -> weight of the selection
 */
class WeightedSelectableObject
{
  public:

    /**
     * @brief Ctr
     * @param w Weight of the selection
     * @note Default selection mode is unselected
     */
    WeightedSelectableObject( const double w = -1.0 ) ;

    /**
     * @brief Weight of the selection
     * @return weight of the selection 
     */
    double selectionWeight( void ) const ;

    /**
     * @brief Set weight of the selection 
     * @param w Weight of the selection 
     */
    void setSelectionWeight( const double w ) ;

    /**
     * @brief get current selection state
     * @retval true if object is selected
     * @retval false if object is not selected
     */
    bool selected( void ) const ;

  private:

    double m_weight ;
} ;

} // namespace openMVG_gui

#endif