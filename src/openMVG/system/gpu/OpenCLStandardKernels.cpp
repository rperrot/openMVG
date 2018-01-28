// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OpenCLStandardKernels.hpp"

#include "openMVG/image/gpu/kernels/image_gpu_arithmetic_kernels.hpp"
#include "openMVG/image/gpu/kernels/image_gpu_convolution_kernels.hpp"
#include "openMVG/image/gpu/kernels/image_gpu_filtering_kernels.hpp"
#include "openMVG/image/gpu/kernels/image_gpu_resampling_kernels.hpp"

#include <sstream>

using namespace openMVG::image::gpu::kernels;

namespace openMVG
{
namespace system
{
namespace gpu
{

/**
 * @brief Compute a program according to the list of kernel to use
 * @param kernels list of kernels to use
 * @return String corresponding to an OpenCL source code where all kernels are in
 */
std::string ComputeOpenCLProgramSource( const std::vector< OpenCLStandardKernels > & kernels )
{
  // Check for Meta value ALL
  for( const auto & it : kernels )
  {
    if( it == OPENCL_STANDARD_KERNELS_ALL )
    {
      std::vector< OpenCLStandardKernels > krns ;
      for( auto i = OPENCL_STANDARD_KERNELS_BEGIN + 1 ; i < OPENCL_STANDARD_KERNELS_END ; ++i )
      {
        krns.emplace_back( OpenCLStandardKernels( i ) ) ;
      }
      return ComputeOpenCLProgramSource( krns ) ;
    }
  }


  std::stringstream res ;

  res << "#define WORK_GROUP_SIZE 16" << std::endl ;

  for( const auto & it : kernels )
  {
    switch( it )
    {
      case OPENCL_STANDARD_KERNELS_IMAGE_ADD:
      {
        res << krnsImageAdd << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_IMAGE_SUB:
      {
        res << krnsImageSub << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_IMAGE_MUL:
      {
        res << krnsImageMul << std::endl;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_CONVOLVE_2D:
      {
        res << krnsImageConvolve2dNaive << std::endl ;
        res << krnsImageConvolve2dLocalMem << std::endl ;
        break;
      }
      case OPENCL_STANDARD_KERNELS_HORIZONTAL_CONVOLUTION:
      {
        res << krnsImageHorizontalConvolveNaive << std::endl ;
        res << krnsImageHorizontalConvolveLocalMem32 << std::endl ;
        res << krnsImageHorizontalConvolveNaiveRegion << std::endl ;
        res << krnsImageHorizontalConvolveLocalMem32Region << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_VERTICAL_CONVOLUTION:
      {
        res << krnsImageVerticalConvolveNaive << std::endl ;
        res << krnsImageVerticalConvolveLocalMem32 << std::endl ;
        res << krnsImageVerticalConvolveNaiveRegion << std::endl ;
        res << krnsImageVerticalConvolveLocalMem32Region << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_X_NORMALIZED_DERIVATIVE_CENTRAL_DIFFERENCE:
      {
        res << krnsImageFilteringDerivativeXNormalizedCentralDiff << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_X_NORMALIZED_DERIVATIVE_SOBEL:
      {
        res << krnsImageFilteringDerivativeXNormalizedSobel << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_X_NORMALIZED_DERIVATIVE_SCHARR:
      {
        res << krnsImageFilteringDerivativeXNormalizedScharr << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_X_UNNORMALIZED_DERIVATIVE_CENTRAL_DIFFERENCE:
      {
        res << krnsImageFilteringDerivativeXUnnormalizedCentralDiff << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_X_UNNORMALIZED_DERIVATIVE_SOBEL:
      {
        res << krnsImageFilteringDerivativeXUnnormalizedSobel << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_X_UNNORMALIZED_DERIVATIVE_SCHARR:
      {
        res << krnsImageFilteringDerivativeXUnnormalizedScharr << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_Y_NORMALIZED_DERIVATIVE_CENTRAL_DIFFERENCE:
      {
        res << krnsImageFilteringDerivativeYNormalizedCentralDiff << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_Y_NORMALIZED_DERIVATIVE_SOBEL:
      {
        res << krnsImageFilteringDerivativeYNormalizedSobel << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_Y_NORMALIZED_DERIVATIVE_SCHARR:
      {
        res << krnsImageFilteringDerivativeYNormalizedScharr << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_Y_UNNORMALIZED_DERIVATIVE_CENTRAL_DIFFERENCE:
      {
        res << krnsImageFilteringDerivativeYUnnormalizedCentralDiff << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_Y_UNNORMALIZED_DERIVATIVE_SOBEL:
      {
        res << krnsImageFilteringDerivativeYUnnormalizedSobel << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_Y_UNNORMALIZED_DERIVATIVE_SCHARR:
      {
        res << krnsImageFilteringDerivativeYUnnormalizedScharr << std::endl ;
        break ;
      }
      case OPENCL_STANDARD_KERNELS_RESAMPLING:
      {
        res << krnsImageResamplingDecimate << std::endl ;
        res << krnsImageResamplingUpsample << std::endl ;
        break ;
      }
      default:
      {
        break ;
      }
    }
  }

  return res.str() ;
}

} // namespace gpu
} // namespace system
} // namespace openMVG
