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

const std::string krnsImageFilteringDerivative =
  R"(
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

} // namespace kernels
} // namespace gpu
} // namespace image
} // namespace openMVG

#endif