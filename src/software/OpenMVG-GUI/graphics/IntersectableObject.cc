// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "IntersectableObject.hh"

namespace openMVG_gui
{

/**
 * @brief Ctr
 * @param enableIntersection Directive to allow intersection on this object
 */
IntersectableObject::IntersectableObject( const bool enableIntersection )
    : m_active( enableIntersection )
{
}

/**
 * @brief Indicate if intersection is enabled for this object
 * @retval true if active
 * @retval false if intersection will always return non intersection
 */
bool IntersectableObject::intersectionEnabled( void ) const
{
  return m_active;
}

/**
 * @brief Enable/Disable intersection test
 * @param enable activation value
 */
void IntersectableObject::setIntersectionEnabled( const bool enable )
{
  m_active = enable;
}

} // namespace openMVG_gui