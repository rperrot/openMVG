// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "testing/testing.h"

#include "openMVG/image/image_container.hpp"
#include "openMVG/image/pixel_types.hpp"
#include "openMVG/image/gpu/image_gpu_interface.hpp"
#include "openMVG/image/gpu/image_gpu_arithmetics.hpp"

#include "openMVG/system/gpu/OpenCLContext.hpp"

#include <random>

using namespace openMVG::image ;
using namespace openMVG::image::gpu ;
using namespace openMVG::system::gpu ;

// Add

TEST( ImageGPUArithmetic , add_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 127 ) ;
  std::mt19937 rng( 0 ) ;

  Image<unsigned char> cpuImgA( nb_col , nb_row ) ;
  Image<unsigned char> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distrib( rng ) ;
      cpuImgB( id_row , id_col ) = distrib( rng ) ;
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

  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , add_f )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_real_distribution<float> distrib( -500.f , 500.f ) ;
  std::mt19937 rng( 0 ) ;

  Image<float> cpuImgA( nb_col , nb_row ) ;
  Image<float> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distrib( rng ) ;
      cpuImgB( id_row , id_col ) = distrib( rng ) ;
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

  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , add_rgb_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 127 ) ;
  std::mt19937 rng( 0 ) ;

  Image<Rgb<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgb<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgb<unsigned char>
                                   (
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng )
                                   );
      cpuImgB( id_row , id_col ) = Rgb<unsigned char>
                                   (
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng )
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

  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , add_rgba_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 127 ) ;
  std::mt19937 rng( 0 ) ;

  Image<Rgba<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgba<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgba<unsigned char>
                                   (
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng )
                                   );
      cpuImgB( id_row , id_col ) = Rgba<unsigned char>
                                   (
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng )
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

  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

