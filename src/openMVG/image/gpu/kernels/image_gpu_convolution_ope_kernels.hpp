// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_GPU_KERNELS_IMAGE_GPU_CONVOLUTIONS_KERNELS_HPP
#define OPENMVG_IMAGE_GPU_KERNELS_IMAGE_GPU_CONVOLUTIONS_KERNELS_HPP

#include <string>

namespace openMVG
{
namespace image
{
namespace gpu
{
namespace kernels
{

// Kernel that computes naive image convolution
// Assuming kernel is odd size (odd width, odd height)
// Assuming filter is given rowwise
const std::string krnsImageConvolve2dNaive =
  R"(
    __kernel void convolve_2d_naive_f( __write_only image2d_t outImg , constant float * filter , __read_only image2d_t img , const int krnHalfWidth , const int krnHalfHeight )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      
      const int2 pos = { get_global_id(0) , get_global_id(1) } ;

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      { 

        float4 sum = (float4) 0.f ; 

        for( int y = - krnHalfHeight ; y <= krnHalfHeight ; ++y )
        {
          for( int x = - krnHalfWidth ; x <= krnHalfWidth ; ++x )
          {
            // Note : index computation 
            // x* = x + krnHalfWidth 
            // y* = y + krnHalfHeight
            // index = (y*) * (2 * krnHalfWidth+1) + (x*) 
            const float filterValue = filter[ ( x + krnHalfWidth ) + ( y + krnHalfHeight ) * ( krnHalfWidth * 2 + 1 ) ] ;
            const float4 pixValue = read_imagef( img , sampler , (pos) + (int2) (x,y) ) ; 
            sum += filterValue * pixValue ; 
          }
        }
        write_imagef( outImg , pos , sum ) ; 
      }
    }
)";

// Available for kernels up to 29x29
// Requires that work group is equal to 16 
// TODO : provide WORK_GROUP_SIZE as de define ? 
const std::string krnsImageConvolve2dLocalMem =
  R"(
    #define WORK_GROUP_SIZE 16
    __kernel void convolve_2d_local_f( __write_only image2d_t outImg , constant float * filter , __read_only image2d_t img , const int krnHalfWidth , const int krnHalfHeight )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      
      // 16 -> 1 per work group
      // 28 (+1) -> Maximum size of the kernel that can be used 
      __local float4 cachedData[ WORK_GROUP_SIZE + 28 ][ WORK_GROUP_SIZE + 28 ] ; 

      int local_x = get_local_id( 0 ) ;
      int local_y = get_local_id( 1 ) ;
      int group_id_x = get_group_id( 0 ) ;
      int group_id_y = get_group_id( 1 ) ; 

      // Center position of the kernel 
      int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
      int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

      int cache_base_x = krnHalfWidth ;
      int cache_base_y = krnHalfHeight ;  

      // Load 9 values per work item 
      // 1 2 3
      // 4 5 6 
      // 7 8 9 
      const int cache_prev_x = cache_base_x - WORK_GROUP_SIZE + local_x ;
      const int cache_cur_x = cache_base_x + local_x ;
      const int cache_next_x = cache_base_x + WORK_GROUP_SIZE + local_x ;
      const int cache_prev_y = cache_base_y - WORK_GROUP_SIZE + local_y ;
      const int cache_cur_y  = cache_base_y + local_y ; 
      const int cache_next_y = cache_base_y + WORK_GROUP_SIZE + local_y ; 

      const int read_prev_x = pix_x - WORK_GROUP_SIZE ;
      const int read_prev_y = pix_y - WORK_GROUP_SIZE ;
      const int read_next_x = pix_x + WORK_GROUP_SIZE ;
      const int read_next_y = pix_y + WORK_GROUP_SIZE ; 

      const bool do_prev_x = cache_prev_x >= 0 ;
      const bool do_next_x = local_x < krnHalfWidth ; 
      const bool do_prev_y = cache_prev_y >= 0 ;
      const bool do_next_y = local_y < krnHalfHeight ; 

