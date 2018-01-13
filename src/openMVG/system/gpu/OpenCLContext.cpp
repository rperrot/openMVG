// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/system/gpu/OpenCLContext.hpp"

#include "openMVG/stl/split.hpp"

// The kernels
#include "openMVG/image/gpu/kernels/image_gpu_arithmetic_ope_kernels.hpp"
#include "openMVG/image/gpu/kernels/image_gpu_convolution_ope_kernels.hpp"

#include <iostream>

#include <limits>

namespace openMVG
{
namespace system
{
namespace gpu
{

/**
 * @brief Ctr
 * @param prefered_device_type Type of the prefered_device to use as default
 * @param device_preference If multiple device are available with the prefered device type, select the one with the prefered setting
 * @param load_standard_kernels Indicate if openMVG kernels are loaded
 */
OpenCLContext::OpenCLContext( const OpenCLDeviceType prefered_device_type ,
                              const OpenCLDevicePreference device_preference ,
                              const bool load_standard_kernels )
  : m_nb_platform( 0 ) ,
    m_current_platform_id( std::numeric_limits<uint32_t>::max() ) ,
    m_current_device_id( std::numeric_limits<uint32_t>::max() ) ,
    m_prefered_device_type( prefered_device_type ) ,
    m_device_preference( device_preference )
{
  fillPlatformsInfos() ;
  fillDevicesInfos() ;

  // Set device
  if( currentPlatformValid() )
  {
    setCurrentPlatform( m_current_platform_id ) ;
  }

  createContexts() ;
  createCommandQueues() ;

  if( load_standard_kernels )
  {
    loadStandardKernels() ;
  }
}

OpenCLContext::OpenCLContext( const OpenCLContext & src )
  : m_nb_platform( src.m_nb_platform ) ,
    m_platforms( src.m_platforms ) ,
    m_current_platform_id( src.m_current_platform_id ) ,
    m_devices_for_platform( src.m_devices_for_platform ) ,
    m_devices_informations( src.m_devices_informations ) ,
    m_current_device_id( src.m_current_device_id ) ,
    m_prefered_device_type( src.m_prefered_device_type ) ,
    m_device_preference( src.m_device_preference ) ,
    m_contexts( src.m_contexts ) ,
    m_command_queues( src.m_command_queues ) ,
    m_standard_programs( src.m_standard_programs ) ,
    m_standard_kernels( src.m_standard_kernels )
{
  // Add 1 to the Ref Count of the contexts
  for( auto ctx : m_contexts )
  {
    if( ctx.second )
    {
      clRetainContext( ctx.second ) ;
    }
  }
  // Add 1 to the Ref count of the command queues
  for( auto cq : m_command_queues )
  {
    if( cq.second )
    {
      clRetainCommandQueue( cq.second ) ;
    }
  }
  // Add 1 to the ref count of the standard programs
  for( auto std_pgm : m_standard_programs )
  {
    if( std_pgm )
    {
      clRetainProgram( std_pgm ) ;
    }
  }
  // Add 1 to the ref count of the standard kernels
  for( auto std_krn : m_standard_kernels )
  {
    if( std_krn.second )
    {
      clRetainKernel( std_krn.second ) ;
    }
  }
}


OpenCLContext & OpenCLContext::operator=( const OpenCLContext & src )
{
  if( this != &src )
  {
    m_nb_platform = src.m_nb_platform ;
    m_platforms = src.m_platforms ;
    m_current_platform_id = src.m_current_platform_id ;
    m_devices_for_platform = src.m_devices_for_platform ;
    m_devices_informations = src.m_devices_informations ;
    m_current_device_id = src.m_current_device_id ;
    m_prefered_device_type = src.m_prefered_device_type ;
    m_device_preference = src.m_device_preference ;
    m_contexts = src.m_contexts ;
    m_command_queues = src.m_command_queues ;
    m_standard_programs = src.m_standard_programs ;
    m_standard_kernels = src.m_standard_kernels ;

    // Add 1 to the Ref Count of the contexts
    for( auto ctx : m_contexts )
    {
      if( ctx.second )
      {
        clRetainContext( ctx.second ) ;
      }
    }

    // Add 1 to the Ref count of the command queues
    for( auto cq : m_command_queues )
    {
      if( cq.second )
      {
        clRetainCommandQueue( cq.second ) ;
      }
    }

    // Add 1 to the ref count of the standard programs
    for( auto std_pgm : m_standard_programs )
    {
      if( std_pgm )
      {
        clRetainProgram( std_pgm ) ;
      }
    }
    // Add 1 to the ref count of the standard kernels
    for( auto std_krn : m_standard_kernels )
    {
      if( std_krn.second )
      {
        clRetainKernel( std_krn.second ) ;
      }
    }

  }
  return ( *this ) ;
}


/**
 * @brief Dtr
 */
OpenCLContext::~OpenCLContext( void )
{
  releaseCommandQueues() ;
  releaseContexts() ;
  releaseStandardKernels() ;
}

/**
 * @brief Indicate if full context is valid (current platform, current device, current context)
 * @retval true if Valid
 * @retval false if Invalid
 */
bool OpenCLContext::valid( void ) const
{
  return currentPlatformValid() &&
         currentDeviceValid() ;
}


/// ---------------------------- PLATFORM ---------------------------------------
/**
 * @brief Get Number of platform
 * @return Number of platform
 */
uint32_t OpenCLContext::nbPlatform( void ) const
{
  return m_nb_platform ;
}

/**
 * @brief Get handle to a plaform
 * @param id Id of the platform to get
 * @return reference to the queried platform
 */
cl_platform_id OpenCLContext::platform( const uint32_t id ) const
{
  if( id < m_nb_platform )
  {
    return m_platforms[ id ] ;
  }
  else
  {
    cl_platform_id res = nullptr ;
    return res ;
  }
}

/**
 * @brief Get handle to the current platform
 * @return handle of the current platform
 */
cl_platform_id OpenCLContext::currentPlatform( void ) const
{
  return platform( m_current_platform_id ) ;
}

/**
 * @brief Set current platform to use
 * @param id Id of the platform to use
 * @retval true if setting the new platform is ok
 * @retval false if setting the new platform fails
 * @note If it fails, no update is performed and the current platform stays as it was before calling this function
 */
bool OpenCLContext::setCurrentPlatform( const uint32_t id )
{
  if( id < m_nb_platform )
  {
    m_current_platform_id = id ;
    if( m_devices_for_platform.at( m_platforms[ m_current_platform_id ] ).size() > 0 )
    {
      m_current_device_id = std::numeric_limits<uint32_t>::max() ;

      const uint32_t nb_dev = m_devices_for_platform.at( m_platforms[ m_current_platform_id ] ).size() ;
      std::vector< uint32_t > availablePreferedDevice;
      for( uint32_t id_dev = 0 ; id_dev < nb_dev ; ++id_dev )
      {
        if( deviceType( m_current_platform_id , id_dev ) == m_prefered_device_type )
        {
          availablePreferedDevice.emplace_back( id_dev ) ;
        }
      }
      // Select the device according to the prefered settings
      if( availablePreferedDevice.size() == 1 )
      {
        m_current_device_id = availablePreferedDevice[0] ;
      }
      else if( availablePreferedDevice.size() > 1 )
      {
        uint32_t best = 0 ;
        const auto plat = m_platforms[ m_current_platform_id ] ;
        // More than one device with the prefered type available
        // Make a selection based on device preference
        if( m_device_preference == OPENCL_DEVICE_PREFER_MAX_COMPUTE_UNIT )
        {
          cl_uint max_cu = 0 ;
          for( const auto & cur_dev_id : availablePreferedDevice )
          {
            const auto dev = m_devices_for_platform.at( plat )[ cur_dev_id ] ;
            const cl_uint cur_max_cu = m_devices_informations.at( { plat , dev } ).m_max_compute_units ;
            if( cur_max_cu > max_cu )
            {
              max_cu = cur_max_cu ;
              best = cur_dev_id ;
            }
          }
        }
        else if( m_device_preference == OPENCL_DEVICE_PREFER_MAX_FREQUENCY )
        {
          cl_uint max_freq = 0 ;
          for( const auto & cur_dev_id : availablePreferedDevice )
          {
            const auto dev = m_devices_for_platform.at( plat )[ cur_dev_id ] ;
            const cl_uint cur_max_freq = m_devices_informations.at( { plat , dev } ).m_max_clock_frequency ;
            if( cur_max_freq > max_freq )
            {
              max_freq = cur_max_freq ;
              best = cur_dev_id ;
            }
          }
        }
        else if( m_device_preference == OPENCL_DEVICE_PREFER_MAX_GLOBAL_MEMORY )
        {
          cl_ulong max_mem = 0 ;
          for( const auto & cur_dev_id : availablePreferedDevice )
          {
            const auto dev = m_devices_for_platform.at( plat )[ cur_dev_id ] ;
            const cl_ulong cur_mem = m_devices_informations.at( { plat , dev } ).m_max_global_memory_size ;
            if( cur_mem > max_mem )
            {
              max_mem = cur_mem ;
              best = cur_dev_id ;
            }
          }
        }
        m_current_device_id = best ;
      }


      if( m_current_device_id == std::numeric_limits<uint32_t>::max() && nb_dev > 0 )
      {
        // No preferable one available -> switch to the first device
        m_current_device_id = 0 ;
      }
    }
    else
    {
      // No device in that platform
      m_current_device_id = std::numeric_limits<uint32_t>::max() ;
    }

    return true ;
  }
  return false ;
}

/**
 * @brief Get profile a specific platform
 * @param id Id of the platform to query
 * @return Profile of the queried platform
 * @retval OPENCL_PLATFORM_PROFILE_INVALID if id is invalid or out of range
 */
OpenCLPlaformProfile OpenCLContext::platformProfile( const uint32_t id ) const
{
  if( id < m_nb_platform )
  {
    size_t outSize ;
    clGetPlatformInfo( m_platforms[ id ] , CL_PLATFORM_PROFILE , 0 , nullptr , &outSize ) ;

    char * paramData = new char[ outSize ] ;
    clGetPlatformInfo( m_platforms[id] , CL_PLATFORM_PROFILE , outSize , paramData , nullptr ) ;

    if( std::string( paramData ) == std::string( "FULL_PROFILE" ) )
    {
      return OPENCL_PLATFORM_PROFILE_FULL_PROFILE ;
    }
    else if( std::string( paramData ) == std::string( "EMBEDDED_PROFILE" ) )
    {
      return OPENCL_PLATFORM_PROFILE_EMBEDDED_PROFILE ;
    }
    else
    {
      return OPENCL_PLATFORM_PROFILE_INVALID ;
    }
    delete[] paramData ;
  }

  return OPENCL_PLATFORM_PROFILE_INVALID ;
}

/**
 * @brief Get profile of the current platform
 * @return Profile of the current platform
 * @retval OPENCL_PLATFORM_PROFILE_INVALID if current platform is invalid
 */
OpenCLPlaformProfile OpenCLContext::currentPlatformProfile( void ) const
{
  return platformProfile( m_current_platform_id ) ;
}

/**
 * @brief Get platform name
 * @param id Id of the platform to query
 * @return Name of the queried platform
 * @retval empty string if id is invalid or out of range
 */
std::string OpenCLContext::platformName( const uint32_t id ) const
{
  if( id < m_nb_platform )
  {
    size_t outSize ;
    clGetPlatformInfo( m_platforms[ id ] , CL_PLATFORM_NAME , 0 , nullptr , &outSize ) ;

    char * paramData = new char[ outSize ] ;
    clGetPlatformInfo( m_platforms[id] , CL_PLATFORM_NAME , outSize , paramData , nullptr ) ;

    std::string res( paramData ) ;

    delete[] paramData ;
    return res ;
  }
  return "" ;
}

/**
 * @brief Get current platform name
 * @return Name of the queried platform
 * @retval empty string if the current platform is invalid
 */
std::string OpenCLContext::currentPlatformName( void ) const
{
  return platformName( m_current_platform_id ) ;
}

/**
 * @brief Get Vendor name of a specific platform
 * @param id Id of the platform to query
 * @return Name of the vendor of the queried platform
 * @retval Empty string if id is invalid or out of range
 */
std::string OpenCLContext::platformVendor( const uint32_t id ) const
{
  if( id < m_nb_platform )
  {
    size_t outSize ;
    clGetPlatformInfo( m_platforms[ id ] , CL_PLATFORM_VENDOR , 0 , nullptr , &outSize ) ;

    char * paramData = new char[ outSize ] ;
    clGetPlatformInfo( m_platforms[id] , CL_PLATFORM_VENDOR , outSize , paramData , nullptr ) ;

    std::string res( paramData ) ;

    delete[] paramData ;
    return res ;
  }

  return "" ;
}

/**
 * @brief Get Vendor name of the current platform
 * @return Name of the vendor of the queried platform
 * @retval Empty string if the current platform is invalid
 */
std::string OpenCLContext::currentPlatformVendor( void ) const
{
  return platformVendor( m_current_platform_id ) ;
}

/**
 * @brief Get list of extensions supported by a specific platform
 * @param id Id of the platform to query
 * @return list of extensions supported by the queried platform
 * @retval Empty list if id is invalid or out of range
 */
std::vector<std::string> OpenCLContext::platformExtensions( const uint32_t id ) const
{
  if( id < m_nb_platform )
  {
    size_t outSize ;
    clGetPlatformInfo( m_platforms[ id ] , CL_PLATFORM_EXTENSIONS , 0 , nullptr , &outSize ) ;

    char * paramData = new char[ outSize ] ;
    clGetPlatformInfo( m_platforms[id] , CL_PLATFORM_EXTENSIONS , outSize , paramData , nullptr ) ;

    std::string tmp( paramData ) ;

    delete[] paramData ;

    std::vector< std::string > res ;
    stl::split( tmp , ' ' , res ) ;
    return res ;
  }

  return {} ;
}

/**
 * @brief Get list of extensions supported by the current platform
 * @return list of extensions supported by the current platform
 * @retval Empty list if the current platform is invalid
 */
std::vector<std::string> OpenCLContext::currentPlatformExtensions( void ) const
{
  return platformExtensions( m_current_platform_id ) ;
}

/// ---------------------------- END OF PLATFORM ---------------------------------------


/// ---------------------------- DEVICES ---------------------------------------
/**
  * @brief Get number of device of the specific platform
  * @param plat_id Id of the platform to query
  * @return number of device on the specific platform
  * @retval std::numeric_limits<uint32_t>::max() if plat_id is invalid
  */
uint32_t OpenCLContext::nbDeviceForPlatform( const uint32_t plat_id ) const
{
  if( plat_id < m_nb_platform )
  {
    return m_devices_for_platform.at( m_platforms[ plat_id ] ).size() ;
  }
  return std::numeric_limits<uint32_t>::max() ;
}

/**
 * @brief Get number of device on the current platform
 * @return number of device on the current platform
 * @retval std::numeric_limits<uint32_t>::max() if current platform is invalid
 */
uint32_t OpenCLContext::currentPlatformNbDevice( void ) const
{
  return nbDeviceForPlatform( m_current_platform_id ) ;
}

/**
 * @brief Get the current device on the current platform
 * @return current device
 */
cl_device_id OpenCLContext::currentDevice( void ) const
{
  if( currentPlatformValid() && currentDeviceValid() )
  {
    return m_devices_for_platform.at( m_platforms[ m_current_platform_id ] )[ m_current_device_id ] ;
  }
  return nullptr ;
}

/**
 * @brief Set the current device on the current platform
 * @param device_id New id of the current device
 * @retval true if set is OK
 * @retval false is setting the device id fails
 */
bool OpenCLContext::setCurrentDevice( const uint32_t device_id )
{
  if( currentPlatformValid() )
  {
    if( device_id < m_devices_for_platform.at( m_platforms[ m_current_platform_id ] ).size() )
    {
      m_current_device_id = device_id ;
      return true ;
    }
    return false  ;
  }
  return false ;
}

/**
 * @brief Get type of a specific device on a specific platform
 * @param plat_id Id of the platform to query
 * @param device_id Id of the device on the specific platform
 * @return type of the queried device
 * @retval OPENCL_DEVICE_TYPE_INVALID is device and/or platform is/are invalid
 */
OpenCLDeviceType OpenCLContext::deviceType( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_nb_platform )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      cl_device_type type ;

