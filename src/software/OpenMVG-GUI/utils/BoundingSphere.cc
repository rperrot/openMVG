// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "BoundingSphere.hh"

namespace openMVG_gui
{

/**
 * @brief compute bounding sphere of a given point list
 * @param pts List of points
 * @param[out] center of the sphere
 * @param[out] rad radius of the sphere
 * @note This is an implementation of the Ritter algorithm
 */
void computeBoundingSphere( const std::vector<openMVG::Vec3> &pts, openMVG::Vec3 &center, double &rad )
{
  // 0 - Pick a point x in pts
  // 1 - Search the point y with maximal distance wrt x
  // 2 - Search the point z with maximal distance wrt y
  // -> Initial center is (y+z) / 2
  // -> Initial radius is || y - z || / 2
  // 3 For all the points : if it's outside, update the bsphere to make it inside

  // 0
  const openMVG::Vec3 &x = pts[ 0 ];
  openMVG::Vec3 y;
  openMVG::Vec3 z;
  double max_dist = std::numeric_limits<double>::lowest();

  // 1
  for ( const auto &pt : pts )
  {
    const double d2 = ( pt - x ).squaredNorm();
    if ( d2 > max_dist )
    {
      y        = pt;
      max_dist = d2;
    }
  }
  // 2
  max_dist = std::numeric_limits<double>::lowest();
  for ( const auto &pt : pts )
  {
    const double d2 = ( pt - y ).squaredNorm();
    if ( d2 > max_dist )
    {
      z        = pt;
      max_dist = d2;
    }
  }

  center = ( y + z ) / 2.0;
  rad    = ( y - center ).norm();

  double rad2 = rad * rad;
  for ( const auto &pt : pts )
  {
    const double d2 = ( pt - center ).squaredNorm();

    if ( d2 > rad2 )
    {
      const double d = std::sqrt( d2 );
      // Point is outside,
      // update radius
      rad  = ( rad + d ) * 0.5;
      rad2 = rad * rad;

      // Update center
      const double delta_rad = d - rad;
      const double inv       = 1.0 / d;
      center                 = ( rad * center + delta_rad * pt ) * inv;
    }
  }
}

/**
 * @brief Compute bounding box of a set of points
 * @param pts List of points
 * @param[out] min Minimum on each coords
 * @param[out] max Maximum on each coords
 */
void computeBoundingBox( const std::vector<openMVG::Vec3> &pts, openMVG::Vec3 &min, openMVG::Vec3 &max )
{
  min = openMVG::Vec3( std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() );
  max = openMVG::Vec3( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest() );

  for ( const auto &pt : pts )
  {
    min[ 0 ] = std::min( pt[ 0 ], min[ 0 ] );
    min[ 1 ] = std::min( pt[ 1 ], min[ 1 ] );
    min[ 2 ] = std::min( pt[ 2 ], min[ 2 ] );

    max[ 0 ] = std::max( pt[ 0 ], max[ 0 ] );
    max[ 1 ] = std::max( pt[ 1 ], max[ 1 ] );
    max[ 2 ] = std::max( pt[ 2 ], max[ 2 ] );
  }
}

} // namespace openMVG_gui