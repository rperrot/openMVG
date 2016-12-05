#ifndef _OPENMVG_MVS_DEPTHMAP_COMPUTATION_COMMON_HPP_
#define _OPENMVG_MVS_DEPTHMAP_COMPUTATION_COMMON_HPP_

#include "Image.hpp"
#include "DepthMapComputationParameters.hpp"

namespace MVS
{
  // Given a metric, get the image to load/save
  ImageLoadType ComputeLoadType( const cost_metric & metric ) ;
}

#endif