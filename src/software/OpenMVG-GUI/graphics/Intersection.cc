// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Intersection.hh"

namespace openMVG_gui
{

/**
 * @brief Ctr
 * @param has_intersection Indicate if the intersection is non empty (true value)
 * @param obj Indicate the object that has intersection with the ray
 * @param distance Indicate intersection distance with the object and the ray
 */
Intersection::Intersection( const bool has_intersection, std::shared_ptr<IntersectableObject> obj,
                            const double distance ) :
    m_has_intersection( has_intersection ),
    m_object( obj ), m_t( distance )
{
}

/**
 * @brief Test if intersection is non empty
 * @retval true if intersection is valid
 * @retval false if no intersection
 */
Intersection::operator bool() const
{
  return m_has_intersection;
}

/**
 * @brief Get intersection object
 * @return Object that has intersection with the ray
 * @retval nullptr if no intersection
 */
std::shared_ptr<IntersectableObject> Intersection::object( void ) const
{
  return m_object;
}

/**
 * @brief (Signed) distance of the intersection betwwen the object and the ray
 * @return signed intersection distance
 * @retval std::numeric_limits<double>::max() if no intersection
 */
double Intersection::distance( void ) const
{
  return m_t;
}

} // namespace openMVG_gui
