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

/**
 * @brief Compute local maxima of the image
 * @param img Image used as input
 * @param ctx OpenCL Context
 * @return Image containing the local maxima
 * @note Input image is a float image
 * @note Return image is a float image (pixel = 1 for local max, 0 else)
 */
cl_mem ImageLocalMaxima( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Compute local maxima of the image
 * @param res Image containing the local maxima
 * @param img Image used as input
 * @param ctx OpenCL Context
 * @retval true computation is ok
 * @retval false if computation fails
 * @note Input image is a float image
 * @note Return image is a float image (pixel = 1 for local max, 0 else)
 */
bool ImageLocalMaxima( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx ) ;


/**
 * @brief Compute local maxima of the image
 * @param imgA Image used as input
 * @param imgB Image used as input (tes is inside)
 * @param imgC Image used as input
 * @param ctx OpenCL Context
 * @return Image containing the local maxima
 * @note Input image is a float image
 * @note Return image is a float image (pixel = 1 for local max, 0 else)
 */
cl_mem ImageLocalMaxima( cl_mem imgA , cl_mem imgB , cl_mem imgC , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Compute local maxima of the image
 * @param res Image containing the local maxima
 * @param imgA Image used as input
 * @param imgB Image used as input (test is inside)
 * @param imgC Image used as input
 * @param ctx OpenCL Context
 * @retval true computation is ok
 * @retval false if computation fails
 * @note Input image is a float image
 * @note Return image is a float image (pixel = 1 for local max, 0 else)
 */
bool ImageLocalMaxima( cl_mem res , cl_mem imgA , cl_mem imgB , cl_mem imgC , openMVG::system::gpu::OpenCLContext & ctx ) ;



} // namespace gpu
} // namespace image
} // namespace openMVG

#endif