// TODO : https://github.com/isazi/Convolution/blob/master/include/Convolution.hpp
// http://www.cmsoft.com.br/opencl-tutorial/case-study-high-performance-convolution-using-opencl-__local-memory/
// https://www.milania.de/blog/Performance_evaluation_of_image_convolution_with_gradient_filters_in_OpenCL
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

} // namespace kernels
} // namespace gpu
} // namespace image
} // namespace openMVG

#endif 