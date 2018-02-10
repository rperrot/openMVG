// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_GPU_KERNELS_IMAGE_GPU_FILTERING_KERNELS_HPP
#define OPENMVG_IMAGE_GPU_KERNELS_IMAGE_GPU_FILTERING_KERNELS_HPP

#include <string>

namespace openMVG
{
namespace image
{
namespace gpu
{
namespace kernels
{

const std::string krnsImageFilteringDerivativeXUnnormalizedCentralDiff =
  R"(
    /* Standard Derivatives */
    __kernel void image_x_derivative_unnormalized( __write_only image2d_t outImg , read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        write_imagef( outImg , pos , read_imagef( img , sampler , pos + (int2)( 1 , 0 ) ) - 
                                     read_imagef( img , sampler , pos + (int2)( - 1 , 0 ) ) );
      }
    }

    )";

const std::string krnsImageFilteringDerivativeXNormalizedCentralDiff =
  R"(
    __kernel void image_x_derivative_normalized( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        write_imagef( outImg , pos , 0.5f * ( read_imagef( img , sampler , pos + (int2)( 1 , 0 ) ) - 
                                              read_imagef( img , sampler , pos + (int2)( - 1 , 0 ) ) ) );
      }      
    }
    
    )";

const std::string krnsImageFilteringDerivativeYUnnormalizedCentralDiff =
  R"(
    __kernel void image_y_derivative_unnormalized( __write_only image2d_t outImg , read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        write_imagef( outImg , pos , read_imagef( img , sampler , pos + (int2)( 0 , 1 ) ) - 
                                     read_imagef( img , sampler , pos + (int2)( 0 , -1 ) ) );
      }
    }
    
    )";

const std::string krnsImageFilteringDerivativeYNormalizedCentralDiff =
  R"(
    __kernel void image_y_derivative_normalized( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        write_imagef( outImg , pos , 0.5f * ( read_imagef( img , sampler , pos + (int2)( 0 , 1 ) ) - 
                                              read_imagef( img , sampler , pos + (int2)( 0 , -1 ) ) ) );
      }      
    }
  )" ;

const std::string krnsImageFilteringDerivativeXUnnormalizedSobel =
  R"(
    /* SOBEL */
    __kernel void image_x_derivative_sobel_unnormalized_naive( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = read_imagef( img , sampler , pos + (int2)( -1 , -1 ) ) + 
                     2.0f * read_imagef( img , sampler , pos + (int2)( -1 , 0 ) ) +
                            read_imagef( img , sampler , pos + (int2)( -1 , 1 ) ) ;
        float4 sobel_next = read_imagef( img , sampler , pos + (int2)( 1 , -1 ) ) +
                     2.0f * read_imagef( img , sampler , pos + (int2)( 1 , 0 ) ) +
                            read_imagef( img , sampler , pos + (int2)( 1 , 1 ) ) ;
        write_imagef( outImg , pos , sobel_next - sobel_prev ) ; 
      }      
    }

    __kernel void image_x_derivative_sobel_unnormalized_local( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      __local float4 cachedData[ WORK_GROUP_SIZE + 2 ][ WORK_GROUP_SIZE + 2 ] ;

      int local_x = get_local_id( 0 ) ;
      int local_y = get_local_id( 1 ) ;
      int group_id_x = get_group_id( 0 ) ;
      int group_id_y = get_group_id( 1 ) ; 

      // Center position of the kernel 
      int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
      int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

      cachedData[ 1 + local_x ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y ) ) ;  
      if( local_x == 0 )
      {
        // Left elements 
        cachedData[ 0 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y ) ) ;
        // Top left
        if( local_y == 0 )
        {
          cachedData[ 0 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ; 
        }
        // Bottom left 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ 0 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ;
        }
      }
      if( local_x == (WORK_GROUP_SIZE - 1) )
      {
        cachedData[ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y ) ) ;
        // Top right 
        if( local_y == 0 )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ; 
        }
        // Bottom right 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ;
        } 
      }
      if( local_y == 0 )
      {
        cachedData[ local_x + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y - 1 ) ) ;
      }
      if( local_y == (WORK_GROUP_SIZE-1) )
      {
        cachedData[ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y + 1 ) ) ; 
      }

      barrier( CLK_LOCAL_MEM_FENCE ) ; 

      if( pix_x < get_image_width( outImg ) && pix_y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = cachedData[ local_x ][ local_y ] + 
                     2.0f * cachedData[ local_x ][ local_y + 1 ] +
                            cachedData[ local_x ][ local_y + 2 ] ;
        float4 sobel_next = cachedData[ local_x + 2 ][ local_y ] +
                     2.0f * cachedData[ local_x + 2 ][ local_y + 1 ] +
                            cachedData[ local_x + 2 ][ local_y + 2 ] ;
        write_imagef( outImg , (int2)( pix_x , pix_y ) , sobel_next - sobel_prev ) ; 
      }
    }
)";

