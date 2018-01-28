// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
  cl_int res = clGetImageInfo( img_obj , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , nullptr ) ;
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
  cl_int res = clGetImageInfo( img_obj , CL_IMAGE_HEIGHT , sizeof( size_t ) , &height , nullptr ) ;
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
  format.image_channel_data_type = CL_UNSIGNED_INT8 ;

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

  format.image_channel_order     = CL_RGBA ;
  format.image_channel_data_type = CL_UNSIGNED_INT8 ;

  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = img.Width() ;
  desc.image_height = img.Height() ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = nullptr ;

  unsigned char * tmp = new unsigned char[ 4 * img.Width() * img.Height() ] ;
  const Rgb<unsigned char> * data = img.data() ;
  for( int y = 0 ; y < img.Height() ; ++y )
  {
    for( int x = 0 ; x < img.Width() ; ++x )
    {
      const int index = y * img.Width() + x ;

      tmp[ 4 * index ] = data[ index ].r() ;
      tmp[ 4 * index + 1 ] = data[ index ].g() ;
      tmp[ 4 * index + 2 ] = data[ index ].b() ;
    }
  }

  cl_int error;
  cl_mem res = clCreateImage( ctx.currentContext() , CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR , &format , &desc , ( void* ) tmp , &error ) ;

  delete[] tmp ;

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
  format.image_channel_data_type = CL_UNSIGNED_INT8 ;

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
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  if( format.image_channel_order != CL_R )
  {
    return false ;
  }
  if( format.image_channel_data_type != CL_UNSIGNED_INT8 )
  {
    return false ;
  }

  size_t origin[] = { 0 , 0 , 0 } ;
  size_t region[] = { w , h , 1 } ;
  cl_int res = clEnqueueReadImage( ctx.currentCommandQueue() , img , CL_TRUE , origin , region , 0 , 0  , outImg.data() , 0 , nullptr , nullptr ) ;
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
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
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
  cl_int res = clEnqueueReadImage( ctx.currentCommandQueue() , img , CL_TRUE , origin , region , 0 , 0  , outImg.data() , 0 , nullptr , nullptr ) ;
  if( res != CL_SUCCESS )
  {
    return false ;
  }

  return true ;
}

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
bool FromOpenCLImage( cl_mem & img , const size_t region_offset[2] , const size_t region_size[2] , Image<float> & outImg , openMVG::system::gpu::OpenCLContext & ctx )
{
  // Get image width/height
  const size_t out_w = region_size[0] - region_offset[0] ;
  const size_t out_h = region_size[1] - region_offset[1] ;

  outImg = openMVG::image::Image<float>( out_w , out_h ) ;

  cl_image_format format ;
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
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

  size_t origin[] = { region_offset[0] , region_offset[1] , 0 } ;
  size_t region[] = { region_size[0] , region_size[1] , 1 } ;
  cl_int res = clEnqueueReadImage( ctx.currentCommandQueue() , img , CL_TRUE , origin , region , 0 , 0  , outImg.data() , 0 , nullptr , nullptr ) ;
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
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
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
  cl_int res = clEnqueueReadImage( ctx.currentCommandQueue() , img , CL_TRUE , origin , region , 0 , 0  , tmp.data() , 0 , nullptr , nullptr ) ;
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
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  if( format.image_channel_order != CL_RGBA )
  {
    return false ;
  }
  if( format.image_channel_data_type != CL_UNSIGNED_INT8 )
  {
    return false ;
  }

  size_t origin[] = { 0 , 0 , 0 } ;
  size_t region[] = { w , h , 1 } ;

  unsigned char * tmp = new unsigned char[ 4 * w * h ] ;
  cl_int res = clEnqueueReadImage( ctx.currentCommandQueue() , img , CL_TRUE , origin , region , 0 , 0  , tmp , 0 , nullptr , nullptr ) ;
  if( res != CL_SUCCESS )
  {
    return false ;
  }

  for( int y = 0 ; y < outImg.Height() ; ++y )
  {
    for( int x = 0 ; x < outImg.Width() ; ++x )
    {
      const int index = y * outImg.Width() + x ;
      outImg( y , x ) = Rgb<unsigned char>( tmp[ 4 * index ] , tmp[ 4 * index + 1 ] , tmp[ 4 * index + 2 ] ) ;
    }
  }

  delete[] tmp ;

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
  cl_int err = clGetImageInfo( img , CL_IMAGE_FORMAT , sizeof( format ) , &format , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return false ;
  }
  if( format.image_channel_order != CL_RGBA )
  {
    return false ;
  }
  if( format.image_channel_data_type != CL_UNSIGNED_INT8 )
  {
    return false ;
  }

  size_t origin[] = { 0 , 0 , 0 } ;
  size_t region[] = { w , h , 1 } ;
  cl_int res = clEnqueueReadImage( ctx.currentCommandQueue() , img , CL_TRUE , origin , region , 0 , 0  , outImg.data() , 0 , nullptr , nullptr ) ;
  if( res != CL_SUCCESS )
  {
    return false ;
  }

  return true ;
}

} // namespace gpu
} // namespace image
} // namespace openMVG