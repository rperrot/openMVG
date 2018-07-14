// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_RAY_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_RAY_HH_

#include "openMVG/numeric/numeric.h"

#include <array>

namespace openMVG_gui
{

/**
 * @brief Class used to represent a ray in 3d
 * @note this is used for intersection with the scene
 */
class Ray
{
public:
  /**
   * @brief Ctr
   * @param origin Origin of the ray in 3d
   * @param direction Direction of the ray
   */
  Ray( const openMVG::Vec3 &origin, const openMVG::Vec3 &direction );

  /**
   * @brief Copy constructor
   * @param src Original ray
   */
  Ray( const Ray &src ) = default;
  /**
   * @brief Move constructor
   * @param src Original ray
   */
  Ray( Ray &&src ) = default;

  /**
   * @brief Assignment operator
   * @param src Original ray
   * @return ray after assignment
   */
  Ray &operator=( const Ray &src ) = default;

  /**
   * @brief Move assignment operator
   * @param src Original ray
   * @return ray after assignment
   */
  Ray &operator=( Ray &&src ) = default;

  /**
   * @brief Get origin of the ray
   * @return 3d position of the ray origin
   */
  openMVG::Vec3 origin( void ) const;

  /**
   * @brief Get direction of the ray
   * @retun 3d vector direction of the ray
   * @note Ray direction is unnormalized
   */
  openMVG::Vec3 direction( void ) const;

  /**
   * @brief Get reciprocal of the direction (ie: 1.0 / direction() for each axis)
   *
   * @return reciprocal
   */
  openMVG::Vec3 inverse( void ) const;

  /**
   * @brief Get sign of the inverse of the direction vector
   *
   * @return sign( 1.0 / direction[i] ) for i in {0,1,2}
   */
  std::array<int, 3> signInvDirection( void ) const;

  /**
   * @brief Compute point with a given parameter
   * @param t Parameter
   * @return 3d point corresponding to the parameter
   * @note Point is equal to Origin + t * Direction
   */
  openMVG::Vec3 operator()( const double t ) const;

  /**
   * @brief Output to stream
   *
   * @param str Input/Output stream
   * @param ray The ray to output
   * @return std::ostream& stream after output
   */
  friend std::ostream &operator<<( std::ostream &str, const Ray &ray );

private:
  /// Origin
  openMVG::Vec3 m_origin;
  /// Direction
  openMVG::Vec3 m_direction;
  /// 1.0 / Direction
  openMVG::Vec3 m_inv_dir;
  /// Sign of 1 / Direction (on each axis)
  std::array<int, 3> m_sign;
};

/**
 * @brief Output to stream
 *
 * @param str Input/Output stream
 * @param ray The ray to output
 * @return std::ostream& stream after output
 */
std::ostream &operator<<( std::ostream &str, const Ray &ray );

} // namespace openMVG_gui

#endif