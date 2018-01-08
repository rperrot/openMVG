#ifndef OPENMVG_IMAGE_GPU_IMAGE_GPU_ARITHMETICS_HPP
#define OPENMVG_IMAGE_GPU_IMAGE_GPU_ARITHMETICS_HPP

#include "openMVG/system/gpu/OpenCLContext.hpp"

namespace openMVG
{
namespace image
{
namespace gpu
{

/**
 * @brief Add two images
 * @param imgA first image
 * @param imgB second image
 * @param ctx Context
 * @return image object which is the addition of the two parameters
 * @note this function assume standard add is already loaded inside the context (which is the default behavior)
 */
cl_mem ImageAdd( cl_mem imgA , cl_mem imgB , openMVG::system::gpu::OpenCLContext & ctx ) ;

} // namespace gpu
} // namespace image
} // namespace openMVG

#endif