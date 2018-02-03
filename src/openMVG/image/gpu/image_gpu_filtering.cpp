// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "image_gpu_filtering.hpp"

#include "openMVG/image/image_filtering.hpp"
#include "openMVG/image/gpu/image_gpu_convolution.hpp"
#include "openMVG/numeric/numeric.h"

namespace openMVG
{
namespace image
{
namespace gpu
{

/**
 * @brief Compute X derivative using central difference
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageXDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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

  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_normalized" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_unnormalized" ) ;
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
 * @brief Compute X derivative using central difference
 * @param[out] res Derivative image (should be allocated before calling this function)
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @retval true if computation is of
 * @retval false if computation fails
 */
bool ImageXDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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

  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_normalized" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_unnormalized" ) ;
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
    return false ;
  }

  return true ;
}


/**
 * @brief Compute Y derivative using central difference
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageYDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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

  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_normalized" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_unnormalized" ) ;
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
 * @brief Compute Y derivative using central difference
 * @param[out] res Derivative image (should be allocated before calling this function)
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @retval true if computation is of
 * @retval false if computation fails
 */
bool ImageYDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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

  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_normalized" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_unnormalized" ) ;
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
    return false ;
  }

  return true ;
}

/**
 * @brief Compute Sobel X derivative
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageSobelXDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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

  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_sobel_normalized_local" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_sobel_unnormalized_local" ) ;
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
 * @brief Compute Sobel X derivative
 * @param[out] res Derivative image
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageSobelXDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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


  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_sobel_normalized_local" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_sobel_unnormalized_local" ) ;
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
    return false ;
  }

  return true ;
}

/**
 * @brief Compute Sobel Y derivative
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageSobelYDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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

  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_sobel_normalized_local" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_sobel_unnormalized_local" ) ;
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
 * @brief Compute Sobel Y derivative
 * @param[out] res Derivative image
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageSobelYDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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


  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_sobel_normalized_local" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_sobel_unnormalized_local" ) ;
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
    return false ;
  }

  return true ;
}

/**
 * @brief Compute Scharr X derivative
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageScharrXDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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

  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_scharr_normalized_local" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_scharr_unnormalized_local" ) ;
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
 * @brief Compute Scharr X derivative
 * @param[out] res Derivative image
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageScharrXDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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


  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_scharr_normalized_local" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_x_derivative_scharr_unnormalized_local" ) ;
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
    return false ;
  }

  return true ;
}

/**
 * @brief Compute Scharr Y derivative
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageScharrYDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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

  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_scharr_normalized_local" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_scharr_unnormalized_local" ) ;
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
 * @brief Compute Scharr Y derivative
 * @param[out] res Derivative image
 * @param img Input image
 * @param ctx OpenCL Context
 * @param normalized Indicate if derivative is normalized
 * @retval true if computation is ok
 * @retval false if computation fails
 */
bool ImageScharrYDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize )
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


  // Get kernel
  cl_kernel cl_krn ;
  if( normalize )
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_scharr_normalized_local" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "image_y_derivative_scharr_unnormalized_local" ) ;
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
    return false ;
  }

  return true ;
}

/**
 * @brief Compute X-derivative using scaled Scharr filter
 * @param img Input image
 * @param scale scale of filter (1 -> 3x3 filter; 2 -> 5x5, ...)
 * @param ctx OpenCL Context
 * @param bNormalize true if kernel must be normalized
 * @return derivative image
 */
cl_mem ImageScaledScharrXDerivative( cl_mem img , const int scale , openMVG::system::gpu::OpenCLContext & ctx , const bool bNormalize )
{
  const int kernel_size = 3 + 2 * ( scale - 1 );

  openMVG::Vec kernel_vert( kernel_size );
  openMVG::Vec kernel_horiz( kernel_size );

  /*
  General X-derivative function
                              | -1   0   1 |
  D = 1 / ( 2 h * ( w + 2 ) ) | -w   0   w |
                              | -1   0   1 |
  */

  kernel_horiz.fill( 0.0 );
  kernel_horiz( 0 )               = -1.0;
  // kernel_horiz( kernel_size / 2 ) = 0.0;
  kernel_horiz( kernel_size - 1 ) = 1.0;

  // Scharr parameter for derivative
  const double w = 10.0 / 3.0;

  kernel_vert.fill( 0.0 );
  kernel_vert( 0 )               = 1.0;
  kernel_vert( kernel_size / 2 ) = w;
  kernel_vert( kernel_size - 1 ) = 1.0;

  if ( bNormalize )
  {
    kernel_vert *= 1.0 / ( 2.0 * scale * ( w + 2.0 ) );
  }

  return ImageSeparableConvolution( img , kernel_horiz , kernel_vert , ctx ) ;
}

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
bool ImageScaledScharrXDerivative( cl_mem res , cl_mem img , const int scale , openMVG::system::gpu::OpenCLContext & ctx , const bool bNormalize )
{
  const int kernel_size = 3 + 2 * ( scale - 1 );

  openMVG::Vec kernel_vert( kernel_size );
  openMVG::Vec kernel_horiz( kernel_size );

  /*
  General X-derivative function
                              | -1   0   1 |
  D = 1 / ( 2 h * ( w + 2 ) ) | -w   0   w |
                              | -1   0   1 |
  */

  kernel_horiz.fill( 0.0 );
  kernel_horiz( 0 )               = -1.0;
  // kernel_horiz( kernel_size / 2 ) = 0.0;
  kernel_horiz( kernel_size - 1 ) = 1.0;

  // Scharr parameter for derivative
  const double w = 10.0 / 3.0;

  kernel_vert.fill( 0.0 );
  kernel_vert( 0 )               = 1.0;
  kernel_vert( kernel_size / 2 ) = w;
  kernel_vert( kernel_size - 1 ) = 1.0;

  if ( bNormalize )
  {
    kernel_vert *= 1.0 / ( 2.0 * scale * ( w + 2.0 ) );
  }

  return ImageSeparableConvolution( res , img , kernel_horiz , kernel_vert , ctx ) ;
}

