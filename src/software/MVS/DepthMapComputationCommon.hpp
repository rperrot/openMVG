#ifndef _OPENMVG_MVS_DEPTHMAP_COMPUTATION_COMMON_HPP_
#define _OPENMVG_MVS_DEPTHMAP_COMPUTATION_COMMON_HPP_

#include "DepthMapComputationParameters.hpp"
#include "Image.hpp"

namespace MVS
{
// Given a metric, get the images to load/save/compute
ImageLoadType ComputeLoadType( const cost_metric & metric ) ;

} // namespace MVS

#endif