#include "DepthMapComputationCommon.hpp"

namespace MVS
{
  ImageLoadType ComputeLoadType( const cost_metric & metric )
  {
    if( metric == COST_METRIC_PM )
    {
      return ImageLoadType( IMAGE_GRAYSCALE | IMAGE_COLOR | IMAGE_GRADIENT ) ;
    }
    else if( metric == COST_METRIC_NCC )
    {
      return ImageLoadType( IMAGE_GRAYSCALE | IMAGE_COLOR ) ;
    }
    else if( metric == COST_METRIC_CENSUS )
    {
      return ImageLoadType( IMAGE_GRAYSCALE | IMAGE_COLOR | IMAGE_CENSUS ) ;
    }
    return IMAGE_ALL ;
  }
}