      // 1 4 7
      if( do_prev_x )
      {
        if( do_prev_y )
        {
          cachedData[ cache_prev_x ][ cache_prev_y ] = read_imagef( img , sampler , (int2)( read_prev_x , read_prev_y ) );
        }
        cachedData[ cache_prev_x ][ cache_cur_y ] = read_imagef( img , sampler , (int2)( read_prev_x , pix_y ) ) ; 
        if( do_next_y )
        {
          cachedData[ cache_prev_x ][ cache_next_y ] = read_imagef( img , sampler, (int2)(read_prev_x, read_next_y ) ) ;
        }
      }
      // 2 5 8 
      {
        if( do_prev_y )
        {
          cachedData[ cache_cur_x ][ cache_prev_y ] = read_imagef( img , sampler , (int2)( pix_x , read_prev_y ) ) ;
        }
        cachedData[ cache_cur_x ][ cache_cur_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y ) );
        if( do_next_y )
        {
          cachedData[ cache_cur_x ][ cache_next_y ] = read_imagef( img , sampler , (int2)( pix_x , read_next_y ) ) ;
        }
      }
      // 3 6 9 
      if( do_next_x )
      {
        if( do_prev_y )
        {
          cachedData[ cache_next_x ][ cache_prev_y ] = read_imagef( img , sampler , (int2)( read_next_x , read_prev_y ) ) ;
        }
        cachedData[ cache_next_x ][ cache_cur_y ] = read_imagef( img , sampler , (int2)( read_next_x , pix_y ) ) ;
        if( do_next_y )
        {
          cachedData[ cache_next_x ][ cache_next_y ] = read_imagef( img , sampler , (int2)( read_next_x , read_next_y ) ) ;
        }
      }

      barrier( CLK_LOCAL_MEM_FENCE ) ; 

      if( pix_x < get_image_width( outImg ) && pix_y < get_image_height( outImg ) ) 
      { 

        float4 sum = (float4) 0.f ; 

        for( int y = - krnHalfHeight ; y <= krnHalfHeight ; ++y )
        {
          for( int x = - krnHalfWidth ; x <= krnHalfWidth ; ++x )
          {
            // Note : index computation 
            // x* = x + krnHalfWidth 
            // y* = y + krnHalfHeight
            // index = (y*) * (2 * krnHalfWidth+1) + (x*) 
            const float filterValue = filter[ ( x + krnHalfWidth ) + ( y + krnHalfHeight ) * ( krnHalfWidth * 2 + 1 ) ] ;

            const float4 pixValue = cachedData[ cache_cur_x + x ][ cache_cur_y + y ] ; 
            sum += filterValue * pixValue ; 
          }
        }
        write_imagef( outImg , (int2)( pix_x , pix_y ) , sum ) ; 
      }
    }
)";

// Naive horizontal convolution 
const std::string krnsImageHorizontalConvolveNaive = 
R"(
__kernel void horizontal_convolve_naive_f( __write_only image2d_t outImg , constant float * filter , __read_only image2d_t img , const int krnHalfSize )
{
  sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      
  const int2 pos = { get_global_id(0) , get_global_id(1) } ;

  if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
  { 
    float4 sum = 0.f ; 
    for( int x = -krnHalfSize ; x <= krnHalfSize ; ++x )
    {
      const float filterValue = filter[ krnHalfSize + x ] ; 
      const float4 pixValue = read_imagef( img , sampler , pos + (int2) ( x , 0 ) ) ; 
      sum += filterValue * pixValue ; 
    }
    write_imagef( outImg , pos , sum ) ;
  }
}
  )" ;
// Horizontal convolution with local memory prefetching
// Up to kernel size equal to 33
const std::string krnsImageHorizontalConvolveLocalMem32 =
  R"(