/**
 * @brief Compute X-derivative using scaled Scharr filter
 * @param img Input image
 * @param scale scale of filter (1 -> 3x3 filter; 2 -> 5x5, ...)
 * @param ctx OpenCL Context
 * @param bNormalize true if kernel must be normalized
 * @return derivative image
 */
cl_mem ImageScaledScharrYDerivative( cl_mem img , const int scale , openMVG::system::gpu::OpenCLContext & ctx , const bool bNormalize )
{
  /*
  General Y-derivative function
                              | -1  -w  -1 |
  D = 1 / ( 2 h * ( w + 2 ) ) |  0   0   0 |
                              |  1   w   1 |

  */
  const int kernel_size = 3 + 2 * ( scale - 1 );

  openMVG::Vec kernel_vert( kernel_size );
  openMVG::Vec kernel_horiz( kernel_size );

  // Scharr parameter for derivative
  const double w = 10.0 / 3.0;


  kernel_horiz.fill( 0.0 );
  kernel_horiz( 0 )               = 1.0;
  kernel_horiz( kernel_size / 2 ) = w;
  kernel_horiz( kernel_size - 1 ) = 1.0;

  if ( bNormalize )
  {
    kernel_horiz *= 1.0 / ( 2.0 * scale * ( w + 2.0 ) );
  }

  kernel_vert.fill( 0.0 );
  kernel_vert( 0 ) = - 1.0;
  // kernel_vert( kernel_size / 2 ) = 0.0;
  kernel_vert( kernel_size - 1 ) = 1.0;

  return ImageSeparableConvolution( img , kernel_horiz , kernel_vert , ctx ) ;
}

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
bool ImageScaledScharrYDerivative( cl_mem res , cl_mem img , const int scale , openMVG::system::gpu::OpenCLContext & ctx , const bool bNormalize )
{
  /*
  General Y-derivative function
                              | -1  -w  -1 |
  D = 1 / ( 2 h * ( w + 2 ) ) |  0   0   0 |
                              |  1   w   1 |

  */
  const int kernel_size = 3 + 2 * ( scale - 1 );

  openMVG::Vec kernel_vert( kernel_size );
  openMVG::Vec kernel_horiz( kernel_size );

  // Scharr parameter for derivative
  const double w = 10.0 / 3.0;


  kernel_horiz.fill( 0.0 );
  kernel_horiz( 0 )               = 1.0;
  kernel_horiz( kernel_size / 2 ) = w;
  kernel_horiz( kernel_size - 1 ) = 1.0;

  if ( bNormalize )
  {
    kernel_horiz *= 1.0 / ( 2.0 * scale * ( w + 2.0 ) );
  }

  kernel_vert.fill( 0.0 );
  kernel_vert( 0 ) = - 1.0;
  // kernel_vert( kernel_size / 2 ) = 0.0;
  kernel_vert( kernel_size - 1 ) = 1.0;

  return ImageSeparableConvolution( res , img , kernel_horiz , kernel_vert , ctx ) ;
}

/**
 * @brief Compute (isotropic) gaussian filtering of an image using filter width of k * sigma
 * @param img Input image
 * @param sigma standard deviation of kernel
 * @param ctx OpenCL Context
 * @param k confidence interval param - kernel is width k * sigma * 2 + 1 -- using k = 3 gives 99% of gaussian curve
 * @return Gaussian filtered image
 */
