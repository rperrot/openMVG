#include "Ray.hh"

namespace openMVG_gui
{
/**
 * @brief Ctr
 * @param origin Origin of the ray in 3d
 * @param direction Direction of the ray
 */
Ray::Ray( const openMVG::Vec3 &origin, const openMVG::Vec3 &direction ) :
    m_origin( origin ), m_direction( direction.normalized() )
{
  m_inv_dir   = m_direction.array().cwiseInverse();
  m_sign[ 0 ] = m_inv_dir[ 0 ] < 0.0;
  m_sign[ 1 ] = m_inv_dir[ 1 ] < 0.0;
  m_sign[ 2 ] = m_inv_dir[ 2 ] < 0.0;
}

/**
 * @brief Get origin of the ray
 * @return 3d position of the ray origin
 */
openMVG::Vec3 Ray::origin( void ) const
{
  return m_origin;
}

/**
 * @brief Get direction of the ray
 * @retun 3d vector direction of the ray
 * @note Ray direction is unnormalized
 */
openMVG::Vec3 Ray::direction( void ) const
{
  return m_direction;
}

/**
 * @brief Get reciprocal of the direction (ie: 1.0 / direction() for each axis)
 *
 * @return reciprocal
 */
openMVG::Vec3 Ray::inverse( void ) const
{
  return m_inv_dir;
}

/**
 * @brief Get sign of the inverse of the direction vector
 *
 * @return sign( 1.0 / direction[i] ) for i in {0,1,2}
 */
std::array<int, 3> Ray::signInvDirection( void ) const
{
  return m_sign;
}

/**
 * @brief Compute point with a given parameter
 * @param t Parameter
 * @return 3d point corresponding to the parameter
 * @note Point is equal to Origin + t * Direction
 */
openMVG::Vec3 Ray::operator()( const double t ) const
{
  return m_origin + t * m_direction;
}

/**
 * @brief Output to stream
 *
 * @param str Input/Output stream
 * @param ray The ray to output
 * @return std::ostream& stream after output
 */
std::ostream &operator<<( std::ostream &str, const Ray &ray )
{
  str << "Ray : [O:(" << ray.m_origin[ 0 ] << "," << ray.m_origin[ 1 ] << "," << ray.m_origin[ 2 ] << ") - D:("
      << ray.m_direction[ 0 ] << "," << ray.m_direction[ 1 ] << "," << ray.m_direction[ 2 ] << ") - ";
  str << "I:(" << ray.m_inv_dir[ 0 ] << "," << ray.m_inv_dir[ 1 ] << "," << ray.m_inv_dir[ 2 ] << ")]";
  return str;
}

} // namespace openMVG_gui
