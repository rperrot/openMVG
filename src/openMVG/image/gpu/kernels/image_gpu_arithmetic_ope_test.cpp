#include "testing/testing.h"

#include "openMVG/image/image_container.hpp"
#include "openMVG/image/pixel_types.hpp"
#include "openMVG/image/gpu/image_gpu_interface.hpp"
#include "openMVG/image/gpu/image_gpu_arithmetics.hpp"

#include "openMVG/system/gpu/OpenCLContext.hpp"

using namespace openMVG::image ;
using namespace openMVG::image::gpu ;
using namespace openMVG::system::gpu ;

TEST( ImageGPUArithmetic , add_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<unsigned char> cpuImgA( nb_col , nb_row ) ;
  Image<unsigned char> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = ( id_row * id_col ) % 128 ;
      cpuImgB( id_row , id_col ) = ( id_row * id_col ) % 128 ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageAdd( gpuImgA , gpuImgB , ctx ) ;

  Image<unsigned char> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ) , ( int ) cpuImgA( id_row , id_col ) + cpuImgB( id_row , id_col ) ) ;
    }
  }
}

TEST( ImageGPUArithmetic , add_f )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<float> cpuImgA( nb_col , nb_row ) ;
  Image<float> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = ( id_row * id_col ) % 128 ;
      cpuImgB( id_row , id_col ) = ( id_row * id_col ) % 128 ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageAdd( gpuImgA , gpuImgB , ctx ) ;

  Image<float> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_NEAR( cpuSum( id_row , id_col ) , cpuImgA( id_row , id_col ) + cpuImgB( id_row , id_col ) , 0.0001 ) ;
    }
  }
}

TEST( ImageGPUArithmetic , add_rgb_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<Rgb<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgb<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgb<unsigned char>
                                   ( ( id_row * id_col ) % 128 ,
                                     ( id_row * id_col + 1 ) % 128 ,
                                     ( id_row * id_col + 2 ) % 128
                                   );
      cpuImgB( id_row , id_col ) = Rgb<unsigned char>
                                   ( ( id_row * id_col ) % 128 ,
                                     ( id_row * id_col + 1 ) % 128 ,
                                     ( id_row * id_col + 2 ) % 128
                                   );
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageAdd( gpuImgA , gpuImgB , ctx ) ;

  Image<Rgb<unsigned char>> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).r() , ( int ) cpuImgA( id_row , id_col ).r() + cpuImgB( id_row , id_col ).r() ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).g() , ( int ) cpuImgA( id_row , id_col ).g() + cpuImgB( id_row , id_col ).g() ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).b() , ( int ) cpuImgA( id_row , id_col ).b() + cpuImgB( id_row , id_col ).b() ) ;
    }
  }
}

TEST( ImageGPUArithmetic , add_rgba_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<Rgba<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgba<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgba<unsigned char>
                                   ( ( id_row * id_col ) % 128 ,
                                     ( id_row * id_col + 1 ) % 128 ,
                                     ( id_row * id_col + 2 ) % 128 ,
                                     ( id_row * id_col + 3 ) % 128
                                   );
      cpuImgB( id_row , id_col ) = Rgba<unsigned char>
                                   ( ( id_row * id_col ) % 128 ,
                                     ( id_row * id_col + 1 ) % 128 ,
                                     ( id_row * id_col + 2 ) % 128 ,
                                     ( id_row * id_col + 3 ) % 128
                                   );
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageAdd( gpuImgA , gpuImgB , ctx ) ;

  Image<Rgba<unsigned char>> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).r() , ( int ) ( cpuImgA( id_row , id_col ).r() + cpuImgB( id_row , id_col ).r() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).g() , ( int ) ( cpuImgA( id_row , id_col ).g() + cpuImgB( id_row , id_col ).g() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).b() , ( int ) ( cpuImgA( id_row , id_col ).b() + cpuImgB( id_row , id_col ).b() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).a() , ( int ) ( cpuImgA( id_row , id_col ).a() + cpuImgB( id_row , id_col ).a() ) ) ;
    }
  }
}


/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
