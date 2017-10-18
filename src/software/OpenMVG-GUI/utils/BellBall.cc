// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "BellBall.hh"

#include <cmath>

namespace openMVG_gui
{

/**
 * @brief radius of the bell ball
 */
BellBall::BellBall( const double radius )
  : m_radius( radius )
  , m_radius2( radius * radius )
{
}

/**
 * @brief get 3d point on hyperbola given a 2d point
 */
openMVG::Vec3 BellBall::get( const double x, const double y ) const
{
  const double radial = x * x + y * y;

  if ( radial > ( m_radius2 * 0.5 ) )
  {
    // Hyperbola
    return openMVG::Vec3( x, y, m_radius2 / ( 2.0 * std::sqrt( radial ) ) );
  }
  else
  {
    // Sphere
    return openMVG::Vec3( x, y, std::sqrt( m_radius2 - radial ) );
  }
}

} // namespace openMVG_gui