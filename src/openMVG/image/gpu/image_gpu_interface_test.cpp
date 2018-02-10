// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/image/gpu/image_gpu_interface.hpp"

#include "testing/testing.h"

using namespace openMVG::image ;
using namespace openMVG::image::gpu ;
using namespace openMVG::system::gpu ;

TEST( ImageGPUInterface , uchar_to_opencl_to_uchar )
{
  OpenCLContext ctx ;

  const int nb_row = 24 ;
  const int nb_col = 32 ;

  Image<unsigned char> cpu_img( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpu_img( id_row , id_col ) = ( id_row * id_col ) % 256 ;
    }
  }

  cl_mem gpu_img = ToOpenCLImage( cpu_img , ctx ) ;

  Image<unsigned char> res ;

  bool valid = FromOpenCLImage( gpu_img , res , ctx ) ;

  EXPECT_EQ( valid , true ) ;
  EXPECT_EQ( res.Width() , cpu_img.Width() ) ;
  EXPECT_EQ( res.Height() , cpu_img.Height() ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( res( id_row , id_col ) , cpu_img( id_row , id_col ) ) ;
    }
  }

  clReleaseMemObject( gpu_img ) ;
}

TEST( ImageGPUInterface , uchar_to_opencl_to_invalid_type )
{
  OpenCLContext ctx ;

  const int nb_row = 24 ;
  const int nb_col = 32 ;

  Image<unsigned char> cpu_img( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpu_img( id_row , id_col ) = ( id_row * id_col ) % 256 ;
    }
  }

  cl_mem gpu_img = ToOpenCLImage( cpu_img , ctx ) ;

  Image<float> res ;

  bool valid = FromOpenCLImage( gpu_img , res , ctx ) ;

  EXPECT_EQ( valid , false ) ;

  clReleaseMemObject( gpu_img ) ;
}


TEST( ImageGPUInterface , float_to_opencl_to_float )
{
  OpenCLContext ctx ;

  const int nb_row = 24 ;
  const int nb_col = 32 ;

  Image<float> cpu_img( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpu_img( id_row , id_col ) = ( id_row * id_col ) % 256 ;
    }
  }

  cl_mem gpu_img = ToOpenCLImage( cpu_img , ctx ) ;

  Image<float> res ;

  bool valid = FromOpenCLImage( gpu_img , res , ctx ) ;

  EXPECT_EQ( valid , true ) ;
  EXPECT_EQ( res.Width() , cpu_img.Width() ) ;
  EXPECT_EQ( res.Height() , cpu_img.Height() ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( res( id_row , id_col ) , cpu_img( id_row , id_col ) ) ;
    }
  }
  clReleaseMemObject( gpu_img ) ;
}

TEST( ImageGPUInterface , float_to_opencl_to_invalid_type )
{
  OpenCLContext ctx ;

  const int nb_row = 24 ;
  const int nb_col = 32 ;

  Image<float> cpu_img( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpu_img( id_row , id_col ) = ( id_row * id_col ) % 256 ;
    }
  }

  cl_mem gpu_img = ToOpenCLImage( cpu_img , ctx ) ;

  Image<unsigned char> res ;

  bool valid = FromOpenCLImage( gpu_img , res , ctx ) ;

  EXPECT_EQ( valid , false ) ;
  clReleaseMemObject( gpu_img ) ;
}


TEST( ImageGPUInterface , rgb_uchar_to_opencl_to_rgb_uchar )
{
  OpenCLContext ctx ;

  const int nb_row = 24 ;
  const int nb_col = 32 ;

  Image<Rgb<unsigned char>> cpu_img( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpu_img( id_row , id_col ) = Rgb<unsigned char>( ( id_row * id_col ) % 256 , ( id_row * id_col + 1 ) % 256 , ( id_row * id_col + 2 ) % 256 ) ;
    }
  }

  cl_mem gpu_img = ToOpenCLImage( cpu_img , ctx ) ;

  Image<Rgb<unsigned char>> res ;

  bool valid = FromOpenCLImage( gpu_img , res , ctx ) ;

  EXPECT_EQ( valid , true ) ;
  EXPECT_EQ( res.Width() , cpu_img.Width() ) ;
  EXPECT_EQ( res.Height() , cpu_img.Height() ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( res( id_row , id_col ).r() , cpu_img( id_row , id_col ).r() ) ;
      EXPECT_EQ( res( id_row , id_col ).g() , cpu_img( id_row , id_col ).g() ) ;
      EXPECT_EQ( res( id_row , id_col ).b() , cpu_img( id_row , id_col ).b() ) ;
    }
  }
  clReleaseMemObject( gpu_img ) ;
}

TEST( ImageGPUInterface , rgb_float_to_opencl_to_rgb_float )
{
  OpenCLContext ctx ;

  const int nb_row = 24 ;
  const int nb_col = 32 ;

  Image<Rgb<float>> cpu_img( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpu_img( id_row , id_col ) = Rgb<float>( ( id_row * id_col ) % 256 , ( id_row * id_col + 1 ) % 256 , ( id_row * id_col + 2 ) % 256 ) ;
    }
  }

  cl_mem gpu_img = ToOpenCLImage( cpu_img , ctx ) ;

  Image<Rgb<float>> res ;

  bool valid = FromOpenCLImage( gpu_img , res , ctx ) ;

  EXPECT_EQ( valid , true ) ;
  EXPECT_EQ( res.Width() , cpu_img.Width() ) ;
  EXPECT_EQ( res.Height() , cpu_img.Height() ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( res( id_row , id_col ).r() , cpu_img( id_row , id_col ).r() ) ;
      EXPECT_EQ( res( id_row , id_col ).g() , cpu_img( id_row , id_col ).g() ) ;
      EXPECT_EQ( res( id_row , id_col ).b() , cpu_img( id_row , id_col ).b() ) ;
    }
  }
  clReleaseMemObject( gpu_img ) ;
}

TEST( ImageGPUInterface , rgba_uchar_to_opencl_to_rgba_uchar )
{
  OpenCLContext ctx ;

  const int nb_row = 24 ;
  const int nb_col = 32 ;

  Image<Rgba<unsigned char>> cpu_img( nb_col , nb_row ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      cpu_img( id_row , id_col ) = Rgba<unsigned char>( ( id_row * id_col ) % 256 , ( id_row * id_col + 1 ) % 256 , ( id_row * id_col + 2 ) % 256 , ( id_row * id_col + 3 ) % 256 ) ;
    }
  }

  cl_mem gpu_img = ToOpenCLImage( cpu_img , ctx ) ;

  Image<Rgba<unsigned char>> res ;

  bool valid = FromOpenCLImage( gpu_img , res , ctx ) ;

  EXPECT_EQ( valid , true ) ;
  EXPECT_EQ( res.Width() , cpu_img.Width() ) ;
  EXPECT_EQ( res.Height() , cpu_img.Height() ) ;

  for( int id_row = 0 ; id_row < nb_row ; ++id_row )
  {
    for( int id_col = 0 ; id_col < nb_col ; ++id_col )
    {
      EXPECT_EQ( res( id_row , id_col ).r() , cpu_img( id_row , id_col ).r() ) ;
      EXPECT_EQ( res( id_row , id_col ).g() , cpu_img( id_row , id_col ).g() ) ;
      EXPECT_EQ( res( id_row , id_col ).b() , cpu_img( id_row , id_col ).b() ) ;
    }
  }
  clReleaseMemObject( gpu_img ) ;
}

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
