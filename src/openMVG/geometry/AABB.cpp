// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "AABB.hpp"

#include <limits>

namespace openMVG
{
namespace geometry
{

/**
 * @brief Construct a null bounding box
 */
AABB::AABB( void )
  : m_min( std::numeric_limits<double>::max() ,
           std::numeric_limits<double>::max() ,
           std::numeric_limits<double>::max() ) ,
    m_max( - std::numeric_limits<double>::max() ,
           - std::numeric_limits<double>::max() ,
           - std::numeric_limits<double>::max() )
{

}

/**
 * @brief Construct a bbox that perfectly fit the set of points
 * @param pts The points
 */
AABB::AABB( const std::vector<openMVG::Vec3> & pts )
  : m_min( std::numeric_limits<double>::max() ,
           std::numeric_limits<double>::max() ,
           std::numeric_limits<double>::max() ) ,
    m_max( - std::numeric_limits<double>::max() ,
           - std::numeric_limits<double>::max() ,
           - std::numeric_limits<double>::max() )
{
  update( pts ) ;
}


/**
 * @brief Construct a bounding box given minimum and maximum extends
 * @param min Minimum extend of the bounding box
 * @param max Maximum extend of the bounding box
 */
AABB::AABB( const openMVG::Vec3 & min , const openMVG::Vec3 & max )
  : m_min( min ) ,
    m_max( max )
{

}

/**
 * @brief Indicate if the bounding box is empty
 * @retval true if the box is empty
 * @retval false if the box is not empty
 */
bool AABB::empty( void ) const
{
  return ( m_min[0] > m_max[0] ) &&
         ( m_min[1] > m_max[1] ) &&
         ( m_min[2] > m_max[2] ) ;
}


/**
 * @brief Get extends in each dimensions
 * @return extend in X,Y,Z of each dimensions
 * @note If bounding box is empty, return { -1 , -1 , -1 }
 */
openMVG::Vec3 AABB::extends( void ) const
{
  return empty() ?
         openMVG::Vec3( -1.0 , -1.0 , -1.0 ) :
         m_max - m_min ;
}

/**
 * @brief Get center of the bounding box
 * @return Center of the bounding box
 * @note If bounding box is empty, return { Inf , Inf , Inf } ;
 */
openMVG::Vec3 AABB::center( void ) const
{
  return empty() ?
         openMVG::Vec3(
           std::numeric_limits<double>::infinity() ,
           std::numeric_limits<double>::infinity() ,
           std::numeric_limits<double>::infinity()
         )
         :
         ( m_min + m_max ) * 0.5 ;
}

/**
 * @brief Get minimum coordinate of the bounding box
 * @return Minimum coordinate of the bounding box
 */
openMVG::Vec3 AABB::min( void ) const
{
  return m_min ;
}

/**
 * @brief Get maximum coordinate of the bounding box
 * @return Maximum coordinate of the bounding box
 */
openMVG::Vec3 AABB::max( void ) const
{
  return m_max ;
}

/**
 * @brief Update the bounding box such as it contains the given point
 * @param pt The point that will be inside the bounding box after update
 */
void AABB::update( const openMVG::Vec3 & pt )
{
  m_min[0] = std::min( m_min[0] , pt[0] ) ;
  m_min[1] = std::min( m_min[1] , pt[1] ) ;
  m_min[2] = std::min( m_min[2] , pt[2] ) ;

  m_max[0] = std::max( m_max[0] , pt[0] ) ;
  m_max[1] = std::max( m_max[1] , pt[1] ) ;
  m_max[2] = std::max( m_max[2] , pt[2] ) ;
}

/**
 * @brief Update the bounding box such as it contains the given pointset
 * @param pts The points that will be inside the bounding box after update
 */
void AABB::update( const std::vector< openMVG::Vec3 > & pts )
{
  for( const auto & pt : pts )
  {
    update( pt ) ;
  }
}

/**
 * @brief Update the bounding box such as it contains the bounding box
 * @param src The bounding box used to update the bounding box
 */
void AABB::update( const AABB & src )
{
  m_min[0] = std::min( m_min[0] , src.m_min[0] ) ;
  m_min[1] = std::min( m_min[1] , src.m_min[1] ) ;
  m_min[2] = std::min( m_min[2] , src.m_min[2] ) ;

  m_max[0] = std::max( m_max[0] , src.m_max[0] ) ;
  m_max[1] = std::max( m_max[1] , src.m_max[1] ) ;
  m_max[2] = std::max( m_max[2] , src.m_max[2] ) ;
}

/**
 * @brief Indicate if the bounding box contains the point
 * @param pt The point to test
 * @retval true If the point is inside the bounding box
 * @retval false If the point is outside the bounding box
 */
bool AABB::contains( const openMVG::Vec3 & pt ) const
{
  return m_min[0] <= pt[0] &&
         m_min[1] <= pt[1] &&
         m_min[2] <= pt[2] &&
         m_max[0] >= pt[0] &&
         m_max[1] >= pt[1] &&
         m_max[2] >= pt[2] ;
}

/**
 * @brief Indicate if the bounding box fully contains the given box
 * @param box The queried bounding box
 * @retval true If the box is (fully) inside the bounding box
 * @retval false If the box is outside the bounding box or partially overlap
 */
bool AABB::contains( const AABB & box ) const
{
  return ( m_min[0] <= box.m_min[0] && m_max[0] >= box.m_max[0] ) &&
         ( m_min[1] <= box.m_min[1] && m_max[1] >= box.m_max[1] ) &&
         ( m_min[2] <= box.m_min[2] && m_max[2] >= box.m_max[2] ) ;
}

/**
 * @brief Indicate if the bounding box fully contains the triangle
 * @param A first vertex of the triangle
 * @param B second vertex of the triangle
 * @param C third vertex of the triangle
 * @retval true if the triangle is fully inside the bounding box
 * @retval false if the triangle is not fully (perhaps partially) inside the bounding box
 */
bool AABB::contains( const openMVG::Vec3 & A ,
                     const openMVG::Vec3 & B ,
                     const openMVG::Vec3 & C ) const
{
  return contains( A ) && contains( B ) && contains( C ) ;
}

/**
 * @brief Indicate if the bounding box has non null intersection with the given one
 * @param box Bounding box to test
 * @retval true if the given box has non null intersection with the current box
 * @retval false if the given box has no intersection with the current box
 */
bool AABB::overlap( const AABB & box ) const
{
  const bool out_x = m_max[0] < box.m_min[0] || m_min[0] > box.m_max[0] ;
  const bool out_y = m_max[1] < box.m_min[1] || m_min[1] > box.m_max[1] ;
  const bool out_z = m_max[2] < box.m_min[2] || m_min[2] > box.m_max[2] ;

  return ! ( out_x || out_y || out_z || empty() || box.empty() ) ;
}

/**
 * @brief Indicate if the bounding box has non null intersection with the triangle
 * @param A first vertex of the triangle
 * @param B second vertex of the triangle
 * @param C third vertex of the triangle
 * @retval true if the triangle has intersection with the bounding box
 * @retval false if the triangle has no intersection with the bounding box
 */
bool AABB::overlap( const openMVG::Vec3 & A ,
                    const openMVG::Vec3 & B ,
                    const openMVG::Vec3 & C ) const
{
  // Note this is an implementation of Akenine Moller test
  // @see Christer Ericson - Real-time Collision Detection
  const openMVG::Vec3 c = center() ;
  const openMVG::Vec3 e = extends() * 0.5 ;

  const openMVG::Vec3 v0 = A - c ;
  const openMVG::Vec3 v1 = B - c ;
  const openMVG::Vec3 v2 = C - c ;

  const openMVG::Vec3 f0 = v1 - v0 ;
  const openMVG::Vec3 f1 = v2 - v1 ;
  const openMVG::Vec3 f2 = v0 - v2 ;


  const double f0x = std::abs( f0[0] ) ;
  const double f0y = std::abs( f0[1] ) ;
  const double f0z = std::abs( f0[2] ) ;
  const double f1x = std::abs( f1[0] ) ;
  const double f1y = std::abs( f1[1] ) ;
  const double f1z = std::abs( f1[2] ) ;
  const double f2x = std::abs( f2[0] ) ;
  const double f2y = std::abs( f2[1] ) ;
  const double f2z = std::abs( f2[2] ) ;
  const double v0x = v0[0] ;
  const double v0y = v0[1] ;
  const double v0z = v0[2] ;
  const double v1x = v1[0] ;
  const double v1y = v1[1] ;
  const double v1z = v1[2] ;
  const double v2x = v2[0] ;
  const double v2y = v2[1] ;
  const double v2z = v2[2] ;

  // Category 3

  // r  = e0 * | u0 . n | + e1 * | u1 . n | + e2 * | u2 . n |
  // p0 = n . v0
  // p1 = n . v1
  // p2 = n . v2
  // n = a00 | a01 | ... | a22

  /* Mathematica said :
     a00
     r00=e2 Abs[f0y]+e1 Abs[f0z]
     p0 = v0z v1y-v0y v1z
     p1 = v0z v1y-v0y v1z
     p2 = (v0z-v1z) v2y+(-v0y+v1y) v2z
  */
  {
    const double r = e[ 1 ] * f0z + e[ 2 ] * f0y ;
    const double p0 = v0z * v1y - v0y * v1z ;
    const double p2 = v2y * ( v0z - v1z ) + v2z * ( v1y - v0y ) ;

    if( std::max( - std::max( p0 , p2 ) , std::min( p0 , p2 ) ) > r )
    {
      return false ;
    }
  }

  /*
     a01
     r01=e2 Abs[f1y]+e1 Abs[f1z]
     p0 = v0z (-v1y+v2y)+v0y (v1z-v2z)
     p1 = v1z v2y-v1y v2z
     p2 = v1z v2y-v1y v2z
  */
  {
    const double r = e[ 1 ] * f1z + e[ 2 ] * f1y ;
    const double p0 = v0z * ( v2y - v1y ) + v0y * ( v1z - v2z ) ;
    const double p2 = v1z * v2y - v1y * v2z ;

    if( std::max( - std::max( p0 , p2 ) , std::min( p0 , p2 ) ) > r )
    {
      return false ;
    }
  }

  /*
     a02
     r02=e2 Abs[f2y]+e1 Abs[f2z]
     p0 = -v0z v2y+v0y v2z
     p1 = v1z (v0y-v2y)+v1y (-v0z+v2z)
     p2 = -v0z v2y+v0y v2z
  */
  {
    const double r = e[ 1 ] * f2z + e[ 2 ] * f2y ;
    const double p0 = v0y * v2z - v0z * v2y ;
    const double p2 = v1z * ( v0y - v2y ) + v1y * ( v2z - v0z ) ;

    if( std::max( - std::max( p0 , p2 ) , std::min( p0 , p2 ) ) > r )
    {
      return false ;
    }
  }

  /*
     a10
     r10=e2 Abs[f0x]+e0 Abs[f0z]
     p0 = -v0z v1x+v0x v1z
     p1 = -v0z v1x+v0x v1z
     p2 = -v0z v2x+v1z v2x+(v0x-v1x) v2z
  */
  {
    const double r = e[ 0 ] * f0z + e[ 2 ] * f0x ;
    const double p0 = v0x * v1z - v0z * v1x ;
    const double p2 = v2x * ( v1z - v0z ) + v2z * ( v0x - v1x ) ;

    if( std::max( - std::max( p0 , p2 ) , std::min( p0 , p2 ) ) > r )
    {
      return false ;
    }
  }

  /*
     a11
     r11=e2 Abs[f1x]+e0 Abs[f1z]
     p0 = v0z (v1x-v2x)+v0x (-v1z+v2z)
     p1 = -v1z v2x+v1x v2z
     p2 = -v1z v2x+v1x v2z
  */
  {
    const double r = e[ 0 ] * f1z + e[ 2 ] * f1x ;
    const double p0 = v0z * ( v1x - v2x ) + v0x * ( v2z - v1z ) ;
    const double p2 = v2z * v1x - v2x * v1z ;

    if( std::max( - std::max( p0 , p2 ) , std::min( p0 , p2 ) ) > r )
    {
      return false ;
    }
  }

  /*
     a12
     r12=e2 Abs[f2x]+e0 Abs[f2z]
     p0 = v0z v2x-v0x v2z
     p1 = v1z (-v0x+v2x)+v1x (v0z-v2z)
     p2 = v0z v2x-v0x v2z
  */
  {
    const double r  = e[ 0 ] * f2z + e[ 2 ] * f2x ;
    const double p0 = v0z * v2x - v0x * v2z ;
    const double p2 = v1z * ( v2x - v0x ) + v1x * ( v0z - v2z ) ;

    if( std::max( - std::max( p0 , p2 ) , std::min( p0 , p2 ) ) > r )
    {
      return false ;
    }
  }

  /*
     a20
     p0 = v0y v1x-v0x v1y
     p1 = v0y v1x-v0x v1y
     p2 = (v0y-v1y) v2x+(-v0x+v1x) v2y
  */
  {
    const double r  = e[ 0 ] * f0y + e[ 1 ] * f0x ;
    const double p0 = v0y * v1x - v0x * v1y ;
    const double p2 = v2x * ( v0y - v1y ) + v2y * ( v1x - v0x ) ;

    if( std::max( - std::max( p0 , p2 ) , std::min( p0 , p2 ) ) > r )
    {
      return false ;
    }
  }

  /*
     a21
     r21=e1 Abs[f1x]+e0 Abs[f1y]
     p0 = v0y (-v1x+v2x)+v0x (v1y-v2y)
     p1 = v1y v2x-v1x v2y
     p2 = v1y v2x-v1x v2y
  */
  {
    const double r  = e[ 0 ] * f1y + e[ 1 ] * f1x ;
    const double p0 = v0y * ( v2x - v1x ) + v0x * ( v1y - v2y ) ;
    const double p2 = v1y * v2x - v1x * v2y ;

    if( std::max( - std::max( p0 , p2 ) , std::min( p0 , p2 ) ) > r )
    {
      return false ;
    }
  }

  /*
     a22
     r22=e1 Abs[f2x]+e0 Abs[f2y]
     p0 = -v0y v2x+v0x v2y
     p1 = v1y (v0x-v2x)+v1x (-v0y+v2y)
     p2 = -v0y v2x+v0x v2y
  */
  {
    const double r  = e[ 0 ] * f2y + e[ 1 ] * f2x ;
    const double p0 = v0x * v2y - v0y * v2x ;
    const double p2 = v1y * ( v0x - v2x ) + v1x * ( v2y - v0y ) ;

    if( std::max( - std::max( p0 , p2 ) , std::min( p0 , p2 ) ) > r )
    {
      return false ;
    }
  }

  // Category 1 (bbox separating axis)
  if( std::max( std::max( v0x , v1x ) , v2x ) < -e[ 0 ] || std::min( std::min( v0x , v1x ) , v2x ) > e[ 0 ] )
  {
    return false ;
  }

  if( std::max( std::max( v0y , v1y ) , v2y ) < -e[ 1 ] || std::min( std::min( v0y , v1y ) , v2y ) > e[ 1 ] )
  {
    return false ;
  }

  if( std::max( std::max( v0z , v1z ) , v2z ) < -e[ 2 ] || std::min( std::min( v0z , v1z ) , v2z ) > e[ 2 ] )
  {
    return false ;
  }

  // Category 2 : supporting plane
  const openMVG::Vec3 cmin = m_min - c ;
  const openMVG::Vec3 cmax = m_max - c ;

  // The plane
  const openMVG::Vec3 pn = f0.cross( f1 ) ;
  const double pd = pn.dot( v0 ) ;

  // centered bounding box and it's extends
  const openMVG::Vec3 c2 = ( cmin + cmax ) * 0.5 ;
  const openMVG::Vec3 tmpe = cmax - c2 ;

  const double r =
    tmpe[ 0 ] * std::abs( pn[0] ) +
    tmpe[ 1 ] * std::abs( pn[1] ) +
    tmpe[ 2 ] * std::abs( pn[2] ) ;

  const double s = c2.dot( pn ) - pd ;

  const bool res = std::abs( s ) <= r ;

  return res ;
}


/**
 * @brief Indicate if AABB is fully contained in the frustum
 * @param fr Frustum
 * @retval true if the box is fully contained in the Frustum
 * @retval false if no intersection or only partially
 */
bool AABB::containedIn( const Frustum & fr ) const
{
  const openMVG::Vec3 c = center() ;
  const openMVG::Vec3 e = extends() ;

  for( auto & cur_plane : fr.planes )
  {
    const openMVG::Vec3 &pn = cur_plane.normal() ;

    // Projection of the interval
    const double r = e.dot( pn.cwiseAbs() ) ;

    // Distance of center wrt the plane
    const double s = cur_plane.signedDistance( c ) ;

    // If intersect of not on the good side -> cant be fully in
    if( std::abs( s ) <= r )
    {
      // It intersects
      return false ;
    }
    else
    {
      // No intersection but are we in the positive or negative ?
      if( s < 0 )
      {
        return false ;
      }
    }
  }

  // Here we pass all the planes with success
  // But we must be careful because fr may have no plane ?
  return fr.planes.size() > 0 ;
}

/**
 * @brief Indicate if the Frustum and the box have non empty intersection
 * @param fr Frustum
 * @retval true if the frustum and the bounding box have non empty intersection
 * @retval false if no intersection
 */
bool AABB::overlap( const Frustum & fr ) const
{
  const openMVG::Vec3 c = center() ;
  const openMVG::Vec3 e = extends() ;

  for( auto & cur_plane : fr.planes )
  {
    const openMVG::Vec3 &pn = cur_plane.normal() ;

    // Projection of the interval
    const double r = e.dot( pn.cwiseAbs() ) ;

    // Distance of center wrt the plane
    const double s = cur_plane.signedDistance( c ) ;

    // No intersection
    if( ! ( std::abs( s ) <= r ) )
    {
      // No intersection but are we in the positive or negative side of the plane ?
      if( s < 0 )
      {
        // Negative : It excludes the box, so stop now
        return false ;
      }
    }
  }

  // Here we pass all the planes with success
  // But we must be careful because fr may have no plane ?
  return fr.planes.size() > 0 ;
}


} // namespace geometry
} // namespace openMVG