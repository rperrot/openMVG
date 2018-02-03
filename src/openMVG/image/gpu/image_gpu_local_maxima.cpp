// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "image_gpu_local_maxima.hpp"

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
cl_mem ImageLocalMaxima( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx )
{
  cl_image_format format ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t width ;
  err = clGetImageInfo( img , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t height ;
  err = clGetImageInfo( img , CL_IMAGE_HEIGHT , sizeof( size_t ) , &height , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }

  cl_image_desc desc ;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = width ;
  desc.image_height = height ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = nullptr ;

  cl_mem res = clCreateImage( ctx.currentContext() , CL_MEM_READ_WRITE , &format , &desc , nullptr , &err ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }


  cl_kernel cl_krn = ctx.standardKernel( "image_local_min_max_2d" ) ;
  if( ! cl_krn )
  {
    return nullptr ; 
  }

  clSetKernelArg( cl_krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( cl_krn , 1 , sizeof( cl_mem ) , &img ) ;

  const size_t dim[] =
  {
    static_cast<size_t>( width ) ,
    static_cast<size_t>( height )
  } ;

  const size_t workDim[] = { 16  , 16 } ;
  if( ! ctx.runKernel2d( cl_krn , dim , workDim ) )
  {
    return nullptr ;
  }

  return res ;
}

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
bool ImageLocalMaxima( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx )
{
  cl_image_format format ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t width ;
  err = clGetImageInfo( img , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t height ;
  err = clGetImageInfo( img , CL_IMAGE_HEIGHT , sizeof( size_t ) , &height , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  cl_image_format formatRes ;
  err = clGetImageInfo( res , CL_IMAGE_FORMAT , sizeof( formatRes ) , &formatRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t widthRes ;
  err = clGetImageInfo( res , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t heightRes ;
  err = clGetImageInfo( res , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }

  if( width != widthRes ||
      height != heightRes ||
      format.image_channel_data_type != formatRes.image_channel_data_type ||
      format.image_channel_order != formatRes.image_channel_order )
  {
    return false ;
  }


  cl_kernel cl_krn = ctx.standardKernel( "image_local_min_max_2d" ) ;

  clSetKernelArg( cl_krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( cl_krn , 1 , sizeof( cl_mem ) , &img ) ;

  const size_t dim[] =
  {
    static_cast<size_t>( widthRes ) ,
    static_cast<size_t>( heightRes )
  } ;

  const size_t workDim[] = { 16  , 16 } ;
  if( ! ctx.runKernel2d( cl_krn , dim , workDim ) )
  {
    return false ;
  }

  return true ;
}


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
cl_mem ImageLocalMaxima( cl_mem imgA , cl_mem imgB , cl_mem imgC , openMVG::system::gpu::OpenCLContext & ctx )
{
  cl_image_format formatA , formatB , formatC ;
  cl_int err = clGetImageInfo( imgA , CL_IMAGE_FORMAT , sizeof( formatA ) , &formatA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t widthA ;
  err = clGetImageInfo( imgA , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t heightA ;
  err = clGetImageInfo( imgA , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  err = clGetImageInfo( imgB , CL_IMAGE_FORMAT , sizeof( formatB ) , &formatB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t widthB ;
  err = clGetImageInfo( imgB , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t heightB ;
  err = clGetImageInfo( imgB , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  err = clGetImageInfo( imgC , CL_IMAGE_FORMAT , sizeof( formatC ) , &formatC , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t widthC ;
  err = clGetImageInfo( imgC , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthC , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t heightC ;
  err = clGetImageInfo( imgC , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightC , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }

  const size_t width  = std::min( std::min( widthA , widthB ) , widthC ) ;
  const size_t height = std::min( std::min( heightA , heightB ) , heightC ) ;

  cl_image_format format ;
  format.image_channel_data_type = CL_FLOAT ;
  format.image_channel_order = CL_R ;

  cl_image_desc desc ;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = width ;
  desc.image_height = height ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = nullptr ;

  cl_mem res = clCreateImage( ctx.currentContext() , CL_MEM_READ_WRITE , &format , &desc , nullptr , &err ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }

  if( formatA.image_channel_data_type != formatB.image_channel_data_type ||
      formatA.image_channel_order != formatB.image_channel_order ||
      formatA.image_channel_data_type != formatC.image_channel_data_type ||
      formatA.image_channel_order != formatC.image_channel_order )
  {
    return nullptr ;
  }

  cl_kernel cl_krn = ctx.standardKernel( "image_local_min_max_3d" ) ;

  clSetKernelArg( cl_krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( cl_krn , 1 , sizeof( cl_mem ) , &imgA ) ;
  clSetKernelArg( cl_krn , 2 , sizeof( cl_mem ) , &imgB ) ;
  clSetKernelArg( cl_krn , 3 , sizeof( cl_mem ) , &imgC ) ;

  const size_t dim[] =
  {
    static_cast<size_t>( width ) ,
    static_cast<size_t>( height )
  } ;

  const size_t workDim[] = { 16  , 16 } ;
  if( ! ctx.runKernel2d( cl_krn , dim , workDim ) )
  {
    return nullptr ;
  }

  return res ;
}

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
bool ImageLocalMaxima( cl_mem res , cl_mem imgA , cl_mem imgB , cl_mem imgC , openMVG::system::gpu::OpenCLContext & ctx )
{
  cl_image_format formatA , formatB , formatC ;
  cl_int err = clGetImageInfo( imgA , CL_IMAGE_FORMAT , sizeof( formatA ) , &formatA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t widthA ;
  err = clGetImageInfo( imgA , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t heightA ;
  err = clGetImageInfo( imgA , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  err = clGetImageInfo( imgB , CL_IMAGE_FORMAT , sizeof( formatB ) , &formatB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t widthB ;
  err = clGetImageInfo( imgB , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t heightB ;
  err = clGetImageInfo( imgB , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  err = clGetImageInfo( imgC , CL_IMAGE_FORMAT , sizeof( formatC ) , &formatC , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t widthC ;
  err = clGetImageInfo( imgC , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthC , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t heightC ;
  err = clGetImageInfo( imgC , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightC , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }

  cl_image_format formatRes ;
  err = clGetImageInfo( res , CL_IMAGE_FORMAT , sizeof( formatRes ) , &formatRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t widthRes ;
  err = clGetImageInfo( res , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t heightRes ;
  err = clGetImageInfo( res , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }

  if( widthRes > widthA ||
      heightRes > heightA ||
      widthRes > widthB ||
      heightRes > heightB ||
      widthRes > widthC ||
      heightRes > heightC ||
      formatA.image_channel_data_type != formatB.image_channel_data_type ||
      formatA.image_channel_order != formatB.image_channel_order ||
      formatA.image_channel_data_type != formatC.image_channel_data_type ||
      formatA.image_channel_order != formatC.image_channel_order ||
      formatRes.image_channel_data_type != CL_FLOAT ||
      formatRes.image_channel_order != CL_R )
  {
    return false ;
  }

  cl_kernel cl_krn = ctx.standardKernel( "image_local_min_max_3d" ) ;

  clSetKernelArg( cl_krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( cl_krn , 1 , sizeof( cl_mem ) , &imgA ) ;
  clSetKernelArg( cl_krn , 2 , sizeof( cl_mem ) , &imgB ) ;
  clSetKernelArg( cl_krn , 3 , sizeof( cl_mem ) , &imgC ) ;

  const size_t dim[] =
  {
    static_cast<size_t>( widthRes ) ,
    static_cast<size_t>( heightRes )
  } ;

  const size_t workDim[] = { 16  , 16 } ;
  if( ! ctx.runKernel2d( cl_krn , dim , workDim ) )
  {
    return false ;
  }

  return true ;
}


} // namespace gpu
} // namespace image
} // namespace openMVG
