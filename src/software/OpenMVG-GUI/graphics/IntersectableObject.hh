// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _INTERSECTABLE_OBJECT_HH_
#define _INTERSECTABLE_OBJECT_HH_

#include "Intersection.hh"
#include "Ray.hh"

namespace openMVG_gui
{

/**
 * @brief Abstract class used to represent an object that allow intersection with a ray
 */
class IntersectableObject : public std::enable_shared_from_this<IntersectableObject>
{
public:
  /**
   * @brief Ctr
   * @param enableIntersection Directive to allow intersection on this object
   */
  IntersectableObject( const bool enableIntersection = true );

  /**
   * @brief Copy constructor
   * @param src Original object
   */
  IntersectableObject( const IntersectableObject &src ) = default;
  /**
   * @brief Move constructor
   * @param src Original object
   */
  IntersectableObject( IntersectableObject &&src ) = default;
  /**
   * @brief Assignment operator
   * @param src Original object
   * @return object after assignment
   */
  IntersectableObject &operator=( const IntersectableObject &src ) = default;
  /**
   * @brief Move assignment operator
   * @param src Original object
   * @return object after assignment
   */
  IntersectableObject &operator=( IntersectableObject &&src ) = default;

  /**
   * @brief Indicate if intersection is enabled for this object
   * @retval true if active
   * @retval false if intersection will always return non intersection
   */
  bool intersectionEnabled( void ) const;

  /**
   * @brief Enable/Disable intersection test
   * @param enable activation value
   */
  void setIntersectionEnabled( const bool enable );

  /**
   * @brief Perform/Test intersection with a given aray
   * @param ray The ray of the intersection
   * @return The intersection object as the result of the intersection
   */
  virtual Intersection intersect( const Ray &ray ) const = 0;

private:
  bool m_active;
};

} // namespace openMVG_gui

#endif