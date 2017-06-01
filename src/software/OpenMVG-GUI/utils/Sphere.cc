#include "Sphere.hh"

namespace openMVG_gui
{


/**
* @brief Ctr
* @param center Center of the sphere
* @param rad Radius of the sphere
*/
Sphere::Sphere( const openMVG::Vec3 & center , const double radius )
  : m_center( center ) ,
    m_radius( radius )
{

}

/*
* @brief Compute intersection point (if it exists) with a line
* @param a First point
* @param b Second point
* @param[out] pt Intersection point
* @retval true if intersection exists
* @retval false if no intersection
*/
bool Sphere::intersect( const openMVG::Vec3 & a ,
                        const openMVG::Vec3 & b ,
                        openMVG::Vec3 & pt ) const
{
  const openMVG::Vec3 L = m_center - a ;
  const openMVG::Vec3 dir = ( b - a ).normalized() ;
  const double tca = L.dot( dir ) ;

  if( tca < 0.0 )
  {
    return false ;
  }

  const double d2 = L.dot( L ) - tca * tca ;
  if( d2 > m_radius * m_radius )
  {
    return false ;
  }

  const double thc = std::sqrt( m_radius * m_radius - d2 ) ;

  double t0 = tca - thc ;
  double t1 = tca + thc ;

  // Get the nearest
  if( t0 > t1 )
  {
    std::swap( t0 , t1 ) ;
  }

  if( t0 < 0.0 )
  {
    if( t1 < 0.0 )
    {
      return false ;
    }

    pt = a + t1 * dir ;
  }
  pt = a + t0 * dir ;
  return true ;
}

} // namespace openMVG_gui