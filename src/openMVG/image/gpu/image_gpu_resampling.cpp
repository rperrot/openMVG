#include "image_gpu_resampling.hpp"

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
cl_mem ImageDecimate( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx )
{
  size_t width ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , nullptr ) ;
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
  cl_image_format format ;
  err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }

  const size_t new_width  = width / 2 ;
  const size_t new_height = height / 2 ;

  cl_image_desc desc ;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = new_width ;
  desc.image_height = new_height ;
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

  cl_kernel krn ;
  if( format.image_channel_data_type == CL_UNSIGNED_INT8 )
  {
    krn = ctx.standardKernel( "image_decimate_ui" ) ;
  }
  else if( format.image_channel_data_type == CL_FLOAT )
  {
    krn = ctx.standardKernel( "image_decimate_f" ) ;
  }

  clSetKernelArg( krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( krn , 1 , sizeof( cl_mem ) , &img ) ;

  const size_t size[2] =
  {
    new_width , new_height
  } ;

  if( ! ctx.runKernel2d( krn , size ) )
  {
    return nullptr ;
  }

  return res ;
}

/**
 * @brief Decimate image (get only one pixel over two - no interpolation)
 * @param[out] Decimated image
 * @param img Image
 * @param ctx OpenCL Context
 * @retval true If success
 * @retval false If something fails
 */
bool ImageDecimate( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx )
{
  size_t width ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , nullptr ) ;
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
  cl_image_format format ;
  err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t widthRes ;
  err = clGetImageInfo( res , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t heightRes ;
  err = clGetImageInfo( res , CL_IMAGE_HEIGHT , sizeof( size_t ) , &height , nullptr ) ;
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

  const size_t new_width  = width / 2 ;
  const size_t new_height = height / 2 ;
  if( widthRes < new_width || heightRes < new_height ||
      format.image_channel_data_type != formatRes.image_channel_data_type ||
      format.image_channel_order != formatRes.image_channel_order )
  {
    return false ;
  }

  cl_kernel krn ;
  if( format.image_channel_data_type == CL_UNSIGNED_INT8 )
  {
    krn = ctx.standardKernel( "image_decimate_ui" ) ;
  }
  else if( format.image_channel_data_type == CL_FLOAT )
  {
    krn = ctx.standardKernel( "image_decimate_f" ) ;
  }

  clSetKernelArg( krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( krn , 1 , sizeof( cl_mem ) , &img ) ;

  const size_t size[2] =
  {
    new_width , new_height
  } ;

  if( ! ctx.runKernel2d( krn , size ) )
  {
    return false ;
  }

  return true ;
}

/**
 * @brief Image upsampling (size -> size * 2 ) using linear interpolation
 * @param img Input image
 * @param ctx OpenCL Context
 * @return Upsampled image
 */
cl_mem ImageUpsample( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx )
{
  size_t width ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , nullptr ) ;
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
  cl_image_format format ;
  err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return nullptr ;
  }

  const size_t new_width  = width * 2 ;
  const size_t new_height = height * 2 ;

  cl_image_desc desc ;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = new_width ;
  desc.image_height = new_height ;
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

  cl_kernel krn ;
  if( format.image_channel_data_type == CL_UNSIGNED_INT8 )
  {
    krn = ctx.standardKernel( "image_upsample_ui" ) ;
  }
  else if( format.image_channel_data_type == CL_FLOAT )
  {
    krn = ctx.standardKernel( "image_upsample_f" ) ;
  }

  clSetKernelArg( krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( krn , 1 , sizeof( cl_mem ) , &img ) ;

  const size_t size[2] =
  {
    new_width , new_height
  } ;

  if( ! ctx.runKernel2d( krn , size ) )
  {
    return nullptr ;
  }

  return res ;
}

/**
 * @brief Image upsampling (size -> size * 2 ) using linear interpolation
 * @param[out] Upsampled image
 * @param img Input image
 * @param ctx OpenCL Context
 * @retval true If success
 * @retval false If something fails
 */
bool ImageUpsample( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx )
{
  size_t width ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , nullptr ) ;
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
  cl_image_format format ;
  err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t widthRes ;
  err = clGetImageInfo( res , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t heightRes ;
  err = clGetImageInfo( res , CL_IMAGE_HEIGHT , sizeof( size_t ) , &height , nullptr ) ;
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

  const size_t new_width  = width * 2 ;
  const size_t new_height = height * 2 ;
  if( widthRes < new_width || heightRes < new_height ||
      format.image_channel_data_type != formatRes.image_channel_data_type ||
      format.image_channel_order != formatRes.image_channel_order )
  {
    return false ;
  }

  cl_kernel krn ;
  if( format.image_channel_data_type == CL_UNSIGNED_INT8 )
  {
    krn = ctx.standardKernel( "image_upsample_ui" ) ;
  }
  else if( format.image_channel_data_type == CL_FLOAT )
  {
    krn = ctx.standardKernel( "image_upsample_f" ) ;
  }

  clSetKernelArg( krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( krn , 1 , sizeof( cl_mem ) , &img ) ;

  const size_t size[2] =
  {
    new_width , new_height
  } ;

  if( ! ctx.runKernel2d( krn , size ) )
  {
    return false ;
  }

  return true ;
}


} // namespace gpu
} // namespace image
} // namespace openMVG