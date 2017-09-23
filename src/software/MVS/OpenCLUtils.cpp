#include "OpenCLUtils.hpp"

#include <iostream>

namespace MVS
{
void GetPlatformInfo( cl_platform_id platform )
{
  size_t size ;

  clGetPlatformInfo( platform , CL_PLATFORM_PROFILE , 0 , NULL , &size ) ;
  {
    char * buffer = new char[ size ] ;
    clGetPlatformInfo( platform , CL_PLATFORM_PROFILE , size , buffer , NULL ) ;
    std::cerr << "CL_PLATFORM_PROFILE : " << buffer << std::endl ;
    delete[] buffer ;
  }

  clGetPlatformInfo( platform , CL_PLATFORM_VERSION , 0 , NULL , &size ) ;
  {
    char * buffer = new char[ size ] ;
    clGetPlatformInfo( platform , CL_PLATFORM_VERSION , size , buffer , NULL ) ;
    std::cerr << "CL_PLATFORM_VERSION : " << buffer << std::endl ;
    delete[] buffer ;
  }

  clGetPlatformInfo( platform , CL_PLATFORM_NAME , 0 , NULL , &size ) ;
  {
    char * buffer = new char[ size ] ;
    clGetPlatformInfo( platform , CL_PLATFORM_NAME , size , buffer , NULL ) ;
    std::cerr << "CL_PLATFORM_NAME : " << buffer << std::endl ;
    delete[] buffer ;
  }

  clGetPlatformInfo( platform , CL_PLATFORM_VENDOR , 0 , NULL , &size );
  {
    char * buffer = new char[ size ] ;
    clGetPlatformInfo( platform , CL_PLATFORM_VENDOR , size , buffer , NULL ) ;
    std::cerr << "CL_PLATFORM_VENDOR : " << buffer << std::endl ;
    delete[] buffer ;
  }

  clGetPlatformInfo( platform , CL_PLATFORM_EXTENSIONS , 0 , NULL , &size ) ;
  {
    char * buffer = new char[ size ] ;
    clGetPlatformInfo( platform , CL_PLATFORM_EXTENSIONS , size , buffer , NULL ) ;
    std::cerr << "CL_PLATFORM_EXTENSIONS : " << buffer << std::endl ;
    delete[] buffer ;
  }
}

void GetDeviceInfo( cl_device_id device )
{
  size_t size ;

  clGetDeviceInfo( device , CL_DEVICE_NAME , 0 , NULL , &size ) ;
  {
    char * buffer = new char[ size ] ;
    clGetDeviceInfo( device , CL_DEVICE_NAME , size , buffer , NULL ) ;
    std::cerr << "CL_DEVICE_NAME : " << buffer << std::endl ;
    delete[] buffer ;
  }

  clGetDeviceInfo( device , CL_DEVICE_VERSION , 0 , NULL , &size ) ;
  {
    char * buffer = new char[ size ] ;
    clGetDeviceInfo( device , CL_DEVICE_VERSION , size , buffer , NULL ) ;
    std::cerr << "CL_DEVICE_VERSION : " << buffer << std::endl ;
    delete[] buffer ;
  }

  clGetDeviceInfo( device , CL_DRIVER_VERSION , 0 , NULL , &size ) ;
  {
    char * buffer = new char[ size ] ;
    clGetDeviceInfo( device , CL_DRIVER_VERSION , size , buffer , NULL ) ;
    std::cerr << "CL_DRIVER_VERSION : " << buffer << std::endl ;
    delete[] buffer ;
  }

  clGetDeviceInfo( device , CL_DEVICE_OPENCL_C_VERSION , 0 , NULL , &size ) ;
  {
    char * buffer = new char[ size ] ;
    clGetDeviceInfo( device , CL_DEVICE_OPENCL_C_VERSION , size , buffer , NULL ) ;
    std::cerr << "CL_DEVICE_OPENCL_C_VERSION : " << buffer << std::endl ;
    delete[] buffer ;
  }

  cl_ulong ul_size ;
  clGetDeviceInfo( device , CL_DEVICE_GLOBAL_MEM_SIZE , sizeof( cl_ulong ) , &ul_size , NULL ) ;
  {
    std::cerr << "CL_DEVICE_GLOBAL_MEM_SIZE : " << ul_size << std::endl ;
  }

  cl_uint ui_size ;
  clGetDeviceInfo( device , CL_DEVICE_MAX_COMPUTE_UNITS , sizeof( cl_uint ) , &ui_size , NULL ) ;
  {
    std::cerr << "CL_DEVICE_MAX_COMPUTE_UNITS : " << ui_size << std::endl ;
  }

  clGetDeviceInfo( device , CL_DEVICE_MAX_SAMPLERS , sizeof( cl_uint ) , &ui_size , NULL ) ;
  {
    std::cerr << "CL_DEVICE_MAX_SAMPLERS : " << ui_size << std::endl ;
  }

  size_t st_size ;
  clGetDeviceInfo( device , CL_DEVICE_MAX_WORK_GROUP_SIZE , sizeof( st_size ) , &st_size , NULL ) ;
  {
    std::cerr << "CL_DEVICE_MAX_WORK_GROUP_SIZE" << st_size << std::endl ;
  }

  clGetDeviceInfo( device , CL_DEVICE_MAX_READ_IMAGE_ARGS , sizeof( cl_uint ) , &ui_size , NULL ) ;
  {
    std::cerr << "CL_DEVICE_MAX_READ_IMAGE_ARGS : " << ui_size << std::endl ;
  }



}

int GetMaxComputeUnitForDevice( cl_device_id device )
{
  cl_uint ui_size ;
  clGetDeviceInfo( device , CL_DEVICE_MAX_COMPUTE_UNITS , sizeof( cl_uint ) , &ui_size , NULL ) ;
  return ui_size ;
}

unsigned long GetGlobalMemoryForDevice( cl_device_id device )
{
  cl_ulong size ;
  clGetDeviceInfo( device , CL_DEVICE_GLOBAL_MEM_SIZE , sizeof( cl_ulong ) , &size , NULL ) ;
  return size ;
}

size_t GetMaxWorkGroupSize( cl_device_id device )
{
  size_t st_size ;
  clGetDeviceInfo( device , CL_DEVICE_MAX_WORK_GROUP_SIZE , sizeof( st_size ) , &st_size , NULL ) ;
  return st_size ;
}

int GetDeviceMaxImage2dWidth( cl_device_id device )
{
  size_t st_size ;
  clGetDeviceInfo( device , CL_DEVICE_IMAGE2D_MAX_WIDTH , sizeof( st_size ) , &st_size , NULL ) ;
  return st_size ;
}

int GetDeviceMaxImage2dHeight( cl_device_id device )
{
  size_t st_size ;
  clGetDeviceInfo( device , CL_DEVICE_IMAGE2D_MAX_HEIGHT , sizeof( st_size ) , &st_size , NULL ) ;
  return st_size ;
}



} // namespace MVS