#include "image_gpu_arithmetics.hpp"

#include <iostream>

namespace openMVG
{
namespace image
{
namespace gpu
{


/**
 * @brief Add two images
 * @param imgA first image
 * @param imgB second image
 * @param ctx Context
 * @return image object which is the addition of the two parameters
 * @note this function assume standard add is already loaded inside the context (which is the default behavior)
 */
cl_mem ImageAdd( cl_mem imgA , cl_mem imgB , openMVG::system::gpu::OpenCLContext & ctx )
{
  cl_image_format formatA ;
  cl_int err = clGetImageInfo( imgA , CL_IMAGE_FORMAT , sizeof( formatA ) , &formatA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  cl_image_format formatB ;
  err = clGetImageInfo( imgB , CL_IMAGE_FORMAT , sizeof( formatB ) , &formatB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }

  if( formatA.image_channel_data_type != formatB.image_channel_data_type ||
      formatA.image_channel_order != formatB.image_channel_order )
  {
    return nullptr ;
  }

  size_t widthA ;
  err = clGetImageInfo( imgA , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t widthB ;
  err = clGetImageInfo( imgA , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t heightA ;
  err = clGetImageInfo( imgB , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightA , nullptr ) ;
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
  if( widthA != widthB || heightA != heightB )
  {
    return nullptr ;
  }

  cl_kernel krn;
  if( formatA.image_channel_data_type == CL_UNSIGNED_INT8 )
  {
    krn = ctx.standardKernel( "image_add_ui" ) ;
  }
  else if( formatA.image_channel_data_type == CL_FLOAT )
  {
    krn = ctx.standardKernel( "image_add_f" ) ;
  }
  else
  {
    return nullptr ;
  }


  cl_image_desc desc ;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = widthA ;
  desc.image_height = heightA ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = nullptr ;

  cl_mem res = clCreateImage( ctx.currentContext() , CL_MEM_READ_WRITE , &formatA , &desc , nullptr , &err ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }

  clSetKernelArg( krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( krn , 1 , sizeof( cl_mem ) , &imgA ) ;
  clSetKernelArg( krn , 2 , sizeof( cl_mem ) , &imgB ) ;

  size_t size[2] =
  {
    widthA , heightA
  } ;

  if( ! ctx.runKernel2d( krn , size ) )
  {
    return nullptr ;
  }

  return res ;
}

/**
 * @brief Subtract two images
 * @param imgA first image
 * @param imgB second image
 * @param ctx Context
 * @return image object which is the addition of the two parameters
 * @note this function assume standard sub(s) is already loaded inside the context (which is the default behavior)
 */
cl_mem ImageSub( cl_mem imgA , cl_mem imgB , openMVG::system::gpu::OpenCLContext & ctx )
{
  cl_image_format formatA ;
  cl_int err = clGetImageInfo( imgA , CL_IMAGE_FORMAT , sizeof( formatA ) , &formatA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  cl_image_format formatB ;
  err = clGetImageInfo( imgB , CL_IMAGE_FORMAT , sizeof( formatB ) , &formatB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }

  if( formatA.image_channel_data_type != formatB.image_channel_data_type ||
      formatA.image_channel_order != formatB.image_channel_order )
  {
    return nullptr ;
  }

  size_t widthA ;
  err = clGetImageInfo( imgA , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t widthB ;
  err = clGetImageInfo( imgA , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t heightA ;
  err = clGetImageInfo( imgB , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightA , nullptr ) ;
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
  if( widthA != widthB || heightA != heightB )
  {
    return nullptr ;
  }

  cl_kernel krn;
  if( formatA.image_channel_data_type == CL_UNSIGNED_INT8 )
  {
    krn = ctx.standardKernel( "image_sub_ui" ) ;
  }
  else if( formatA.image_channel_data_type == CL_FLOAT )
  {
    krn = ctx.standardKernel( "image_sub_f" ) ;
  }
  else
  {
    return nullptr ;
  }


  cl_image_desc desc ;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = widthA ;
  desc.image_height = heightA ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = nullptr ;

  cl_mem res = clCreateImage( ctx.currentContext() , CL_MEM_READ_WRITE , &formatA , &desc , nullptr , &err ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }

  clSetKernelArg( krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( krn , 1 , sizeof( cl_mem ) , &imgA ) ;
  clSetKernelArg( krn , 2 , sizeof( cl_mem ) , &imgB ) ;

  size_t size[2] =
  {
    widthA , heightA
  } ;

  if( ! ctx.runKernel2d( krn , size ) )
  {
    return nullptr ;
  }

  return res ;
}

/**
 * @brief Multiply (component-wise) two images
 * @param imgA first image
 * @param imgB second image
 * @param ctx Context
 * @return image object which is the addition of the two parameters
 * @note this function assume standard sub(s) is already loaded inside the context (which is the default behavior)
 */
cl_mem ImageMul( cl_mem imgA , cl_mem imgB , openMVG::system::gpu::OpenCLContext & ctx )
{
  cl_image_format formatA ;
  cl_int err = clGetImageInfo( imgA , CL_IMAGE_FORMAT , sizeof( formatA ) , &formatA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  cl_image_format formatB ;
  err = clGetImageInfo( imgB , CL_IMAGE_FORMAT , sizeof( formatB ) , &formatB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }

  if( formatA.image_channel_data_type != formatB.image_channel_data_type ||
      formatA.image_channel_order != formatB.image_channel_order )
  {
    return nullptr ;
  }

  size_t widthA ;
  err = clGetImageInfo( imgA , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthA , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t widthB ;
  err = clGetImageInfo( imgA , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthB , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }
  size_t heightA ;
  err = clGetImageInfo( imgB , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightA , nullptr ) ;
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
  if( widthA != widthB || heightA != heightB )
  {
    return nullptr ;
  }

  cl_kernel krn;
  if( formatA.image_channel_data_type == CL_UNSIGNED_INT8 )
  {
    krn = ctx.standardKernel( "image_mul_ui" ) ;
  }
  else if( formatA.image_channel_data_type == CL_FLOAT )
  {
    krn = ctx.standardKernel( "image_mul_f" ) ;
  }
  else
  {
    return nullptr ;
  }


  cl_image_desc desc ;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = widthA ;
  desc.image_height = heightA ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = nullptr ;

  cl_mem res = clCreateImage( ctx.currentContext() , CL_MEM_READ_WRITE , &formatA , &desc , nullptr , &err ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }

  clSetKernelArg( krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( krn , 1 , sizeof( cl_mem ) , &imgA ) ;
  clSetKernelArg( krn , 2 , sizeof( cl_mem ) , &imgB ) ;

  size_t size[2] =
  {
    widthA , heightA
  } ;

  if( ! ctx.runKernel2d( krn , size ) )
  {
    return nullptr ;
  }

  return res ;
}


} // namespace gpu
} // namespace image
} // namespace openMVG
