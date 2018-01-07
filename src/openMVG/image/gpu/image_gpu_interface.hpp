#ifndef OPENMVG_IMAGE_GPU_IMAGE_GPU_INTERFACE_HPP
#define OPENMVG_IMAGE_GPU_IMAGE_GPU_INTERFACE_HPP

#include "openMVG/image/image_container.hpp"
#include "openMVG/image/pixel_types.hpp"

#include "openMVG/system/gpu/OpenCLContext.hpp"

namespace openMVG
{
namespace image
{
namespace gpu
{

/************************************************
*                                               *
*             openMVG -> OpenCL                 *
*                                               *
*************************************************/

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<unsigned char> & img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<float> & img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<double> & img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<Rgb<unsigned char>> & img , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Create an openCL image object given a CPU image object
 * @param img Input CPU image object
 * @param ctx OpenCL context
 * @return Memory object corresponding to the OpenCL image object
 */
cl_mem ToOpenCLImage( const Image<Rgba<unsigned char>> & img , openMVG::system::gpu::OpenCLContext & ctx ) ;


/************************************************
*                                               *
*             OpenCL -> openMVG                 *
*                                               *
*************************************************/

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<unsigned char> & outImg , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<float> & outImg , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<double> & outImg , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<Rgb<unsigned char>> & outImg , openMVG::system::gpu::OpenCLContext & ctx ) ;

/**
 * @brief Convert an OpenCL image to a openMVG image
 * @param img Input Image to convert
 * @param[out] outImg Output image
 * @param ctx OpenCL context
 * @retval true if conversion is OK
 * @retval false if conversion fails
 */
bool FromOpenCLImage( cl_mem & img , Image<Rgba<unsigned char>> & outImg , openMVG::system::gpu::OpenCLContext & ctx ) ;


} // namespace gpu
} // namespace image
} // namespace openMVG

#endif