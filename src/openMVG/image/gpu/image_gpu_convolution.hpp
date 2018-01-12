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


} // namespace gpu
} // namespace image
} // namespace openMVG

#endif