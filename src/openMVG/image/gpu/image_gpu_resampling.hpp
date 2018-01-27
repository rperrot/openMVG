// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_GPU_IMAGE_GPU_RESAMPLING_HPP
#define OPENMVG_IMAGE_GPU_IMAGE_GPU_RESAMPLING_HPP

#include "openMVG/system/gpu/OpenCLContext.hpp"

namespace openMVG
{
namespace image
{
namespace gpu
{

/**
 * @brief Decimate image (get only one pixel over two - no interpolation)
 * @param img Image
 * @param ctx OpenCL Context
 * @return Decimated image
 */
cl_mem ImageDecimate( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Decimate image (get only one pixel over two - no interpolation)
 * @param[out] Decimated image
 * @param img Image
 * @param ctx OpenCL Context
 * @retval true If success
 * @retval false If something fails
 */
bool ImageDecimate( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Image upsampling (size -> size * 2 ) using linear interpolation
 * @param img Input image
 * @param ctx OpenCL Context
 * @return Upsampled image
 */
cl_mem ImageUpsample( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Image upsampling (size -> size * 2 ) using linear interpolation
 * @param[out] Upsampled image
 * @param img Input image
 * @param ctx OpenCL Context
 * @retval true If success
 * @retval false If something fails
 */
bool ImageUpsample( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx ) ;


} // namespace gpu
} // namespace image
} // namespace openMVG

#endif