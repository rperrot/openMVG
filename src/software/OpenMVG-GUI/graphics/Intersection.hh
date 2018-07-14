// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _INTERSECTION_HH_
#define _INTERSECTION_HH_

#include <limits>
#include <memory>

namespace openMVG_gui
{

class IntersectableObject;

/**
 * @brief Class used to represent result of an intersection with a ray
 */
class Intersection
{
public:
  /**
   * @brief Ctr
   * @param has_intersection Indicate if the intersection is non empty (true value)
   * @param obj Indicate the object that has intersection with the ray
   * @param distance Indicate intersection distance with the object and the ray
   */
  Intersection( const bool has_intersection = false, std::shared_ptr<IntersectableObject> obj = nullptr,
                const double distance = std::numeric_limits<double>::max() );

  /**
   * @brief Copy constructor
   * @param src Original object
   */
  Intersection( const Intersection &src ) = default;
  /**
   * @brief Move constructor
   * @param src Original object
   */
  Intersection( Intersection &&src ) = default;
  /**
   * @brief Assignment operator
   * @param src Original object
   * @return object after assignment
   */
  Intersection &operator=( const Intersection &src ) = default;
  /**
   * @brief Move assignment operator
   * @param src Original object
   * @return object after assignment
   */
  Intersection &operator=( Intersection &&src ) = default;

  /**
   * @brief Test if intersection is non empty
   * @retval true if intersection is valid
   * @retval false if no intersection
   */
  operator bool() const;

  /**
   * @brief Get intersection object
   * @return Object that has intersection with the ray
   * @retval nullptr if no intersection
   */
  std::shared_ptr<IntersectableObject> object( void ) const;

  /**
   * @brief (Signed) distance of the intersection betwwen the object and the ray
   * @return signed intersection distance
   * @retval std::numeric_limits<double>::max() if no intersection
   */
  double distance( void ) const;

private:
  /// Intersection value
  bool m_has_intersection;
  /// Object
  std::shared_ptr<IntersectableObject> m_object;
  /// Distance
  double m_t;
};

bool operator==( const Intersection &obj1, const Intersection &obj2 );

} // namespace openMVG_gui

#endif