// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_GPU_IMAGE_GPU_INTERFACE_HPP
#define OPENMVG_IMAGE_GPU_IMAGE_GPU_INTERFACE_HPP

#include "openMVG/image/image_container.hpp"
#include "openMVG/image/pixel_types.hpp"

#include "openMVG/system/gpu/OpenCLContext.hpp"

namespace openMVG
{
namespace image
{
namespace gpu
{

/************************************************
*                                               *
*             openMVG -> OpenCL                 *
*                                               *
*************************************************/

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<unsigned char> & img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<float> & img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<double> & img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<Rgb<unsigned char>> & img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<Rgba<unsigned char>> & img , openMVG::system::gpu::OpenCLContext & ctx ) ;


/************************************************
*                                               *
*             OpenCL -> openMVG                 *
*                                               *
*************************************************/

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<unsigned char> & outImg , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<float> & outImg , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param region_offset (x,y) Offset to get the image elements 
 * @param region_size (w,h) Size of the region to get 
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 * @note outImg will have size equal to (w-x,h-y)
 */
bool FromOpenCLImage( cl_mem & img , const size_t region_offset[2] , const size_t region_size[2] , Image<float> & outImg , openMVG::system::gpu::OpenCLContext & ctx ) ;


/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<double> & outImg , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<Rgb<unsigned char>> & outImg , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<Rgba<unsigned char>> & outImg , openMVG::system::gpu::OpenCLContext & ctx ) ;


} // namespace gpu
} // namespace image
} // namespace openMVG

#endif