#define WORK_GROUP_SIZE 16
__kernel void horizontal_convolve_local_32_f( __write_only image2d_t outImg , constant float * filter , __read_only image2d_t img , const int krnHalfSize )
{
  sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
  
  int local_x = get_local_id( 0 ) ;
  int local_y = get_local_id( 1 ) ;
  int group_id_x = get_group_id( 0 ) ;
  int group_id_y = get_group_id( 1 ) ; 

  // WORK_GROUP_SIZE before + WORK_GROUP_SIZE + WORK_GROUP_SIZE after 
  __local float4 cachedData[ WORK_GROUP_SIZE * 3 ][ WORK_GROUP_SIZE ] ; 

  // Center position of the kernel 
  int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
  int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

  // fetch data using at most 3 read per work item 
  int cache_base_x = krnHalfSize ;

  int cache_prev_x = cache_base_x - WORK_GROUP_SIZE + local_x ; 
  int cache_cur_x  = cache_base_x + local_x ;
  int cache_next_x = cache_base_x + WORK_GROUP_SIZE + local_x ; 

  if( cache_prev_x >= 0 )
  {
    cachedData[ cache_prev_x ][ local_y ] = read_imagef( img , sampler , (int2) ( pix_x - WORK_GROUP_SIZE , pix_y ) ) ;
  } 
  cachedData[ cache_cur_x ][ local_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y ) ) ;
  if( local_x < krnHalfSize )
  {
    cachedData[ cache_next_x ][ local_y ] = read_imagef( img , sampler , (int2)( pix_x + WORK_GROUP_SIZE , pix_y ) ) ;
  }

  barrier( CLK_LOCAL_MEM_FENCE ) ; 

  if( pix_x < get_image_width( outImg ) && pix_y < get_image_height( outImg ) ) 
  { 
    float4 sum = 0.f ; 
    for( int x = -krnHalfSize ; x <= krnHalfSize ; ++x )
    {
      const float filterValue = filter[ krnHalfSize + x ] ; 
      const float4 pixValue = cachedData[ cache_cur_x + x ][ local_y ] ;
      sum += filterValue * pixValue ; 
    }
    write_imagef( outImg , (int2)( pix_x , pix_y ) , sum ) ;
  }
}
  )" ;

// Naive vertical convolution
const std::string krnsImageVerticalConvolveNaive =
  R"(
__kernel void vertical_convolve_naive_f( __write_only image2d_t outImg , constant float * filter , __read_only image2d_t img , const int krnHalfSize )
{
  sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      
  const int2 pos = { get_global_id(0) , get_global_id(1) } ;

  if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
  { 
    float4 sum = 0.f ; 
    for( int x = -krnHalfSize ; x <= krnHalfSize ; ++x )
    {
      const float filterValue = filter[ krnHalfSize + x ] ; 
      const float4 pixValue = read_imagef( img , sampler , pos + (int2) ( 0 , x ) ) ; 
      sum += filterValue * pixValue ; 
    }
    write_imagef( outImg , pos , sum ) ;
  }
}
  )" ;
// Vertical convolution with local memory prefetching
// Up to kernel size equal to 33
const std::string krnsImageVerticalConvolveLocalMem32 =
  R"(
#define WORK_GROUP_SIZE 16
__kernel void vertical_convolve_local_32_f( __write_only image2d_t outImg , constant float * filter , __read_only image2d_t img , const int krnHalfSize )
{
  sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
  
  int local_x = get_local_id( 0 ) ;
  int local_y = get_local_id( 1 ) ;
  int group_id_x = get_group_id( 0 ) ;
  int group_id_y = get_group_id( 1 ) ; 

  // WORK_GROUP_SIZE before + WORK_GROUP_SIZE + WORK_GROUP_SIZE after 
  __local float4 cachedData[ WORK_GROUP_SIZE ][ WORK_GROUP_SIZE * 3 ] ; 

  // Center position of the kernel 
  int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
  int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

  // fetch data using at most 3 read per work item 
  int cache_base_y = krnHalfSize ;

  int cache_prev_y = cache_base_y - WORK_GROUP_SIZE + local_y ; 
  int cache_cur_y  = cache_base_y + local_y ;
  int cache_next_y = cache_base_y + WORK_GROUP_SIZE + local_y ; 

  if( cache_prev_y >= 0 )
  {
    cachedData[ local_x ][ cache_prev_y ] = read_imagef( img , sampler , (int2) ( pix_x , pix_y - WORK_GROUP_SIZE ) ) ;
  } 
  cachedData[ local_x ][ cache_cur_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y ) ) ;
  if( local_y < krnHalfSize )
  {
    cachedData[ local_x ][ cache_next_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y + WORK_GROUP_SIZE ) ) ;
  }

  barrier( CLK_LOCAL_MEM_FENCE ) ; 

  if( pix_x < get_image_width( outImg ) && pix_y < get_image_height( outImg ) ) 
  { 
    float4 sum = 0.f ; 
    for( int x = -krnHalfSize ; x <= krnHalfSize ; ++x )
    {
      const float filterValue = filter[ krnHalfSize + x ] ; 
      const float4 pixValue = cachedData[ local_x ][ cache_cur_y + x ] ;
      sum += filterValue * pixValue ; 
    }
    write_imagef( outImg , (int2)( pix_x , pix_y ) , sum ) ;
  }
}
  )" ;

} // namespace kernels
} // namespace gpu
} // namespace image
} // namespace openMVG

#endif