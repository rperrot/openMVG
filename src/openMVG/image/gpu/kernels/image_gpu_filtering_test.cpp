#include "testing/testing.h"


#include "openMVG/image/image_container.hpp"
#include "openMVG/image/image_filtering.hpp"
#include "openMVG/image/gpu/image_gpu_interface.hpp"
#include "openMVG/image/gpu/image_gpu_filtering.hpp"

#include "openMVG/system/gpu/OpenCLContext.hpp"

#include <random>

using namespace openMVG::image ;
using namespace openMVG::image::gpu ;
using namespace openMVG::system::gpu ;



// X derivative
TEST( ImageGPUFiltering , x_derivative_unnormalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuDerivX = ImageXDerivative( gpuImg , ctx , false ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuDerivX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuDerivX ;
  ImageXDerivative( cpuImg , cpuDerivX , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuDerivX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuDerivX ) ;
}

// X derivative
// Provide result as parameter
TEST( ImageGPUFiltering , x_derivative_unnormalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuDerivX = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuDerivX == nullptr ) ;

  const bool ok = ImageXDerivative( gpuDerivX , gpuImg , ctx , false ) ;
  EXPECT_EQ( true , ok ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuDerivX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuDerivX ;
  ImageXDerivative( cpuImg , cpuDerivX , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuDerivX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuDerivX ) ;
}

// X derivative normalized
TEST( ImageGPUFiltering , x_derivative_normalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuDerivX = ImageXDerivative( gpuImg , ctx , true ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuDerivX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuDerivX ;
  ImageXDerivative( cpuImg , cpuDerivX , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuDerivX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuDerivX ) ;
}

// X derivative normalized
// Provide result as parameter
TEST( ImageGPUFiltering , x_derivative_normalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuDerivX = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuDerivX == nullptr ) ;

  const bool ok = ImageXDerivative( gpuDerivX , gpuImg , ctx , true ) ;
  EXPECT_EQ( true , ok ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuDerivX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuDerivX ;
  ImageXDerivative( cpuImg , cpuDerivX , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuDerivX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuDerivX ) ;
}

// Y derivatives


// Y derivative
TEST( ImageGPUFiltering , y_derivative_unnormalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuDerivY = ImageYDerivative( gpuImg , ctx , false ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuDerivY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuDerivY ;
  ImageYDerivative( cpuImg , cpuDerivY , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuDerivY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuDerivY ) ;
}

// Y derivative
// Provide result as parameter
TEST( ImageGPUFiltering , y_derivative_unnormalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuDerivY = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuDerivY == nullptr ) ;

  const bool ok = ImageYDerivative( gpuDerivY , gpuImg , ctx , false ) ;
  EXPECT_EQ( true , ok ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuDerivY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuDerivY ;
  ImageYDerivative( cpuImg , cpuDerivY , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuDerivY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuDerivY ) ;
}

// X derivative normalized
TEST( ImageGPUFiltering , y_derivative_normalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuDerivY = ImageYDerivative( gpuImg , ctx , true ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuDerivY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuDerivY ;
  ImageYDerivative( cpuImg , cpuDerivY , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuDerivY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuDerivY ) ;
}

// X derivative normalized
// Provide result as parameter
TEST( ImageGPUFiltering , y_derivative_normalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuDerivY = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuDerivY == nullptr ) ;

  const bool ok = ImageYDerivative( gpuDerivY , gpuImg , ctx , true ) ;
  EXPECT_EQ( true , ok ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuDerivY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuDerivY ;
  ImageYDerivative( cpuImg , cpuDerivY , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuDerivY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuDerivY ) ;
}

// Sobel X
TEST( ImageGPUFiltering , x_derivative_sobel_unnormalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuSobelX = ImageSobelXDerivative( gpuImg , ctx , false ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuSobelX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuSobelX ;
  ImageSobelXDerivative( cpuImg , cpuSobelX , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuSobelX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuSobelX ) ;
}

TEST( ImageGPUFiltering , x_derivative_sobel_normalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuSobelX = ImageSobelXDerivative( gpuImg , ctx , true ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuSobelX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuSobelX ;
  ImageSobelXDerivative( cpuImg , cpuSobelX , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuSobelX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuSobelX ) ;
}

TEST( ImageGPUFiltering , x_derivative_sobel_unnormalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuSobelX = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuSobelX == nullptr ) ;

  const bool ok = ImageSobelXDerivative( gpuSobelX , gpuImg , ctx , false ) ;
  EXPECT_EQ( true , ok ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuSobelX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuSobelX ;
  ImageSobelXDerivative( cpuImg , cpuSobelX , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuSobelX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuSobelX ) ;
}

TEST( ImageGPUFiltering , x_derivative_sobel_normalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuSobelX = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuSobelX == nullptr ) ;

  const bool ok = ImageSobelXDerivative( gpuSobelX , gpuImg , ctx , true ) ;
  EXPECT_EQ( true , ok ) ;
  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuSobelX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuSobelX ;
  ImageSobelXDerivative( cpuImg , cpuSobelX , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuSobelX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuSobelX ) ;
}
// Sobel Y
TEST( ImageGPUFiltering , y_derivative_sobel_unnormalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuSobelY = ImageSobelYDerivative( gpuImg , ctx , false ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuSobelY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuSobelY ;
  ImageSobelYDerivative( cpuImg , cpuSobelY , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuSobelY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuSobelY ) ;
}

TEST( ImageGPUFiltering , y_derivative_sobel_normalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuSobelY = ImageSobelYDerivative( gpuImg , ctx , true ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuSobelY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuSobelY ;
  ImageSobelYDerivative( cpuImg , cpuSobelY , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuSobelY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuSobelY ) ;
}

TEST( ImageGPUFiltering , y_derivative_sobel_unnormalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuSobelY = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuSobelY == nullptr ) ;

  const bool ok = ImageSobelYDerivative( gpuSobelY , gpuImg , ctx , false ) ;
  EXPECT_EQ( true , ok ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuSobelY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuSobelY ;
  ImageSobelYDerivative( cpuImg , cpuSobelY , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuSobelY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuSobelY ) ;
}

TEST( ImageGPUFiltering , y_derivative_sobel_normalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuSobelY = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuSobelY == nullptr ) ;

  const bool ok = ImageSobelYDerivative( gpuSobelY , gpuImg , ctx , true ) ;
  EXPECT_EQ( true , ok ) ;
  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuSobelY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuSobelY ;
  ImageSobelYDerivative( cpuImg , cpuSobelY , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuSobelY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuSobelY ) ;
}

// Scharr
// Scharr X
TEST( ImageGPUFiltering , x_derivative_scharr_unnormalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuScharrX = ImageScharrXDerivative( gpuImg , ctx , false ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuScharrX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrX ;
  ImageScharrXDerivative( cpuImg , cpuScharrX , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuScharrX ) ;
}

TEST( ImageGPUFiltering , x_derivative_scharr_normalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuScharrX = ImageScharrXDerivative( gpuImg , ctx , true ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuScharrX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrX ;
  ImageScharrXDerivative( cpuImg , cpuScharrX , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuScharrX ) ;
}

TEST( ImageGPUFiltering , x_derivative_scharr_unnormalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuScharrX = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuScharrX == nullptr ) ;

  const bool ok = ImageScharrXDerivative( gpuScharrX , gpuImg , ctx , false ) ;
  EXPECT_EQ( true , ok ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuScharrX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrX ;
  ImageScharrXDerivative( cpuImg , cpuScharrX , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuScharrX ) ;
}

TEST( ImageGPUFiltering , x_derivative_scharr_normalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuScharrX = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuScharrX == nullptr ) ;

  const bool ok = ImageScharrXDerivative( gpuScharrX , gpuImg , ctx , true ) ;
  EXPECT_EQ( true , ok ) ;
  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( gpuScharrX , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrX ;
  ImageScharrXDerivative( cpuImg , cpuScharrX , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuScharrX ) ;
}
// Scharr Y
TEST( ImageGPUFiltering , y_derivative_scharr_unnormalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuScharrY = ImageScharrYDerivative( gpuImg , ctx , false ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuScharrY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrY ;
  ImageScharrYDerivative( cpuImg , cpuScharrY , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuScharrY ) ;
}

TEST( ImageGPUFiltering , y_derivative_scharr_normalized )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuScharrY = ImageScharrYDerivative( gpuImg , ctx , true ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuScharrY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrY ;
  ImageScharrYDerivative( cpuImg , cpuScharrY , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuScharrY ) ;
}

TEST( ImageGPUFiltering , y_derivative_scharr_unnormalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuScharrY = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuScharrY == nullptr ) ;

  const bool ok = ImageScharrYDerivative( gpuScharrY , gpuImg , ctx , false ) ;
  EXPECT_EQ( true , ok ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuScharrY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrY ;
  ImageScharrYDerivative( cpuImg , cpuScharrY , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuScharrY ) ;
}

TEST( ImageGPUFiltering , y_derivative_scharr_normalized_cl_res )
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

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem gpuScharrY = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuScharrY == nullptr ) ;

  const bool ok = ImageScharrYDerivative( gpuScharrY , gpuImg , ctx , true ) ;
  EXPECT_EQ( true , ok ) ;
  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( gpuScharrY , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrY ;
  ImageScharrYDerivative( cpuImg , cpuScharrY , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuScharrY ) ;
}

// Scaled Scharr
TEST( ImageGPUFiltering , x_derivative_scaled_scharr_unnormalized )
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
  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem res = ImageScaledScharrXDerivative( gpuImg , 3 , ctx , false ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( res , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrX ;
  ImageScaledScharrXDerivative( cpuImg , cpuScharrX , 3 , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( res ) ;
}

TEST( ImageGPUFiltering , x_derivative_scaled_scharr_normalized )
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
  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem res = ImageScaledScharrXDerivative( gpuImg , 3 , ctx , true ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( res , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrX ;
  ImageScaledScharrXDerivative( cpuImg , cpuScharrX , 3 , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( res ) ;
}

TEST( ImageGPUFiltering , x_derivative_scaled_scharr_unnormalized_cl_res )
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
  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem res = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , res == nullptr ) ;

  const bool ok = ImageScaledScharrXDerivative( res , gpuImg , 3 , ctx , false ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( res , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrX ;
  ImageScaledScharrXDerivative( cpuImg , cpuScharrX , 3 , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( res ) ;
}

TEST( ImageGPUFiltering , x_derivative_scaled_scharr_normalized_cl_res )
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
  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem res = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , res == nullptr ) ;

  const bool ok = ImageScaledScharrXDerivative( res , gpuImg , 3 , ctx , true ) ;
  EXPECT_EQ( ok , true ) ;

  Image<float> resDerivX ;
  bool cvtRes = FromOpenCLImage( res , resDerivX , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrX ;
  ImageScaledScharrXDerivative( cpuImg , cpuScharrX , 3 , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrX( y , x ) , resDerivX( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( res ) ;
}
// Scaled-Y
TEST( ImageGPUFiltering , y_derivative_scaled_scharr_unnormalized )
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
  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem res = ImageScaledScharrYDerivative( gpuImg , 3 , ctx , false ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( res , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrY ;
  ImageScaledScharrYDerivative( cpuImg , cpuScharrY , 3 , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( res ) ;
}

TEST( ImageGPUFiltering , y_derivative_scaled_scharr_normalized )
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
  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem res = ImageScaledScharrYDerivative( gpuImg , 3 , ctx , true ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( res , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrY ;
  ImageScaledScharrYDerivative( cpuImg , cpuScharrY , 3 , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( res ) ;
}

TEST( ImageGPUFiltering , y_derivative_scaled_scharr_unnormalized_cl_res )
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
  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem res = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , res == nullptr ) ;

  const bool ok = ImageScaledScharrYDerivative( res , gpuImg , 3 , ctx , false ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( res , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrY ;
  ImageScaledScharrYDerivative( cpuImg , cpuScharrY , 3 , false ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( res ) ;
}

TEST( ImageGPUFiltering , y_derivative_scaled_scharr_normalized_cl_res )
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
  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem res = ctx.createImage( w , h , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , res == nullptr ) ;

  const bool ok = ImageScaledScharrYDerivative( res , gpuImg , 3 , ctx , true ) ;
  EXPECT_EQ( ok , true ) ;

  Image<float> resDerivY ;
  bool cvtRes = FromOpenCLImage( res , resDerivY , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  Image<float> cpuScharrY ;
  ImageScaledScharrYDerivative( cpuImg , cpuScharrY , 3 , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      EXPECT_NEAR( cpuScharrY( y , x ) , resDerivY( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( res ) ;
}

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
