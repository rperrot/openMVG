#ifndef _OPENMVG_MVS_OPENCL_WRAPPER_HH_
#define _OPENMVG_MVS_OPENCL_WRAPPER_HH_

#include "openMVG/image/image_container.hpp"
#include "openMVG/numeric/numeric.h"

#ifdef __APPLE__
  #include <OpenCL/OpenCL.h>
#else
  #include <CL/cl.h>
#endif

#include <string>

namespace MVS
{
/**
* @brief Helper class for openCL computation
*/
class OpenCLWrapper
{
  public:

    // The devices to load
    static const int OPENCL_DEVICE_CPU ;
    static const int OPENCL_DEVICE_GPU ;

    // How images are created for opencl kernels
    static const int OPENCL_IMAGE_READ_ONLY ;
    static const int OPENCL_IMAGE_WRITE_ONLY ;

    // how buffer are managed
    static const int OPENCL_BUFFER_READ_ONLY ;
    static const int OPENCL_BUFFER_WRITE_ONLY ;
    static const int OPENCL_BUFFER_READ_WRITE ;

    /**
    * @brief Ctr
    * @param device_type Type of the device to use for computation
    */
    OpenCLWrapper( const int device_type = OPENCL_DEVICE_GPU ) ;

    /**
    * @brief Dtr
    */
    ~OpenCLWrapper( void ) ;

    /**
    * @brief Get platform value
    * @return current platform value
    */
    cl_platform_id getPlatform( void ) ;

    /**
    * @brief Get device
    * @return current device
    */
    cl_device_id getDevice( void ) ;

    /**
    * @brief Get Context
    * @return current context
    */
    cl_context getContext( void ) ;

    /**
    * @brief Get command queue
    * @return current execution queue
    */
    cl_command_queue getCommandQueue( void ) ;


    /**
    * @brief Compiles and link a program given a source code
    * @param src Source code
    * @return the program
    */
    cl_program createProgramFromSource( const std::string & src ) ;

    /**
    * @brief Retrieve a kernel from a program
    * @param pgm Program in which the kernel is located
    * @param kernel_name Name of the kernel to retrieve
    */
    cl_kernel getKernelFromName( cl_program pgm , const std::string & kernel_name ) ;


    /**
    * @brief Create an openCL image buffer
    * @param img Input image
    * @param access_type Kind of access (R,W,R/W) to this image
    * @return image buffer
    */
    cl_mem createImage( const openMVG::image::Image<unsigned char> & img , const int access_type ) ;

    /**
    * @brief Create an openCL image buffer
    * @param img Input image
    * @param access_type Kind of access (R,W,R/W) to this image
    * @return image buffer
    */
    cl_mem createImage( const openMVG::image::Image<double> & img , const int access_type ) ;

    /**
    * @brief Create an openCL image buffer
    * @param img Input image
    * @param access_type Kind of access (R,W,R/W) to this image
    * @return image buffer
    */
    cl_mem createImage( const openMVG::image::Image<openMVG::Vec4> & img , const int access_type ) ;

    /**
    * @brief Create a generic buffer
    * @param size Size of the buffer to create
    * @param access_type Kind of access (R,W,R/W) to this buffer
    * @param data Input data to pass to the buffer
    * @return buffer
    */
    cl_mem createBuffer( const size_t size , const int access_type = OPENCL_BUFFER_READ_ONLY , const cl_float * data = NULL ) ;

    /**
    * @brief Create a read only buffer from a 3x3 matrix
    * @param mat Input matrix
    * @return buffer
    */
    cl_mem createBuffer( const openMVG::Mat3 & mat ) ;

    /**
    * @brief Create a read only buffer from a 3d vector
    * @param mat Input vector
    * @return buffer
    */
    cl_mem createBuffer( const openMVG::Vec3 & vec ) ;

    /**
    * @brief Create a read only buffer from a 2d integer vector
    * @param mat Input vector
    * @return buffer
    */
    cl_mem createBuffer( const openMVG::Vec2i & vec ) ;

    /**
    * @brief Create a read only buffer from a 3x4 matrix
    * @param mat Input matrix
    * @return buffer
    */
    cl_mem createBuffer( const openMVG::Mat34 & mat ) ;

    /**
    * @brief Create a buffer object (not an image buffer object) from an image
    * @param img Input image
    * @return buffer
    */
    cl_mem createBuffer( const openMVG::image::Image<openMVG::Vec4> & img ) ;

    /**
    * @brief Create a buffer object (not an image buffer object) from an image
    */
    cl_mem createBuffer( const openMVG::image::Image<unsigned long long> & img ) ;

    /**
    * @brief Read an image from device to host
    * @param img_obj Device image object
    * @param[out] img Host image object
    */
    void readImage( cl_mem img_obj , openMVG::image::Image<float> & img ) ;



    /**
    * @brief Clear memory associated with openCL buffer
    */
    void clearMemory( cl_mem mem_obj ) ;

    /**
    * @brief Get image width
    * @param img_obj Device image object
    * @return width of this image
    */
    int imageWidth( cl_mem img_obj ) ;

    /**
    * @brief Get image height
    * @param img_obj Device image object
    * @return height of this image
    */
    int imageHeight( cl_mem img_obj ) ;

    /**
    * @brief Execute a 2d kernel
    * @param krn kernel to execute
    * @param working_size Size of the working area
    */
    void runKernel2d( cl_kernel krn , const int working_size[2] ) ;

    /**
    * @brief Copy Device data to host data
    * @param buffer Input device buffer
    * @param outData Output hosst buffer
    * @note outData must be allocated to the correct size
    */
    void copyDeviceToHost( cl_mem buffer , cl_float * outData ) ;

    /**
    * @brief Copy Host data to device data
    * @param outData Host data
    * @param buffer Device buffer
    * @note total transfered data is equal to the size of the device buffer
    */
    void copyHostToDevice( const cl_float * outData , cl_mem buffer ) ;

  private:

    int getDeviceIDWithMaxComputeUnit( const int device_type ) ;
    int getDeviceIDWithMaxGlobalMemory( const int device_type ) ;
    int getNbDevices( const int device_type ) ;

    // Prepare all components of the wrapper
    bool prepare( const int device_type ) ;

    // OpenCL variables
    cl_platform_id   m_platform ;
    cl_device_id     m_device ;
    cl_context       m_context ;
    cl_command_queue m_queue ;

} ;

} // namespace MVS

#endif