#include "testing/testing.h"

#include "openMVG/image/image_container.hpp"
#include "openMVG/image/image_resampling.hpp"
#include "openMVG/image/gpu/image_gpu_interface.hpp"
#include "openMVG/image/gpu/image_gpu_resampling.hpp"

#include "openMVG/system/gpu/OpenCLContext.hpp"

#include <random>

using namespace openMVG::image ;
using namespace openMVG::image::gpu ;
using namespace openMVG::system::gpu ;


TEST( ImageGPUResampling , decimate_f )
{
  std::cerr << (int) ( (unsigned char) 255.4) << std::endl ; 

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
  EXPECT_EQ( gpuImg != nullptr , true ) ;

  cl_mem gpuResampled = ImageDecimate( gpuImg , ctx ) ;
  EXPECT_EQ( gpuResampled != nullptr , true ) ;

  Image<float> cpuResampled ;
  bool cvtRes = FromOpenCLImage( gpuResampled , cpuResampled , ctx ) ;
  EXPECT_EQ( cvtRes , true ) ;

  Image<float> reference ;
  ImageDecimate( cpuImg , reference ) ;

  for( int y = 0 ; y < h / 2 ; ++y )
  {
    for( int x = 0 ; x < w / 2; ++x )
    {
      EXPECT_NEAR( reference( y , x ) , cpuResampled( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuResampled ) ;
}

TEST( ImageGPUResampling , decimate_ui )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<unsigned char> cpuImg( w , h ) ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 255 ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;
  EXPECT_EQ( gpuImg != nullptr , true ) ;

  cl_mem gpuResampled = ImageDecimate( gpuImg , ctx ) ;
  EXPECT_EQ( gpuResampled != nullptr , true ) ;

  Image<unsigned char> cpuResampled ;
  bool cvtRes = FromOpenCLImage( gpuResampled , cpuResampled , ctx ) ;
  EXPECT_EQ( cvtRes , true ) ;

  Image<unsigned char> reference ;
  ImageDecimate( cpuImg , reference ) ;

  for( int y = 0 ; y < h / 2 ; ++y )
  {
    for( int x = 0 ; x < w / 2; ++x )
    {
      EXPECT_NEAR( reference( y , x ) , cpuResampled( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuResampled ) ;
}

TEST( ImageGPUResampling , decimate_f_cl_res )
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
  EXPECT_EQ( gpuImg != nullptr , true ) ;

  cl_mem gpuResampled = ctx.createImage( w / 2 , h / 2 , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;

  const bool ok = ImageDecimate( gpuResampled , gpuImg , ctx ) ;
  EXPECT_EQ( ok , true ) ;

  Image<float> cpuResampled ;
  bool cvtRes = FromOpenCLImage( gpuResampled , cpuResampled , ctx ) ;
  EXPECT_EQ( cvtRes , true ) ;

  Image<float> reference ;
  ImageDecimate( cpuImg , reference ) ;

  for( int y = 0 ; y < h / 2 ; ++y )
  {
    for( int x = 0 ; x < w / 2; ++x )
    {
      EXPECT_NEAR( reference( y , x ) , cpuResampled( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuResampled ) ;
}

TEST( ImageGPUResampling , decimate_ui_cl_res )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<unsigned char> cpuImg( w , h ) ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 255 ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;
  EXPECT_EQ( gpuImg != nullptr , true ) ;

  cl_mem gpuResampled = ctx.createImage( w / 2 , h / 2 , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_U_INT_8 ) ;

  const bool ok = ImageDecimate( gpuResampled , gpuImg , ctx ) ;
  EXPECT_EQ( ok , true ) ;

  Image<unsigned char> cpuResampled ;
  bool cvtRes = FromOpenCLImage( gpuResampled , cpuResampled , ctx ) ;
  EXPECT_EQ( cvtRes , true ) ;

  Image<unsigned char> reference ;
  ImageDecimate( cpuImg , reference ) ;

  for( int y = 0 ; y < h / 2 ; ++y )
  {
    for( int x = 0 ; x < w / 2; ++x )
    {
      EXPECT_NEAR( reference( y , x ) , cpuResampled( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuResampled ) ;
}

TEST( ImageGPUResampling , upsample_f )
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
  EXPECT_EQ( gpuImg != nullptr , true ) ;

  cl_mem gpuResampled = ImageUpsample( gpuImg , ctx ) ;
  EXPECT_EQ( gpuResampled != nullptr , true ) ;

  Image<float> cpuResampled ;
  bool cvtRes = FromOpenCLImage( gpuResampled , cpuResampled , ctx ) ;
  EXPECT_EQ( cvtRes , true ) ;

  Image<float> reference ;
  ImageUpsample( cpuImg , reference ) ;

  for( int y = 0 ; y < h * 2 ; ++y )
  {
    for( int x = 0 ; x < w * 2; ++x )
    {
      EXPECT_NEAR( reference( y , x ) , cpuResampled( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuResampled ) ;
}

TEST( ImageGPUResampling , upsample_ui )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<unsigned char> cpuImg( w , h ) ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 255 ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;
  EXPECT_EQ( gpuImg != nullptr , true ) ;

  cl_mem gpuResampled = ImageUpsample( gpuImg , ctx ) ;
  EXPECT_EQ( gpuResampled != nullptr , true ) ;

  Image<unsigned char> cpuResampled ;
  bool cvtRes = FromOpenCLImage( gpuResampled , cpuResampled , ctx ) ;
  EXPECT_EQ( cvtRes , true ) ;

  Image<unsigned char> reference ;
  ImageUpsample( cpuImg , reference ) ;

  for( int y = 0 ; y < h * 2 ; ++y )
  {
    for( int x = 0 ; x < w * 2; ++x )
    {
      EXPECT_NEAR( reference( y , x ) , cpuResampled( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuResampled ) ;
}

TEST( ImageGPUResampling , upsample_f_cl_res )
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
  EXPECT_EQ( gpuImg != nullptr , true ) ;

  cl_mem gpuResampled = ctx.createImage( w * 2 , h * 2 , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;

  const bool ok = ImageUpsample( gpuResampled , gpuImg , ctx ) ;
  EXPECT_EQ( ok , true ) ;

  Image<float> cpuResampled ;
  bool cvtRes = FromOpenCLImage( gpuResampled , cpuResampled , ctx ) ;
  EXPECT_EQ( cvtRes , true ) ;

  Image<float> reference ;
  ImageUpsample( cpuImg , reference ) ;

  for( int y = 0 ; y < h * 2 ; ++y )
  {
    for( int x = 0 ; x < w * 2; ++x )
    {
      EXPECT_NEAR( reference( y , x ) , cpuResampled( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuResampled ) ;
}

TEST( ImageGPUResampling , upsample_ui_cl_res )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<unsigned char> cpuImg( w , h ) ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 255 ) ;
  std::mt19937 rng( 0 ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      cpuImg( y , x ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;
  EXPECT_EQ( gpuImg != nullptr , true ) ;

  cl_mem gpuResampled = ctx.createImage( w * 2 , h * 2 , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_U_INT_8 ) ;

  const bool ok = ImageUpsample( gpuResampled , gpuImg , ctx ) ;
  EXPECT_EQ( ok , true ) ;

  Image<unsigned char> cpuResampled ;
  bool cvtRes = FromOpenCLImage( gpuResampled , cpuResampled , ctx ) ;
  EXPECT_EQ( cvtRes , true ) ;

  Image<unsigned char> reference ;
  ImageUpsample( cpuImg , reference ) ;

  for( int y = 0 ; y < h * 2 ; ++y )
  {
    for( int x = 0 ; x < w * 2; ++x )
    {
      EXPECT_NEAR( reference( y , x ) , cpuResampled( y , x ) , 0.001 ) ;
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( gpuResampled ) ;
}



/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
