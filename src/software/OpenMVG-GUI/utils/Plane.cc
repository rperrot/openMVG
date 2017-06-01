#include "Plane.hh"

namespace openMVG_gui
{
/**
* @brief Constructor
* @param Plane with a specified normal passing through a point
*/
Plane::Plane( const openMVG::Vec3 & n , const openMVG::Vec3 & pt )
  : m_n( n ) ,
    m_p( pt )
{

}

/**
* @brief Intersection distance of plane and a line
* @param a First point of the line
* @param b Second point of the line
* @return Distance of the intersection point between line and plane
* @note distance of the intersection is wrt to a
*/
double Plane::intersectionDistance( const openMVG::Vec3 & a , const openMVG::Vec3 & b ) const
{
  return ( m_p - a ).dot( m_n ) / ( b - a ).dot( m_n ) ;
}

/**
* @brief Compute Intersection point of a plane and a line
* @param a First point of the line
* @param b Second point of the line
* @return Intersection point of the line and the plane
*/
openMVG::Vec3 Plane::intersectionPosition( const openMVG::Vec3 & a , const openMVG::Vec3 & b ) const
{
  const openMVG::Vec3 l = b - a ;
  const double d = ( m_p - a ).dot( m_n ) / l.dot( m_n ) ;

  return a + d * l ;
}

} // namespace openMVG_gui