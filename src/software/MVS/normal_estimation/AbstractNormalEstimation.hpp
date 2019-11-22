#ifndef _OPENMVG_MVS_NORMAL_ESTIMATION_ABSTRACT_NORMAL_ESTIMATION_HPP_
#define _OPENMVG_MVS_NORMAL_ESTIMATION_ABSTRACT_NORMAL_ESTIMATION_HPP_

#include "Camera.hpp"
#include "DepthMap.hpp"

namespace MVS
{

/**
 * @brief Abstract class for normal computation of a depth map 
 */
class AbstractNormalEstimation
{
public:
  virtual ~AbstractNormalEstimation() = default;

  /**
     * @brief Main function used to compute normal of a depth map
     * 
     * @param cam     Camera associated with the depth map 
     * @param map     Depth map to use (if normals exists, it would be replaced)
     * @param scale   Scale of the computation 
     */
  virtual void computeNormals( const Camera& cam,
                               DepthMap&     map,
                               const int     scale = -1 ) = 0;

private:
};

} // namespace MVS

#endif