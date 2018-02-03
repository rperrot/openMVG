// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_GPU_KERNELS_IMAGE_GPU_MIN_MAX_KERNELS_HPP
#define OPENMVG_IMAGE_GPU_KERNELS_IMAGE_GPU_MIN_MAX_KERNELS_HPP

#include <string>

namespace openMVG
{
namespace image
{
namespace gpu
{
namespace kernels
{
const std::string krnsImageLocalMaxima =
  R"(

      // Local maxima in 2d image 
      // output is 1.f if pixel is a local max 0.f if not
      __kernel void image_local_min_max_2d( write_only image2d_t res , read_only image2d_t img )
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

        cachedData[ 1 + local_x ][ 1 + local_y ] = fabs( read_imagef( img , sampler , (int2)( pix_x , pix_y ) ) ) ;  
        if( local_x == 0 )
        {
          // Left elements 
          cachedData[ 0 ][ 1 + local_y ] = fabs( read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y ) ) ) ;
          // Top left
          if( local_y == 0 )
          {
            cachedData[ 0 ][ 0 ] = fabs( read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ) ; 
          }
          // Bottom left 
          if( local_y == (WORK_GROUP_SIZE-1) )
          {
            cachedData[ 0 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( img , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ) ;
          }
        }
        if( local_x == (WORK_GROUP_SIZE - 1) )
        {
          cachedData[ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = fabs( read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y ) ) ) ;
          // Top right 
          if( local_y == 0 )
          {
            cachedData[ WORK_GROUP_SIZE + 1 ][ 0 ] = fabs( read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ) ; 
          }
          // Bottom right 
          if( local_y == (WORK_GROUP_SIZE-1) )
          {
            cachedData[ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( img , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ) ;
          }
        }
        if( local_y == 0 )
        {
          cachedData[ local_x + 1 ][ 0 ] = fabs( read_imagef( img , sampler , (int2)( pix_x , pix_y - 1 ) ) ) ;
        }
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( img , sampler , (int2)( pix_x , pix_y + 1 ) ) ) ; 
        }

        barrier( CLK_LOCAL_MEM_FENCE ) ; 

        const float4 pix = cachedData[ local_x + 1 ][ local_y + 1 ] ; 

        int4 local_min_max = 
          ( cachedData[ local_x ][ local_y ]         < pix ) && 
          ( cachedData[ local_x + 1 ][ local_y ]     < pix ) && 
          ( cachedData[ local_x + 2 ][ local_y ]     < pix ) &&

          ( cachedData[ local_x ][ local_y + 1 ]     < pix ) && 
          ( cachedData[ local_x + 2 ][ local_y + 1 ] < pix ) && 

          ( cachedData[ local_x ][ local_y + 2 ]     < pix ) && 
          ( cachedData[ local_x + 1 ][ local_y + 2 ] < pix ) && 
          ( cachedData[ local_x + 2 ][ local_y + 2 ] < pix ) ; 

