// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "testing/testing.h"

#include "openMVG/image/image_container.hpp"
#include "openMVG/image/image_convolution.hpp"
#include "openMVG/image/gpu/image_gpu_interface.hpp"
#include "openMVG/image/gpu/image_gpu_convolution.hpp"

#include "openMVG/system/gpu/OpenCLContext.hpp"

#include <random>

using namespace openMVG::image ;
using namespace openMVG::image::gpu ;
using namespace openMVG::system::gpu ;


// Convolution 2d
TEST( ImageGPUConvolution , conv2d )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;


  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Mat kernel( 3 , 3 ) ;

  kernel( 0 , 0 ) = - 1.0 ;
  kernel( 0 , 1 ) =   0.0 ;
  kernel( 0 , 2 ) =   1.0 ;

  kernel( 1 , 0 ) = - 1.0 ;
  kernel( 1 , 1 ) =   0.0 ;
  kernel( 1 , 2 ) =   1.0 ;

  kernel( 2 , 0 ) = - 1.0 ;
  kernel( 2 , 1 ) =   0.0 ;
  kernel( 2 , 2 ) =   1.0 ;

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuConvolved = ImageConvolution2d( gpuImg , kernel , ctx ) ;

  EXPECT_EQ( gpuConvolved != nullptr , true ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageConvolution( cpuImg , kernel , cpuConvolved ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
}

// Convolution 2d
// - provide kernel as OpenCL buffer
TEST( ImageGPUConvolution , conv2d_ocl_kernel )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;


  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Mat kernel( 3 , 3 ) ;

  kernel( 0 , 0 ) = - 1.0 ;
  kernel( 0 , 1 ) =   0.0 ;
  kernel( 0 , 2 ) =   1.0 ;

  kernel( 1 , 0 ) = - 1.0 ;
  kernel( 1 , 1 ) =   0.0 ;
  kernel( 1 , 2 ) =   1.0 ;

  kernel( 2 , 0 ) = - 1.0 ;
  kernel( 2 , 1 ) =   0.0 ;
  kernel( 2 , 2 ) =   1.0 ;

  float * krnData = new float[ 9 ] ;
  for( int y = 0 ; y < 3 ; ++y )
  {
    for( int x = 0 ; x < 3 ; ++x )
    {
      krnData[ y * 3 + x ] = kernel( y , x ) ;
    }
  }

  cl_mem kernelBuffer = ctx.createBuffer( 9 * sizeof( float ) , OPENCL_BUFFER_ACCESS_READ_ONLY , krnData ) ;
  delete[] krnData ;

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuConvolved = ImageConvolution2d( gpuImg , kernelBuffer , 3 , 3 , ctx ) ;

  EXPECT_EQ( gpuConvolved != nullptr , true ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageConvolution( cpuImg , kernel , cpuConvolved ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( kernelBuffer ) ;
}

// Convolution 2d
// - provide result as parameter (as an OpenCL image)
TEST( ImageGPUConvolution , conv2d_ocl_res )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;


  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Mat kernel( 3 , 3 ) ;

  kernel( 0 , 0 ) = - 1.0 ;
  kernel( 0 , 1 ) =   0.0 ;
  kernel( 0 , 2 ) =   1.0 ;

  kernel( 1 , 0 ) = - 1.0 ;
  kernel( 1 , 1 ) =   0.0 ;
  kernel( 1 , 2 ) =   1.0 ;

  kernel( 2 , 0 ) = - 1.0 ;
  kernel( 2 , 1 ) =   0.0 ;
  kernel( 2 , 2 ) =   1.0 ;

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;
  cl_mem gpuConvolved = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuConvolved == nullptr ) ;

  bool ok = ImageConvolution2d( gpuConvolved , gpuImg , kernel , ctx ) ;

  EXPECT_EQ( ok , true ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageConvolution( cpuImg , kernel , cpuConvolved ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
}

// Convolution 2d
// - provide result as parameter (as an OpenCL image)
// - provide kernel as OpenCL buffer
TEST( ImageGPUConvolution , conv2d_ocl_res_ocl_kernel )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;


  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Mat kernel( 3 , 3 ) ;

  kernel( 0 , 0 ) = - 1.0 ;
  kernel( 0 , 1 ) =   0.0 ;
  kernel( 0 , 2 ) =   1.0 ;

  kernel( 1 , 0 ) = - 1.0 ;
  kernel( 1 , 1 ) =   0.0 ;
  kernel( 1 , 2 ) =   1.0 ;

  kernel( 2 , 0 ) = - 1.0 ;
  kernel( 2 , 1 ) =   0.0 ;
  kernel( 2 , 2 ) =   1.0 ;

  float * krnData = new float[ 9 ] ;
  for( int y = 0 ; y < 3 ; ++y )
  {
    for( int x = 0 ; x < 3 ; ++x )
    {
      krnData[ y * 3 + x ] = kernel( y , x ) ;
    }
  }

  cl_mem kernelBuffer = ctx.createBuffer( 9 * sizeof( float ) , OPENCL_BUFFER_ACCESS_READ_ONLY , krnData ) ;
  delete[] krnData ;

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuConvolved = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuConvolved == nullptr ) ;

  bool ok = ImageConvolution2d( gpuConvolved , gpuImg , kernelBuffer , 3 , 3 , ctx ) ;

  EXPECT_EQ( true , ok ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageConvolution( cpuImg , kernel , cpuConvolved ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( kernelBuffer ) ;
}

// Horizontal image convolution
TEST( ImageGPUConvolution , horizontal )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Vec kernel( 3 );
  kernel[0] = -1.0 ;
  kernel[1] =  0.0 ;
  kernel[2] =  1.0 ;

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuConvolved = ImageHorizontalConvolution( gpuImg , kernel , ctx ) ;

  EXPECT_EQ( gpuConvolved != nullptr , true ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageHorizontalConvolution( cpuImg , kernel , cpuConvolved ) ;

  EXPECT_EQ( cpuConvolved.Width() , w ) ;
  EXPECT_EQ( cpuConvolved.Height() , h ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
}

// Horizontal image convolution
// - provide result as parameter (as an OpenCL image)
TEST( ImageGPUConvolution , horizontal_cl_res )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Vec kernel( 3 );
  kernel[0] = -1.0 ;
  kernel[1] =  0.0 ;
  kernel[2] =  1.0 ;

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuConvolved = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuConvolved == nullptr ) ;

  bool ok = ImageHorizontalConvolution( gpuConvolved , gpuImg , kernel , ctx ) ;

  EXPECT_EQ( ok , true ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageHorizontalConvolution( cpuImg , kernel , cpuConvolved ) ;

  EXPECT_EQ( cpuConvolved.Width() , w ) ;
  EXPECT_EQ( cpuConvolved.Height() , h ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
}

// Horizontal image convolution
// - provide kernel as OpenCL buffer
TEST( ImageGPUConvolution , horizontal_cl_kernel )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Vec kernel( 3 );
  kernel[0] = -1.0 ;
  kernel[1] =  0.0 ;
  kernel[2] =  1.0 ;

  float * krnData = new float[ 9 ] ;
  for( int x = 0 ; x < 3 ; ++x )
  {
    krnData[ x ] = kernel[ x ] ;
  }

  cl_mem kernelBuffer = ctx.createBuffer( 3 * sizeof( float ) , OPENCL_BUFFER_ACCESS_READ_ONLY , krnData ) ;
  delete[] krnData ;

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuConvolved = ImageHorizontalConvolution( gpuImg , kernelBuffer , 3 , ctx ) ;

  EXPECT_EQ( gpuConvolved != nullptr , true ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageHorizontalConvolution( cpuImg , kernel , cpuConvolved ) ;

  EXPECT_EQ( cpuConvolved.Width() , w ) ;
  EXPECT_EQ( cpuConvolved.Height() , h ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( kernelBuffer ) ;
}

// Horizontal image convolution
// - provide result as parameter (as an OpenCL image)
// - provide kernel as OpenCL buffer
TEST( ImageGPUConvolution , horizontal_cl_res_cl_kernel )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Vec kernel( 3 );
  kernel[0] = -1.0 ;
  kernel[1] =  0.0 ;
  kernel[2] =  1.0 ;

  float * krnData = new float[ 9 ] ;
  for( int x = 0 ; x < 3 ; ++x )
  {
    krnData[ x ] = kernel[ x ] ;
  }

  cl_mem kernelBuffer = ctx.createBuffer( 3 * sizeof( float ) , OPENCL_BUFFER_ACCESS_READ_ONLY , krnData ) ;
  delete[] krnData ;


  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuConvolved = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuConvolved == nullptr ) ;

  bool ok = ImageHorizontalConvolution( gpuConvolved , gpuImg , kernelBuffer , 3 , ctx ) ;

  EXPECT_EQ( ok , true ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageHorizontalConvolution( cpuImg , kernel , cpuConvolved ) ;

  EXPECT_EQ( cpuConvolved.Width() , w ) ;
  EXPECT_EQ( cpuConvolved.Height() , h ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( kernelBuffer ) ;
}

// Vertical image convolution
TEST( ImageGPUConvolution , vertical )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Vec kernel( 3 );
  kernel[0] = -1.0 ;
  kernel[1] =  0.0 ;
  kernel[2] =  1.0 ;

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuConvolved = ImageVerticalConvolution( gpuImg , kernel , ctx ) ;

  EXPECT_EQ( gpuConvolved != nullptr , true ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageVerticalConvolution( cpuImg , kernel , cpuConvolved ) ;

  EXPECT_EQ( cpuConvolved.Width() , w ) ;
  EXPECT_EQ( cpuConvolved.Height() , h ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
}

// Vertical image convolution
// - provide result as parameter (as an OpenCL image)
TEST( ImageGPUConvolution , vertical_cl_res )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Vec kernel( 3 );
  kernel[0] = -1.0 ;
  kernel[1] =  0.0 ;
  kernel[2] =  1.0 ;

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuConvolved = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuConvolved == nullptr ) ;

  bool ok = ImageVerticalConvolution( gpuConvolved , gpuImg , kernel , ctx ) ;

  EXPECT_EQ( ok , true ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageVerticalConvolution( cpuImg , kernel , cpuConvolved ) ;

  EXPECT_EQ( cpuConvolved.Width() , w ) ;
  EXPECT_EQ( cpuConvolved.Height() , h ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
}

// Vertical image convolution
// - provide kernel as OpenCL buffer
TEST( ImageGPUConvolution , vertical_cl_kernel )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Vec kernel( 3 );
  kernel[0] = -1.0 ;
  kernel[1] =  0.0 ;
  kernel[2] =  1.0 ;

  float * krnData = new float[ 9 ] ;
  for( int x = 0 ; x < 3 ; ++x )
  {
    krnData[ x ] = kernel[ x ] ;
  }

  cl_mem kernelBuffer = ctx.createBuffer( 3 * sizeof( float ) , OPENCL_BUFFER_ACCESS_READ_ONLY , krnData ) ;
  delete[] krnData ;

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuConvolved = ImageVerticalConvolution( gpuImg , kernelBuffer , 3 , ctx ) ;

  EXPECT_EQ( gpuConvolved != nullptr , true ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageVerticalConvolution( cpuImg , kernel , cpuConvolved ) ;

  EXPECT_EQ( cpuConvolved.Width() , w ) ;
  EXPECT_EQ( cpuConvolved.Height() , h ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( kernelBuffer ) ;
}

// Vertical image convolution
// - provide result as parameter (as an OpenCL image)
// - provide kernel as OpenCL buffer
TEST( ImageGPUConvolution , vertical_cl_res_cl_kernel )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  std::uniform_real_distribution<float> distrib( 0.f , 1.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  openMVG::Vec kernel( 3 );
  kernel[0] = -1.0 ;
  kernel[1] =  0.0 ;
  kernel[2] =  1.0 ;

  float * krnData = new float[ 9 ] ;
  for( int x = 0 ; x < 3 ; ++x )
  {
    krnData[ x ] = kernel[ x ] ;
  }

  cl_mem kernelBuffer = ctx.createBuffer( 3 * sizeof( float ) , OPENCL_BUFFER_ACCESS_READ_ONLY , krnData ) ;
  delete[] krnData ;


  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuConvolved = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuConvolved == nullptr ) ;

  bool ok = ImageVerticalConvolution( gpuConvolved , gpuImg , kernelBuffer , 3 , ctx ) ;

  EXPECT_EQ( ok , true ) ;

  Image<float> resConvolved;
  bool cvtRes = FromOpenCLImage( gpuConvolved , resConvolved , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuConvolved ;
  ImageVerticalConvolution( cpuImg , kernel , cpuConvolved ) ;

  EXPECT_EQ( cpuConvolved.Width() , w ) ;
  EXPECT_EQ( cpuConvolved.Height() , h ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuConvolved( y , x ) , resConvolved( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuConvolved ) ;
  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( kernelBuffer ) ;
}

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
