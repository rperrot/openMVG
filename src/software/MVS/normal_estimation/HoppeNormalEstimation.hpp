#ifndef _OPENMVG_MVS_NORMAL_ESTIMATION_HOPPE_NORMAL_ESTIMATION_HH_
#define _OPENMVG_MVS_NORMAL_ESTIMATION_HOPPE_NORMAL_ESTIMATION_HH_

#include "normal_estimation/AbstractNormalEstimation.hpp"

namespace MVS
{

/**
 * @brief This class computes normals using Hoppes method 
 * 
 * H. Hoppe, T. DeRose, T. Duchamp, J. McDonald, and W. Stuetzle
 * "Surface reconstruction from unorganized points."
 * ACM SIGGRAPH. 1992.
 */
class HoppeNormalEstimation : public AbstractNormalEstimation
{
public:
  /**
   * @brief Construct a new Hoppe Normal Computation object
   * 
   * @param size Size of the neighboring for normal estimation 
   */
  HoppeNormalEstimation( const int size = 7 );

  /**
   * @brief Destroy the Hoppe Normal Estimation object
   * 
   */
  virtual ~HoppeNormalEstimation() = default;

  /**
     * @brief Main function used to compute normal of a depth map
     * 
     * @param cam     Camera associated with the depth map 
     * @param map     Depth map to use
     * @param scale   Scale of the computation 
     */
  virtual void computeNormals( const Camera& cam, DepthMap& map, const int scale = -1 );

private:
  int m_size; // Size of the neighboring for computation
};

} // namespace MVS

#endif