      clGetDeviceInfo( dev , CL_DEVICE_TYPE , sizeof( cl_device_type ) , &type , nullptr ) ;

      if( ( type & CL_DEVICE_TYPE_CPU ) != 0 )
      {
        return OPENCL_DEVICE_TYPE_CPU ;
      }
      else if( ( type & CL_DEVICE_TYPE_GPU ) != 0 )
      {
        return OPENCL_DEVICE_TYPE_GPU ;
      }
      else
      {
        return OPENCL_DEVICE_TYPE_OTHER ;
      }
    }
    return OPENCL_DEVICE_TYPE_INVALID ;
  }
  return OPENCL_DEVICE_TYPE_INVALID ;
}

/**
 * @brief Get current device type (on the current platform)
 * @return type of the current device
 * @retval OPENCL_DEVICE_TYPE_INVALID is device and/or platform is/are invalid
 */
OpenCLDeviceType OpenCLContext::currentDeviceType( void ) const
{
  return deviceType( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Get global memory size of a specific device on a specific platform
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return Global Memory size of the given device
 * @retval 0 if platform or device are invalid
 */
cl_ulong OpenCLContext::deviceGlobalMemorySize( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_max_global_memory_size ;
    }
    return 0 ;
  }
  return 0 ;
}

/**
 * @brief Get global memory size of the current device (on the current platform)
 * @return Global Memory size of current device
 * @retval 0 if current platform or current device are invalid
 */
