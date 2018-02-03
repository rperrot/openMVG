// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "testing/testing.h"

#include "openMVG/image/image_container.hpp"
#include "openMVG/image/gpu/image_gpu_interface.hpp"
#include "openMVG/image/gpu/image_gpu_local_maxima.hpp"

#include "openMVG/system/gpu/OpenCLContext.hpp"

using namespace openMVG::image ;
using namespace openMVG::image::gpu ;
using namespace openMVG::system::gpu ;


// Local min max in 2d
TEST( ImageGPULocalMinMax , local_min_max2d )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;

  Image<float> cpuImg( w , h ) ;

  // 1st local max 2,2
  cpuImg( 2 , 2 ) = 1.f ;

  // 2nd local max : 11,10
  cpuImg( 10 , 10 ) = 0.9f ;
  cpuImg( 11 , 10 ) = 1.f ;

  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;

  cl_mem local_max = ImageLocalMaxima( gpuImg , ctx ) ;

  Image<float> resLocalMax ;
  bool cvtRes = FromOpenCLImage( local_max , resLocalMax , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      if( ( y == 2 && x == 2 ) || ( y == 11 && x == 10 ) )
      {
        EXPECT_NEAR( 1.f , resLocalMax( y , x ) , 0.0001 ) ;
      }
      else
      {
        EXPECT_NEAR( 0.f , resLocalMax( y , x ) , 0.0001 ) ;
      }
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( local_max ) ;
}

// Local min max in 2d
TEST( ImageGPULocalMinMax , local_min_max3d )
{
  OpenCLContext ctx ;

  int w = 32 ;
  int h = 24 ;


  Image<float> cpuImgLow( w , h ) ;
  Image<float> cpuImg( w , h ) ;
  Image<float> cpuImgAbove( w , h ) ;

  // 1st local max 2,2
  cpuImg( 2 , 2 ) = 1.f ;

  // 2nd local max : 11,10
  cpuImg( 10 , 10 ) = 0.9f ;
  cpuImg( 11 , 10 ) = 1.f ;

  // Not a local max
  cpuImgLow( 20 , 20 ) = 1.f ;
  cpuImg( 20 , 20 ) = 2.f ;
  cpuImgAbove( 20 , 19 ) = 2.9f ;

  cl_mem gpuImgLow = ToOpenCLImage( cpuImgLow , ctx ) ;
  cl_mem gpuImg = ToOpenCLImage( cpuImg , ctx ) ;
  cl_mem gpuImgAbove = ToOpenCLImage( cpuImgAbove , ctx ) ;

  cl_mem local_max = ImageLocalMaxima( gpuImgLow , gpuImg , gpuImgAbove , ctx ) ;

  Image<float> resLocalMax ;
  bool cvtRes = FromOpenCLImage( local_max , resLocalMax , ctx ) ;

  EXPECT_EQ( cvtRes , true ) ;

  for( int y = 0 ; y < h ; ++y )
  {
    for( int x = 0 ; x < w ; ++x )
    {
      if( ( y == 2 && x == 2 ) || ( y == 11 && x == 10 ) )
      {
        EXPECT_NEAR( 1.f , resLocalMax( y , x ) , 0.0001 ) ;
      }
      else
      {
        EXPECT_NEAR( 0.f , resLocalMax( y , x ) , 0.0001 ) ;
      }
    }
  }

  clReleaseMemObject( gpuImg ) ;
  clReleaseMemObject( local_max ) ;
}

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
