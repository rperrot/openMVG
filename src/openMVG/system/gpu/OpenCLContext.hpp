// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_SYSTEM_GPU_OPENCL_CONTEXT_HPP
#define OPENMVG_SYSTEM_GPU_OPENCL_CONTEXT_HPP

#include "openMVG/system/gpu/OpenCLDeviceInfos.hpp"
#include "openMVG/system/gpu/OpenCLStandardKernels.hpp"

#ifdef APPLE
  #include <OpenCL/cl.h>
#else
  #include <CL/cl.h>
#endif

#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace openMVG
{
namespace system
{

/**
 * @brief namespace handling various openCL context primitives
 */
namespace gpu
{


/**
 * @brief OpenCL Platform profiles values
 */
enum OpenCLPlaformProfile
{
  // Stanard values
  OPENCL_PLATFORM_PROFILE_FULL_PROFILE ,
  OPENCL_PLATFORM_PROFILE_EMBEDDED_PROFILE ,
  // Error value
  OPENCL_PLATFORM_PROFILE_INVALID ,
} ;

/**
 * @brief OpenCL device type values
 */
enum OpenCLDeviceType
{
  OPENCL_DEVICE_TYPE_CPU ,
  OPENCL_DEVICE_TYPE_GPU ,

  OPENCL_DEVICE_TYPE_OTHER ,
  // error value
  OPENCL_DEVICE_TYPE_INVALID
} ;

/**
 * @brief Prefered setting if multiple device are present
 */
enum OpenCLDevicePreference
{
  OPENCL_DEVICE_PREFER_MAX_GLOBAL_MEMORY ,
  OPENCL_DEVICE_PREFER_MAX_COMPUTE_UNIT ,
  OPENCL_DEVICE_PREFER_MAX_FREQUENCY
} ;


/**
 * @brief Type for Image creation
 */
enum OpenCLImageDataType
{
  OPENCL_IMAGE_DATA_TYPE_U_INT_8   ,   // Unsigned int 8
  OPENCL_IMAGE_DATA_TYPE_U_INT_32  ,   // Unsigned int 32

  OPENCL_IMAGE_DATA_TYPE_SU_INT_8  ,   // Signed int 8
  OPENCL_IMAGE_DATA_TYPE_SU_INT_32 ,   // Signed int 32

  OPENCL_IMAGE_DATA_TYPE_UN_INT_8  ,   // Unsigned int 8 - Normalized (ie: 0-1 and read_imagef/write_imagef )

  OPENCL_IMAGE_DATA_TYPE_FLOAT     ,   // Float
} ;

/**
 * @brief Channel order for image creation
 */
enum OpenCLImageChannelOrder
{
  OPENCL_IMAGE_CHANNEL_ORDER_R ,
  OPENCL_IMAGE_CHANNEL_ORDER_RGBA ,
  OPENCL_IMAGE_CHANNEL_ORDER_BGRA
} ;

/**
 * @brief Access type for image creation
 */
enum OpenCLImageAccessType
{
  OPENCL_IMAGE_ACCESS_READ_ONLY ,
  OPENCL_IMAGE_ACCESS_WRITE_ONLY ,
  OPENCL_IMAGE_ACCESS_READ_WRITE ,
} ;

/**
 * @brief Access type for buffer creation
 */
enum OpenCLBufferAccessType
{
  OPENCL_BUFFER_ACCESS_READ_ONLY ,
  OPENCL_BUFFER_ACCESS_WRITE_ONLY ,
  OPENCL_BUFFER_ACCESS_READ_WRITE
} ;

/**
 * @brief Main class holding an OpenCL context
 */
class OpenCLContext
{
  public:

    /**
     * @brief Ctr
     * @param prefered_device_type Type of the prefered_device to use as default
     * @param device_preference If multiple device are available with the prefered device type, select the one with the prefered setting
     * @param kernels_to_load Indicate if openMVG kernels are loaded
     */
    OpenCLContext( const OpenCLDeviceType prefered_device_type = OPENCL_DEVICE_TYPE_GPU ,
                   const OpenCLDevicePreference device_preference = OPENCL_DEVICE_PREFER_MAX_GLOBAL_MEMORY ,
                   const std::vector< OpenCLStandardKernels > & kernels_to_load = { OPENCL_STANDARD_KERNELS_ALL } ) ;

    OpenCLContext( const OpenCLContext & ) ;
    OpenCLContext( OpenCLContext && ) = default ;
    OpenCLContext & operator=( const OpenCLContext & ) ;
    OpenCLContext & operator=( OpenCLContext && ) = default ;

    /**
     * @brief Dtr
     */
    ~OpenCLContext( void ) ;

    /**
     * @brief Indicate if full context is valid (current platform, current device, current context)
     * @retval true if Valid
     * @retval false if Invalid
     */
    bool valid( void ) const ;


    /// ---------------------------- PLATFORM ---------------------------------------
    /**
     * @brief Get Number of platform
     * @return Number of platform
     */
    uint32_t nbPlatform( void ) const ;

    /**
     * @brief Get handle to a plaform
     * @param id Id of the platform to get
     * @return reference to the queried platform
     */
    cl_platform_id platform( const uint32_t id ) const ;

    /**
     * @brief Get handle to the current platform
     * @return handle of the current platform
     * @note return nullptr if current platform is invalid
     */
    cl_platform_id currentPlatform( void ) const ;

    /**
     * @brief Set current platform to use
     * @param id Id of the platform to use
     * @retval true if setting the new platform is ok
     * @retval false if setting the new platform fails
     * @note If it fails, no update is performed and the current platform stays as it was before calling this function
     */
    bool setCurrentPlatform( const uint32_t id ) ;

    /**
     * @brief Get profile a specific platform
     * @param id Id of the platform to query
     * @return Profile of the queried platform
     * @retval OPENCL_PLATFORM_PROFILE_INVALID if id is invalid or out of range
     */
    OpenCLPlaformProfile platformProfile( const uint32_t id ) const ;

    /**
     * @brief Get profile of the current platform
     * @return Profile of the current platform
     * @retval OPENCL_PLATFORM_PROFILE_INVALID if current platform is invalid
     */
    OpenCLPlaformProfile currentPlatformProfile( void ) const ;

    /**
     * @brief Get platform name
     * @param id Id of the platform to query
     * @return Name of the queried platform
     * @retval empty string if id is invalid or out of range
     */
    std::string platformName( const uint32_t id ) const ;

    /**
     * @brief Get current platform name
     * @return Name of the queried platform
     * @retval empty string if the current platform is invalid
     */
    std::string currentPlatformName( void ) const;

    /**
     * @brief Get Vendor name of a specific platform
     * @param id Id of the platform to query
     * @return Name of the vendor of the queried platform
     * @retval Empty string if id is invalid or out of range
     */
    std::string platformVendor( const uint32_t id ) const ;

    /**
     * @brief Get Vendor name of the current platform
     * @return Name of the vendor of the queried platform
     * @retval Empty string if the current platform is invalid
     */
    std::string currentPlatformVendor( void ) const ;

    /**
     * @brief Get list of extensions supported by a specific platform
     * @param id Id of the platform to query
     * @return list of extensions supported by the queried platform
     * @retval Empty list if id is invalid or out of range
     */
    std::vector<std::string> platformExtensions( const uint32_t id ) const ;

    /**
     * @brief Get list of extensions supported by the current platform
     * @return list of extensions supported by the current platform
     * @retval Empty list if the current platform is invalid
     */
    std::vector<std::string> currentPlatformExtensions( void ) const ;

    /// ---------------------------- END OF PLATFORM ---------------------------------------

    /// ---------------------------- DEVICES ---------------------------------------
    /**
      * @brief Get number of device of the specific platform
      * @param plat_id Id of the platform to query
      * @return number of device on the specific platform
      * @retval std::numeric_limits<uint32_t>::max() if plat_id is invalid
      */
    uint32_t nbDeviceForPlatform( const uint32_t plat_id ) const ;

    /**
     * @brief Get number of device on the current platform
     * @return number of device on the current platform
     * @retval std::numeric_limits<uint32_t>::max() if current platform is invalid
     */
    uint32_t currentPlatformNbDevice( void ) const ;

    /**
     * @brief Get the current device on the current platform
     * @return current device
     */
    cl_device_id currentDevice( void ) const ;

    /**
     * @brief Set the current device on the current platform
     * @param device_id New id of the current device
     * @retval true if set is OK
     * @retval false is setting the device id fails
     */
    bool setCurrentDevice( const uint32_t device_id ) ;

    /**
     * @brief Get type of a specific device on a specific platform
     * @param plat_id Id of the platform to query
     * @param device_id Id of the device on the specific platform
     * @return type of the queried device
     * @retval OPENCL_DEVICE_TYPE_INVALID is device and/or platform is/are invalid
     */
    OpenCLDeviceType deviceType( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get current device type (on the current platform)
     * @return type of the current device
     * @retval OPENCL_DEVICE_TYPE_INVALID is device and/or platform is/are invalid
     */
    OpenCLDeviceType currentDeviceType( void ) const ;

    /**
     * @brief Get global memory size of a specific device on a specific platform
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return Global Memory size of the given device
     * @retval 0 if platform or device are invalid
     */
    cl_ulong deviceGlobalMemorySize( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get global memory size of the current device (on the current platform)
     * @return Global Memory size of current device
     * @retval 0 if current platform or current device are invalid
     */
    cl_ulong currentDeviceGlobalMemorySize( void ) const ;

    /**
     * @brief Get maximum width of a 2D image on the given device
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return maximum width of a 2D image on the given device
     * @retval 0 if image 2D is not supported on the given device or if platform/device is invalid
     */
    size_t deviceMaxImage2DWidth( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get maximum width of a 2D image on the current device
     * @return maximum width of a 2D image on the current device
     * @retval 0 if image 2D is not supported on the current device or if current platform/device is invalid
     */
    size_t currentDeviceMaxImage2DWidth( void ) const ;

    /**
     * @brief Get maximum height of a 2D image on the given device
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return maximum height of a 2D image on the given device
     * @retval 0 if image 2D is not supported on the given device or if platform/device is invalid
     */
    size_t deviceMaxImage2DHeight( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get maximum height of a 2D image on the current device
     * @return maximum height of a 2D image on the current device
     * @retval 0 if image 2D is not supported on the current device or if current platform/device is invalid
     */
    size_t currentDeviceMaxImage2DHeight( void ) const ;

    /**
     * @brief Indicate if a given device supports 2D images
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @retval true if image 2D is supported
     * @retval false if image 2D is unsupported
     */
    bool deviceSupportImage2D( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Indicate if a current supports 2D images
     * @retval true if image 2D is supported
     * @retval false if image 2D is unsupported
     */
    bool currentDeviceSupportImage2D( void ) const ;

    /**
     * @brief Get maximum width of a 3D image on the given device
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return maximum width of a 3D image on the given device
     * @retval 0 if 3D image is not supported on the given device or if platform/device is invalid
     */
    size_t deviceMaxImage3DWidth( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get maximum width of a 3D image on the current device
     * @return maximum width of a 3D image on the current device
     * @retval 0 if image 3D is not supported on the current device or if current platform/device is invalid
     */
    size_t currentDeviceMaxImage3DWidth( void ) const ;

    /**
     * @brief Get maximum height of a 3D image on the given device
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return maximum height of a 3D image on the given device
     * @retval 0 if image 3D is not supported on the given device or if platform/device is invalid
     */
    size_t deviceMaxImage3DHeight( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get maximum height of a 3D image on the current device
     * @return maximum height of a 3D image on the current device
     * @retval 0 if image 3D is not supported on the current device or if current platform/device is invalid
     */
    size_t currentDeviceMaxImage3DHeight( void ) const ;

    /**
     * @brief Get maximum depth of a 3D image on the given device
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return maximum depth of a 3D image on the given device
     * @retval 0 if image 3D is not supported on the given device or if platform/device is invalid
     */
    size_t deviceMaxImage3DDepth( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get maximum depth of a 3D image on the current device
     * @return Maximum depth of a 3D image on the current device
     * @retval 0 if image 3D is not supported on the current device or if current platform/device is invalid
     */
    size_t currentDeviceMaxImage3DDepth( void ) const ;

    /**
     * @brief Indicate if a given device supports 3D images
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @retval true if image 3D is supported
     * @retval false if image 3D is unsupported
     */
    bool deviceSupportImage3D( const uint32_t plat_it , const uint32_t device_id ) const ;

    /**
     * @brief Indicate if a current supports 3D images
     * @retval true if image 3D is supported
     * @retval false if image 3D is unsupported
     */
    bool currentDeviceSupportImage3D( void ) const ;

    /**
     * @brief Get maximum clock frequency of a given device
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return Maximum clock frequency of the given device
     * @retval 0 if plat_id/device_id is invalid
     */
    cl_uint deviceMaxClockFrequency( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get maximum clock frequency of a the current device
     * @return Maximum clock frequency of the current device
     * @retval 0 if current platform/device is invalid
     */
    cl_uint currentDeviceMaxClockFrequency( void ) const ;

    /**
     * @brief Get maximum compute units of the given device
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return Maximum compute units for the given device
     * @retval 0 if plat_id/device_id is invalid
     */
    cl_uint deviceMaxComputeUnits( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get maximum compute units of the current device
     * @return Maximum compute units for the current device
     * @retval 0 if current platform/device is invalid
     */
    cl_uint currentDeviceMaxComputeUnits( void ) const ;

    /**
     * @brief Get name of the given device
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return Name of the queried device
     * @retval Empty string if plat_id/device_id is invalid
     */
    std::string deviceName( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get name of the current device
     * @return Name of the current device
     * @retval Empty string if current platform/device is invalid
     */
    std::string currentDeviceName( void ) const ;

    /**
     * @brief Get vendor name of the given device
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return Vendor name of the queried device
     * @retval Empty string if plat_id/device_id is invalid
     */
    std::string deviceVendor( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get vendor name of the current device
     * @return Vendor name of the current device
     * @retval Empty string if current plaform/device is invalid
     */
    std::string currentDeviceVendor( void ) const ;
    /// ---------------------------- END OF DEVICES ---------------------------------------

    /// ----------------------------- CONTEXTS --------------------------------------------

    /**
     * @brief get context for a specific pair platform/device
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return context with the specified pair platform/device
     * @retval nullptr if pair plat_id/device_id is invalid
     */
    cl_context context( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief get context for current platform/device
     * @return context of the current platform/device
     * @retval nullptr if pair plat_id/device_id is invalid
     */
    cl_context currentContext( void ) const ;

    /// ---------------------------- END OF CONTEXTS --------------------------------------

    /// ------------------------------- PROGRAMS ------------------------------------------

    /**
     * @brief Create a program and build it (ie: compile and link) then return the given program
     * @param program_source Source code of the program
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return The compiled program
     * @retval nullptr If plat_id/device_id is invalid
     * @retval nullptr If creation fails
     * @retval nullptr If compilation/link fails
     * @note Compile on the context associated with the pair plat_id/device_id
     */
    cl_program createAndBuildProgram( const std::string & program_source , const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Create a program and build it (ie: compile and link) then return the given program
     * @param program_source Source code of the program
     * @return The compiled program
     * @retval nullptr If something fails
     * @retval nullptr If current context is invalid
     * @retval nullptr If creation fails
     * @note Compile on the current context
     * @note if compilation fails the function returns the created program (so the user must use valid(pgm) to check if program build is ok)
     */
    cl_program createAndBuildProgram( const std::string & program_source ) const ;

    /**
     * @brief Check if the program build is ok
     * @param pgm The program to check
     * @retval true if program build is ok
     * @retval false if program build fails
     */
    bool valid( cl_program pgm ) const ;

    /**
     * @brief Get build log for a specific program
     * @param pgm Program
     * @return build log
     * @note The function looks for the device it was previously compiled on so no pair plat/device is needed
     */
    std::string programBuildLog( cl_program pgm ) const ;

    /// ------------------------------- KERNELS ------------------------------------------

    /**
     * @brief Create a kernel given it's program and it's name
     * @param pgm The program in which the kernel is looked
     * @param kernel_name Name of the kernel to create
     * @return the created kernel
     * @retval nullptr if creation fails
     */
    cl_kernel createKernel( cl_program pgm , const std::string & kernel_name ) const ;

    /**
     * @brief Create all kernels situated inside the program
     * @param pgm The program in which the kernels are looked
     * @return List of kernels (and their associated function names) inside the program
     */
    std::map< std::string , cl_kernel > createKernels( cl_program pgm ) const ;

    /**
     * @brief Get function name of the given kernel
     * @param krn The kernel to query
     * @return Function name associated with the given kernel
     */
    std::string kernelName( cl_kernel krn ) const ;

    /**
     * @brief Get number of argument of the given kernel
     * @param krn The kernel to query
     * @return Number of arguments of the kernel
     */
    cl_uint kernelNumberOfArgument( cl_kernel krn ) const ;

    /**
     * @brief Get maximum work-group size of the given kernel
     * @param krn The kernel to query
     * @return Maximum workgroup size
     */
    size_t kernelMaxWorkgroupSize( cl_kernel krn ) const ;

    /**
     * @brief Get maximum global size used to execute the given kernel
     * @param krn The kernel to query
     * @return Maximum global size (in each dimension)
     */
    std::tuple<size_t, size_t, size_t> kernelGlobalWorkSize( cl_kernel krn ) const;

    /**
     * @brief Prefered work-group size multiple used to execute the kernel
     * @param krn The kernel to query
     * @return prefered workgroup size multiple
     */
    size_t kernelPreferedWorkGroupSizeMultiple( cl_kernel krn ) const ;

    /**
     * @brief Run 2d kernel on the current platform/device
     * @param krn Kernel
     * @param work_dim Work dimension (width,height)
     * @param group_size Work group size (if nullptr -> Compute best value)
     * @retval true If run is ok
     * @retval false If run fails
     */
    bool runKernel2d( cl_kernel krn , const size_t * work_dim , const size_t * group_size = nullptr ) const ;

    /// ------------------------------- END OF KERNELS ------------------------------------------

    /// ------------------------------- COMMAND QUEUES ------------------------------------------
    /**
     * @brief Get default command queue associated with the specified pair platform/device
     * @param plat_id Id of the platform
     * @param device_id Id of the device
     * @return the command queue associated with the specified pair
     * @retval nullptr if the specified pair (platform/device) is invalid
     */
    cl_command_queue commandQueue( const uint32_t plat_id , const uint32_t device_id ) const ;

    /**
     * @brief Get command queue associated with the current pair platform/device
     * @return the command queue associated with the current pair platform/device
     */
    cl_command_queue currentCommandQueue( void ) const ;

    /// ---------------------------- END OF COMMAND QUEUES --------------------------------------

    /// Standard KERNELS
    /**
     * @brief Get standard kernels (ie: defined by openMVG)
     * @param kernel_name Name of the kernel to get
     * @return Corresponding kernel
     * @retval nullptr if kernel_name is not a valid openMVG kernel name
     */
    cl_kernel standardKernel( const std::string & kernel_name ) const ;

    /**
     * @brief Create standard kernels
     * @param kernels Kernels to load
     */
    void loadStandardKernels( const std::vector< OpenCLStandardKernels > & kernels ) ;


    //// Image and buffer creations

    /**
     * @brief Create an image
     * @param width Width
     * @param height Height
     * @param order Order of the channels
     * @param dtype Data type of the channels
     * @param access Access type of the image
     * @return image created
     * @retval nullptr if there's an error during creation
     */
    cl_mem createImage( const size_t width , const size_t height ,
                        const OpenCLImageChannelOrder order ,
                        const OpenCLImageDataType dtype ,
                        const OpenCLImageAccessType access = OPENCL_IMAGE_ACCESS_READ_WRITE ,
                        void * data = nullptr ) const ;


    /**
     * @brief fill with black values inside the specified region
     */
    void fillBlackImage( cl_mem img , const size_t offset_region[2] , const size_t region_size[2] ) ;

    /**
     * @brief Create a buffer
     * @param size Size (in byte) of the buffer to create
     * @param access Access type for the newly created buffer
     * @param data Data to provide to the buffer
     * @return Buffer created
     * @retval nullptr if there is an error during creation
     */
    cl_mem createBuffer( const size_t size , const OpenCLBufferAccessType access = OPENCL_BUFFER_ACCESS_READ_WRITE , void * data = nullptr ) const ;

  private:

    /**
     * @brief Indicate if current platform is valid
     * @retval true if valid
     * @retval false if invalid
     */
    bool currentPlatformValid( void ) const ;

    /**
     * @brief Indicate if current device on current platform is valid
     * @retval true if valid
     * @retval false if invalid
     * @note if current platform is invalid, returns false
     */
    bool currentDeviceValid( void ) const ;

    /**
     * @brief fill platform informations
     */
    void fillPlatformsInfos( void ) ;

    /**
     * @brief Fill devices information (for all platforms)
     */
    void fillDevicesInfos( void ) ;

    /**
     * @brief Create contexts for all pairs platform/devices
     */
    void createContexts( void ) ;

    /**
     * @brief Releases contexts and destroy them if their reference count is 0
     */
    void releaseContexts( void ) ;

    /**
     * @brief Create command queues for all pairs platform/devices
     */
    void createCommandQueues( void ) ;

    /**
     * @brief Release command queues
     */
    void releaseCommandQueues( void ) ;


    /**
     * @brief Release stdandard kernels
     */
    void releaseStandardKernels( void ) ;

    // Platforms values
    /// Number of platforms
    uint32_t m_nb_platform ;
    /// The list of the platforms
    std::vector<cl_platform_id> m_platforms ;
    /// The ID of the current platform
    uint32_t m_current_platform_id ;

    // Devices values
    /// For all platforms get the devices
    std::map< cl_platform_id , std::vector< cl_device_id > > m_devices_for_platform ;
    /// Informations of all platform/device pairs
    std::map< std::pair<cl_platform_id, cl_device_id> , OpenCLDeviceInfos > m_devices_informations ;
    /// Id of the current device (on the current platform)
    uint32_t m_current_device_id ;
    /// Type of the prefered device
    OpenCLDeviceType m_prefered_device_type ;
    OpenCLDevicePreference m_device_preference ;

    // Context values
    /// A context per plaform/device pair
    std::map< std::pair<cl_platform_id, cl_device_id> , cl_context > m_contexts ;

    // Command queues
    /// A default command queue per platform/device pair
    std::map< std::pair<cl_platform_id, cl_device_id> , cl_command_queue > m_command_queues ;


    // Standard kernels
    /// List of programs defined by openMVG
    std::vector< cl_program > m_standard_programs ;
    /// List of kernels defined by openMVG
    std::map< std::string , cl_kernel > m_standard_kernels ;
} ;

} // namespace gpu
} // namespace system
} // namespace openMVG

#endif