cl_mem ImageGaussianFilter( cl_mem img , const double sigma , openMVG::system::gpu::OpenCLContext & ctx , const int k )
{
  // Compute Gaussian filter
  const int k_size    = ( int ) 2 * k * sigma + 1;
  const int half_k_size = k_size / 2;

  const double exp_scale = 1.0 / ( 2.0 * sigma * sigma );

  // Compute 1D Gaussian filter
  openMVG::Vec kernel_horiz( k_size );

  double sum = 0;
  for ( int i = 0; i < k_size; ++i )
  {
    const double dx = ( i - half_k_size );
    kernel_horiz( i ) = exp( - dx * dx * exp_scale );
    sum += kernel_horiz( i );
  }

  // Normalize kernel (to have \sum_i kernel_horiz( i ) = 1 and avoid energy loss)
  const double inv = 1.0 / sum;
  for ( int i = 0; i < k_size; ++i )
  {
    kernel_horiz( i ) *= inv;
  }

  // Vertical kernel is the same as the horizontal one
  const openMVG::Vec & kernel_vert = kernel_horiz;

  return ImageSeparableConvolution( img , kernel_horiz , kernel_vert , ctx ) ;
}

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
bool ImageGaussianFilter( cl_mem res , cl_mem img , const double sigma , openMVG::system::gpu::OpenCLContext & ctx , const int k )
{
  // Compute Gaussian filter
  const int k_size    = ( int ) 2 * k * sigma + 1;
  const int half_k_size = k_size / 2;

  const double exp_scale = 1.0 / ( 2.0 * sigma * sigma );

  // Compute 1D Gaussian filter
  openMVG::Vec kernel_horiz( k_size );

  double sum = 0;
  for ( int i = 0; i < k_size; ++i )
  {
    const double dx = ( i - half_k_size );
    kernel_horiz( i ) = exp( - dx * dx * exp_scale );
    sum += kernel_horiz( i );
  }

  // Normalize kernel (to have \sum_i kernel_horiz( i ) = 1 and avoid energy loss)
  const double inv = 1.0 / sum;
  for ( int i = 0; i < k_size; ++i )
  {
    kernel_horiz( i ) *= inv;
  }

  // Vertical kernel is the same as the horizontal one
  const openMVG::Vec & kernel_vert = kernel_horiz;

  return ImageSeparableConvolution( res , img , kernel_horiz , kernel_vert , ctx ) ;
}

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
                          const int k )
{
  // Compute Gaussian filter
  const int k_size    = ( int ) 2 * k * sigma + 1;
  const int half_k_size = k_size / 2;

  const double exp_scale = 1.0 / ( 2.0 * sigma * sigma );

  // Compute 1D Gaussian filter
  openMVG::Vec kernel_horiz( k_size );

  double sum = 0;
  for ( int i = 0; i < k_size; ++i )
  {
    const double dx = ( i - half_k_size );
    kernel_horiz( i ) = exp( - dx * dx * exp_scale );
    sum += kernel_horiz( i );
  }

  // Normalize kernel (to have \sum_i kernel_horiz( i ) = 1 and avoid energy loss)
  const double inv = 1.0 / sum;
  for ( int i = 0; i < k_size; ++i )
  {
    kernel_horiz( i ) *= inv;
  }

  // Vertical kernel is the same as the horizontal one
  const openMVG::Vec & kernel_vert = kernel_horiz;

  return ImageSeparableConvolution( res , img , kernel_horiz , kernel_vert , offset_region , region_size , ctx ) ;
}

/**
 * @brief Compute gaussian filtering of an image using user defined filter widths
 * @param img Input image
 * @param sigma standard deviation of kernel
 * @param kernel_size_x Size of horizontal kernel (must be an odd number or 0 for automatic computation)
 * @param kernel_size_y Size of vertical kernel (must be an add number or 0 for automatic computation)
 * @param ctx OpenCL Context
 * @return Gaussian filtered image
 */
cl_mem ImageGaussianFilter( cl_mem img , const double sigma , const size_t kernel_size_x , const size_t kernel_size_y , openMVG::system::gpu::OpenCLContext & ctx )
{
  assert( kernel_size_x % 2 == 1 || kernel_size_x == 0 );
  assert( kernel_size_y % 2 == 1 || kernel_size_y == 0 );

  const Vec kernel_horiz = ComputeGaussianKernel( kernel_size_x , sigma );
  const Vec kernel_vert  = ComputeGaussianKernel( kernel_size_y , sigma );

  return ImageSeparableConvolution( img , kernel_horiz , kernel_vert , ctx ) ;
}


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
bool ImageGaussianFilter( cl_mem res , cl_mem img , const double sigma , const size_t kernel_size_x , const size_t kernel_size_y , openMVG::system::gpu::OpenCLContext & ctx )
{
  assert( kernel_size_x % 2 == 1 || kernel_size_x == 0 );
  assert( kernel_size_y % 2 == 1 || kernel_size_y == 0 );

  const Vec kernel_horiz = ComputeGaussianKernel( kernel_size_x , sigma );
  const Vec kernel_vert  = ComputeGaussianKernel( kernel_size_y , sigma );

  return ImageSeparableConvolution( res , img , kernel_horiz , kernel_vert , ctx ) ;
}


} // namespace gpu
} // namespace image
} // namespace openMVG
