// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/sift/hierarchical_gaussian_scale_space.hpp"
#include "openMVG/features/sift/gpu/hierarchical_gaussian_scale_space_gpu.hpp"

#include "openMVG/image/gpu/image_gpu_interface.hpp"

#include "openMVG/system/gpu/OpenCLContext.hpp"

#include "testing/testing.h"

using namespace openMVG::features ;
using namespace openMVG::features::gpu ;
using namespace openMVG::image ;
using namespace openMVG::image::gpu ;
using namespace openMVG::system::gpu ;

#include <random>

TEST( HierarchicalGaussianScaleSpaceGPU , compare_with_cpu_no_upscaling )
{
  OpenCLContext ctx ;

  const int supplementary_images = 3 ;
  HierarchicalGaussianScaleSpace octave_gen_cpu(
    6,
    3,
    GaussianScaleSpaceParams( 1.6f, 1.0f, 0.5f, supplementary_images ) );

  HierarchicalGaussianScaleSpaceGPU octave_gen_gpu(
    6,
    3,
    GaussianScaleSpaceParams( 1.6f , 1.0f , 0.5f , supplementary_images ) , ctx ) ;



  int w = 1280 ;
  int h = 720 ;

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

  octave_gen_cpu.SetImage( cpuImg ) ;
  octave_gen_gpu.SetImage( cpuImg ) ;

  Octave cpu_octave ;
  GPUOctave gpu_octave ;
  while( 1 )
  {
    bool next_cpu = octave_gen_cpu.NextOctave( cpu_octave ) ;
    bool next_gpu = octave_gen_gpu.NextOctave( gpu_octave ) ;

    EXPECT_EQ( next_cpu , next_gpu ) ;

    if( ! next_cpu )
    {
      break ;
    }

    // Now compare the octaves
    EXPECT_EQ( cpu_octave.octave_level , gpu_octave.octave_level ) ;
    EXPECT_EQ( cpu_octave.delta , gpu_octave.delta ) ;
    EXPECT_EQ( cpu_octave.sigmas.size() , gpu_octave.sigmas.size() ) ;
    for( size_t id_sigma = 0 ; id_sigma < cpu_octave.sigmas.size() ; ++id_sigma )
    {
      EXPECT_NEAR( cpu_octave.sigmas[ id_sigma ] , gpu_octave.sigmas[ id_sigma ] , 0.0001f ) ;
    }

    EXPECT_EQ( cpu_octave.slices.size() , gpu_octave.slices.size() ) ;
    for( size_t id_slice = 0 ; id_slice < cpu_octave.slices.size() ; ++id_slice )
    {
      Image<float> & cpu_image = cpu_octave.slices[ id_slice ] ;
      cl_mem gpu_image = gpu_octave.slices[ id_slice ] ;

      const size_t cpu_width = cpu_image.Width() ;
      const size_t cpu_height = cpu_image.Height() ;
      size_t gpu_width , gpu_height ;

      size_t width ;
      cl_int err = clGetImageInfo( gpu_image , CL_IMAGE_WIDTH , sizeof( size_t ) , &gpu_width , nullptr ) ;
      EXPECT_EQ( err , CL_SUCCESS ) ;
      size_t height ;
      err = clGetImageInfo( gpu_image , CL_IMAGE_HEIGHT , sizeof( size_t ) , &gpu_height , nullptr ) ;
      EXPECT_EQ( err , CL_SUCCESS ) ;

      EXPECT_EQ( cpu_width , gpu_width ) ;
      EXPECT_EQ( cpu_height , gpu_height ) ;

      Image<float> converted_gpu ;
      FromOpenCLImage( gpu_image , converted_gpu , ctx ) ;

      for( size_t i = 0 ; i < cpu_height ; ++i )
      {
        for( size_t j = 0 ; j < cpu_width ; ++j )
        {
          EXPECT_NEAR( cpu_image( i , j ) , converted_gpu( i, j ) , 0.0001f ) ;
        }
      }
    }
  }
}

TEST( HierarchicalGaussianScaleSpaceGPU , compare_with_cpu_with_upscaling )
{
  OpenCLContext ctx ;

  const int supplementary_images = 3 ;
  HierarchicalGaussianScaleSpace octave_gen_cpu(
    6,
    3,
    GaussianScaleSpaceParams( 1.6f / 2.0f, 1.0f / 2.0f, 0.5f, supplementary_images ) );

  HierarchicalGaussianScaleSpaceGPU octave_gen_gpu(
    6,
    3,
    GaussianScaleSpaceParams( 1.6f / 2.0f, 1.0f / 2.0f, 0.5f, supplementary_images ) , ctx ) ;



  int w = 1280 ;
  int h = 720 ;

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

  octave_gen_cpu.SetImage( cpuImg ) ;
  octave_gen_gpu.SetImage( cpuImg ) ;

  Octave cpu_octave ;
  GPUOctave gpu_octave ;
  while( 1 )
  {
    bool next_cpu = octave_gen_cpu.NextOctave( cpu_octave ) ;
    bool next_gpu = octave_gen_gpu.NextOctave( gpu_octave ) ;

    EXPECT_EQ( next_cpu , next_gpu ) ;

    if( ! next_cpu )
    {
      break ;
    }

    // Now compare the octaves
    EXPECT_EQ( cpu_octave.octave_level , gpu_octave.octave_level ) ;
    EXPECT_EQ( cpu_octave.delta , gpu_octave.delta ) ;
    EXPECT_EQ( cpu_octave.sigmas.size() , gpu_octave.sigmas.size() ) ;
    for( size_t id_sigma = 0 ; id_sigma < cpu_octave.sigmas.size() ; ++id_sigma )
    {
      EXPECT_NEAR( cpu_octave.sigmas[ id_sigma ] , gpu_octave.sigmas[ id_sigma ] , 0.0001f ) ;
    }

    EXPECT_EQ( cpu_octave.slices.size() , gpu_octave.slices.size() ) ;
    for( size_t id_slice = 0 ; id_slice < cpu_octave.slices.size() ; ++id_slice )
    {
      Image<float> & cpu_image = cpu_octave.slices[ id_slice ] ;
      cl_mem gpu_image = gpu_octave.slices[ id_slice ] ;

      const size_t cpu_width = cpu_image.Width() ;
      const size_t cpu_height = cpu_image.Height() ;
      size_t gpu_width , gpu_height ;

      size_t width ;
      cl_int err = clGetImageInfo( gpu_image , CL_IMAGE_WIDTH , sizeof( size_t ) , &gpu_width , nullptr ) ;
      EXPECT_EQ( err , CL_SUCCESS ) ;
      size_t height ;
      err = clGetImageInfo( gpu_image , CL_IMAGE_HEIGHT , sizeof( size_t ) , &gpu_height , nullptr ) ;
      EXPECT_EQ( err , CL_SUCCESS ) ;

      EXPECT_EQ( cpu_width , gpu_width ) ;
      EXPECT_EQ( cpu_height , gpu_height ) ;

      Image<float> converted_gpu ;
      FromOpenCLImage( gpu_image , converted_gpu , ctx ) ;

      for( size_t i = 0 ; i < cpu_height ; ++i )
      {
        for( size_t j = 0 ; j < cpu_width ; ++j )
        {
          EXPECT_NEAR( cpu_image( i , j ) , converted_gpu( i, j ) , 0.0001f ) ;
        }
      }
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
