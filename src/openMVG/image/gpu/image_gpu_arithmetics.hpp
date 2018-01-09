// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_GPU_IMAGE_GPU_ARITHMETICS_HPP
#define OPENMVG_IMAGE_GPU_IMAGE_GPU_ARITHMETICS_HPP

#include "openMVG/system/gpu/OpenCLContext.hpp"

namespace openMVG
{
namespace image
{
namespace gpu
{

/**
 * @brief Add (component-wise) two images
 * @param imgA first image
 * @param imgB second image
 * @param ctx Context
 * @return image object which is the addition of the two parameters
 * @note this function assume standard add(s) is already loaded inside the context (which is the default behavior)
 */
cl_mem ImageAdd( cl_mem imgA , cl_mem imgB , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Add (component-wise) two images (using already allocated result image)
 * @param res Output of the operation
 * @param imgA first image
 * @param imgB second image
 * @param ctx Context
 * @retval true if operation is OK
 * @retval false if operation fails
 * @note this function assume standard add(s) is already loaded inside the context (which is the default behavior)
 * @note this function assume res is already allocated at correct size
 */
bool ImageAdd( cl_mem res , cl_mem imgA , cl_mem imgB , openMVG::system::gpu::OpenCLContext & ctx ) ;


/**
 * @brief Subtract (component-wise) two images
 * @param imgA first image
 * @param imgB second image
 * @param ctx Context
 * @return image object which is the difference (component-wise) of the two parameters
 * @note this function assume standard sub(s) is already loaded inside the context (which is the default behavior)
 */
cl_mem ImageSub( cl_mem imgA , cl_mem imgB , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Subtract (component-wise) two images (using already allocated result image)
 * @param res Output of the operation
 * @param imgA first image
 * @param imgB second image
 * @param ctx Context
 * @retval true if operation is OK
 * @retval false if operation fails
 * @note this function assume standard sub(s) is already loaded inside the context (which is the default behavior)
 * @note this function assume res is already allocated at correct size
 */
bool ImageSub( cl_mem res , cl_mem imgA , cl_mem imgB , openMVG::system::gpu::OpenCLContext & ctx ) ;


/**
 * @brief Multiply (component-wise) two images
 * @param imgA first image
 * @param imgB second image
 * @param ctx Context
 * @return image object which is the product of the two parameters
 * @note this function assume standard mul(s) is already loaded inside the context (which is the default behavior)
 */
cl_mem ImageMul( cl_mem imgA , cl_mem imgB , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Multiply (component-wise) two images
 * @param res Output of the operation
 * @param imgA first image
 * @param imgB second image
 * @param ctx Context
 * @retval true if operation is OK
 * @retval false if operation fails
 * @note this function assume standard mul(s) is already loaded inside the context (which is the default behavior)
 * @note this function assume res is already allocated at correct size
 */
bool ImageMul( cl_mem res , cl_mem imgA , cl_mem imgB , openMVG::system::gpu::OpenCLContext & ctx ) ;

} // namespace gpu
} // namespace image
} // namespace openMVG

#endif