        if( pix_x < get_image_width( res ) && pix_y < get_image_height( res ) ) 
        {
          write_imagef( res , (int2)( pix_x , pix_y ) , convert_float4( abs( local_min_max ) ) ) ;
        }
      }

      // Local maxima in 3d (ie: against 3 images)
      // output is 1.f if pixel is a local max 0.f if not
      // Note local max search is in B
      __kernel void image_local_min_max_3d( write_only image2d_t res , 
                                            read_only image2d_t imgA , 
                                            read_only image2d_t imgB ,
                                            read_only image2d_t imgC )
      {
        sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
        __local float4 cachedData[3][ WORK_GROUP_SIZE + 2 ][ WORK_GROUP_SIZE + 2 ] ;

        int local_x = get_local_id( 0 ) ;
        int local_y = get_local_id( 1 ) ;
        int group_id_x = get_group_id( 0 ) ;
        int group_id_y = get_group_id( 1 ) ; 

        // Center position of the kernel 
        int pix_x = group_id_x * WORK_GROUP_SIZE + local_x ;
        int pix_y = group_id_y * WORK_GROUP_SIZE + local_y ;

        cachedData[0][ 1 + local_x ][ 1 + local_y ] = fabs( read_imagef( imgA , sampler , (int2)( pix_x , pix_y ) ) ) ;  
        cachedData[1][ 1 + local_x ][ 1 + local_y ] = fabs( read_imagef( imgB , sampler , (int2)( pix_x , pix_y ) ) ) ;  
        cachedData[2][ 1 + local_x ][ 1 + local_y ] = fabs( read_imagef( imgC , sampler , (int2)( pix_x , pix_y ) ) ) ;  

        if( local_x == 0 )
        {
          // Left elements 
          cachedData[0][ 0 ][ 1 + local_y ] = fabs( read_imagef( imgA , sampler , (int2)( pix_x - 1 , pix_y ) ) ) ;
          cachedData[1][ 0 ][ 1 + local_y ] = fabs( read_imagef( imgB , sampler , (int2)( pix_x - 1 , pix_y ) ) ) ;
          cachedData[2][ 0 ][ 1 + local_y ] = fabs( read_imagef( imgC , sampler , (int2)( pix_x - 1 , pix_y ) ) ) ;

          // Top left
          if( local_y == 0 )
          {
            cachedData[0][ 0 ][ 0 ] = fabs( read_imagef( imgA , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ) ; 
            cachedData[1][ 0 ][ 0 ] = fabs( read_imagef( imgB , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ) ; 
            cachedData[2][ 0 ][ 0 ] = fabs( read_imagef( imgC , sampler , (int2)( pix_x - 1 , pix_y - 1 ) ) ) ; 
          }
          // Bottom left 
          if( local_y == (WORK_GROUP_SIZE-1) )
          {
            cachedData[0][ 0 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( imgA , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ) ;
            cachedData[1][ 0 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( imgB , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ) ;
            cachedData[2][ 0 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( imgC , sampler , (int2)( pix_x - 1 , pix_y + 1 ) ) ) ;
          }
        }
        if( local_x == (WORK_GROUP_SIZE - 1) )
        {
          cachedData[0][ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = fabs( read_imagef( imgA , sampler , (int2)( pix_x + 1 , pix_y ) ) ) ;
          cachedData[1][ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = fabs( read_imagef( imgB , sampler , (int2)( pix_x + 1 , pix_y ) ) ) ;
          cachedData[2][ WORK_GROUP_SIZE + 1 ][ 1 + local_y ] = fabs( read_imagef( imgC , sampler , (int2)( pix_x + 1 , pix_y ) ) ) ;
          // Top right 
          if( local_y == 0 )
          {
            cachedData[0][ WORK_GROUP_SIZE + 1 ][ 0 ] = fabs( read_imagef( imgA , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ) ; 
            cachedData[1][ WORK_GROUP_SIZE + 1 ][ 0 ] = fabs( read_imagef( imgB , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ) ; 
            cachedData[2][ WORK_GROUP_SIZE + 1 ][ 0 ] = fabs( read_imagef( imgC , sampler , (int2)( pix_x + 1 , pix_y - 1 ) ) ) ; 
          }
          // Bottom right 
          if( local_y == (WORK_GROUP_SIZE-1) )
          {
            cachedData[0][ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( imgA , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ) ;
            cachedData[1][ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( imgB , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ) ;
            cachedData[2][ WORK_GROUP_SIZE + 1 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( imgC , sampler , (int2)( pix_x + 1 , pix_y + 1  ) ) ) ;
          }
        }
        if( local_y == 0 )
        {
          cachedData[0][ local_x + 1 ][ 0 ] = fabs( read_imagef( imgA , sampler , (int2)( pix_x , pix_y - 1 ) ) ) ;
          cachedData[1][ local_x + 1 ][ 0 ] = fabs( read_imagef( imgB , sampler , (int2)( pix_x , pix_y - 1 ) ) ) ;
          cachedData[2][ local_x + 1 ][ 0 ] = fabs( read_imagef( imgC , sampler , (int2)( pix_x , pix_y - 1 ) ) ) ;
        }
        if( local_y == (WORK_GROUP_SIZE-1) )
        {
          cachedData[0][ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( imgA , sampler , (int2)( pix_x , pix_y + 1 ) ) ) ; 
          cachedData[1][ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( imgB , sampler , (int2)( pix_x , pix_y + 1 ) ) ) ; 
          cachedData[2][ local_x + 1 ][ WORK_GROUP_SIZE + 1 ] = fabs( read_imagef( imgC , sampler , (int2)( pix_x , pix_y + 1 ) ) ) ; 
        }

        barrier( CLK_LOCAL_MEM_FENCE ) ; 

        const float4 pix = cachedData[1][ local_x + 1 ][ local_y + 1 ] ; 

        int4 local_min_max = 
          // Bellow 
          cachedData[0][ local_x ][ local_y ]         < pix && 
          cachedData[0][ local_x + 1 ][ local_y ]     < pix && 
          cachedData[0][ local_x + 2 ][ local_y ]     < pix &&

          cachedData[0][ local_x ][ local_y + 1 ]     < pix && 
          cachedData[0][ local_x + 1][ local_y + 1 ]  < pix && 
          cachedData[0][ local_x + 2 ][ local_y + 1 ] < pix && 

          cachedData[0][ local_x ][ local_y + 2 ]     < pix && 
          cachedData[0][ local_x + 1 ][ local_y + 2 ] < pix && 
          cachedData[0][ local_x + 2 ][ local_y + 2 ] < pix &&

          // Same slice 
          cachedData[1][ local_x ][ local_y ]         < pix && 
          cachedData[1][ local_x + 1 ][ local_y ]     < pix && 
          cachedData[1][ local_x + 2 ][ local_y ]     < pix &&

          cachedData[1][ local_x ][ local_y + 1 ]     < pix && 
          cachedData[1][ local_x + 2 ][ local_y + 1 ] < pix && 

          cachedData[1][ local_x ][ local_y + 2 ]     < pix && 
          cachedData[1][ local_x + 1 ][ local_y + 2 ] < pix && 
          cachedData[1][ local_x + 2 ][ local_y + 2 ] < pix &&

          // Above
          cachedData[2][ local_x ][ local_y ]         < pix && 
          cachedData[2][ local_x + 1 ][ local_y ]     < pix && 
          cachedData[2][ local_x + 2 ][ local_y ]     < pix &&

          cachedData[2][ local_x ][ local_y + 1 ]     < pix && 
          cachedData[2][ local_x + 1][ local_y + 1 ]  < pix && 
          cachedData[2][ local_x + 2 ][ local_y + 1 ] < pix && 

          cachedData[2][ local_x ][ local_y + 2 ]     < pix && 
          cachedData[2][ local_x + 1 ][ local_y + 2 ] < pix && 
          cachedData[2][ local_x + 2 ][ local_y + 2 ] < pix ;          


        if( pix_x < get_image_width( res ) && pix_y < get_image_height( res ) ) 
        {
          write_imagef( res , (int2)( pix_x , pix_y ) , convert_float4( abs( local_min_max ) ) ) ;
        }
      }




    )" ;

} // namespace kernels
} // namespace gpu
} // namespace image
} // namespace openMVG

#endif