cl_ulong OpenCLContext::currentDeviceGlobalMemorySize( void ) const
{
  return deviceGlobalMemorySize( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Get maximum width of a 2D image on the given device
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return maximum width of a 2D image on the given device
 * @retval 0 if image 2D is not supported on the given device or if platform/device is invalid
 */
size_t OpenCLContext::deviceMaxImage2DWidth( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_image_2D_max_width ;
    }
    return 0 ;
  }
  return 0 ;
}

/**
 * @brief Get maximum width of a 2D image on the current device
 * @return maximum width of a 2D image on the current device
 * @retval 0 if image 2D is not supported on the current device or if current platform/device is invalid
 */
size_t OpenCLContext::currentDeviceMaxImage2DWidth( void ) const
{
  return deviceMaxImage2DWidth( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Get maximum height of a 2D image on the given device
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return maximum height of a 2D image on the given device
 * @retval 0 if image 2D is not supported on the given device or if platform/device is invalid
 */
size_t OpenCLContext::deviceMaxImage2DHeight( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_image_2D_max_height ;
    }
    return 0 ;
  }
  return 0 ;
}

/**
 * @brief Get maximum height of a 2D image on the current device
 * @return maximum height of a 2D image on the current device
 * @retval 0 if image 2D is not supported on the current device or if current platform/device is invalid
 */
size_t OpenCLContext::currentDeviceMaxImage2DHeight( void ) const
{
  return deviceMaxImage2DHeight( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Indicate if a given device supports 2D images
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @retval true if image 2D is supported
 * @retval false if image 2D is unsupported
 */
bool OpenCLContext::deviceSupportImage2D( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_support_image_2D ;
    }
    return false ;
  }
  return false ;
}

/**
 * @brief Indicate if a current supports 2D images
 * @retval true if image 2D is supported
 * @retval false if image 2D is unsupported
 */
bool OpenCLContext::currentDeviceSupportImage2D( void ) const
{
  return deviceSupportImage2D( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Get maximum width of a 3D image on the given device
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return maximum width of a 3D image on the given device
 * @retval 0 if 3D image is not supported on the given device or if platform/device is invalid
 */
size_t OpenCLContext::deviceMaxImage3DWidth( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_image_3D_max_width ;
    }
    return 0 ;
  }
  return 0 ;
}