const std::string krnsImageFilteringDerivativeXNormalizedSobel =
R"(
    __kernel void image_x_derivative_sobel_normalized_naive( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = read_imagef( img , sampler , pos + (int2)( -1 , -1 ) ) + 
                     2.0f * read_imagef( img , sampler , pos + (int2)( -1 , 0 ) ) +
                            read_imagef( img , sampler , pos + (int2)( -1 , 1 ) ) ;
        float4 sobel_next = read_imagef( img , sampler , pos + (int2)( 1 , -1 ) ) +
                     2.0f * read_imagef( img , sampler , pos + (int2)( 1 , 0 ) ) +
                            read_imagef( img , sampler , pos + (int2)( 1 , 1 ) ) ;
        write_imagef( outImg , pos , 0.125f * ( sobel_next - sobel_prev ) ) ; 
      }
    }

    __kernel void image_x_derivative_sobel_normalized_local( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      __local float4 cachedData[ WORK_GROUP_SIZE + 2 ][ WORK_GROUP_SIZE + 2 ] ;

      int local_x = get_local_id( 0 ) ;
      int local_y = get_local_id( 1 ) ;
      int group_id_x = get_group_id( 0 ) ;
      int group_id_y = get_group_id( 1 ) ; 

      // Center position of the kernel 
      int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
      int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

      cachedData[ 1 + local_x ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y ) ) ;  
      if( local_x == 0 )
      {
        // Left elements 
        cachedData[ 0 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y ) ) ;
        // Top left
        if( local_y == 0 )
        {
          cachedData[ 0 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ; 
        }
        // Bottom left 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ 0 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ;
        }
      }
      if( local_x == (WORK_GROUP_SIZE - 1) )
      {
        cachedData[ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y ) ) ;
        // Top right 
        if( local_y == 0 )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ; 
        }
        // Bottom right 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ;
        } 
      }
      if( local_y == 0 )
      {
        cachedData[ local_x + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y - 1 ) ) ;
      }
      if( local_y == (WORK_GROUP_SIZE-1) )
      {
        cachedData[ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y + 1 ) ) ; 
      }

      barrier( CLK_LOCAL_MEM_FENCE ) ; 

      if( pix_x < get_image_width( outImg ) && pix_y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = cachedData[ local_x ][ local_y ] + 
                     2.0f * cachedData[ local_x ][ local_y + 1 ] +
                            cachedData[ local_x ][ local_y + 2 ] ;
        float4 sobel_next = cachedData[ local_x + 2 ][ local_y ] +
                     2.0f * cachedData[ local_x + 2 ][ local_y + 1 ] +
                            cachedData[ local_x + 2 ][ local_y + 2 ] ;
        write_imagef( outImg , (int2)( pix_x , pix_y ) , 0.125f * ( sobel_next - sobel_prev ) ) ; 
      }
    }

    )";

