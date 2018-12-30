#ifndef _OPENMVG_MVS_GENERATORS_HPP_
#define _OPENMVG_MVS_GENERATORS_HPP_

#include "openMVG/numeric/numeric.h"

#include <random>

namespace MVS
{

/**
 * @brief Base class for generators 
 * 
 */
class Generator
{
public:
  typedef std::mt19937 rng_type;
};

/**
 * @brief Class used to generate/perturb depth values 
 * 
 */
class DepthGenerator : public Generator
{
public:
  /**
   * @brief Construct a new Depth Generator object
   * 
   * @param min_depth   Lower bound of the depth to generate/perturb
   * @param max_depth   Upper bound of the depth to generate/perturb
   */
  DepthGenerator( const double min_depth, const double max_depth );

  /**
   * @brief Copy ctr 
   * 
   * @param src Source
   */
  DepthGenerator( const DepthGenerator& src ) = default;

  /**
   * @brief Move ctr 
   * 
   * @param src Source
   */
  DepthGenerator( DepthGenerator&& src ) = default;

  /**
   * @brief Assignment operator 
   * 
   * @param src     Source
   * @return        Self after assignment  
   */
  DepthGenerator& operator=( const DepthGenerator& src ) = default;

  /**
   * @brief Move assignment operator 
   * 
   * @param src     Source
   * @return        Self after assignment  
   */
  DepthGenerator& operator=( DepthGenerator&& src ) = default;

  /**
   * @brief Get minimum depth value 
   * 
   * @return    Lower bound of the depth generator/perturbator
   */
  double minDepth( void ) const;

  /**
   * @brief Get maximum depth value 
   * 
   * @return    Upper bound of the depth generator/perturbator 
   */
  double maxDepth( void ) const;

  /**
   * @brief Set the Minimum depth value 
   * 
   * @param d   The new lower bound of the depth generator/perturbator 
   */
  void setMinDepth( const double d );

  /**
   * @brief Set the Maximum depth value 
   * 
   * @param d   The new upper bound of the depth generator/perturbator 
   */
  void setMaxDepth( const double d );

  /**
   * @brief Generate random depth in the valid range
   * 
   * @param rng     Random number generator
   * @return        Random depth in the valid range
   */
  double random( rng_type& rng );

  /**
   * @brief Perturb a depth value 
   * 
   * @param initial_depth     Initial depth value to perturb
   * @param max_perturbation  Maximum amount of perturbation to apply to the initial_depth (in percentage: between 0-1)
   * @param rng               Random number generator 
   * @return double           Perturbed depth 
   */
  double perturb( const double initial_depth,
                  const double max_perturbation,
                  rng_type&    rng );

private:
  double m_min_depth;
  double m_max_depth;

  std::uniform_real_distribution<double> m_distrib_depth;
};

/**
 * @brief Class used to generate/perturb normal values 
 * 
 */
class NormalGenerator : public Generator
{
public:
  /**
   * @brief Construct a new Normal Generator 
   * 
   * @param max_angle   Maximum angle (in degree) used to generate normals around an axis 
   */
  NormalGenerator( const double max_angle = 80.0 );

  /**
   * @brief Generate random normal around an axis 
   * 
   * @param axis    Axis around which normals are generated 
   * @param rng     Random number generator 
   * @return The random normal 
   */
  openMVG::Vec3 random( const openMVG::Vec3& axis, rng_type& rng );

  /**
   * @brief Get maximum sampling angle 
   * 
   * @return The maximum sampling angle  
   */
  double maxAngle( void ) const;

  /**
   * @brieg Set the maximum sampling angle 
   * 
   * @param angle The nuew maximum sampling angle 
   */
  void setMaxAngle( const double angle );

  /**
   * @brief Perturb a normal 
   * 
   * @param initial_normal    The normal to perturb 
   * @param max_perturbation  The maximum amount of perturbation (wrt the maximum angle) - percentage value (between 0-1)
   * @param axis              The axis around which normal is perturbed 
   * @param rng               Random number generator 
   * @return The perturbed normal 
   */
  openMVG::Vec3 perturb( const openMVG::Vec3& initial_normal,
                         const double         max_perturbation,
                         const openMVG::Vec3& axis,
                         rng_type&            rng );

private:
  /**
   * @brief Generate random distribution around axis and maximum cosinus angle 
   * 
   * @param axis 
   * @param cos_max_angle 
   * @param rng 
   * @return openMVG::Vec3 
   */
  openMVG::Vec3 random( const openMVG::Vec3& axis, const double cos_max_angle, rng_type& rng );

  double                                 m_max_angle;     // Maximum sampling angle (in radian)
  double                                 m_cos_max_angle; // Cosinus of max angle
  std::uniform_real_distribution<double> m_distrib_1_1;   // Uniform distribution in range [-1;1]
  std::uniform_real_distribution<double> m_distrib_0_1;   // Uniform distribution in range [0;1];
};

} // namespace MVS

#endif