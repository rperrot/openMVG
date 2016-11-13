#ifndef _OPENMVG_MVS_OPENCL_UTILS_HPP_
#define _OPENMVG_MVS_OPENCL_UTILS_HPP_

#ifdef __APPLE__
  #include <OpenCL/OpenCL.h>
#else
  #include <CL/cl.h>
#endif

namespace MVS
{
  /**
  * @brief Dump Platform information
  * @param platform Platform to dump
  */
  void GetPlatformInfo( cl_platform_id platform ) ;

  /**
  * @brief Dump device information
  * @param device Device to dump
  */
  void GetDeviceInfo( cl_device_id device ) ;

  /**
  * @brief Get number of compute unit given a device
  * @param device Device to query
  * @return number of compute unit for this device
  */
  int GetMaxComputeUnitForDevice( cl_device_id device ) ;

  /**
  * @brief Get total memory for this device
  * @param device Device to query
  * @return total memory
  */
  unsigned long GetGlobalMemoryForDevice( cl_device_id device ) ;

  /**
  * @brief Get maximum work group size
  * @param device Device to query
  * @return maximum WG size for this device
  */
  size_t GetMaxWorkGroupSize( cl_device_id device ) ;

  /**
  * @brief Get Maximum allocatable size of a 2d image
  * @param device Device to query
  * @return maximum image width
  */
  int GetDeviceMaxImage2dWidth( cl_device_id device ) ;

  /**
  * @brief Get Maximum allocatable size of a 2d image
  * @param device Device to query
  * @return maximum image height
  */
  int GetDeviceMaxImage2dHeight( cl_device_id device ) ;
}

#endif