const std::string krnsImageFilteringDerivativeYUnnormalizedSobel =
  R"(
    __kernel void image_y_derivative_sobel_unnormalized_naive( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = read_imagef( img , sampler , pos + (int2)( -1 , -1 ) ) + 
                     2.0f * read_imagef( img , sampler , pos + (int2)( 0 , -1 ) ) +
                            read_imagef( img , sampler , pos + (int2)( 1 , -1 ) ) ;
        float4 sobel_next = read_imagef( img , sampler , pos + (int2)( -1 , 1 ) ) +
                     2.0f * read_imagef( img , sampler , pos + (int2)( 0 , 1 ) ) +
                            read_imagef( img , sampler , pos + (int2)( 1 , 1 ) ) ;
        write_imagef( outImg , pos , sobel_next - sobel_prev ) ; 
      }      
    }

    __kernel void image_y_derivative_sobel_unnormalized_local( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      __local float4 cachedData[ WORK_GROUP_SIZE + 2 ][ WORK_GROUP_SIZE + 2 ] ;

      int local_x = get_local_id( 0 ) ;
      int local_y = get_local_id( 1 ) ;
      int group_id_x = get_group_id( 0 ) ;
      int group_id_y = get_group_id( 1 ) ; 

      // Center position of the kernel 
      int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
      int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

      cachedData[ 1 + local_x ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y ) ) ;  
      if( local_x == 0 )
      {
        // Left elements 
        cachedData[ 0 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y ) ) ;
        // Top left
        if( local_y == 0 )
        {
          cachedData[ 0 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ; 
        }
        // Bottom left 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ 0 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ;
        }
      }
      if( local_x == (WORK_GROUP_SIZE - 1) )
      {
        cachedData[ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y ) ) ;
        // Top right 
        if( local_y == 0 )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ; 
        }
        // Bottom right 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ;
        } 
      }
      if( local_y == 0 )
      {
        cachedData[ local_x + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y - 1 ) ) ;
      }
      if( local_y == (WORK_GROUP_SIZE-1) )
      {
        cachedData[ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y + 1 ) ) ; 
      }

      barrier( CLK_LOCAL_MEM_FENCE ) ; 

      if( pix_x < get_image_width( outImg ) && pix_y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = cachedData[ local_x     ][ local_y ] + 
                     2.0f * cachedData[ local_x + 1 ][ local_y ] +
                            cachedData[ local_x + 2 ][ local_y ] ;
        float4 sobel_next = cachedData[ local_x     ][ local_y + 2 ] +
                     2.0f * cachedData[ local_x + 1 ][ local_y + 2 ] +
                            cachedData[ local_x + 2 ][ local_y + 2 ] ;
        write_imagef( outImg , (int2)( pix_x , pix_y ) , sobel_next - sobel_prev ) ; 
      }
    }
)";

const std::string krnsImageFilteringDerivativeYNormalizedSobel =
R"(
    __kernel void image_y_derivative_sobel_normalized_naive( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = read_imagef( img , sampler , pos + (int2)( -1 , -1 ) ) + 
                     2.0f * read_imagef( img , sampler , pos + (int2)( 0 , -1 ) ) +
                            read_imagef( img , sampler , pos + (int2)( 1 , -1 ) ) ;
        float4 sobel_next = read_imagef( img , sampler , pos + (int2)( -1 , 1 ) ) +
                     2.0f * read_imagef( img , sampler , pos + (int2)( 0 , 1 ) ) +
                            read_imagef( img , sampler , pos + (int2)( 1 , 1 ) ) ;
        write_imagef( outImg , pos , 0.125f * ( sobel_next - sobel_prev ) ) ; 
      }
    }

    __kernel void image_y_derivative_sobel_normalized_local( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      __local float4 cachedData[ WORK_GROUP_SIZE + 2 ][ WORK_GROUP_SIZE + 2 ] ;

      int local_x = get_local_id( 0 ) ;
      int local_y = get_local_id( 1 ) ;
      int group_id_x = get_group_id( 0 ) ;
      int group_id_y = get_group_id( 1 ) ; 

      // Center position of the kernel 
      int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
      int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

      cachedData[ 1 + local_x ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y ) ) ;  
      if( local_x == 0 )
      {
        // Left elements 
        cachedData[ 0 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y ) ) ;
        // Top left
        if( local_y == 0 )
        {
          cachedData[ 0 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ; 
        }
        // Bottom left 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ 0 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ;
        }
      }
      if( local_x == (WORK_GROUP_SIZE - 1) )
      {
        cachedData[ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y ) ) ;
        // Top right 
        if( local_y == 0 )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ; 
        }
        // Bottom right 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ;
        } 
      }
      if( local_y == 0 )
      {
        cachedData[ local_x + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y - 1 ) ) ;
      }
      if( local_y == (WORK_GROUP_SIZE-1) )
      {
        cachedData[ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y + 1 ) ) ; 
      }

      barrier( CLK_LOCAL_MEM_FENCE ) ; 

      if( pix_x < get_image_width( outImg ) && pix_y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = cachedData[ local_x ][ local_y ] + 
                     2.0f * cachedData[ local_x + 1][ local_y ] +
                            cachedData[ local_x + 2][ local_y ] ;
        float4 sobel_next = cachedData[ local_x  ][ local_y + 2 ] +
                     2.0f * cachedData[ local_x + 1 ][ local_y + 2 ] +
                            cachedData[ local_x + 2 ][ local_y + 2 ] ;
        write_imagef( outImg , (int2)( pix_x , pix_y ) , 0.125f * ( sobel_next - sobel_prev ) ) ; 
      }
    }
)";

    /* Sharr */
