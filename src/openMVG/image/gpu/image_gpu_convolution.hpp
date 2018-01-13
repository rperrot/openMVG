// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_GPU_IMAGE_GPU_ARITHMETICS_HPP
#define OPENMVG_IMAGE_GPU_IMAGE_GPU_ARITHMETICS_HPP

#include "openMVG/system/gpu/OpenCLContext.hpp"

#include "openMVG/numeric/numeric.h"

namespace openMVG
{
namespace image
{
namespace gpu
{

// Convolution 2d

/**
 * @brief Perform OpenCL 2d convolution
 * @param img Input image
 * @param kernel Convolution kernel
 * @param ctx OpenCL context
 * @return convolution result
 * @note for out of range values, copy edge value
 */
cl_mem ImageConvolution2d( cl_mem img , const openMVG::Mat & kernel , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform OpenCL 2d convolution
 * @param img Input image
 * @param kernel Convolution kernel
 * @param ctx OpenCL context
 * @return convolution result
 * @note for out of range values, copy edge value
 */
cl_mem ImageConvolution2d( cl_mem img , cl_mem kernel , const size_t kernel_w , const size_t kernel_h , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform OpenCL 2d convolution
 * @param[out] res convolution result
 * @param img Input image
 * @param kernel Convolution kernel
 * @param ctx OpenCL context
 * @note for out of range values, copy edge value
 */
bool ImageConvolution2d( cl_mem res , cl_mem img , const openMVG::Mat & kernel , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform OpenCL 2d convolution
 * @param[out] res convolution result
 * @param img Input image
 * @param kernel Convolution kernel
 * @param ctx OpenCL context
 * @note for out of range values, copy edge value
 */
bool ImageConvolution2d( cl_mem res , cl_mem img , cl_mem kernel , const size_t kernel_w , const size_t kernel_h , openMVG::system::gpu::OpenCLContext & ctx ) ;

// Horizontal convolution
/**
 * @brief Perform Horizontal convolution by a given kernel
 * @param img Image to convolve
 * @param kernel Kernel
 * @param ctx OpenCL context
 * @return Convolved image
 */
cl_mem ImageHorizontalConvolution( cl_mem img , const openMVG::Vec & kernel , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform Horizontal convolution by a given kernel
 * @param img Image to convolve
 * @param kernel Kernel
 * @param kernel_w Kernel size
 * @param ctx OpenCL context
 * @return Convolved image
 */
cl_mem ImageHorizontalConvolution( cl_mem img , cl_mem kernel , const size_t kernel_w , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform Horizontal convolution by a given kernel
 * @param[out] res Convolved image (need to be already allocated)
 * @param img Image to convolve
 * @param kernel Kernel
 * @param ctx OpenCL context
 */
bool ImageHorizontalConvolution( cl_mem res , cl_mem img , const openMVG::Vec & kernel , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform Horizontal convolution by a given kernel
 * @param[out] res Convolved image (need to be already allocated)
 * @param img Image to convolve
 * @param kernel Kernel
 * @param kernel_w Kernel size
 * @param ctx OpenCL context
 */
bool ImageHorizontalConvolution( cl_mem res , cl_mem img , cl_mem kernel , const size_t kernel_w , openMVG::system::gpu::OpenCLContext & ctx ) ;

// Vertical convolution
/**
 * @brief Perform Vertical convolution by a given kernel
 * @param img Image to convolve
 * @param kernel Kernel
 * @param ctx OpenCL context
 * @return Convolved image
 */
cl_mem ImageVerticalConvolution( cl_mem img , const openMVG::Vec & kernel , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform Vertical convolution by a given kernel
 * @param img Image to convolve
 * @param kernel Kernel
 * @param kernel_w Kernel size
 * @param ctx OpenCL context
 * @return Convolved image
 */
cl_mem ImageVerticalConvolution( cl_mem img , cl_mem kernel , const size_t kernel_w , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform Vertical convolution by a given kernel
 * @param[out] res Convolved image (need to be already allocated)
 * @param img Image to convolve
 * @param kernel Kernel
 * @param ctx OpenCL context
 */
bool ImageVerticalConvolution( cl_mem res , cl_mem img , const openMVG::Vec & kernel , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform Vertical convolution by a given kernel
 * @param[out] res Convolved image (need to be already allocated)
 * @param img Image to convolve
 * @param kernel Kernel
 * @param kernel_w Kernel size
 * @param ctx OpenCL context
 */
bool ImageVerticalConvolution( cl_mem res ,
                               cl_mem img ,
                               cl_mem kernel ,
                               const size_t kernel_w ,
                               openMVG::system::gpu::OpenCLContext & ctx ) ;

// Separable convolution
/**
 * @brief Perform Separable convolution
 * @param img Image to convolve
 * @param hKernel Horizontal kernel
 * @param vKernel Vertical kernel
 * @param ctx OpenCL context
 * @return convolved image
 */
cl_mem ImageSeparableConvolution( cl_mem img ,
                                  const openMVG::Vec & hKernel ,
                                  const openMVG::Vec & vKernel ,
                                  openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform Separable convolution
 * @param img Image to convolve
 * @param hKernel Horizontal kernel
 * @param vKernel Vertical kernel
 * @param h_kernel_size Horizontal kernel size
 * @param v_kernel_size Vertical kernel size
 * @param ctx OpenCL context
 * @return convolved image
 */
cl_mem ImageSeparableConvolution( cl_mem img ,
                                  cl_mem hKernel ,
                                  cl_mem vKernel ,
                                  const size_t h_kernel_size ,
                                  const size_t v_kernel_size ,
                                  openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform Separable convolution
 * @param[out] res Convolved image (need to be at correct size before calling the function)
 * @param img Image to convolve
 * @param hKernel Horizontal kernel
 * @param vKernel Vertical kernel
 * @param ctx OpenCL context
 * @retval true if convolution is ok
 * @retval false if something fails
 */
bool ImageSeparableConvolution( cl_mem res ,
                                cl_mem img ,
                                const openMVG::Vec & hKernel ,
                                const openMVG::Vec & vKernel ,
                                openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Perform Separable convolution
 * @param[out] res Convolved image (need to be at correct size before calling the function)
 * @param img Image to convolve
 * @param hKernel Horizontal kernel
 * @param vKernel Vertical kernel
 * @param h_kernel_size Horizontal kernel size
 * @param v_kernel_size Vertical kernel size
 * @param ctx OpenCL context
 * @retval true if convolution is ok
 * @retval false if something fails
 */
bool ImageSeparableConvolution( cl_mem res ,
                                cl_mem img ,
                                cl_mem hKernel ,
                                cl_mem vKernel ,
                                const size_t h_kernel_size ,
                                const size_t v_kernel_size ,
                                openMVG::system::gpu::OpenCLContext &ctx ) ;

} // namespace gpu
} // namespace image
} // namespace openMVG

#endif