/**
 * @brief Get maximum width of a 3D image on the current device
 * @return maximum width of a 3D image on the current device
 * @retval 0 if image 3D is not supported on the current device or if current platform/device is invalid
 */
size_t OpenCLContext::currentDeviceMaxImage3DWidth( void ) const
{
  return deviceMaxImage3DWidth( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Get maximum height of a 3D image on the given device
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return maximum height of a 3D image on the given device
 * @retval 0 if image 3D is not supported on the given device or if platform/device is invalid
 */
size_t OpenCLContext::deviceMaxImage3DHeight( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_image_3D_max_height ;
    }
    return 0 ;
  }
  return 0 ;
}

/**
 * @brief Get maximum height of a 3D image on the current device
 * @return maximum height of a 3D image on the current device
 * @retval 0 if image 3D is not supported on the current device or if current platform/device is invalid
 */
size_t OpenCLContext::currentDeviceMaxImage3DHeight( void ) const
{
  return deviceMaxImage3DHeight( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Get maximum depth of a 3D image on the given device
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return maximum depth of a 3D image on the given device
 * @retval 0 if image 3D is not supported on the given device or if platform/device is invalid
 */
size_t OpenCLContext::deviceMaxImage3DDepth( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_image_3D_max_depth ;
    }
    return 0 ;
  }
  return 0 ;
}

/**
 * @brief Get maximum depth of a 3D image on the current device
 * @return Maximum depth of a 3D image on the current device
 * @retval 0 if image 3D is not supported on the current device or if current platform/device is invalid
 */
size_t OpenCLContext::currentDeviceMaxImage3DDepth( void ) const
{
  return deviceMaxImage3DDepth( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Indicate if a given device supports 3D images
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @retval true if image 3D is supported
 * @retval false if image 3D is unsupported
 */
bool OpenCLContext::deviceSupportImage3D( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_support_image_3D ;
    }
    return false ;
  }
  return false ;
}

/**
 * @brief Indicate if a current supports 3D images
 * @retval true if image 3D is supported
 * @retval false if image 3D is unsupported
 */
bool OpenCLContext::currentDeviceSupportImage3D( void ) const
{
  return deviceSupportImage3D( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Get maximum clock frequency of a given device
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return Maximum clock frequency of the given device
 * @retval 0 if plat_id/device_id is invalid
 */
cl_uint OpenCLContext::deviceMaxClockFrequency( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_max_clock_frequency ;
    }
    return 0 ;
  }
  return 0 ;
}

/**
 * @brief Get maximum clock frequency of a the current device
 * @return Maximum clock frequency of the current device
 * @retval 0 if current platform/device is invalid
 */
cl_uint OpenCLContext::currentDeviceMaxClockFrequency( void ) const
{
  return deviceMaxClockFrequency( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Get maximum compute units of the given device
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return Maximum compute units for the given device
 * @retval 0 if plat_id/device_id is invalid
 */
cl_uint OpenCLContext::deviceMaxComputeUnits( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_max_compute_units ;
    }
    return 0 ;
  }
  return 0 ;
}

/**
 * @brief Get maximum compute units of the current device
 * @return Maximum compute units for the current device
 * @retval 0 if current platform/device is invalid
 */
cl_uint OpenCLContext::currentDeviceMaxComputeUnits( void ) const
{
  return deviceMaxComputeUnits( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Get name of the given device
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return Name of the queried device
 * @retval Empty string if plat_id/device_id is invalid
 */
std::string OpenCLContext::deviceName( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;

    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_name ;
    }
    return "" ;
  }
  return "" ;
}

/**
 * @brief Get name of the current device
 * @return Name of the current device
 * @retval Empty string if current platform/device is invalid
 */
std::string OpenCLContext::currentDeviceName( void ) const
{
  return deviceName( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Get vendor name of the given device
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return Vendor name of the queried device
 * @retval Empty string if plat_id/device_id is invalid
 */
std::string OpenCLContext::deviceVendor( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_devices_informations.at( { plat , dev } ).m_vendor_name ;
    }
    return "" ;
  }
  return "" ;
}

/**
 * @brief Get vendor name of the current device
 * @return Vendor name of the current device
 * @retval Empty string if current plaform/device is invalid
 */
std::string OpenCLContext::currentDeviceVendor( void ) const
{
  return deviceVendor( m_current_platform_id , m_current_device_id ) ;
}

/// ---------------------------- END OF DEVICES ---------------------------------------

/**
 * @brief fill platform informations
 */
void OpenCLContext::fillPlatformsInfos( void )
{
  cl_uint nb_plat ;
  clGetPlatformIDs( 0 , nullptr , &nb_plat ) ;
  m_nb_platform = nb_plat ;

  if( m_nb_platform > 0 )
  {
    m_platforms.resize( m_nb_platform ) ;
    clGetPlatformIDs( nb_plat , &m_platforms[0] , nullptr ) ;

    // Set the platform 0 as the default one
    m_current_platform_id = 0 ;
  }
}

/**
 * @brief Get standard kernels (ie: defined by openMVG)
 * @param kernel_name Name of the kernel to get
 * @return Corresponding kernel
 * @retval nullptr if kernel_name is not a valid openMVG kernel name
 */
cl_kernel OpenCLContext::standardKernel( const std::string & kernel_name ) const
{
  if( m_standard_kernels.count( kernel_name ) == 0 )
  {
    return nullptr ;
  }
  else
  {
    return m_standard_kernels.at( kernel_name ) ;
  }
}


/**
 * @brief Indicate if current platform is valid
 * @retval true if valid
 * @retval false if invalid
 */
bool OpenCLContext::currentPlatformValid( void ) const
{
  return m_current_platform_id < m_nb_platform ;
}

/**
 * @brief Indicate if current device on current platform is valid
 * @retval true if valid
 * @retval false if invalid
 * @note if current platform is invalid, returns false
 */
bool OpenCLContext::currentDeviceValid( void ) const
{
  return currentPlatformValid() &&
         m_current_device_id < m_devices_for_platform.at( m_platforms[ m_current_platform_id ] ).size() ;
}

/**
 * @brief Fill devices information (for all platforms)
 */
