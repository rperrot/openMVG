// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_GPU_IMAGE_GPU_FILTERING_HPP
#define OPENMVG_IMAGE_GPU_IMAGE_GPU_FILTERING_HPP

#include "openMVG/system/gpu/OpenCLContext.hpp"

namespace openMVG
{
namespace image
{
namespace gpu
{

/* CENTRAL DIFFERENCE */

// X derivative
/**
 * @brief Compute X derivative using central difference
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageXDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;

/**
 * @brief Compute X derivative using central difference
 * @param[out] res Derivative image (should be allocated before calling this function)
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @retval true if computation is of
 * @retval false if computation fails
 */
bool ImageXDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true );

// Y derivative
/**
 * @brief Compute Y derivative using central difference
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageYDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;

/**
 * @brief Compute Y derivative using central difference
 * @param[out] res Derivative image (should be allocated before calling this function)
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @retval true if computation is of
 * @retval false if computation fails
 */
bool ImageYDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true );

/* SOBEL */

/**
 * @brief Compute Sobel X derivative
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageSobelXDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;

/**
 * @brief Compute Sobel X derivative
 * @param[out] res Derivative image
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageSobelXDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;

/**
 * @brief Compute Sobel Y derivative
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageSobelYDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;

/**
 * @brief Compute Sobel Y derivative
 * @param[out] res Derivative image
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageSobelYDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;

/* SCHARR */

/**
 * @brief Compute Scharr X derivative
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageScharrXDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;

/**
 * @brief Compute Scharr X derivative
 * @param[out] res Derivative image
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageScharrXDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;

/**
 * @brief Compute Scharr Y derivative
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageScharrYDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;

/**
 * @brief Compute Scharr Y derivative
 * @param[out] res Derivative image
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageScharrYDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;


/**
 * @brief Compute X-derivative using scaled Scharr filter
 * @param img Input image
 * @param scale scale of filter (1 -> 3x3 filter; 2 -> 5x5, ...)
 * @param ctx OpenCL Context
 * @param bNormalize true if kernel must be normalized
 * @return derivative image
 */
cl_mem ImageScaledScharrXDerivative( cl_mem img , const int scale , openMVG::system::gpu::OpenCLContext & ctx , const bool bNormalize = true ) ;

/**
 * @brief Compute X-derivative using scaled Scharr filter
 * @param[out] res Derivative image
 * @param img Input image
 * @param scale scale of filter (1 -> 3x3 filter; 2 -> 5x5, ...)
 * @param ctx OpenCL Context
 * @param bNormalize true if kernel must be normalized
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageScaledScharrXDerivative( cl_mem res , cl_mem img , const int scale , openMVG::system::gpu::OpenCLContext & ctx , const bool bNormalize = true ) ;

/**
 * @brief Compute X-derivative using scaled Scharr filter
 * @param img Input image
 * @param scale scale of filter (1 -> 3x3 filter; 2 -> 5x5, ...)
 * @param ctx OpenCL Context
 * @param bNormalize true if kernel must be normalized
 * @return derivative image
 */
cl_mem ImageScaledScharrYDerivative( cl_mem img , const int scale , openMVG::system::gpu::OpenCLContext & ctx , const bool bNormalize = true ) ;

/**
 * @brief Compute X-derivative using scaled Scharr filter
 * @param[out] res Derivative image
 * @param img Input image
 * @param scale scale of filter (1 -> 3x3 filter; 2 -> 5x5, ...)
 * @param ctx OpenCL Context
 * @param bNormalize true if kernel must be normalized
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageScaledScharrYDerivative( cl_mem res , cl_mem img , const int scale , openMVG::system::gpu::OpenCLContext & ctx , const bool bNormalize = true ) ;


/**
 * @brief Compute (isotropic) gaussian filtering of an image using filter width of k * sigma
 * @param img Input image
 * @param sigma standard deviation of kernel
 * @param ctx OpenCL Context
 * @param k confidence interval param - kernel is width k * sigma * 2 + 1 -- using k = 3 gives 99% of gaussian curve
 * @return Gaussian filtered image
 */
cl_mem ImageGaussianFilter( cl_mem img , const double sigma , openMVG::system::gpu::OpenCLContext & ctx , const int k = 3 ) ;

/**
 * @brief Compute (isotropic) gaussian filtering of an image using filter width of k * sigma
 * @param img Input image
 * @param[out] res Gaussian filtered image
 * @param sigma standard deviation of kernel
 * @param ctx OpenCL Context
 * @param k confidence interval param - kernel is width k * sigma * 2 + 1 -- using k = 3 gives 99% of gaussian curve
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageGaussianFilter( cl_mem res , cl_mem img , const double sigma , openMVG::system::gpu::OpenCLContext & ctx , const int k = 3 ) ;

/**
 * @brief Compute (isotropic) gaussian filtering of an image using filter width of k * sigma
 * @param img Input image
 * @param[out] res Gaussian filtered image
 * @param sigma standard deviation of kernel
 * @param ctx OpenCL Context
 * @param offset_region Region of the input image to work with
 * @param region_size Region size of the input image to work with
 * @param k confidence interval param - kernel is width k * sigma * 2 + 1 -- using k = 3 gives 99% of gaussian curve
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageGaussianFilter( cl_mem res ,
                          cl_mem img ,
                          const double sigma ,
                          const size_t offset_region[2] ,
                          const size_t region_size[2] ,
                          openMVG::system::gpu::OpenCLContext & ctx ,
                          const int k = 3 ) ;


/**
 * @brief Compute gaussian filtering of an image using user defined filter widths
 * @param img Input image
 * @param sigma standard deviation of kernel
 * @param kernel_size_x Size of horizontal kernel (must be an odd number or 0 for automatic computation)
 * @param kernel_size_y Size of vertical kernel (must be an add number or 0 for automatic computation)
 * @param ctx OpenCL Context
 * @return Gaussian filtered image
 */
cl_mem ImageGaussianFilter( cl_mem img , const double sigma , const size_t kernel_size_x , const size_t kernel_size_y , openMVG::system::gpu::OpenCLContext & ctx ) ;


/**
 * @brief Compute gaussian filtering of an image using user defined filter widths
 * @param[out] Gaussian filtered image
 * @param img Input image
 * @param sigma standard deviation of kernel
 * @param kernel_size_x Size of horizontal kernel (must be an odd number or 0 for automatic computation)
 * @param kernel_size_y Size of vertical kernel (must be an add number or 0 for automatic computation)
 * @param ctx OpenCL Context
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageGaussianFilter( cl_mem res , cl_mem img , const double sigma , const size_t kernel_size_x , const size_t kernel_size_y , openMVG::system::gpu::OpenCLContext & ctx ) ;


/**
 * @brief Compute gaussian filtering of an image using user defined filter widths
 * @param[out] Gaussian filtered image
 * @param img Input image
 * @param sigma standard deviation of kernel
 * @param kernel_size_x Size of horizontal kernel (must be an odd number or 0 for automatic computation)
 * @param kernel_size_y Size of vertical kernel (must be an add number or 0 for automatic computation)
 * @param ctx OpenCL Context
 * @param offset_region Region of the input image to work with
 * @param region_size Region size of the input image to work with
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageGaussianFilter( cl_mem res , cl_mem img , const double sigma , const size_t kernel_size_x , const size_t kernel_size_y , openMVG::system::gpu::OpenCLContext & ctx , const size_t offset_region[2] , const size_t region_size[2] ) ;


} // namespace gpu
} // namespace image
} // namespace openMVG

#endif