// Add 2
TEST( ImageGPUArithmetic , add_ui_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 127 ) ;
  std::mt19937 rng( 0 ) ;

  Image<unsigned char> cpuImgA( nb_col , nb_row ) ;
  Image<unsigned char> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distrib( rng ) ;
      cpuImgB( id_row , id_col ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_U_INT_8 ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageAdd( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

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

  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , add_f_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_real_distribution<float> distrib( -500.f , 500.f ) ;
  std::mt19937 rng( 0 ) ;

  Image<float> cpuImgA( nb_col , nb_row ) ;
  Image<float> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distrib( rng ) ;
      cpuImgB( id_row , id_col ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageAdd( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

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

  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , add_rgb_ui_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 127 ) ;
  std::mt19937 rng( 0 ) ;

  Image<Rgb<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgb<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgb<unsigned char>
                                   (
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng )
                                   );
      cpuImgB( id_row , id_col ) = Rgb<unsigned char>
                                   (
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng )
                                   );
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_RGBA , OPENCL_IMAGE_DATA_TYPE_U_INT_8 ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageAdd( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

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

  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , add_rgba_ui_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 127 ) ;
  std::mt19937 rng( 0 ) ;

  Image<Rgba<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgba<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgba<unsigned char>
                                   (
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng )
                                   );
      cpuImgB( id_row , id_col ) = Rgba<unsigned char>
                                   (
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng ) ,
                                     distrib( rng )
                                   );
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_RGBA , OPENCL_IMAGE_DATA_TYPE_U_INT_8 ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageAdd( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

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

  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

// Sub
TEST( ImageGPUArithmetic , sub_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_int_distribution<unsigned char> distribA( 128 , 255 ) ;
  std::uniform_int_distribution<unsigned char> distribB( 0 , 128 ) ;
  std::mt19937 rng( 0 ) ;

  Image<unsigned char> cpuImgA( nb_col , nb_row ) ;
  Image<unsigned char> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distribA( rng ) ;
      cpuImgB( id_row , id_col ) = distribB( rng ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageSub( gpuImgA , gpuImgB , ctx ) ;

  Image<unsigned char> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ) , ( int ) cpuImgA( id_row , id_col ) - ( int ) cpuImgB( id_row , id_col ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , sub_f )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<float> cpuImgA( nb_col , nb_row ) ;
  Image<float> cpuImgB( nb_col , nb_row ) ;

  std::uniform_real_distribution<float> distrib( -500.f , 500.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distrib( rng ) ;
      cpuImgB( id_row , id_col ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageSub( gpuImgA , gpuImgB , ctx ) ;

  Image<float> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_NEAR( cpuSum( id_row , id_col ) , cpuImgA( id_row , id_col ) - cpuImgB( id_row , id_col ) , 0.0001 ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , sub_rgb_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<Rgb<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgb<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  std::uniform_int_distribution<unsigned char> distribA( 128 , 255 ) ;
  std::uniform_int_distribution<unsigned char> distribB( 0 , 127 ) ;

  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgb<unsigned char>( distribA( rng ) , distribA( rng ) , distribA( rng ) ) ;
      cpuImgB( id_row , id_col ) = Rgb<unsigned char>( distribB( rng ) , distribB( rng ) , distribB( rng ) ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageSub( gpuImgA , gpuImgB , ctx ) ;

  Image<Rgb<unsigned char>> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).r() , ( int ) ( cpuImgA( id_row , id_col ).r() - cpuImgB( id_row , id_col ).r() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).g() , ( int ) ( cpuImgA( id_row , id_col ).g() - cpuImgB( id_row , id_col ).g() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).b() , ( int ) ( cpuImgA( id_row , id_col ).b() - cpuImgB( id_row , id_col ).b() ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , sub_rgba_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<Rgba<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgba<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  std::uniform_int_distribution<unsigned char> distribA( 128 , 255 ) ;
  std::uniform_int_distribution<unsigned char> distribB( 0 , 128 ) ;

  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgba<unsigned char>( distribA( rng ) , distribA( rng ) , distribA( rng ) , distribA( rng ) ) ;
      cpuImgB( id_row , id_col ) = Rgba<unsigned char>( distribB( rng ) , distribB( rng ) , distribB( rng ) , distribB( rng ) ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageSub( gpuImgA , gpuImgB , ctx ) ;

  Image<Rgba<unsigned char>> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).r() , ( int ) ( cpuImgA( id_row , id_col ).r() - cpuImgB( id_row , id_col ).r() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).g() , ( int ) ( cpuImgA( id_row , id_col ).g() - cpuImgB( id_row , id_col ).g() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).b() , ( int ) ( cpuImgA( id_row , id_col ).b() - cpuImgB( id_row , id_col ).b() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).a() , ( int ) ( cpuImgA( id_row , id_col ).a() - cpuImgB( id_row , id_col ).a() ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

// Sub 2
TEST( ImageGPUArithmetic , sub_ui_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_int_distribution<unsigned char> distribA( 128 , 255 ) ;
  std::uniform_int_distribution<unsigned char> distribB( 0 , 128 ) ;
  std::mt19937 rng( 0 ) ;

  Image<unsigned char> cpuImgA( nb_col , nb_row ) ;
  Image<unsigned char> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distribA( rng ) ;
      cpuImgB( id_row , id_col ) = distribB( rng ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_U_INT_8 ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageSub( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

  Image<unsigned char> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ) , ( int ) cpuImgA( id_row , id_col ) - ( int ) cpuImgB( id_row , id_col ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , sub_f_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<float> cpuImgA( nb_col , nb_row ) ;
  Image<float> cpuImgB( nb_col , nb_row ) ;

  std::uniform_real_distribution<float> distrib( -500.f , 500.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distrib( rng ) ;
      cpuImgB( id_row , id_col ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageSub( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

  Image<float> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_NEAR( cpuSum( id_row , id_col ) , cpuImgA( id_row , id_col ) - cpuImgB( id_row , id_col ) , 0.0001 ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , sub_rgb_ui_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<Rgb<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgb<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  std::uniform_int_distribution<unsigned char> distribA( 128 , 255 ) ;
  std::uniform_int_distribution<unsigned char> distribB( 0 , 127 ) ;

  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgb<unsigned char>( distribA( rng ) , distribA( rng ) , distribA( rng ) ) ;
      cpuImgB( id_row , id_col ) = Rgb<unsigned char>( distribB( rng ) , distribB( rng ) , distribB( rng ) ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_RGBA , OPENCL_IMAGE_DATA_TYPE_U_INT_8 ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageSub( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

  Image<Rgb<unsigned char>> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).r() , ( int ) ( cpuImgA( id_row , id_col ).r() - cpuImgB( id_row , id_col ).r() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).g() , ( int ) ( cpuImgA( id_row , id_col ).g() - cpuImgB( id_row , id_col ).g() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).b() , ( int ) ( cpuImgA( id_row , id_col ).b() - cpuImgB( id_row , id_col ).b() ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , sub_rgba_ui_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<Rgba<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgba<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  std::uniform_int_distribution<unsigned char> distribA( 128 , 255 ) ;
  std::uniform_int_distribution<unsigned char> distribB( 0 , 128 ) ;

  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgba<unsigned char>( distribA( rng ) , distribA( rng ) , distribA( rng ) , distribA( rng ) ) ;
      cpuImgB( id_row , id_col ) = Rgba<unsigned char>( distribB( rng ) , distribB( rng ) , distribB( rng ) , distribB( rng ) ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_RGBA , OPENCL_IMAGE_DATA_TYPE_U_INT_8 ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageSub( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

  Image<Rgba<unsigned char>> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).r() , ( int ) ( cpuImgA( id_row , id_col ).r() - cpuImgB( id_row , id_col ).r() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).g() , ( int ) ( cpuImgA( id_row , id_col ).g() - cpuImgB( id_row , id_col ).g() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).b() , ( int ) ( cpuImgA( id_row , id_col ).b() - cpuImgB( id_row , id_col ).b() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).a() , ( int ) ( cpuImgA( id_row , id_col ).a() - cpuImgB( id_row , id_col ).a() ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

// Mul
TEST( ImageGPUArithmetic , mul_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 15 ) ;
  std::mt19937 rng( 0 ) ;

  Image<unsigned char> cpuImgA( nb_col , nb_row ) ;
  Image<unsigned char> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distrib( rng ) ;
      cpuImgB( id_row , id_col ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageMul( gpuImgA , gpuImgB , ctx ) ;

  Image<unsigned char> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ) , ( int ) cpuImgA( id_row , id_col ) * ( int ) cpuImgB( id_row , id_col ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , mul_f )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<float> cpuImgA( nb_col , nb_row ) ;
  Image<float> cpuImgB( nb_col , nb_row ) ;

  std::uniform_real_distribution<float> distrib( -128.f , 128.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distrib( rng ) ;
      cpuImgB( id_row , id_col ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageMul( gpuImgA , gpuImgB , ctx ) ;

  Image<float> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_NEAR( cpuSum( id_row , id_col ) , cpuImgA( id_row , id_col ) * cpuImgB( id_row , id_col ) , 0.001 ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , mul_rgb_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<Rgb<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgb<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 15 ) ;
  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgb<unsigned char>( distrib( rng ) , distrib( rng ) , distrib( rng ) ) ;
      cpuImgB( id_row , id_col ) = Rgb<unsigned char>( distrib( rng ) , distrib( rng ) , distrib( rng ) ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageMul( gpuImgA , gpuImgB , ctx ) ;

  Image<Rgb<unsigned char>> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).r() , ( int ) ( cpuImgA( id_row , id_col ).r() * cpuImgB( id_row , id_col ).r() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).g() , ( int ) ( cpuImgA( id_row , id_col ).g() * cpuImgB( id_row , id_col ).g() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).b() , ( int ) ( cpuImgA( id_row , id_col ).b() * cpuImgB( id_row , id_col ).b() ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , mul_rgba_ui )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<Rgba<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgba<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 15 ) ;
  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgba<unsigned char>( distrib( rng ) , distrib( rng ) , distrib( rng ) , distrib( rng ) ) ;
      cpuImgB( id_row , id_col ) = Rgba<unsigned char>( distrib( rng ) , distrib( rng ) , distrib( rng ) , distrib( rng ) ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ImageMul( gpuImgA , gpuImgB , ctx ) ;

  Image<Rgba<unsigned char>> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).r() , ( int ) ( cpuImgA( id_row , id_col ).r() * cpuImgB( id_row , id_col ).r() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).g() , ( int ) ( cpuImgA( id_row , id_col ).g() * cpuImgB( id_row , id_col ).g() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).b() , ( int ) ( cpuImgA( id_row , id_col ).b() * cpuImgB( id_row , id_col ).b() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).a() , ( int ) ( cpuImgA( id_row , id_col ).a() * cpuImgB( id_row , id_col ).a() ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

// Mul2
TEST( ImageGPUArithmetic , mul_ui_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 15 ) ;
  std::mt19937 rng( 0 ) ;

  Image<unsigned char> cpuImgA( nb_col , nb_row ) ;
  Image<unsigned char> cpuImgB( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distrib( rng ) ;
      cpuImgB( id_row , id_col ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_U_INT_8 ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageMul( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

  Image<unsigned char> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ) , ( int ) cpuImgA( id_row , id_col ) * ( int ) cpuImgB( id_row , id_col ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , mul_f_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<float> cpuImgA( nb_col , nb_row ) ;
  Image<float> cpuImgB( nb_col , nb_row ) ;

  std::uniform_real_distribution<float> distrib( -128.f , 128.f ) ;
  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = distrib( rng ) ;
      cpuImgB( id_row , id_col ) = distrib( rng ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageMul( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

  Image<float> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_NEAR( cpuSum( id_row , id_col ) , cpuImgA( id_row , id_col ) * cpuImgB( id_row , id_col ) , 0.001 ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , mul_rgb_ui_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<Rgb<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgb<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 15 ) ;
  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgb<unsigned char>( distrib( rng ) , distrib( rng ) , distrib( rng ) ) ;
      cpuImgB( id_row , id_col ) = Rgb<unsigned char>( distrib( rng ) , distrib( rng ) , distrib( rng ) ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_RGBA , OPENCL_IMAGE_DATA_TYPE_U_INT_8 ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageMul( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

  Image<Rgb<unsigned char>> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).r() , ( int ) ( cpuImgA( id_row , id_col ).r() * cpuImgB( id_row , id_col ).r() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).g() , ( int ) ( cpuImgA( id_row , id_col ).g() * cpuImgB( id_row , id_col ).g() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).b() , ( int ) ( cpuImgA( id_row , id_col ).b() * cpuImgB( id_row , id_col ).b() ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}

TEST( ImageGPUArithmetic , mul_rgba_ui_2 )
{
  OpenCLContext ctx ;

  int nb_col = 32 ;
  int nb_row = 24 ;

  Image<Rgba<unsigned char>> cpuImgA( nb_col , nb_row ) ;
  Image<Rgba<unsigned char>> cpuImgB( nb_col , nb_row ) ;

  std::uniform_int_distribution<unsigned char> distrib( 0 , 15 ) ;
  std::mt19937 rng( 0 ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpuImgA( id_row , id_col ) = Rgba<unsigned char>( distrib( rng ) , distrib( rng ) , distrib( rng ) , distrib( rng ) ) ;
      cpuImgB( id_row , id_col ) = Rgba<unsigned char>( distrib( rng ) , distrib( rng ) , distrib( rng ) , distrib( rng ) ) ;
    }
  }

  cl_mem gpuImgA = ToOpenCLImage( cpuImgA , ctx ) ;
  cl_mem gpuImgB = ToOpenCLImage( cpuImgB , ctx ) ;

  cl_mem gpuSum = ctx.createImage( nb_col , nb_row , OPENCL_IMAGE_CHANNEL_ORDER_RGBA , OPENCL_IMAGE_DATA_TYPE_U_INT_8 ) ;
  EXPECT_EQ( false , gpuSum == nullptr ) ;

  bool ok = ImageMul( gpuSum , gpuImgA , gpuImgB , ctx ) ;

  EXPECT_EQ( true , ok ) ;

  Image<Rgba<unsigned char>> cpuSum ;
  bool res = FromOpenCLImage( gpuSum , cpuSum , ctx ) ;

  EXPECT_EQ( true , res ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).r() , ( int ) ( cpuImgA( id_row , id_col ).r() * cpuImgB( id_row , id_col ).r() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).g() , ( int ) ( cpuImgA( id_row , id_col ).g() * cpuImgB( id_row , id_col ).g() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).b() , ( int ) ( cpuImgA( id_row , id_col ).b() * cpuImgB( id_row , id_col ).b() ) ) ;
      EXPECT_EQ( ( int ) cpuSum( id_row , id_col ).a() , ( int ) ( cpuImgA( id_row , id_col ).a() * cpuImgB( id_row , id_col ).a() ) ) ;
    }
  }
  clReleaseMemObject( gpuImgA ) ;
  clReleaseMemObject( gpuImgB ) ;
  clReleaseMemObject( gpuSum ) ;
}


/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