void OpenCLContext::fillDevicesInfos( void )
{
  for( auto & plat_id : m_platforms )
  {
    std::vector< cl_device_id > devices_for_current_platform ;

    cl_uint nb_device_for_current_platform ;

    clGetDeviceIDs( plat_id , CL_DEVICE_TYPE_ALL , 0 , nullptr , &nb_device_for_current_platform ) ;

    if( nb_device_for_current_platform > 0 )
    {
      devices_for_current_platform.resize( nb_device_for_current_platform ) ;
      clGetDeviceIDs( plat_id , CL_DEVICE_TYPE_ALL , nb_device_for_current_platform , &devices_for_current_platform[0] , nullptr ) ;
    }

    for( auto & device_id : devices_for_current_platform )
    {
      OpenCLDeviceInfos infos ;
      // std::string m_name ;
      {
        size_t paramSize ;
        clGetDeviceInfo( device_id , CL_DEVICE_NAME , 0 , nullptr , &paramSize ) ;

        char * name = new char[ paramSize ] ;
        clGetDeviceInfo( device_id , CL_DEVICE_NAME , paramSize , name , nullptr ) ;

        infos.m_name = std::string( name ) ;
        delete[] name ;
      }
      // std::string m_vendor_name ;
      {
        size_t paramSize ;
        clGetDeviceInfo( device_id , CL_DEVICE_VENDOR , 0 , nullptr , &paramSize ) ;

        char * vendorName = new char[ paramSize ] ;
        clGetDeviceInfo( device_id , CL_DEVICE_VENDOR , paramSize , vendorName , nullptr ) ;

        infos.m_vendor_name = std::string( vendorName ) ;
        delete[] vendorName ;
      }
      // cl_ulong m_max_global_memory_size ;
      {
        clGetDeviceInfo( device_id , CL_DEVICE_GLOBAL_MEM_SIZE , sizeof( cl_ulong ) , &infos.m_max_global_memory_size , nullptr ) ;
      }
      // cl_uint m_max_clock_frequency ;
      {
        clGetDeviceInfo( device_id , CL_DEVICE_MAX_CLOCK_FREQUENCY , sizeof( cl_uint ) , &infos.m_max_clock_frequency , nullptr ) ;
      }
      // cl_uint m_max_compute_units ;
      {
        clGetDeviceInfo( device_id , CL_DEVICE_MAX_COMPUTE_UNITS , sizeof( cl_uint ) , &infos.m_max_compute_units , nullptr ) ;
      }
      // bool m_support_image_2D ;
      // bool m_support_image_3D ;
      {
        cl_bool supportImages ;
        clGetDeviceInfo( device_id , CL_DEVICE_IMAGE_SUPPORT , sizeof( cl_bool ) , &supportImages , nullptr ) ;
        infos.m_support_image_2D = supportImages ;
        infos.m_support_image_3D = supportImages ;
      }
      // size_t m_image_2D_max_width ;
      {
        clGetDeviceInfo( device_id , CL_DEVICE_IMAGE2D_MAX_WIDTH , sizeof( size_t ) , &infos.m_image_2D_max_width , nullptr ) ;
      }
      // size_t m_image_2D_max_height ;
      {
        clGetDeviceInfo( device_id , CL_DEVICE_IMAGE2D_MAX_HEIGHT , sizeof( size_t ) , &infos.m_image_2D_max_height , nullptr ) ;
      }
      // size_t m_image_3D_max_width ;
      {
        clGetDeviceInfo( device_id , CL_DEVICE_IMAGE3D_MAX_WIDTH , sizeof( size_t ) , &infos.m_image_3D_max_width , nullptr ) ;
      }
      // size_t m_image_3D_max_height ;
      {
        clGetDeviceInfo( device_id , CL_DEVICE_IMAGE3D_MAX_HEIGHT , sizeof( size_t ) , &infos.m_image_3D_max_height , nullptr ) ;
      }
      // size_t m_image_3D_max_depth ;
      {
        clGetDeviceInfo( device_id , CL_DEVICE_IMAGE3D_MAX_DEPTH , sizeof( size_t ) , &infos.m_image_3D_max_depth , nullptr ) ;
      }
      m_devices_informations.insert( { { plat_id , device_id } , infos } ) ;
    }
    m_devices_for_platform.insert( { plat_id , devices_for_current_platform } ) ;
  }
}


/**
 * @brief get context for a specific pair platform/device
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return context with the specified pair platform/device
 * @retval nullptr if pair plat_id/device_id is invalid
 */
cl_context OpenCLContext::context( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    const auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      const auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      return m_contexts.at( { plat , dev } ) ;
    }
    return nullptr ;
  }
  return nullptr ;
}

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
cl_program OpenCLContext::createAndBuildProgram( const std::string & program_source , const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    const auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      const auto dev = m_devices_for_platform.at( plat )[ device_id ] ;
      auto ctx = m_contexts.at( { plat , dev } ) ;

      // 1 - Create program
      const char * c_str = program_source.c_str() ;
      const size_t len = program_source.size() ;
      cl_int error ;
      cl_program pgm = clCreateProgramWithSource( ctx , 1 , &c_str , &len , &error ) ;
      if( error != CL_SUCCESS )
      {
        return nullptr ;
      }

      // 2 - build it
      error = clBuildProgram( pgm , 1 , &dev , nullptr , nullptr , nullptr ) ;
      if( error != CL_SUCCESS )
      {
        std::cerr << "Could not build program" << std::endl ;

        size_t log_size ;
        clGetProgramBuildInfo( pgm , dev , CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size );

        char * log = new char[ log_size ] ;

        // Get the log
        clGetProgramBuildInfo( pgm , dev , CL_PROGRAM_BUILD_LOG, log_size, log, NULL );

        std::cerr << "Log is " << log << std::endl ;

        delete[] log ;

        return pgm ;
      }

      // Note : we do not destroy the program in order to allow looking at the build log
      return pgm ;
    }
    return nullptr ;
  }
  return nullptr ;
}

/**
 * @brief Create a program and build it (ie: compile and link) then return the given program
 * @param program_source Source code of the program
 * @return The compiled program
 * @retval nullptr If something fails
 * @retval nullptr If current context is invalid
 * @retval nullptr If creation fails
 * @retval nullptr If compilation/link fails
 * @note Compile on the current context
 */
