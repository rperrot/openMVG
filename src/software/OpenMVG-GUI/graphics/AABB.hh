// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_AABB_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_AABB_HH_

#include "Intersection.hh"
#include "Ray.hh"

#include "openMVG/numeric/numeric.h"

namespace openMVG_gui
{

/**
 * @brief class managing a standard Axis Aligned Bounding Box
 */
class AABB
{
public:
  /**
   * @brief ctr
   */
  AABB( void );

  /**
   * @brief ctr around a point
   */
  AABB( const openMVG::Vec3 &pt );

  /**
   * @brief Full ctr
   */
  AABB( const openMVG::Vec3 &min, const openMVG::Vec3 &max );

  /**
   * @brief Copy ctr
   */
  AABB( const AABB &src ) = default;

  /**
   * @brief Move ctr
   */
  AABB( AABB &&src ) = default;

  /**
   * @brief Assignment operator
   */
  AABB &operator=( const AABB &src ) = default;

  /**
   * @brief Move assignment operator
   */
  AABB &operator=( AABB &&src ) = default;

  /**
   * @brief Minimum
   */
  openMVG::Vec3 min( void ) const;

  /**
   * @brief Maximum
   */
  openMVG::Vec3 max( void ) const;

  /**
   * @brief Length in each dim
   */
  openMVG::Vec3 extends( void ) const;

  /**
   * @brief center of the bounding box
   */
  openMVG::Vec3 center( void ) const;

  /**
   * @brief Union of two bounding boxes
   */
  friend AABB makeUnion( const AABB &a, const AABB &b );

  /**
   * @brief Union of a bounding box and a point
   */
  friend AABB makeUnion( const AABB &a, const openMVG::Vec3 &pt );

  /**
   * @brief Union of a bounding box and a point
   */
  friend AABB makeUnion( const openMVG::Vec3 &pt, const AABB &a );

  /**
   * @brief Perform intersection with the bounding box
   */
  Intersection intersect( const Ray &ray ) const;

  /**
   * @brief Output box to stream
   *
   * @param out Input/output stream
   * @param box Bounding box to send to the stream
   * @return stream after box sent have been sent to it
   */
  friend std::ostream &operator<<( std::ostream &out, const AABB &box );

private:
  openMVG::Vec3 m_min;
  openMVG::Vec3 m_max;
};

/**
 * @brief Union of two bounding boxes
 */
AABB makeUnion( const AABB &a, const AABB &b );

/**
 * @brief Union of a bounding box and a point
 */
AABB makeUnion( const AABB &a, const openMVG::Vec3 &pt );

/**
 * @brief Union of a bounding box and a point
 */
AABB makeUnion( const openMVG::Vec3 &pt, const AABB &a );

/**
 * @brief Output box to stream
 *
 * @param out Input/output stream
 * @param box Bounding box to send to the stream
 * @return stream after box sent have been sent to it
 */
std::ostream &operator<<( std::ostream &out, const AABB &box );

} // namespace openMVG_gui

#endif