const std::string krnsImageFilteringDerivativeXUnnormalizedScharr =
R"(
    __kernel void image_x_derivative_scharr_unnormalized_naive( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = 3.0f * read_imagef( img , sampler , pos + (int2)( -1 , -1 ) ) + 
                           10.0f * read_imagef( img , sampler , pos + (int2)( -1 , 0 ) ) +
                            3.0f * read_imagef( img , sampler , pos + (int2)( -1 , 1 ) ) ;
        float4 sobel_next = 3.0f * read_imagef( img , sampler , pos + (int2)( 1 , -1 ) ) +
                           10.0f * read_imagef( img , sampler , pos + (int2)( 1 , 0 ) ) +
                            3.0f * read_imagef( img , sampler , pos + (int2)( 1 , 1 ) ) ;
        write_imagef( outImg , pos , sobel_next - sobel_prev ) ; 
      }      
    }

    __kernel void image_x_derivative_scharr_unnormalized_local( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      __local float4 cachedData[ WORK_GROUP_SIZE + 2 ][ WORK_GROUP_SIZE + 2 ] ;

      int local_x = get_local_id( 0 ) ;
      int local_y = get_local_id( 1 ) ;
      int group_id_x = get_group_id( 0 ) ;
      int group_id_y = get_group_id( 1 ) ; 

      // Center position of the kernel 
      int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
      int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

      cachedData[ 1 + local_x ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y ) ) ;  
      if( local_x == 0 )
      {
        // Left elements 
        cachedData[ 0 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y ) ) ;
        // Top left
        if( local_y == 0 )
        {
          cachedData[ 0 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ; 
        }
        // Bottom left 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ 0 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ;
        }
      }
      if( local_x == (WORK_GROUP_SIZE - 1) )
      {
        cachedData[ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y ) ) ;
        // Top right 
        if( local_y == 0 )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ; 
        }
        // Bottom right 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ;
        } 
      }
      if( local_y == 0 )
      {
        cachedData[ local_x + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y - 1 ) ) ;
      }
      if( local_y == (WORK_GROUP_SIZE-1) )
      {
        cachedData[ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y + 1 ) ) ; 
      }

      barrier( CLK_LOCAL_MEM_FENCE ) ; 

      if( pix_x < get_image_width( outImg ) && pix_y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = 3.0f * cachedData[ local_x ][ local_y ] + 
                           10.0f * cachedData[ local_x ][ local_y + 1 ] +
                            3.0f * cachedData[ local_x ][ local_y + 2 ] ;
        float4 sobel_next = 3.0f * cachedData[ local_x + 2 ][ local_y ] +
                           10.0f * cachedData[ local_x + 2 ][ local_y + 1 ] +
                            3.0f * cachedData[ local_x + 2 ][ local_y + 2 ] ;
        write_imagef( outImg , (int2)( pix_x , pix_y ) , sobel_next - sobel_prev ) ; 
      }
    }
)";

