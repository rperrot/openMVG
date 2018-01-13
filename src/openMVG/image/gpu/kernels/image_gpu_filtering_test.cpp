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


/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
