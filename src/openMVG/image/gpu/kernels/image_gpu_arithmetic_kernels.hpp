// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_GPU_KERNELS_IMAGE_GPU_ARITHMETIC_KERNELS_HPP
#define OPENMVG_IMAGE_GPU_KERNELS_IMAGE_GPU_ARITHMETIC_KERNELS_HPP

#include <string>

namespace openMVG
{
namespace image
{
namespace gpu
{
namespace kernels
{

/**
 * @brief Kernel that add two images
 * Assuming same image sizes
 */
static const std::string krnsImageAdd =
  R"(
/* Image Add for ui images */
__kernel void image_add_ui( __write_only image2d_t output , __read_only image2d_t imgA , __read_only image2d_t imgB ) 
{ 
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST ; 
  const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
  if( pos.x < get_image_width( output ) && pos.y < get_image_height( output ) ) 
  { 
    write_imageui( output , pos , read_imageui( imgA , sampler , pos ) + read_imageui( imgB , sampler , pos ) ) ; 
  } 
} 

/* Image Add for i images */
__kernel void image_add_i( __write_only image2d_t output , __read_only image2d_t imgA , __read_only image2d_t imgB ) 
{ 
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST ; 
  const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
  if( pos.x < get_image_width( output ) && pos.y < get_image_height( output ) ) 
  { 
    write_imagei( output , pos , read_imagei( imgA , sampler , pos ) + read_imagei( imgB , sampler , pos ) ) ; 
  } 
}

/* Image Add for f images */
__kernel void image_add_f( __write_only image2d_t output , __read_only image2d_t imgA , __read_only image2d_t imgB ) 
{ 
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST ; 
  const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
  if( pos.x < get_image_width( output ) && pos.y < get_image_height( output ) ) 
  { 
    write_imagef( output , pos , read_imagef( imgA , sampler , pos ) + read_imagef( imgB , sampler , pos ) ) ; 
  } 
}

  )" ;

/**
* @brief Kernel that subtract two images
* Assuming same image sizes
*/
static const std::string krnsImageSub =
  R"(
/* Image Sub for ui images */
__kernel void image_sub_ui( __write_only image2d_t output , __read_only image2d_t imgA , __read_only image2d_t imgB ) 
{ 
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST ; 
  const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
  if( pos.x < get_image_width( output ) && pos.y < get_image_height( output ) ) 
  { 
    write_imageui( output , pos , read_imageui( imgA , sampler , pos ) - read_imageui( imgB , sampler , pos ) ) ; 
  } 
} 

/* Image Sub for i images */
__kernel void image_sub_i( __write_only image2d_t output , __read_only image2d_t imgA , __read_only image2d_t imgB ) 
{ 
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST ; 
  const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
  if( pos.x < get_image_width( output ) && pos.y < get_image_height( output ) ) 
  { 
    write_imagei( output , pos , read_imagei( imgA , sampler , pos ) - read_imagei( imgB , sampler , pos ) ) ; 
  } 
}

/* Image Sub for f images */
__kernel void image_sub_f( __write_only image2d_t output , __read_only image2d_t imgA , __read_only image2d_t imgB ) 
{ 
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST ; 
  const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
  if( pos.x < get_image_width( output ) && pos.y < get_image_height( output ) ) 
  { 
    write_imagef( output , pos , read_imagef( imgA , sampler , pos ) - read_imagef( imgB , sampler , pos ) ) ; 
  } 
}

  )" ;

/**
* @brief Kernel that perform cwise product of two images
* Assuming same image sizes
*/
static const std::string krnsImageMul =
  R"(
/* Image CWise Product for ui images */
__kernel void image_mul_ui( __write_only image2d_t output , __read_only image2d_t imgA , __read_only image2d_t imgB ) 
{ 
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST ; 
  const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
  if( pos.x < get_image_width( output ) && pos.y < get_image_height( output ) ) 
  { 
    write_imageui( output , pos , read_imageui( imgA , sampler , pos ) * read_imageui( imgB , sampler , pos ) ) ; 
  } 
} 

/* Image CWise Product for i images */
__kernel void image_mul_i( __write_only image2d_t output , __read_only image2d_t imgA , __read_only image2d_t imgB ) 
{ 
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST ; 
  const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
  if( pos.x < get_image_width( output ) && pos.y < get_image_height( output ) ) 
  { 
    write_imagei( output , pos , read_imagei( imgA , sampler , pos ) * read_imagei( imgB , sampler , pos ) ) ; 
  } 
}

/* Image CWise Product for f images */
__kernel void image_mul_f( __write_only image2d_t output , __read_only image2d_t imgA , __read_only image2d_t imgB ) 
{ 
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST ; 
  const int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
  if( pos.x < get_image_width( output ) && pos.y < get_image_height( output ) ) 
  { 
    write_imagef( output , pos , read_imagef( imgA , sampler , pos ) * read_imagef( imgB , sampler , pos ) ) ; 
  } 
}

  )" ;

} // namespace kernels ;
} // namespace gpu
} // namespace image
} // namesace openMVG

#endif