// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/system/gpu/OpenCLContext.hpp"

#include "openMVG/stl/split.hpp"

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
 */
OpenCLContext::OpenCLContext( const OpenCLDeviceType prefered_device_type )
  : m_nb_platform( 0 ) ,
    m_current_platform_id( std::numeric_limits<uint32_t>::max() ) ,
    m_current_device_id( std::numeric_limits<uint32_t>::max() ) ,
    m_prefered_device_type( prefered_device_type )
{
  fillPlatformsInfos() ;
  fillDevicesInfos() ;
}

/**
 * @brief Dtr
 */
OpenCLContext::~OpenCLContext( void )
{
  // TODO
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
    // TODO : set the device id on the specific platform
    if( m_devices_for_platform.at( m_platforms[ m_current_platform_id ] ).size() > 0 )
    {
      m_current_device_id = std::numeric_limits<uint32_t>::max() ;

      const uint32_t nb_dev = m_devices_for_platform.at( m_platforms[ m_current_platform_id ] ).size() ;
      for( uint32_t id_dev = 0 ; id_dev < nb_dev ; ++id_dev )
      {
        if( deviceType( m_current_platform_id , id_dev ) == m_prefered_device_type )
        {
          m_current_device_id = id_dev ;
          break ;
        }
      }
      if( m_current_device_id == std::numeric_limits<uint32_t>::max() )
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
    clGetPlatformInfo( m_platforms[ id ] , CL_PLATFORM_VENDOR , 0 , nullptr , &outSize ) ;

    char * paramData = new char[ outSize ] ;
    clGetPlatformInfo( m_platforms[id] , CL_PLATFORM_VENDOR , outSize , paramData , nullptr ) ;

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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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
      auto dev = m_devices_for_platform.at( plat )[ m_current_device_id ] ;
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

        infos.m_name = std::string( vendorName ) ;
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

} // namespace gpu
} // namespace system
} // namespace openMVG