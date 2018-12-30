#include "Generators.hpp"

#include "Util.hpp"

#include <cmath>

namespace MVS
{

/**
   * @brief Construct a new Depth Generator object
   * 
   * @param min_depth   Lower bound of the depth to generate/perturb
   * @param max_depth   Upper bound of the depth to generate/perturb
   */
DepthGenerator::DepthGenerator( const double min_depth, const double max_depth )
    : m_min_depth( std::min( min_depth, max_depth ) ),
      m_max_depth( std::max( min_depth, max_depth ) ),
      m_distrib_depth( std::min( min_depth, max_depth ), std::max( min_depth, max_depth ) )
{
}

/**
  * @brief Get minimum depth value 
  * 
  * @return    Lower bound of the depth generator/perturbator
  */
double DepthGenerator::minDepth( void ) const
{
  return m_min_depth;
}

/**
   * @brief Get maximum depth value 
   * 
   * @return    Upper bound of the depth generator/perturbator 
   */
double DepthGenerator::maxDepth( void ) const
{
  return m_max_depth;
}

/**
   * @brief Set the Minimum depth value 
   * 
   * @param d   The new lower bound of the depth generator/perturbator 
   */
void DepthGenerator::setMinDepth( const double d )
{
  m_min_depth     = d;
  m_distrib_depth = std::uniform_real_distribution<double>( m_min_depth, m_max_depth );
}

/**
   * @brief Set the Maximum depth value 
   * 
   * @param d   The new upper bound of the depth generator/perturbator 
   */
void DepthGenerator::setMaxDepth( const double d )
{
  m_max_depth     = d;
  m_distrib_depth = std::uniform_real_distribution<double>( m_min_depth, m_max_depth );
}

/**
   * @brief Generate random depth in the valid range
   * 
   * @param rng     Random number generator
   * @return        Random depth in the valid range
   */
double DepthGenerator::random( DepthGenerator::rng_type& rng )
{
  return m_distrib_depth( rng );
}

/**
   * @brief Perturb a depth value 
   * 
   * @param initial_depth     Initial depth value to perturb
   * @param max_perturbation  Maximum amount of perturbation to apply to the initial_depth (in percentage: between 0-1)
   * @param rng               Random number generator 
   * @return double           Perturbed depth 
   */
double DepthGenerator::perturb( const double initial_depth, const double max_perturbation, DepthGenerator::rng_type& rng )
{
  const double tmp_min = ( 1 - max_perturbation ) * initial_depth;
  const double tmp_max = ( 1 + max_perturbation ) * initial_depth;

  const double min = std::min( tmp_min, tmp_max );
  const double max = std::max( tmp_min, tmp_max );

  std::uniform_real_distribution<double> distrib_depth( Clamp( min, m_min_depth, m_max_depth ), Clamp( max, m_min_depth, m_max_depth ) );

  return m_distrib_depth( rng );
}

/**
  * @brief Construct a new Normal Generator 
  * 
  * @param max_angle   Maximum angle (in degree) used to generate normals around an axis 
  */
NormalGenerator::NormalGenerator( const double max_angle )
    : m_max_angle( openMVG::D2R( max_angle ) ),
      m_cos_max_angle( std::cos( openMVG::D2R( max_angle ) ) ),
      m_distrib_1_1( -1.0, 1.0 ),
      m_distrib_0_1( 0.0, 1.0 )
{
}

/**
  * @brief Get maximum sampling angle 
  * 
  * @return The maximum sampling angle  
  */
double NormalGenerator::maxAngle( void ) const
{
  return m_max_angle;
}

/**
  * @brieg Set the maximum sampling angle 
  * 
  * @param angle The nuew maximum sampling angle 
  */
void NormalGenerator::setMaxAngle( const double angle )
{
  m_max_angle = angle;
}

/**
  * @brief Generate random normal around an axis 
  * 
  * @param axis    Axis around which normals are generated 
  * @param rng     Random number generator 
  * @return The random normal 
  */
openMVG::Vec3 NormalGenerator::random( const openMVG::Vec3& axis, rng_type& rng )
{
  return random( axis, m_cos_max_angle, rng );
}

/**
  * @brief Perturb a normal 
  * 
  * @param initial_normal    The normal to perturb 
  * @param max_perturbation  The maximum amount of perturbation (wrt the maximum angle) - percentage value (between 0-1)
  * @param axis              The axis around which normal is perturbed 
  * @param rng               Random number generator 
  * @return The perturbed normal 
  */
openMVG::Vec3 NormalGenerator::perturb( const openMVG::Vec3& initial_normal,
                                        const double         max_perturbation,
                                        const openMVG::Vec3& axis,
                                        rng_type&            rng )
{
  // rperrot: from pwmvs
  const double        perturbation_angle = max_perturbation * ( m_max_angle / 4.0 );
  const openMVG::Vec3 n                  = random( initial_normal, std::cos( perturbation_angle ), rng );
  const double        cos_a              = CosAngleBetween( n, axis );

  if ( cos_a >= m_cos_max_angle )
  {
    return n;
  }
  else
  {
    const double        angle  = std::acos( cos_a ) - m_max_angle;
    const openMVG::Vec3 n_axis = n.cross( axis );

    const openMVG::Mat3 R = Eigen::AngleAxis<double>( 2.0 * angle, n_axis ).toRotationMatrix();
    return ( R * n ).normalized();
  }
}

/**
  * @brief Generate random distribution around axis and maximum cosinus angle 
  * 
  * @param axis 
  * @param cos_max_angle 
  * @param rng 
  * @return openMVG::Vec3 
  */
openMVG::Vec3 NormalGenerator::random( const openMVG::Vec3& axis, const double cos_max_angle, rng_type& rng )
{
  static const double TWO_PI = 6.2831853071795864769252867665590057683943387987502;

  openMVG::Vec3 u, v;
  GenerateNormalizedFrame( axis, u, v );

  const double phi   = TWO_PI * m_distrib_0_1( rng );
  const double z     = m_distrib_0_1( rng ) * ( 1.0 - cos_max_angle ) + cos_max_angle;
  const double theta = std::acos( z );

  return std::sin( theta ) * ( std::cos( phi ) * u + std::sin( phi ) * v ) + std::cos( theta ) * axis;
}

} // namespace MVS