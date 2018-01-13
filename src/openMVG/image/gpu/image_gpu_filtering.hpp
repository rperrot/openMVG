#ifndef OPENMVG_IMAGE_GPU_IMAGE_GPU_FILTERING_HPP
#define OPENMVG_IMAGE_GPU_IMAGE_GPU_FILTERING_HPP

#include "openMVG/system/gpu/OpenCLContext.hpp"

namespace openMVG
{
namespace image
{
namespace gpu
{

// X derivative
/**
 * @brief Compute X derivative using central difference
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageXDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;

/**
 * @brief Compute X derivative using central difference
 * @param[out] res Derivative image (should be allocated before calling this function)
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @retval true if computation is of
 * @retval false if computation fails
 */
bool ImageXDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true );

// Y derivative
/**
 * @brief Compute Y derivative using central difference
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @return derivative image
 */
cl_mem ImageYDerivative( cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true ) ;

/**
 * @brief Compute Y derivative using central difference
 * @param[out] res Derivative image (should be allocated before calling this function)
 * @param img Image
 * @param ctx OpenCL context
 * @param normalize Indicate if derivative is normalized
 * @retval true if computation is of
 * @retval false if computation fails
 */
bool ImageYDerivative( cl_mem res , cl_mem img , openMVG::system::gpu::OpenCLContext & ctx , const bool normalize = true );


} // namespace gpu
} // namespace image
} // namespace openMVG

#endif