const std::string krnsImageFilteringDerivativeXNormalizedScharr =
R"(
    __kernel void image_x_derivative_scharr_normalized_naive( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = 3.0f * read_imagef( img , sampler , pos + (int2)( -1 , -1 ) ) + 
                           10.0f * read_imagef( img , sampler , pos + (int2)( -1 , 0 ) ) +
                            3.0f * read_imagef( img , sampler , pos + (int2)( -1 , 1 ) ) ;
        float4 sobel_next = 3.0f * read_imagef( img , sampler , pos + (int2)( 1 , -1 ) ) +
                           10.0f * read_imagef( img , sampler , pos + (int2)( 1 , 0 ) ) +
                            3.0f * read_imagef( img , sampler , pos + (int2)( 1 , 1 ) ) ;
        write_imagef( outImg , pos , 0.03125f * ( sobel_next - sobel_prev ) ) ; 
      }
    }

    __kernel void image_x_derivative_scharr_normalized_local( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      __local float4 cachedData[ WORK_GROUP_SIZE + 2 ][ WORK_GROUP_SIZE + 2 ] ;

      int local_x = get_local_id( 0 ) ;
      int local_y = get_local_id( 1 ) ;
      int group_id_x = get_group_id( 0 ) ;
      int group_id_y = get_group_id( 1 ) ; 

      // Center position of the kernel 
      int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
      int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

      cachedData[ 1 + local_x ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y ) ) ;  
      if( local_x == 0 )
      {
        // Left elements 
        cachedData[ 0 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y ) ) ;
        // Top left
        if( local_y == 0 )
        {
          cachedData[ 0 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ; 
        }
        // Bottom left 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ 0 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ;
        }
      }
      if( local_x == (WORK_GROUP_SIZE - 1) )
      {
        cachedData[ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y ) ) ;
        // Top right 
        if( local_y == 0 )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ; 
        }
        // Bottom right 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ;
        } 
      }
      if( local_y == 0 )
      {
        cachedData[ local_x + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y - 1 ) ) ;
      }
      if( local_y == (WORK_GROUP_SIZE-1) )
      {
        cachedData[ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y + 1 ) ) ; 
      }

      barrier( CLK_LOCAL_MEM_FENCE ) ; 

      if( pix_x < get_image_width( outImg ) && pix_y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = 3.0f * cachedData[ local_x ][ local_y ] + 
                           10.0f * cachedData[ local_x ][ local_y + 1 ] +
                            3.0f * cachedData[ local_x ][ local_y + 2 ] ;
        float4 sobel_next = 3.0f * cachedData[ local_x + 2 ][ local_y ] +
                           10.0f * cachedData[ local_x + 2 ][ local_y + 1 ] +
                            3.0f * cachedData[ local_x + 2 ][ local_y + 2 ] ;
        write_imagef( outImg , (int2)( pix_x , pix_y ) , 0.03125f * ( sobel_next - sobel_prev ) ) ; 
      }
    }
)";

const std::string krnsImageFilteringDerivativeYUnnormalizedScharr =
R"(
    __kernel void image_y_derivative_scharr_unnormalized_naive( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = 3.0f * read_imagef( img , sampler , pos + (int2)( -1 , -1 ) ) + 
                           10.0f * read_imagef( img , sampler , pos + (int2)( 0 , -1 ) ) +
                            3.0f * read_imagef( img , sampler , pos + (int2)( 1 , -1 ) ) ;
        float4 sobel_next = 3.0f * read_imagef( img , sampler , pos + (int2)( -1 , 1 ) ) +
                           10.0f * read_imagef( img , sampler , pos + (int2)( 0 , 1 ) ) +
                            3.0f * read_imagef( img , sampler , pos + (int2)( 1 , 1 ) ) ;
        write_imagef( outImg , pos , sobel_next - sobel_prev ) ; 
      }      
    }

    __kernel void image_y_derivative_scharr_unnormalized_local( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      __local float4 cachedData[ WORK_GROUP_SIZE + 2 ][ WORK_GROUP_SIZE + 2 ] ;

      int local_x = get_local_id( 0 ) ;
      int local_y = get_local_id( 1 ) ;
      int group_id_x = get_group_id( 0 ) ;
      int group_id_y = get_group_id( 1 ) ; 

      // Center position of the kernel 
      int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
      int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

      cachedData[ 1 + local_x ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y ) ) ;  
      if( local_x == 0 )
      {
        // Left elements 
        cachedData[ 0 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y ) ) ;
        // Top left
        if( local_y == 0 )
        {
          cachedData[ 0 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ; 
        }
        // Bottom left 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ 0 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ;
        }
      }
      if( local_x == (WORK_GROUP_SIZE - 1) )
      {
        cachedData[ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y ) ) ;
        // Top right 
        if( local_y == 0 )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ; 
        }
        // Bottom right 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ;
        } 
      }
      if( local_y == 0 )
      {
        cachedData[ local_x + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y - 1 ) ) ;
      }
      if( local_y == (WORK_GROUP_SIZE-1) )
      {
        cachedData[ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y + 1 ) ) ; 
      }

      barrier( CLK_LOCAL_MEM_FENCE ) ; 

      if( pix_x < get_image_width( outImg ) && pix_y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = 3.0f * cachedData[ local_x     ][ local_y ] + 
                           10.0f * cachedData[ local_x + 1 ][ local_y ] +
                            3.0f * cachedData[ local_x + 2 ][ local_y ] ;
        float4 sobel_next = 3.0f * cachedData[ local_x     ][ local_y + 2 ] +
                           10.0f * cachedData[ local_x + 1 ][ local_y + 2 ] +
                            3.0f * cachedData[ local_x + 2 ][ local_y + 2 ] ;
        write_imagef( outImg , (int2)( pix_x , pix_y ) , sobel_next - sobel_prev ) ; 
      }
    }
)";

