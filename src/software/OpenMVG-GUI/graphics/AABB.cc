#include "AABB.hh"

#include <limits>

namespace openMVG_gui
{

/**
 * @brief ctr
 */
AABB::AABB( void ) :
    m_min( std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() ),
    m_max( -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),
           -std::numeric_limits<double>::max() )
{
}

/**
 * @brief ctr around a point
 */
AABB::AABB( const openMVG::Vec3 &pt ) : m_min( pt ), m_max( pt )
{
}

/**
 * @brief Full ctr
 */
AABB::AABB( const openMVG::Vec3 &min, const openMVG::Vec3 &max ) : m_min( min ), m_max( max )
{
}

/**
 * @brief Minimum
 */
openMVG::Vec3 AABB::min( void ) const
{
  return m_min;
}

/**
 * @brief Maximum
 */
openMVG::Vec3 AABB::max( void ) const
{
  return m_max;
}

/**
 * @brief Length in each dim
 */
openMVG::Vec3 AABB::extends( void ) const
{
  return ( m_max - m_min );
}

/**
 * @brief center of the bounding box
 */
openMVG::Vec3 AABB::center( void ) const
{
  return ( m_min + m_max ) / 2.0;
}

/**
 * @brief Union of two bounding boxes
 */
AABB makeUnion( const AABB &a, const AABB &b )
{
  const openMVG::Vec3 min = {std::min( a.m_min[ 0 ], b.m_min[ 0 ] ), std::min( a.m_min[ 1 ], b.m_min[ 1 ] ),
                             std::min( a.m_min[ 2 ], b.m_min[ 2 ] )};

  const openMVG::Vec3 max = {std::max( a.m_max[ 0 ], b.m_max[ 0 ] ), std::max( a.m_max[ 1 ], b.m_max[ 1 ] ),
                             std::max( a.m_max[ 2 ], b.m_max[ 2 ] )};

  return AABB( min, max );
}

/**
 * @brief Union of a bounding box and a point
 */
AABB makeUnion( const AABB &a, const openMVG::Vec3 &pt )
{
  const openMVG::Vec3 min = {std::min( a.m_min[ 0 ], pt[ 0 ] ), std::min( a.m_min[ 1 ], pt[ 1 ] ),
                             std::min( a.m_min[ 2 ], pt[ 2 ] )};
  const openMVG::Vec3 max = {std::max( a.m_max[ 0 ], pt[ 0 ] ), std::max( a.m_max[ 1 ], pt[ 1 ] ),
                             std::max( a.m_max[ 2 ], pt[ 2 ] )};

  return AABB( min, max );
}

/**
 * @brief Union of a bounding box and a point
 */
AABB makeUnion( const openMVG::Vec3 &pt, const AABB &a )
{
  return makeUnion( a, pt );
}

/**
 * @brief Perform intersection with the bounding box
 */
Intersection AABB::intersect( const Ray &ray ) const
{
  const double inv_x = ray.inverse()[ 0 ];
  const double inv_y = ray.inverse()[ 1 ];
  const double inv_z = ray.inverse()[ 2 ];

  double tmin = ( m_min[ 0 ] - ray.origin()[ 0 ] ) * inv_x;
  double tmax = ( m_max[ 0 ] - ray.origin()[ 0 ] ) * inv_x;

  if ( tmin > tmax )
  {
    std::swap( tmin, tmax );
  }

  double tminy = ( m_min[ 1 ] - ray.origin()[ 1 ] ) * inv_y;
  double tmaxy = ( m_max[ 1 ] - ray.origin()[ 1 ] ) * inv_y;

  if ( tminy > tmaxy )
  {
    std::swap( tminy, tmaxy );
  }

  if ( ( tmin > tmaxy ) || ( tminy > tmax ) )
    return Intersection();

  if ( tminy > tmin )
    tmin = tminy;

  if ( tmaxy < tmax )
    tmax = tmaxy;

  double tminz = ( m_min[ 2 ] - ray.origin()[ 2 ] ) * inv_z;
  double tmaxz = ( m_max[ 2 ] - ray.origin()[ 2 ] ) * inv_z;

  if ( tminz > tmaxz )
  {
    std::swap( tminz, tmaxz );
  }

  if ( ( tmin > tmaxz ) || ( tminz > tmax ) )
    return Intersection();

  if ( tminz > tmin )
    tmin = tminz;

  if ( tmaxz < tmax )
    tmax = tmaxz;

  if ( tmin < 0 && tmax < 0 )
  {
    return Intersection();
  }
  if ( tmin < 0.0 )
  {
    tmin = tmax;
  }

  return Intersection( true, nullptr, tmin );
}

/**
 * @brief Output box to stream
 *
 * @param out Input/output stream
 * @param box Bounding box to send to the stream
 * @return stream after box sent have been sent to it
 */
std::ostream &operator<<( std::ostream &out, const AABB &box )
{
  out << "Box : [m:(" << box.m_min[ 0 ] << "," << box.m_min[ 1 ] << "," << box.m_min[ 2 ] << ") - M:(" << box.m_max[ 0 ]
      << "," << box.m_max[ 1 ] << "," << box.m_max[ 2 ] << ")]";
  return out;
}

} // namespace openMVG_gui
