#include "DepthMapComputationCommon.hpp"

namespace MVS
{
  ImageLoadType ComputeLoadType( const cost_metric & metric )
  {
    switch( metric )
    {
      case COST_METRIC_PM :
      {
        return ImageLoadType( IMAGE_GRAYSCALE | IMAGE_COLOR | IMAGE_GRADIENT ) ;
      }
      case COST_METRIC_NCC:
      {
        return ImageLoadType( IMAGE_GRAYSCALE | IMAGE_COLOR ) ;
      }
      case COST_METRIC_CENSUS: 
      {
        return ImageLoadType( IMAGE_GRAYSCALE | IMAGE_COLOR | IMAGE_CENSUS ) ;
      }
      default: 
      {
        return IMAGE_ALL ;
      }
    }
  }
}