const std::string krnsImageFilteringDerivativeYNormalizedScharr =
R"(
    __kernel void image_y_derivative_scharr_normalized_naive( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 

      if( pos.x < get_image_width( outImg ) && pos.y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = 3.0f * read_imagef( img , sampler , pos + (int2)( -1 , -1 ) ) + 
                           10.0f * read_imagef( img , sampler , pos + (int2)( 0 , -1 ) ) +
                            3.0f * read_imagef( img , sampler , pos + (int2)( 1 , -1 ) ) ;
        float4 sobel_next = 3.0f * read_imagef( img , sampler , pos + (int2)( -1 , 1 ) ) +
                           10.0f * read_imagef( img , sampler , pos + (int2)( 0 , 1 ) ) +
                            3.0f * read_imagef( img , sampler , pos + (int2)( 1 , 1 ) ) ;
        write_imagef( outImg , pos , 0.03125f * ( sobel_next - sobel_prev ) ) ; 
      }
    }

    __kernel void image_y_derivative_scharr_normalized_local( __write_only image2d_t outImg , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
      __local float4 cachedData[ WORK_GROUP_SIZE + 2 ][ WORK_GROUP_SIZE + 2 ] ;

      int local_x = get_local_id( 0 ) ;
      int local_y = get_local_id( 1 ) ;
      int group_id_x = get_group_id( 0 ) ;
      int group_id_y = get_group_id( 1 ) ; 

      // Center position of the kernel 
      int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
      int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

      cachedData[ 1 + local_x ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x , pix_y ) ) ;  
      if( local_x == 0 )
      {
        // Left elements 
        cachedData[ 0 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y ) ) ;
        // Top left
        if( local_y == 0 )
        {
          cachedData[ 0 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ; 
        }
        // Bottom left 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ 0 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ;
        }
      }
      if( local_x == (WORK_GROUP_SIZE - 1) )
      {
        cachedData[ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y ) ) ;
        // Top right 
        if( local_y == 0 )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ; 
        }
        // Bottom right 
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ;
        } 
      }
      if( local_y == 0 )
      {
        cachedData[ local_x + 1 ][ 0 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y - 1 ) ) ;
      }
      if( local_y == (WORK_GROUP_SIZE-1) )
      {
        cachedData[ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = read_imagef( img , sampler , (int2)( pix_x , pix_y + 1 ) ) ; 
      }

      barrier( CLK_LOCAL_MEM_FENCE ) ; 

      if( pix_x < get_image_width( outImg ) && pix_y < get_image_height( outImg ) ) 
      {
        float4 sobel_prev = 3.0f * cachedData[ local_x ][ local_y ] + 
                           10.0f * cachedData[ local_x + 1][ local_y ] +
                            3.0f * cachedData[ local_x + 2][ local_y ] ;
        float4 sobel_next = 3.0f * cachedData[ local_x  ][ local_y + 2 ] +
                           10.0f * cachedData[ local_x + 1 ][ local_y + 2 ] +
                            3.0f * cachedData[ local_x + 2 ][ local_y + 2 ] ;
        write_imagef( outImg , (int2)( pix_x , pix_y ) , 0.03125f * ( sobel_next - sobel_prev ) ) ; 
      }
    }

  )" ;



} // namespace kernels
} // namespace gpu
} // namespace image
} // namespace openMVG

#endif