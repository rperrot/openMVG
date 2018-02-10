// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_SIFT_GPU_KEYPOINT_EXTRACTION_KERNELS_HPP
#define OPENMVG_FEATURES_SIFT_GPU_KEYPOINT_EXTRACTION_KERNELS_HPP

#include <string>

namespace openMVG
{
namespace features
{
namespace sift
{
namespace gpu
{

static const std::string krnsSiftExtractKeypoints =
  R"(

    float sift_edge_response( read_only image2d_t dog_cur , const int2 pos )
    {

    }
    
    // Return dx , dy , ds , dvalue
    float4 sift_refine_position( read_only image2d_t dog_prev , 
                                 read_only image2d_t dog_cur , 
                                 read_only image2d_t doc_next ,
                                 const int2 pos )
    {
      
    }

    // Return 
    __kernel void sift_extract_keypoints( global float * keypoints , // i, j , x, y , scale, value 
                                          read_only image2d_t min_max ,
                                          read_only image2d_t dog_prev ,
                                          read_only image2d_t dog_cur , 
                                          read_only image2d_t dog_next ,
                                          const int nb_refinement , 
                                          const float peak_threshold , 
                                          const float edge_threshold ,
                                          const int2 offset_region , 
                                          const int2 region_size )
    {
      const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST ; 

      const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ;
      
      const int min_x = offset_region.x ;
      const int min_y = offset_region.y ;
      const int max_x = offset_region.x + region_size.x ;
      const int max_y = offset_region.y + region_size.y ; 

      const float max_offset = 0.6f ; 
  
      if( pos.x >= min_x && pos.x < min_x && pos.y >= min_y && pos.y < max_y )
      {
        if( read_imagef( min_max , sampler , pos ).r > 0.f )
        {
          // we're on a max, let's refine it !

          int2 cur_pos = pos ; 
          bool converged = false ; 
          float4 refined_delta = (float4) 0 ;
          for( int id_refine = 0 ; id_refine < nb_refine && ! converged ; ++id_refine )
          {
            if( pos.x >= min_x && pos.x < min_x && pos.y >= min_y && pos.y < max_y )
            {
              refined_delta = sift_refine_position( dog_prev , dog_cur , dog_next , cur_pos ) ;

              if( fabs( refined_delta.x ) < max_offset && fabs( refined_delta.y ) < max_offset && fabs( refined_delta.z ) < max_offset )
              {
                converged = true ; 
              }
            }
            else 
            {
              if( refined_delta.x > max_offset && (cur_pos.x + 1) < max_x )
              {
                cur_pos.x += 1 ; 
              }
              if( refined_delta.x < -max_offset && (cur_pos.x - 1 ) >= min_x )
              {
                cur_pos.x -= 1 ; 
              }
              if( refined_delta.y > max_offset && (cur_pos.y + 1) < max_y )
              {
                cur_pos.y += 1 ;
              }
              if( refined_delta.y < -max_offset && (cur_pos.y -1) >= min_y )
              {
                cur_pos.y -= 1 ; 
              }
            }
          }

          int out_index = pos.y * max_x + pos.x ; 
          
          // i j x y sigma value 
          float i,j,x,y,sigma,value ;

          if( converged )
          {           
            // Try to add the point if it's a corner one 
            const float edge_resp = sift_edge_response( dog_cur , cur_pos ) ; 
            if( edge_resp >= 0 && edge_resp < edge_threshold )
            {
              i = (float)cur_pos.y ;
              j = (float)cur_pos.x ;
              x = (((float)cur_pos.x) + refined_delta.x) * delta ;  
              y = (((float)cur_pos.y) + refined_delta.y) * delta ; 
              sigma = slice_sigma * pow( sigma_ratio , refined_delta.z ) ;
              value = refined_delta.w ; 
            }
          }

          keypoints[ 6 * out_index ] = i ;
          keypoints[ 6 * out_index + 1 ] = j ; 
          keypoints[ 6 * out_index + 2 ] = x ;
          keypoints[ 6 * out_index + 3 ] = y ; 
          keypoints[ 6 * out_index + 4 ] = sigma ;
          keypoints[ 6 * out_index + 5 ] = value ;
        }
      }
    }
  )" ;

} // namespace gpu
} // namespace sift
} // namespace features
} // namespace openMVG

#endif