cl_program OpenCLContext::createAndBuildProgram( const std::string & program_source ) const
{
  return createAndBuildProgram( program_source , m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Check if the program build is ok
 * @param pgm The program to check
 * @retval true if program build is ok
 * @retval false if program build fails
 */
bool OpenCLContext::valid( cl_program pgm ) const
{
  if( pgm )
  {
    cl_device_id dev[1] ;
    cl_int error = clGetProgramInfo( pgm , CL_PROGRAM_DEVICES , sizeof( cl_device_id ) , dev , nullptr ) ;
    if( error != CL_SUCCESS )
    {
      return false ;
    }

    cl_build_status status ;
    clGetProgramBuildInfo( pgm , dev[0] , CL_PROGRAM_BUILD_STATUS , sizeof( cl_build_status ) , &status , nullptr ) ;

    return status == CL_BUILD_SUCCESS ;
  }

  return false ;
}

/**
 * @brief Get build log for a specific program
 * @param pgm Program
 * @return build log
 * @note The function looks for the device it was previously compiled on so no pair plat/device is needed
 */
std::string OpenCLContext::programBuildLog( cl_program pgm ) const
{
  if( pgm )
  {
    cl_device_id dev[1] ;
    cl_int error = clGetProgramInfo( pgm , CL_PROGRAM_DEVICES , sizeof( cl_device_id ) , dev , nullptr ) ;
    if( error != CL_SUCCESS )
    {
      return "" ;
    }

    size_t log_size ;
    clGetProgramBuildInfo( pgm , dev[0] , CL_PROGRAM_BUILD_LOG , 0 , nullptr , &log_size ) ;

    char * log = new char[ log_size ] ;
    clGetProgramBuildInfo( pgm , dev[0] , CL_PROGRAM_BUILD_LOG , log_size , log , nullptr ) ;

    std::string res( log ) ;

    delete[] log ;
    return res ;
  }
  return "" ;
}

/// ------------------------------- KERNELS ------------------------------------------

/**
 * @brief Create a kernel given it's program and it's name
 * @param pgm The program in which the kernel is looked
 * @param kernel_name Name of the kernel to create
 * @return the created kernel
 * @retval nullptr if creation fails
 */
cl_kernel OpenCLContext::createKernel( cl_program pgm , const std::string & kernel_name ) const
{
  return clCreateKernel( pgm , kernel_name.c_str() , nullptr ) ;
}

/**
 * @brief Create all kernels situated inside the program
 * @param pgm The program in which the kernels are looked
 * @return List of kernels (and their associated function names) inside the program
 */
std::map< std::string , cl_kernel > OpenCLContext::createKernels( cl_program pgm ) const
{
  cl_uint nb_kernel ;
  clCreateKernelsInProgram( pgm , 0 , nullptr , &nb_kernel ) ;

  cl_kernel * krns = new cl_kernel[ nb_kernel ] ;
  clCreateKernelsInProgram( pgm , nb_kernel , krns , nullptr ) ;

  std::map< std::string , cl_kernel > res ;
  for( cl_uint id_krn = 0 ; id_krn < nb_kernel ; ++id_krn )
  {
    res.insert( { kernelName( krns[ id_krn ] ) , krns[ id_krn ] } ) ;
  }

  delete[] krns ;

  return res ;
}

/**
 * @brief Get function name of the given kernel
 * @param krn The kernel to query
 * @return Function name associated with the given kernel
 */
std::string OpenCLContext::kernelName( cl_kernel krn ) const
{
  size_t fctNameLog ;
  clGetKernelInfo( krn , CL_KERNEL_FUNCTION_NAME , 0 , nullptr , &fctNameLog ) ;

  char * name = new char[ fctNameLog ] ;
  clGetKernelInfo( krn , CL_KERNEL_FUNCTION_NAME , fctNameLog , name , nullptr ) ;

  std::string res( name ) ;
  delete[] name ;

  return res ;
}

/**
 * @brief Get number of argument of the given kernel
 * @param krn The kernel to query
 * @return Number of arguments of the kernel
 */
cl_uint OpenCLContext::kernelNumberOfArgument( cl_kernel krn ) const
{
  cl_uint res ;

  clGetKernelInfo( krn , CL_KERNEL_NUM_ARGS , sizeof( cl_uint ) , &res , nullptr ) ;

  return res ;
}

/**
 * @brief Get maximum work-group size of the given kernel
 * @param krn The kernel to query
 * @return Maximum workgroup size
 */
size_t OpenCLContext::kernelMaxWorkgroupSize( cl_kernel krn ) const
{
  size_t res ;

  clGetKernelWorkGroupInfo( krn , nullptr , CL_KERNEL_WORK_GROUP_SIZE , sizeof( size_t ) , &res , nullptr ) ;

  return res ;
}

/**
 * @brief Get maximum global size used to execute the given kernel
 * @param krn The kernel to query
 * @return Maximum global size (in each dimension)
 */
std::tuple<size_t, size_t, size_t> OpenCLContext::kernelGlobalWorkSize( cl_kernel krn ) const
{
  size_t tmp[3] ;

  clGetKernelWorkGroupInfo( krn , nullptr , CL_KERNEL_GLOBAL_WORK_SIZE , 3 * sizeof( size_t ) , tmp , nullptr ) ;

  return std::make_tuple( tmp[0] , tmp[1] , tmp[2] ) ;
}


/**
 * @brief Prefered work-group size multiple used to execute the kernel
 * @param krn The kernel to query
 * @return prefered workgroup size multiple
 */
size_t OpenCLContext::kernelPreferedWorkGroupSizeMultiple( cl_kernel krn ) const
{
  size_t res ;

  clGetKernelWorkGroupInfo( krn , nullptr , CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE , sizeof( size_t ) , &res , nullptr ) ;

  return res ;
}

static inline size_t NextMultipleOf( const size_t N , const size_t K )
{
  return N + ( K - N % K ) % K ;
}

/**
 * @brief Run 2d kernel on the current platform/device
 * @param krn Kernel
 * @param work_dim Work dimension (width,height)
 * @retval true If run is ok
 * @retval false If run fails
 */
bool OpenCLContext::runKernel2d( cl_kernel krn , const size_t * work_dim , const size_t * group_size ) const
{
  size_t workGroupSize[2] ;

  if( ! group_size )
  {
    size_t preferedWorkGroupSize ;
    cl_int err = clGetKernelWorkGroupInfo( krn , nullptr , CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE , sizeof( size_t ) , &preferedWorkGroupSize , nullptr ) ;
    if( err != CL_SUCCESS )
    {
      return false ;
    }
    // TODO : provide something more automatic wrt to the maximum number of work item
    // ie : CL_DEVICE_MAX_WORK_GROUP_SIZE
    workGroupSize[0] = std::min( preferedWorkGroupSize , static_cast<size_t>( 16 ) ) ;
    workGroupSize[1] = std::min( preferedWorkGroupSize , static_cast<size_t>( 16 ) ) ;
  }
  else
  {
    workGroupSize[0] = group_size[0] ;
    workGroupSize[1] = group_size[1] ;
  }

  const size_t realSize[2] =
  {
    NextMultipleOf( work_dim[0] , workGroupSize[0] ) ,
    NextMultipleOf( work_dim[1] , workGroupSize[1] )
  } ;

  cl_command_queue queue = currentCommandQueue() ;
  cl_int err = clEnqueueNDRangeKernel( queue , krn , 2 , nullptr , realSize , workGroupSize , 0 , nullptr , nullptr ) ;

  if( err != CL_SUCCESS )
  {
    std::cerr << "Cannot enqueue kernel" << std::endl ;

    if( err == CL_INVALID_PROGRAM_EXECUTABLE )
    {
      std::cerr << "CL_INVALID_PROGRAM_EXECUTABLE" << std::endl ;
    }
    else if( err == CL_INVALID_COMMAND_QUEUE )
    {
      std::cerr << "CL_INVALID_COMMAND_QUEUE" << std::endl ;
    }
    else if( err == CL_INVALID_KERNEL )
    {
      std::cerr << "CL_INVALID_KERNEL" << std::endl ;
    }
    else if( err == CL_INVALID_CONTEXT )
    {
      std::cerr << "CL_INVALID_CONTEXT" << std::endl ;
    }
    else if( err == CL_INVALID_KERNEL_ARGS )
    {
      std::cerr << "CL_INVALID_KERNEL_ARGS" << std::endl ;
    }
    else if( err == CL_INVALID_WORK_DIMENSION )
    {
      std::cerr << "CL_INVALID_WORK_DIMENSION" << std::endl ;
    }
    else if( err == CL_INVALID_WORK_GROUP_SIZE )
    {
      std::cerr << "CL_INVALID_WORK_GROUP_SIZE" << std::endl ;
    }
    else if( err == CL_INVALID_WORK_ITEM_SIZE )
    {
      std::cerr << "CL_INVALID_WORK_ITEM_SIZE" << std::endl ;
    }
    else if( err == CL_INVALID_GLOBAL_OFFSET )
    {
      std::cerr << "CL_INVALID_GLOBAL_OFFSET" << std::endl ;
    }
    else if( err == CL_OUT_OF_RESOURCES )
    {
      std::cerr << "CL_OUT_OF_RESOURCES" << std::endl ;
    }
    else if( err == CL_MEM_OBJECT_ALLOCATION_FAILURE )
    {
      std::cerr << "CL_MEM_OBJECT_ALLOCATION_FAILURE" << std::endl ;
    }
    else if( err == CL_INVALID_EVENT_WAIT_LIST )
    {
      std::cerr << "CL_INVALID_EVENT_WAIT_LIST" << std::endl ;
    }
    else if( err == CL_OUT_OF_HOST_MEMORY )
    {
      std::cerr << "CL_OUT_OF_HOST_MEMORY" << std::endl ;
    }
    return false ;
  }

  clFinish( queue ) ;

  return true ;
}

/// ------------------------------- END OF KERNELS ------------------------------------------


/// ------------------------------- COMMAND QUEUES ------------------------------------------
/**
 * @brief Get default command queue associated with the specified pair platform/device
 * @param plat_id Id of the platform
 * @param device_id Id of the device
 * @return the command queue associated with the specified pair
 * @retval nullptr if the specified pair (platform/device) is invalid
 */
cl_command_queue OpenCLContext::commandQueue( const uint32_t plat_id , const uint32_t device_id ) const
{
  if( plat_id < m_platforms.size() )
  {
    auto plat = m_platforms[ plat_id ] ;
    if( device_id < m_devices_for_platform.at( plat ).size() )
    {
      auto dev = m_devices_for_platform.at( plat )[ device_id ] ;

      return m_command_queues.at( { plat , dev } ) ;
    }
    return nullptr ;
  }
  return nullptr ;
}

/**
 * @brief Get command queue associated with the current pair platform/device
 * @return the command queue associated with the current pair platform/device
 */
cl_command_queue OpenCLContext::currentCommandQueue( void ) const
{
  return commandQueue( m_current_platform_id , m_current_device_id ) ;
}

/// ---------------------------- END OF COMMAND QUEUES --------------------------------------

/**
 * @brief get context for current platform/device
 * @return context of the current platform/device
 * @retval nullptr if pair plat_id/device_id is invalid
 */
cl_context OpenCLContext::currentContext( void ) const
{
  return context( m_current_platform_id , m_current_device_id ) ;
}

/**
 * @brief Create contexts for all pairs platform/devices
 */
void OpenCLContext::createContexts( void )
{
  for( auto plat : m_platforms )
  {
    for( auto dev : m_devices_for_platform.at( plat ) )
    {
      cl_context_properties properties [] =
      {
        CL_CONTEXT_PLATFORM ,
        reinterpret_cast<cl_context_properties>( plat ) ,
        0
      } ;

      cl_context ctx = clCreateContext( properties , 1 , &dev , nullptr , nullptr , nullptr ) ;

      m_contexts.insert( { { plat , dev } , ctx } ) ;
    }
  }
}

/**
 * @brief Releases contexts and destroy them if their reference count is 0
 */
void OpenCLContext::releaseContexts( void )
{
  for( auto plat : m_platforms )
  {
    for( auto dev : m_devices_for_platform.at( plat ) )
    {
      cl_context ctx = m_contexts.at( { plat , dev } );
      if( ctx )
      {
        clReleaseContext( ctx );
      }
    }
  }
}


/**
 * @brief Create command queues for all pairs platform/devices
 */
void OpenCLContext::createCommandQueues( void )
{
  for( auto plat : m_platforms )
  {
    for( auto device : m_devices_for_platform.at( plat ) )
    {
      auto ctx = m_contexts.at( { plat , device } ) ;

      auto cq = clCreateCommandQueue( ctx , device , 0 , nullptr ) ;

      m_command_queues.insert( { { plat , device } , cq } ) ;
    }
  }
}

/**
 * @brief Release command queues
 */
void OpenCLContext::releaseCommandQueues( void )
{
  for( auto & cq : m_command_queues )
  {
    if( cq.second )
    {
      clReleaseCommandQueue( cq.second ) ;
    }
  }
}

/**
 * @brief Create standard kernels
 */
void OpenCLContext::loadStandardKernels( void )
{
  // Image kernels
  {
    // Image add
    {
      cl_program pgm = createAndBuildProgram( image::gpu::kernels::krnsImageAdd ) ;
      m_standard_programs.emplace_back( pgm ) ;
      const std::vector<std::string> kernelsAddList = { "image_add_ui" , "image_add_i" , "image_add_f" } ;
      for( const auto & cur_krn : kernelsAddList )
      {
        if( ! m_standard_kernels.count( cur_krn ) )
        {
          m_standard_kernels.insert( { cur_krn , createKernel( pgm , cur_krn ) } ) ;
        }
      }
    }
    // Image sub
    {
      cl_program pgm = createAndBuildProgram( image::gpu::kernels::krnsImageSub ) ;
      m_standard_programs.emplace_back( pgm ) ;
      const std::vector<std::string> kernelsAddList = { "image_sub_ui" , "image_sub_i" , "image_sub_f" } ;
      for( const auto & cur_krn : kernelsAddList )
      {
        if( ! m_standard_kernels.count( cur_krn ) )
        {
          m_standard_kernels.insert( { cur_krn , createKernel( pgm , cur_krn ) } ) ;
        }
      }
    }
    // Image product
    {
      cl_program pgm = createAndBuildProgram( image::gpu::kernels::krnsImageMul ) ;
      m_standard_programs.emplace_back( pgm ) ;
      const std::vector<std::string> kernelsAddList = { "image_mul_ui" , "image_mul_i" , "image_mul_f" } ;
      for( const auto & cur_krn : kernelsAddList )
      {
        if( ! m_standard_kernels.count( cur_krn ) )
        {
          m_standard_kernels.insert( { cur_krn , createKernel( pgm , cur_krn ) } ) ;
        }
      }
    }
    // Image convolution
    {
      // Naive 2d kernel
      cl_program pgm = createAndBuildProgram( image::gpu::kernels::krnsImageConvolve2dNaive ) ;
      m_standard_programs.emplace_back( pgm ) ;
      if( ! m_standard_kernels.count( "convolve_2d_naive_f" ) )
      {
        m_standard_kernels.insert( { "convolve_2d_naive_f" , createKernel( pgm , "convolve_2d_naive_f" ) } );
      }
      // 2D + local fetch
      pgm = createAndBuildProgram( image::gpu::kernels::krnsImageConvolve2dLocalMem ) ;
      m_standard_programs.emplace_back( pgm ) ;
      if( ! m_standard_kernels.count( "convolve_2d_local_f" ) )
      {
        m_standard_kernels.insert( { "convolve_2d_local_f" , createKernel( pgm , "convolve_2d_local_f" ) } ) ;
      }
      // Naive horizontal convolution
      pgm = createAndBuildProgram( image::gpu::kernels::krnsImageHorizontalConvolveNaive ) ;
      m_standard_programs.emplace_back( pgm ) ;
      if( ! m_standard_kernels.count( "horizontal_convolve_naive_f" ) )
      {
        m_standard_kernels.insert( { "horizontal_convolve_naive_f" , createKernel( pgm , "horizontal_convolve_naive_f" ) } ) ;
      }
      // Naive vertical convolution
      pgm = createAndBuildProgram( image::gpu::kernels::krnsImageVerticalConvolveNaive ) ;
      m_standard_programs.emplace_back( pgm ) ;
      if( ! m_standard_kernels.count( "vertical_convolve_naive_f" ) )
      {
        m_standard_kernels.insert( { "vertical_convolve_naive_f" , createKernel( pgm , "vertical_convolve_naive_f" ) } ) ;
      }
    } // Image convolution
  } // Image kernels

}

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
cl_mem OpenCLContext::createImage( const size_t width , const size_t height ,
                                   const OpenCLImageChannelOrder order ,
                                   const OpenCLImageDataType dtype ,
                                   const OpenCLImageAccessType access ,
                                   void * data ) const
{
  cl_image_format format ;
  cl_image_desc desc ;

  switch( order )
  {
    case OPENCL_IMAGE_CHANNEL_ORDER_R:
    {
      format.image_channel_order     = CL_R ;
      break ;
    }
    case OPENCL_IMAGE_CHANNEL_ORDER_RGBA:
    {
      format.image_channel_order     = CL_RGBA ;
      break ;
    }
    case OPENCL_IMAGE_CHANNEL_ORDER_BGRA:
    {
      format.image_channel_order     = CL_ARGB ;
      break;
    }
  }
  switch( dtype )
  {
    case OPENCL_IMAGE_DATA_TYPE_U_INT_8 :   // Unsigned int 8
    {
      format.image_channel_data_type = CL_UNSIGNED_INT8 ;
      break ;
    }
    case OPENCL_IMAGE_DATA_TYPE_U_INT_32 :   // Unsigned int 32
    {
      format.image_channel_data_type = CL_UNSIGNED_INT32 ;
      break ;
    }
    case OPENCL_IMAGE_DATA_TYPE_SU_INT_8 :   // Signed int 8
    {
      format.image_channel_data_type = CL_SIGNED_INT8 ;
      break ;
    }
    case OPENCL_IMAGE_DATA_TYPE_SU_INT_32 :   // Signed int 32
    {
      format.image_channel_data_type = CL_SIGNED_INT32 ;
      break ;
    }
    case OPENCL_IMAGE_DATA_TYPE_UN_INT_8 :   // Unsigned int 8 - Normalized (ie: 0-1 and read_imagef/write_imagef )
    {
      format.image_channel_data_type = CL_UNORM_INT8 ;
      break ;
    }
    case OPENCL_IMAGE_DATA_TYPE_FLOAT :   // Float
    {
      format.image_channel_data_type = CL_FLOAT ;
      break ;
    }
  }


  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = width ;
  desc.image_height = height ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = nullptr ;

  cl_int error;

  cl_mem_flags flags = 0 ;
  switch( access )
  {
    case OPENCL_IMAGE_ACCESS_READ_ONLY:
    {
      flags = CL_MEM_READ_ONLY ;
      break ;
    }
    case OPENCL_IMAGE_ACCESS_WRITE_ONLY:
    {
      flags = CL_MEM_WRITE_ONLY ;
      break ;
    }
    case OPENCL_IMAGE_ACCESS_READ_WRITE:
    {
      flags = CL_MEM_READ_WRITE ;
      break ;
    }
  }

  if( data != nullptr )
  {
    flags |= CL_MEM_COPY_HOST_PTR ;
  }

  cl_mem res = clCreateImage( currentContext() , flags , &format , &desc , data , &error ) ;

  if( error != CL_SUCCESS )
  {
    return nullptr ;
  }
  return res ;
}

/**
 * @brief Create a buffer
 * @param size Size (in byte) of the buffer to create
 * @param access Access type for the newly created buffer
 * @param data Data to provide to the buffer
 * @return Buffer created
 * @retval nullptr if there is an error during creation
 */
cl_mem OpenCLContext::createBuffer( const size_t size , const OpenCLBufferAccessType access , void * data ) const
{
  cl_mem_flags flags = 0 ;

  switch( access )
  {
    case OPENCL_BUFFER_ACCESS_READ_ONLY:
    {
      flags = CL_MEM_READ_ONLY ;
      break ;
    }
    case OPENCL_BUFFER_ACCESS_WRITE_ONLY:
    {
      flags = CL_MEM_WRITE_ONLY ;
      break ;
    }
    case OPENCL_BUFFER_ACCESS_READ_WRITE:
    {
      flags = CL_MEM_READ_WRITE ;
      break ;
    }
  }

  if( data != nullptr )
  {
    flags |= CL_MEM_COPY_HOST_PTR ;
  }

  cl_int error ;
  cl_mem res = clCreateBuffer( currentContext() , flags , size , data , &error ) ;

  return ( error == CL_SUCCESS ) ? res : nullptr ;

}


/**
 * @brief Release stdandard kernels
 */
void OpenCLContext::releaseStandardKernels( void )
{
  for( auto it : m_standard_kernels )
  {
    if( it.second )
    {
      clReleaseKernel( it.second ) ;
    }
  }
  for( auto it : m_standard_programs )
  {
    if( it )
    {
      clReleaseProgram( it ) ;
    }
  }
}

} // namespace gpu
} // namespace system
} // namespace openMVG