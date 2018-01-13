#include "image_gpu_convolution.hpp"

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
 * @note for out of range values, pad with 0 values
 */
cl_mem ImageConvolution2d( cl_mem img , const openMVG::Mat & kernel , openMVG::system::gpu::OpenCLContext & ctx )
{
  const int w = kernel.cols() ;
  const int h = kernel.rows() ;
  const int nbKernelCoef = w * h ;
  float * krn = new float[ nbKernelCoef ] ;
  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      krn[ y * w + x ] = static_cast<float>( kernel( y , x ) ) ;
    }
  }

  cl_mem buffer = ctx.createBuffer( nbKernelCoef * sizeof( float ) , openMVG::system::gpu::OPENCL_BUFFER_ACCESS_READ_ONLY , krn ) ;
  delete[] krn ;

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


  cl_kernel cl_krn;
  if( w <= 29 && h <= 29 )
  {
    cl_krn = ctx.standardKernel( "convolve_2d_local_f" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "convolve_2d_naive_f" ) ;
  }

  cl_int half_w = w / 2 ;
  cl_int half_h = h / 2 ;

  clSetKernelArg( cl_krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( cl_krn , 1 , sizeof( cl_mem ) , &buffer ) ;
  clSetKernelArg( cl_krn , 2 , sizeof( cl_mem ) , &img ) ;
  clSetKernelArg( cl_krn , 3 , sizeof( cl_int ) , &half_w ) ;
  clSetKernelArg( cl_krn , 4 , sizeof( cl_int ) , &half_h ) ;

  const size_t dim[] =
  {
    static_cast<size_t>( width ) ,
    static_cast<size_t>( height )
  } ;

  ctx.runKernel2d( cl_krn , dim ) ;

  clReleaseMemObject( buffer ) ;

  return res ;
}

/**
 * @brief Perform OpenCL 2d convolution
 * @param img Input image
 * @param kernel Convolution kernel
 * @param ctx OpenCL context
 * @return convolution result
 * @note for out of range values, pad with 0 values
 */
cl_mem ImageConvolution2d( cl_mem img , cl_mem kernel , const size_t kernel_w , const size_t kernel_h , openMVG::system::gpu::OpenCLContext & ctx )
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


  cl_kernel cl_krn;
  if( kernel_w <= 29 && kernel_h <= 29 )
  {
    cl_krn = ctx.standardKernel( "convolve_2d_local_f" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "convolve_2d_naive_f" ) ;
  }

  cl_int half_w = kernel_w / 2 ;
  cl_int half_h = kernel_h / 2 ;

  clSetKernelArg( cl_krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( cl_krn , 1 , sizeof( cl_mem ) , &kernel ) ;
  clSetKernelArg( cl_krn , 2 , sizeof( cl_mem ) , &img ) ;
  clSetKernelArg( cl_krn , 3 , sizeof( cl_int ) , &half_w ) ;
  clSetKernelArg( cl_krn , 4 , sizeof( cl_int ) , &half_h ) ;

  const size_t dim[] =
  {
    static_cast<size_t>( width ) ,
    static_cast<size_t>( height )
  } ;

  ctx.runKernel2d( cl_krn , dim ) ;

  return res ;
}

/**
 * @brief Perform OpenCL 2d convolution
 * @param[out] res convolution result
 * @param img Input image
 * @param kernel Convolution kernel
 * @param ctx OpenCL context
 * @note for out of range values, pad with 0 values
 */
bool ImageConvolution2d( cl_mem res , cl_mem img , const openMVG::Mat & kernel , openMVG::system::gpu::OpenCLContext & ctx )
{
  const int w = kernel.cols() ;
  const int h = kernel.rows() ;
  const int nbKernelCoef = w * h ;
  float * krn = new float[ nbKernelCoef ] ;
  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      krn[ y * w + x ] = static_cast<float>( kernel( y , x ) ) ;
    }
  }

  cl_mem buffer = ctx.createBuffer( nbKernelCoef * sizeof( float ) , openMVG::system::gpu::OPENCL_BUFFER_ACCESS_READ_ONLY , krn ) ;
  delete[] krn ;


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
  err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( formatRes ) , &formatRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t widthRes ;
  err = clGetImageInfo( img , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t heightRes ;
  err = clGetImageInfo( img , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }

  if( width != widthRes ||
      height != heightRes ||
      format.image_channel_order != formatRes.image_channel_order ||
      format.image_channel_data_type != formatRes.image_channel_data_type )
  {
    return false ;
  }


  cl_kernel cl_krn;
  if( w <= 29 && h <= 29 )
  {
    cl_krn = ctx.standardKernel( "convolve_2d_local_f" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "convolve_2d_naive_f" ) ;
  }

  cl_int half_w = w / 2 ;
  cl_int half_h = h / 2 ;

  clSetKernelArg( cl_krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( cl_krn , 1 , sizeof( cl_mem ) , &buffer ) ;
  clSetKernelArg( cl_krn , 2 , sizeof( cl_mem ) , &img ) ;
  clSetKernelArg( cl_krn , 3 , sizeof( cl_int ) , &half_w ) ;
  clSetKernelArg( cl_krn , 4 , sizeof( cl_int ) , &half_h ) ;

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

  clReleaseMemObject( buffer ) ;

  return true ;
}

/**
 * @brief Perform OpenCL 2d convolution
 * @param[out] res convolution result
 * @param img Input image
 * @param kernel Convolution kernel
 * @param ctx OpenCL context
 * @note for out of range values, pad with 0 values
 */
bool ImageConvolution2d( cl_mem res , cl_mem img , cl_mem kernel , const size_t kernel_w , const size_t kernel_h , openMVG::system::gpu::OpenCLContext & ctx )
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
  err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( formatRes ) , &formatRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t widthRes ;
  err = clGetImageInfo( img , CL_IMAGE_WIDTH , sizeof( size_t ) , &widthRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  size_t heightRes ;
  err = clGetImageInfo( img , CL_IMAGE_HEIGHT , sizeof( size_t ) , &heightRes , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }

  if( width != widthRes ||
      height != heightRes ||
      format.image_channel_order != formatRes.image_channel_order ||
      format.image_channel_data_type != formatRes.image_channel_data_type )
  {
    return false ;
  }


  cl_kernel cl_krn;
  if( kernel_w <= 29 && kernel_h <= 29 )
  {
    cl_krn = ctx.standardKernel( "convolve_2d_local_f" ) ;
  }
  else
  {
    cl_krn = ctx.standardKernel( "convolve_2d_naive_f" ) ;
  }

  cl_int half_w = kernel_w / 2 ;
  cl_int half_h = kernel_h / 2 ;

  clSetKernelArg( cl_krn , 0 , sizeof( cl_mem ) , &res ) ;
  clSetKernelArg( cl_krn , 1 , sizeof( cl_mem ) , &kernel ) ;
  clSetKernelArg( cl_krn , 2 , sizeof( cl_mem ) , &img ) ;
  clSetKernelArg( cl_krn , 3 , sizeof( cl_int ) , &half_w ) ;
  clSetKernelArg( cl_krn , 4 , sizeof( cl_int ) , &half_h ) ;

  const size_t dim[] =
  {
    static_cast<size_t>( width ) ,
    static_cast<size_t>( height )
  } ;

  ctx.runKernel2d( cl_krn , dim ) ;

  return true ;
}



} // namespace gpu
} // namespace image
} // namespace openMVG