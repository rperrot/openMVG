#ifndef OPENMVG_IMAGE_GPU_KERNELS_IMAGE_GPU_RESAMPLING_KERNELS_HPP
#define OPENMVG_IMAGE_GPU_KERNELS_IMAGE_GPU_RESAMPLING_KERNELS_HPP

#include <string>

namespace openMVG
{
namespace image
{
namespace gpu
{
namespace kernels
{

const std::string krnsImageResamplingDecimate =
  R"(
    // Note index are given according to the result image 
    __kernel void image_decimate_f( __write_only image2d_t res , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST ;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
      
      if( pos.x < get_image_width( res ) && pos.y < get_image_height( res ) )
      {
        write_imagef( res , pos , read_imagef( img , sampler , 2 * pos ) ) ;
      }
    }

    __kernel void image_decimate_ui( __write_only image2d_t res , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST ;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
      
      if( pos.x < get_image_width( res ) && pos.y < get_image_height( res ) )
      {
        write_imageui( res , pos , read_imageui( img , sampler , 2 * pos ) ) ;
      }
    }
    
    __kernel void image_decimate_i( __write_only image2d_t res , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST ;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
      
      if( pos.x < get_image_width( res ) && pos.y < get_image_height( res ) )
      {
        write_imagei( res , pos , read_imagei( img , sampler , 2 * pos ) ) ;
      }
    }
    )" ;

const std::string krnsImageResamplingUpsample =
  R"(
    // Note index are given according to the result image 
    __kernel void image_upsample_f( __write_only image2d_t res , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR ;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
      
      if( pos.x < get_image_width( res ) && pos.y < get_image_height( res ) )
      {
        float2 rpos = (float2) ( ( pos.x + 1.0f) / ( 2.0f * get_image_width( img ) ) , (pos.y+1.0f) / (2.0f * get_image_height( img ) ) ) ;
        write_imagef( res , pos , read_imagef( img , sampler , rpos ) ) ;
      }
    }

    // Because CLK_FILTER_LINEAR is not compatible with read_imageui we need to make interpolation by hand
    __kernel void image_upsample_ui( __write_only image2d_t res , __read_only image2d_t img )
    {
      sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST ;

      int2 pos = { get_global_id( 0 ) , get_global_id( 1 ) } ; 
      
      if( pos.x < get_image_width( res ) && pos.y < get_image_height( res ) )
      {
        float2 mid = convert_float2( pos ) / 2.0f ; 
        float2 fx = floor( mid ) ;
        float2 delta = mid - fx ;

        float wx[2] = { 1.0f - delta.x , delta.x } ;
        float wy[2] = { 1.0f - delta.y , delta.y } ;  

        int2 base = convert_int2( fx ) ;

        float4 a = convert_float4( read_imageui( img , sampler , base ) ) ;
        float4 b = convert_float4( read_imageui( img , sampler , base + (int2)( 1 , 0 ) ) ) ;
        float4 c = convert_float4( read_imageui( img , sampler , base + (int2)( 0 , 1 ) ) ) ;
        float4 d = convert_float4( read_imageui( img , sampler , base + (int2)( 1 , 1 ) ) ) ;

        float4 value = wx[0] * wy[0] * a +
                       wx[1] * wy[0] * b +
                       wx[0] * wy[1] * c +
                       wx[1] * wy[1] * d ;
        
        write_imageui( res , pos , convert_uint4( value + (float4)0.5f ) ) ;
      }
    }
)" ;

} // namespace kernels
} // namespace gpu
} // namespace image
} // namespace openMVG

#endif