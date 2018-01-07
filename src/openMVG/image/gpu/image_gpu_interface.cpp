#include "image_gpu_interface.hpp"

namespace openMVG
{
namespace image
{
namespace gpu
{

// Image width
static inline size_t ImageWidth( cl_mem img_obj )
{
  size_t width ;
  cl_int res = clGetImageInfo( img_obj , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , NULL ) ;
  if( res != CL_SUCCESS )
  {
    return 0 ;
  }
  return width ;
}

// Image height
static inline size_t ImageHeight( cl_mem img_obj )
{
  size_t height ;
  cl_int res = clGetImageInfo( img_obj , CL_IMAGE_HEIGHT , sizeof( size_t ) , &height , NULL ) ;
  if( res != CL_SUCCESS )
  {
    return 0 ;
  }
  return height ;
}

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<unsigned char> & img , openMVG::system::gpu::OpenCLContext & ctx )
{
  cl_image_format format ;
  cl_image_desc desc ;

  format.image_channel_order     = CL_R ;
  format.image_channel_data_type = CL_UNORM_INT8 ;

  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = img.Width() ;
  desc.image_height = img.Height() ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = nullptr ;

  cl_int error;
  cl_mem res = clCreateImage( ctx.currentContext() , CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR , &format , &desc , ( void* ) img.data() , &error ) ;

  return res ;
}

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<float> & img , openMVG::system::gpu::OpenCLContext & ctx )
{
  cl_image_format format ;
  cl_image_desc desc ;

  format.image_channel_order     = CL_R ;
  format.image_channel_data_type = CL_FLOAT ;

  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = img.Width() ;
  desc.image_height = img.Height() ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = nullptr ;

  cl_int error;
  cl_mem res = clCreateImage( ctx.currentContext() , CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR , &format , &desc , ( void* ) img.data() , &error ) ;

  return res ;
}

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<double> & img , openMVG::system::gpu::OpenCLContext & ctx )
{
  openMVG::image::Image<float> tmp ;
  tmp = img.GetMat().cast<float>() ;
  return ToOpenCLImage( tmp , ctx ) ;
}

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<Rgb<unsigned char>> & img , openMVG::system::gpu::OpenCLContext & ctx )
{
  cl_image_format format ;
  cl_image_desc desc ;

  format.image_channel_order     = CL_RGB ;
  format.image_channel_data_type = CL_UNORM_INT8 ;

  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = img.Width() ;
  desc.image_height = img.Height() ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = nullptr ;

  cl_int error;
  cl_mem res = clCreateImage( ctx.currentContext() , CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR , &format , &desc , ( void* ) img.data() , &error ) ;

  return res ;
}

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<Rgba<unsigned char>> & img , openMVG::system::gpu::OpenCLContext & ctx )
{
  cl_image_format format ;
  cl_image_desc desc ;

  format.image_channel_order     = CL_RGBA ;
  format.image_channel_data_type = CL_UNORM_INT8 ;

  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = img.Width() ;
  desc.image_height = img.Height() ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = nullptr ;

  cl_int error;
  cl_mem res = clCreateImage( ctx.currentContext() , CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR , &format , &desc , ( void* ) img.data() , &error ) ;

  return res ;
}

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<unsigned char> & outImg , openMVG::system::gpu::OpenCLContext & ctx )
{
  // Get image width/height
  const size_t w = ImageWidth( img ) ;
  const size_t h = ImageHeight( img ) ;

  outImg = openMVG::image::Image<unsigned char>( w , h ) ;

  cl_image_format format ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , NULL ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  if( format.image_channel_order != CL_R )
  {
    return false ;
  }
  if( format.image_channel_data_type != CL_UNORM_INT8 )
  {
    return false ;
  }

  size_t origin[] = { 0 , 0 , 0 } ;
  size_t region[] = { w , h , 1 } ;
  cl_int res = clEnqueueReadImage( ctx.currentCommandQueue() , img , CL_TRUE , origin , region , 0 , 0  , outImg.data() , 0 , NULL , NULL ) ;
  if( res != CL_SUCCESS )
  {
    return false ;
  }

  return true ;
}

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<float> & outImg , openMVG::system::gpu::OpenCLContext & ctx )
{
  // Get image width/height
  const size_t w = ImageWidth( img ) ;
  const size_t h = ImageHeight( img ) ;

  outImg = openMVG::image::Image<float>( w , h ) ;

  cl_image_format format ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , NULL ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  if( format.image_channel_order != CL_R )
  {
    return false ;
  }
  if( format.image_channel_data_type != CL_FLOAT )
  {
    return false ;
  }

  size_t origin[] = { 0 , 0 , 0 } ;
  size_t region[] = { w , h , 1 } ;
  cl_int res = clEnqueueReadImage( ctx.currentCommandQueue() , img , CL_TRUE , origin , region , 0 , 0  , outImg.data() , 0 , NULL , NULL ) ;
  if( res != CL_SUCCESS )
  {
    return false ;
  }

  return true ;
}

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<double> & outImg , openMVG::system::gpu::OpenCLContext & ctx )
{
  // Get image width/height
  const size_t w = ImageWidth( img ) ;
  const size_t h = ImageHeight( img ) ;

  openMVG::image::Image<float> tmp = openMVG::image::Image<float>( w , h ) ;

  cl_image_format format ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , NULL ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  if( format.image_channel_order != CL_R )
  {
    return false ;
  }
  if( format.image_channel_data_type != CL_FLOAT )
  {
    return false ;
  }

  size_t origin[] = { 0 , 0 , 0 } ;
  size_t region[] = { w , h , 1 } ;
  cl_int res = clEnqueueReadImage( ctx.currentCommandQueue() , img , CL_TRUE , origin , region , 0 , 0  , tmp.data() , 0 , NULL , NULL ) ;
  if( res != CL_SUCCESS )
  {
    return false ;
  }

  outImg = tmp.GetMat().cast<double>() ;

  return true ;
}

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<Rgb<unsigned char>> & outImg , openMVG::system::gpu::OpenCLContext & ctx )
{
  // Get image width/height
  const size_t w = ImageWidth( img ) ;
  const size_t h = ImageHeight( img ) ;

  outImg = openMVG::image::Image<Rgb<unsigned char>>( w , h ) ;

  cl_image_format format ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , NULL ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  if( format.image_channel_order != CL_RGB )
  {
    return false ;
  }
  if( format.image_channel_data_type != CL_UNORM_INT8 )
  {
    return false ;
  }

  size_t origin[] = { 0 , 0 , 0 } ;
  size_t region[] = { w , h , 1 } ;
  cl_int res = clEnqueueReadImage( ctx.currentCommandQueue() , img , CL_TRUE , origin , region , 0 , 0  , outImg.data() , 0 , NULL , NULL ) ;
  if( res != CL_SUCCESS )
  {
    return false ;
  }

  return true ;
}

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<Rgba<unsigned char>> & outImg , openMVG::system::gpu::OpenCLContext & ctx )
{
  // Get image width/height
  const size_t w = ImageWidth( img ) ;
  const size_t h = ImageHeight( img ) ;

  outImg = openMVG::image::Image<Rgba<unsigned char>>( w , h ) ;

  cl_image_format format ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , NULL ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  if( format.image_channel_order != CL_RGBA )
  {
    return false ;
  }
  if( format.image_channel_data_type != CL_UNORM_INT8 )
  {
    return false ;
  }

  size_t origin[] = { 0 , 0 , 0 } ;
  size_t region[] = { w , h , 1 } ;
  cl_int res = clEnqueueReadImage( ctx.currentCommandQueue() , img , CL_TRUE , origin , region , 0 , 0  , outImg.data() , 0 , NULL , NULL ) ;
  if( res != CL_SUCCESS )
  {
    return false ;
  }

  return true ;
}

} // namespace gpu
} // namespace image
} // namespace openMVG