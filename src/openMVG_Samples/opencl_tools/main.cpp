#include "openMVG/system/gpu/OpenCLContext.hpp"

#include <iostream>

using namespace openMVG::system::gpu;

int main( void )
{
  OpenCLContext ctx ;

  std::cout << "Nb OpenCL Platforms : " << ctx.nbPlatform() << std::endl ;
  for( uint32_t id_plat = 0 ; id_plat < ctx.nbPlatform() ; ++id_plat )
  {
    std::cout << "Platform " << id_plat << std::endl ;
    std::cout << " * Name        : " << ctx.platformName( id_plat ) << std::endl ;
    std::cout << " * Vendor Name : " << ctx.platformVendor( id_plat ) << std::endl ;
    if( ctx.platformProfile( id_plat ) == OPENCL_PLATFORM_PROFILE_FULL_PROFILE )
    {
      std::cout << " * Profile     : FULL" << std::endl;
    }
    else if( ctx.platformProfile( id_plat ) == OPENCL_PLATFORM_PROFILE_EMBEDDED_PROFILE )
    {
      std::cout << " * Profile     : EMBEDDED" << std::endl ;
    }
    else
    {
      std::cout << " * Profile     : INVALID" << std::endl ;
    }
    std::vector< std::string > platExt = ctx.platformExtensions( id_plat ) ;
    std::cout << " * Extensions  :" << std::endl ;
    for( const auto & it_ext : platExt )
    {
      std::cout << "   -> " << it_ext << std::endl;
    }

    std::cout << "Nb device on platform : " << ctx.nbDeviceForPlatform( id_plat ) << std::endl ;
    for( uint32_t id_device = 0 ; id_device < ctx.nbDeviceForPlatform( id_plat ) ; ++id_device )
    {
      std::cout << " * Device " << id_device << " : " << std::endl ;
      std::cout << "   - Name               : " << ctx.deviceName( id_plat , id_device ) << std::endl ;
      std::cout << "   - Vendor Name        : " << ctx.deviceVendor( id_plat , id_device ) << std::endl ;
      if( ctx.deviceType( id_plat , id_device ) == OPENCL_DEVICE_TYPE_CPU )
      {
        std::cout << "   - Type               : CPU" << std::endl ;
      }
      else if( ctx.deviceType( id_plat , id_device ) == OPENCL_DEVICE_TYPE_GPU )
      {
        std::cout << "   - Type               : GPU" << std::endl ;
      }
      else if( ctx.deviceType( id_plat , id_device ) == OPENCL_DEVICE_TYPE_OTHER )
      {
        std::cout << "   - Type               : Other" << std::endl ;
      }
      else
      {
        std::cout << "   - Type               : INVALID" << std::endl ;
      }

      std::cout << "   - Global Memory      : " << ctx.deviceGlobalMemorySize( id_plat , id_device ) << std::endl;
      std::cout << "   - Compute Units      : " << ctx.deviceMaxComputeUnits( id_plat , id_device ) << std::endl ;
      std::cout << "   - Max Clock Freq     : " << ctx.deviceMaxClockFrequency( id_plat , id_device ) << std::endl ;
      if( ctx.deviceSupportImage2D( id_plat , id_device ) )
      {
        std::cout << "   - Support Image 2D   : Yes" << std::endl ;
        std::cout << "   - Image2D Max Width  : " << ctx.deviceMaxImage2DWidth( id_plat , id_device ) << std::endl ;
        std::cout << "   - Image2D Max Height : " << ctx.deviceMaxImage2DHeight( id_plat , id_device ) << std::endl ;
      }
      else
      {
        std::cout << "   - Support Image 2D : No" << std::endl ;
      }
      if( ctx.deviceSupportImage3D( id_plat , id_device ) )
      {
        std::cout << "   - Support Image 3D   : Yes" << std::endl ;
        std::cout << "   - Image3D Max Width  : " << ctx.deviceMaxImage3DWidth( id_plat , id_device ) << std::endl ;
        std::cout << "   - Image3D Max Height : " << ctx.deviceMaxImage3DHeight( id_plat , id_device ) << std::endl ;
        std::cout << "   - Image3D Max Depth  : " << ctx.deviceMaxImage3DDepth( id_plat , id_device ) << std::endl ;
      }
    }
  }

  if( ctx.valid() )
  {
    std::cout << "Current platform/Device valid" << std::endl ;

    std::cout << "Current context : " << std::endl ;
    std::cout << " * Platform Name        : " << ctx.currentPlatformName() << std::endl ;
    std::cout << " * Platform Vendor Name : " << ctx.currentPlatformVendor() << std::endl ;
    std::cout << " * Device Name          : " << ctx.currentDeviceName() << std::endl ;
    std::cout << " * Device Vendor Name   : " << ctx.currentDeviceVendor() << std::endl ;
  }
  else
  {
    std::cout << "Current platform/Device invalid" << std::endl ;
  }

  return EXIT_SUCCESS ;
}