// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_SYSTEM_GPU_OPENCL_CONTEXT_HPP
#define OPENMVG_SYSTEM_GPU_OPENCL_CONTEXT_HPP

#include "openMVG/system/gpu/OpenCLDeviceInfos.hpp"

#ifdef APPLE
  #include <OpenCL/cl.h>
#else
  #include <CL/cl.h>
#endif

#include <map>
#include <string>
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
 * @brief Main class holding an OpenCL context
 */
class OpenCLContext
{
  public:

    /**
     * @brief Ctr
     * @param prefered_device_type Type of the prefered_device to use as default
     * @param device_preference If multiple device are available with the prefered device type, select the one with the prefered setting
     */
    OpenCLContext( const OpenCLDeviceType prefered_device_type = OPENCL_DEVICE_TYPE_GPU ,
                   const OpenCLDevicePreference device_preference = OPENCL_DEVICE_PREFER_MAX_GLOBAL_MEMORY ) ;

    /// @note @rperrot : since it may be delicate, we delete the assignement ctr functions for now
    /// @todo (@rperrot) : provide a safe implementation of these funtions
    OpenCLContext( const OpenCLContext & ) = delete ;
    OpenCLContext( OpenCLContext && ) = delete ;
    OpenCLContext & operator=( const OpenCLContext & ) = delete ;
    OpenCLContext & operator=( const OpenCLContext && ) = delete ;

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
} ;

} // namespace gpu
} // namespace system
} // namespace openMVG

#endif