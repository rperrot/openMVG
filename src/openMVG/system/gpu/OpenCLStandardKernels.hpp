// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_SYSTEM_GPU_STANDARD_KERNELS_HPP
#define OPENMVG_SYSTEM_GPU_STANDARD_KERNELS_HPP

#include <string>
#include <vector>

namespace openMVG
{
namespace system
{
namespace gpu
{

// List of standard kernels
enum OpenCLStandardKernels
{
  OPENCL_STANDARD_KERNELS_BEGIN = 0 ,

  // Arithmetic
  OPENCL_STANDARD_KERNELS_IMAGE_ADD ,
  OPENCL_STANDARD_KERNELS_IMAGE_SUB ,
  OPENCL_STANDARD_KERNELS_IMAGE_MUL ,

  // Convolution
  OPENCL_STANDARD_KERNELS_CONVOLVE_2D ,
  OPENCL_STANDARD_KERNELS_HORIZONTAL_CONVOLUTION ,
  OPENCL_STANDARD_KERNELS_VERTICAL_CONVOLUTION ,

  // Derivatives
  OPENCL_STANDARD_KERNELS_X_UNNORMALIZED_DERIVATIVE_CENTRAL_DIFFERENCE ,
  OPENCL_STANDARD_KERNELS_X_UNNORMALIZED_DERIVATIVE_SOBEL ,
  OPENCL_STANDARD_KERNELS_X_UNNORMALIZED_DERIVATIVE_SCHARR ,

  OPENCL_STANDARD_KERNELS_X_NORMALIZED_DERIVATIVE_CENTRAL_DIFFERENCE ,
  OPENCL_STANDARD_KERNELS_X_NORMALIZED_DERIVATIVE_SOBEL ,
  OPENCL_STANDARD_KERNELS_X_NORMALIZED_DERIVATIVE_SCHARR ,

  OPENCL_STANDARD_KERNELS_Y_UNNORMALIZED_DERIVATIVE_CENTRAL_DIFFERENCE ,
  OPENCL_STANDARD_KERNELS_Y_UNNORMALIZED_DERIVATIVE_SOBEL ,
  OPENCL_STANDARD_KERNELS_Y_UNNORMALIZED_DERIVATIVE_SCHARR ,

  OPENCL_STANDARD_KERNELS_Y_NORMALIZED_DERIVATIVE_CENTRAL_DIFFERENCE ,
  OPENCL_STANDARD_KERNELS_Y_NORMALIZED_DERIVATIVE_SOBEL ,
  OPENCL_STANDARD_KERNELS_Y_NORMALIZED_DERIVATIVE_SCHARR ,


  // Resampling
  OPENCL_STANDARD_KERNELS_RESAMPLING ,

  // /!\ Do not change the order of the following two lines (and do not place something after or between)
  OPENCL_STANDARD_KERNELS_END ,
  OPENCL_STANDARD_KERNELS_NONE , 
  OPENCL_STANDARD_KERNELS_ALL 
} ;


/**
 * @brief Compute a program according to the list of kernel to use
 * @param kernels list of kernels to use
 * @return String corresponding to an OpenCL source code where all kernels are in
 */
std::string ComputeOpenCLProgramSource( const std::vector< OpenCLStandardKernels > & kernels ) ;


} // namespace gpu
} // namespace system
} // namespace openMVG

#endif