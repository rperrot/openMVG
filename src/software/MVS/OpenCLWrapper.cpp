#include "OpenCLWrapper.hpp"

#include "OpenCLUtils.hpp"

#include <iostream>

namespace MVS
{
const int OpenCLWrapper::OPENCL_DEVICE_CPU = 0 ;
const int OpenCLWrapper::OPENCL_DEVICE_GPU = 1;

const int OpenCLWrapper::OPENCL_IMAGE_READ_ONLY = 0 ;
const int OpenCLWrapper::OPENCL_IMAGE_WRITE_ONLY = 1 ;

const int OpenCLWrapper::OPENCL_BUFFER_READ_ONLY = 0 ;
const int OpenCLWrapper::OPENCL_BUFFER_WRITE_ONLY = 1 ;
const int OpenCLWrapper::OPENCL_BUFFER_READ_WRITE = 2 ;


OpenCLWrapper::OpenCLWrapper( const int device_type  )
{
  if( ! prepare( device_type ) )
  {
    std::cerr << "Could not initialize with specified device" << std::endl ;
  }
}

OpenCLWrapper::~OpenCLWrapper( void )
{
  clReleaseCommandQueue( m_queue ) ;
  clReleaseContext( m_context ) ;
  clReleaseDevice( m_device ) ;
}


cl_platform_id OpenCLWrapper::getPlatform( void )
{
  return m_platform ;
}

cl_device_id OpenCLWrapper::getDevice( void )
{
  return m_device ;
}

cl_context OpenCLWrapper::getContext( void )
{
  return m_context ;
}

cl_command_queue OpenCLWrapper::getCommandQueue( void )
{
  return m_queue ;
}

bool OpenCLWrapper::prepare( const int device_type )
{
  cl_int res = clGetPlatformIDs( 1, &m_platform, NULL );
  if( res != CL_SUCCESS )
  {
    std::cerr << "Could not get OpenCL platform" << std::endl ;
    return false ;
  }

  cl_device_type dev = ( device_type == OPENCL_DEVICE_CPU ) ? CL_DEVICE_TYPE_CPU : CL_DEVICE_TYPE_GPU ;

  const int selected_device = getDeviceIDWithMaxGlobalMemory( device_type ) ;

  if( selected_device == 0 )
  {
    res = clGetDeviceIDs( m_platform , dev , 1 , &m_device , NULL ) ;
  }
  else
  {
    // Not the first id so we have multiple devices then load all ans select the good
    cl_uint nb_device ;
    res = clGetDeviceIDs( m_platform , dev , 0 , NULL , &nb_device ) ;
    cl_device_id * devices = new cl_device_id[ nb_device ] ;
    res = clGetDeviceIDs( m_platform , dev , nb_device , devices , NULL ) ;
    m_device = devices[ selected_device ] ;
    for( cl_uint id = 0 ; id < nb_device ; ++id )
    {
      if( id != selected_device )
      {
        clReleaseDevice( devices[ id ] ) ;
      }
    }
    delete[] devices ;
  }

  if( res != CL_SUCCESS )
  {
    std::cerr << "Could not get OpenCL device" << std::endl ;
    return false ;
  }

  cl_context_properties properties[3] ;
  properties[0] = CL_CONTEXT_PLATFORM ;
  properties[1] = ( cl_context_properties ) m_platform ;
  properties[2] = 0 ;

  m_context = clCreateContext( properties , 1 , &m_device , NULL , NULL , &res ) ;
  if( res != CL_SUCCESS )
  {
    std::cerr << "Could not create OpenCL context" << std::endl ;
    return false;
  }

  m_queue = clCreateCommandQueue( m_context , m_device , 0 , &res ) ;
  if( res != CL_SUCCESS )
  {
    std::cerr << "Could not create OpenCL command queue" << std::endl ;
    return false ;
  }

  return true ;
}

cl_program OpenCLWrapper::createProgramFromSource( const std::string & str )
{
  cl_program pgm ;
  cl_int res ;
  const char * c_str = str.c_str() ;
  pgm = clCreateProgramWithSource( m_context , 1 , &c_str , NULL , &res ) ;
  if( res != CL_SUCCESS )
  {
    std::cerr << "Could not create program" << std::endl ;
    return pgm ;
  }

  res = clBuildProgram( pgm , 1 , &m_device , NULL , NULL , NULL ) ;
  if( res != CL_SUCCESS )
  {
    std::cerr << "Could not build program" << std::endl ;

    size_t log_size ;
    clGetProgramBuildInfo( pgm , m_device , CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size );

    char * log = new char[ log_size ] ;

    // Get the log
    clGetProgramBuildInfo( pgm , m_device , CL_PROGRAM_BUILD_LOG, log_size, log, NULL );

    std::cerr << "Log is " << log << std::endl ;

    delete[] log ;

    return pgm ;
  }

  return pgm ;
}

cl_kernel OpenCLWrapper::getKernelFromName( cl_program pgm , const std::string & kernel_name )
{
  cl_int res ;
  cl_kernel krn = clCreateKernel( pgm , kernel_name.c_str() , &res ) ;
  if( res != CL_SUCCESS )
  {
    std::cerr << "Could not create kernel, for kernel : " << kernel_name << std::endl ;
    return krn ;
  }

  return krn;
}

cl_mem OpenCLWrapper::createImage( const openMVG::image::Image<unsigned char> & img , const int access_type )
{
  cl_mem res ;
  cl_int err ;

  cl_mem_flags flags = access_type == OPENCL_IMAGE_READ_ONLY ?
                       CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR :
                       CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR ;

  cl_image_format format ;
  format.image_channel_order = CL_R ;
  format.image_channel_data_type = CL_UNORM_INT8 ;

  cl_image_desc desc ;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = img.Width() ;
  desc.image_height = img.Height() ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = NULL ;

  res = clCreateImage( m_context , flags , &format , &desc , ( void* ) img.data() , &err ) ;

  if( err != CL_SUCCESS )
  {
    std::cerr << "Cannot create image2d from Image<unsigned char> data" << std::endl ;
  }

  return res ;
}

cl_mem OpenCLWrapper::createImage( const openMVG::image::Image<double> & img , const int access_type )
{
  cl_mem res ;
  cl_int err ;

  cl_mem_flags flags = access_type == OPENCL_IMAGE_READ_ONLY ?
                       CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR :
                       CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR ;

  cl_image_format format ;
  format.image_channel_order = CL_R ;
  format.image_channel_data_type = CL_FLOAT ;

  cl_image_desc desc ;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = img.Width() ;
  desc.image_height = img.Height() ;
  desc.image_depth = 0 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = NULL ;

  openMVG::image::Image<float> tmp ;
  tmp = img.GetMat().cast<float>() ;

  res = clCreateImage( m_context , flags , &format , &desc , ( void* ) tmp.data() , &err ) ;

  if( err != CL_SUCCESS )
  {
    std::cerr << "Cannot create image2d from Image<double> data" << std::endl ;
  }


  return res ;
}

cl_mem OpenCLWrapper::createImage( const openMVG::image::Image<openMVG::Vec4> & img , const int access_type )
{
  cl_mem res ;
  cl_int err ;

  cl_mem_flags flags = access_type == OPENCL_IMAGE_READ_ONLY ?
                       CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR :
                       CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR ;

  cl_image_format format ;
  format.image_channel_order = CL_RGBA ;
  format.image_channel_data_type = CL_FLOAT ;

  cl_image_desc desc ;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D ;
  desc.image_width = img.Width() ;
  desc.image_height = img.Height() ;
  desc.image_depth = 1 ;
  desc.image_row_pitch = 0 ;
  desc.image_slice_pitch = 0 ;
  desc.num_mip_levels = 0 ;
  desc.num_samples = 0 ;
  desc.buffer = NULL ;

  float * tmp_data = new float[ 4 * img.Width() * img.Height() ] ;
  for( int y = 0 ; y < img.Height() ; ++y )
  {
    for( int x = 0 ; x < img.Width() ; ++x )
    {
      const int index = y * img.Width() + x ;
      const openMVG::Vec4 & cur = img( y , x ) ;
      tmp_data[ 4 * index ] = cur[0] ;
      tmp_data[ 4 * index + 1 ] = cur[1] ;
      tmp_data[ 4 * index + 2 ] = cur[2] ;
      tmp_data[ 4 * index + 3 ] = cur[3] ;
    }
  }

  res = clCreateImage( m_context , flags , &format , &desc , tmp_data , &err ) ;

  if( err != CL_SUCCESS )
  {
    std::cerr << "Cannot create image2d from Image<double> data" << std::endl ;
  }

  delete[] tmp_data ;

  return res ;
}

cl_mem OpenCLWrapper::createBuffer( const size_t size , const int access_type , const cl_float * ptr )
{
  cl_int err ;

  cl_mem_flags flags ;

  if( ! ptr )
  {
    if( access_type == OPENCL_BUFFER_READ_ONLY )
    {
      std::cerr << "Warning passing NULL data for read only is a nonsense" << std::endl ;
      flags = CL_MEM_READ_ONLY ;
    }
    else if( access_type == OPENCL_BUFFER_WRITE_ONLY )
    {
      flags = CL_MEM_WRITE_ONLY ;
    }
    else if( access_type == OPENCL_BUFFER_READ_WRITE )
    {
      flags = CL_MEM_READ_WRITE ;
    }
  }
  else
  {
    if( access_type == OPENCL_BUFFER_READ_ONLY )
    {
      flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR ;
    }
    else if( access_type == OPENCL_BUFFER_WRITE_ONLY )
    {
      flags = CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR ;
    }
    else if( access_type == OPENCL_BUFFER_READ_WRITE )
    {
      flags = CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR ;
    }
  }

  cl_mem res = clCreateBuffer( m_context , flags , size , ( void* ) ptr , &err ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not create buffer with size : " << size << std::endl;
  }
  return res ;
}

cl_mem OpenCLWrapper::createBuffer( const openMVG::Mat3 & mat )
{
  cl_int err ;
  cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR ;

  const cl_float tmp[9] = { static_cast<cl_float>( mat( 0 , 0 ) ) ,
                            static_cast<cl_float>( mat( 0 , 1 ) ) ,
                            static_cast<cl_float>( mat( 0 , 2 ) ) ,

                            static_cast<cl_float>( mat( 1 , 0 ) ) ,
                            static_cast<cl_float>( mat( 1 , 1 ) ) ,
                            static_cast<cl_float>( mat( 1 , 2 ) ) ,

                            static_cast<cl_float>( mat( 2 , 0 ) ) ,
                            static_cast<cl_float>( mat( 2 , 1 ) ) ,
                            static_cast<cl_float>( mat( 2 , 2 ) )
                          } ;

  cl_mem res = clCreateBuffer( m_context , flags , 9 * sizeof( cl_float ) , ( void* ) tmp , &err ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not create buffer with Mat3" << std::endl;
  }
  return res ;
}

cl_mem OpenCLWrapper::createBuffer( const openMVG::Vec3 & vec )
{
  cl_int err ;
  cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR ;

  const cl_float tmp[3] = { static_cast<cl_float>( vec[0] ) ,
                            static_cast<cl_float>( vec[1] ) ,
                            static_cast<cl_float>( vec[2] )
                          } ;

  cl_mem res = clCreateBuffer( m_context , flags , 3 * sizeof( cl_float ) , ( void* ) tmp , &err ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not create buffer with Vec3" << std::endl;
  }
  return res ;
}

cl_mem OpenCLWrapper::createBuffer( const openMVG::Vec2i & vec )
{
  cl_int err ;
  cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR ;

  const cl_int tmp[2] = { static_cast<cl_int>( vec[0] ) ,
                          static_cast<cl_int>( vec[1] )
                        } ;

  cl_mem res = clCreateBuffer( m_context , flags , 2 * sizeof( cl_int ) , ( void* ) tmp , &err ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not create buffer with Vec2i" << std::endl;
  }
  return res ;
}

cl_mem OpenCLWrapper::createBuffer( const openMVG::Mat34 & mat )
{
  cl_int err ;
  cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR ;

  const cl_float tmp[12] = { static_cast<cl_float>( mat( 0, 0 ) ) ,
                             static_cast<cl_float>( mat( 0, 1 ) ) ,
                             static_cast<cl_float>( mat( 0, 2 ) ) ,
                             static_cast<cl_float>( mat( 0, 3 ) ) ,

                             static_cast<cl_float>( mat( 1, 0 ) ) ,
                             static_cast<cl_float>( mat( 1, 1 ) ) ,
                             static_cast<cl_float>( mat( 1, 2 ) ) ,
                             static_cast<cl_float>( mat( 1, 3 ) ) ,

                             static_cast<cl_float>( mat( 2, 0 ) ) ,
                             static_cast<cl_float>( mat( 2, 1 ) ) ,
                             static_cast<cl_float>( mat( 2, 2 ) ) ,
                             static_cast<cl_float>( mat( 2, 3 ) )
                           } ;

  cl_mem res = clCreateBuffer( m_context , flags , 12 * sizeof( cl_float ) , ( void* ) tmp , &err ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not create buffer with Vec3" << std::endl;
  }
  return res ;
}

cl_mem OpenCLWrapper::createBuffer( const openMVG::image::Image<openMVG::Vec4> & img )
{
  cl_int err ;
  cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR ;

  cl_float * tmp_data = new cl_float[ 4 * img.Width() * img.Height() ] ;
  for( int y = 0 ; y < img.Height() ; ++y )
  {
    for( int x = 0 ; x < img.Width() ; ++x )
    {
      const int index = y * img.Width() + x ;
      const openMVG::Vec4 & cur = img( y , x ) ;
      tmp_data[ 4 * index ]     = cur[0] ;
      tmp_data[ 4 * index + 1 ] = cur[1] ;
      tmp_data[ 4 * index + 2 ] = cur[2] ;
      tmp_data[ 4 * index + 3 ] = cur[3] ;
    }
  }

  cl_mem res = clCreateBuffer( m_context , flags , 4 * img.Width() * img.Height() * sizeof( cl_float ) , ( void* ) tmp_data , &err ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not create buffer with Image<Vec4>" << std::endl;
  }
  delete[] tmp_data ;

  return res ;
}

/**
 * @brief Create a buffer object (not an image buffer object) from an image
 */
cl_mem OpenCLWrapper::createBuffer( const openMVG::image::Image<unsigned long long> & img )
{
  cl_int err ;
  cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR ;

  cl_ulong * tmp_data = new cl_ulong[ img.Width() * img.Height() ] ;
  int index = 0 ;
  for( int y = 0 ; y < img.Height() ; ++y )
  {
    for( int x = 0 ; x < img.Width() ; ++x )
    {
      tmp_data[ index ] = img( y , x ) ;
      ++index ;
    }
  }

  cl_mem res = clCreateBuffer( m_context , flags , img.Width() * img.Height() * sizeof( cl_ulong ) , ( void* ) tmp_data , &err ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not create buffer with Image<Vec4>" << std::endl;
  }
  delete[] tmp_data ;

  return res ;
}



void OpenCLWrapper::clearMemory( cl_mem mem_obj )
{
  cl_int res = clReleaseMemObject( mem_obj ) ;
  if( res != CL_SUCCESS )
  {
    std::cerr << "Could not release object" << std::endl ;

    if( res == CL_INVALID_MEM_OBJECT )
    {
      std::cerr << "CL_INVALID_MEM_OBJECT" << std::endl ;
    }
    else if( res == CL_OUT_OF_RESOURCES )
    {
      std::cerr << "CL_OUT_OF_RESOURCES" << std::endl ;
    }
    else if( res == CL_OUT_OF_HOST_MEMORY )
    {
      std::cerr << "CL_OUT_OF_HOST_MEMORY" << std::endl ;
    }
  }
}

// Read from gpu to cpu
void OpenCLWrapper::readImage( cl_mem img_obj , openMVG::image::Image<float> & img )
{
  // Get image width/height
  const size_t w = imageWidth( img_obj ) ;
  const size_t h = imageHeight( img_obj ) ;
  img = openMVG::image::Image<float>( w , h ) ;

  cl_image_format format ;
  cl_int err = clGetImageInfo( img_obj , CL_IMAGE_FORMAT , sizeof( format ) , &format , NULL ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not get image format" << std::endl ;
  }
  if( format.image_channel_order != CL_R )
  {
    std::cerr << "Image has multiple components, could not read from it" ;
  }
  if( format.image_channel_data_type != CL_FLOAT )
  {
    std::cerr << "Image is not a float image" << std::endl ;
  }

  size_t origin[] = { 0 , 0 , 0 } ;
  size_t region[] = { w , h , 1 } ;
  cl_int res = clEnqueueReadImage( m_queue , img_obj , CL_TRUE , origin , region , 0 , 0  , img.data() , 0 , NULL , NULL ) ;
  if( res != CL_SUCCESS )
  {
    std::cerr << "Could not enqueueread image" << std::endl ;
  }
}

int OpenCLWrapper::imageWidth( cl_mem img_obj )
{
  size_t width ;
  cl_int res = clGetImageInfo( img_obj , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , NULL ) ;
  if( res != CL_SUCCESS )
  {
    std::cerr << "Could not read width of the image " << std::endl ;
  }
  return static_cast<int>( width ) ;
}

int OpenCLWrapper::imageHeight( cl_mem img_obj )
{
  size_t height ;
  cl_int res = clGetImageInfo( img_obj , CL_IMAGE_HEIGHT , sizeof( size_t ) , &height , NULL ) ;
  if( res != CL_SUCCESS )
  {
    std::cerr << "Could not read height of the image " << std::endl ;
  }
  return static_cast<int>( height ) ;
}

// Next multiple
int NextMultipleOf( int N , int K )
{
  return N + ( K - N % K ) % K ;
}

void OpenCLWrapper::runKernel2d( cl_kernel krn , const int working_size[2] )
{
  // Compute size (to be a multiple of)
  size_t multiple ;
  cl_int err = clGetKernelWorkGroupInfo( krn , m_device , CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE , sizeof( size_t ) , &multiple , NULL ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not read prefered working group size" << std::endl ;
  }

  // Make the working size a multiple of prefered size
  size_t effective_global_size[2] ;
  effective_global_size[0] = NextMultipleOf( working_size[0] , multiple ) ;
  effective_global_size[1] = NextMultipleOf( working_size[1] , multiple ) ;

  size_t max_work_group_size = GetMaxWorkGroupSize( m_device ) ;

  err = clEnqueueNDRangeKernel( m_queue , krn , 2 , NULL ,
                                effective_global_size , NULL /* Let the driver choose optimal local size */ ,
                                0 , NULL , NULL ) ;
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
  }

  // Wait for finish
  clFinish( m_queue ) ;
}

void OpenCLWrapper::copyDeviceToHost( cl_mem buffer , cl_float * outData )
{
  // Get size of the memory object
  size_t size ;
  cl_int err = clGetMemObjectInfo( buffer , CL_MEM_SIZE , sizeof( size_t ) , &size , NULL ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not read size of the memory buffer" << std::endl ;
  }

  err = clEnqueueReadBuffer( m_queue , buffer , CL_TRUE , 0 , size , outData , 0 , NULL , NULL ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not read data from device" << std::endl ;
  }
}

// Assuming buffer is already allocated and at the same size of the initial buffer
void OpenCLWrapper::copyHostToDevice( const cl_float * outData , cl_mem buffer )
{
  // Get size of the memory object
  size_t size ;
  cl_int err = clGetMemObjectInfo( buffer , CL_MEM_SIZE , sizeof( size_t ) , &size , NULL ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not read size of the memory buffer" << std::endl ;
  }

  err = clEnqueueWriteBuffer( m_queue , buffer , CL_TRUE , 0 , size , outData , 0 , NULL , NULL ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not write data from host to device" << std::endl ;
  }
}

int OpenCLWrapper::getDeviceIDWithMaxComputeUnit( const int device_type )
{
  cl_uint count ;

  cl_device_type dev_type = ( device_type == OPENCL_DEVICE_CPU ) ? CL_DEVICE_TYPE_CPU : CL_DEVICE_TYPE_GPU ;

  cl_int err = clGetDeviceIDs( m_platform , dev_type , 0 , NULL, &count ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not get the number of devices" << std::endl ;
  }

  int id = -1 ;
  int max = -1 ;
  cl_device_id * devices = new cl_device_id[ count ] ;
  clGetDeviceIDs( m_platform , dev_type , count , devices , NULL ) ;

  for( int i = 0 ; i < count ; ++i )
  {
    const int nb = GetMaxComputeUnitForDevice( devices[i] ) ;

    if( nb > max )
    {
      max = nb ;
      id = i ;
    }
  }

  for( int i = 0 ; i < count ; ++i )
  {
    clReleaseDevice( devices[ i ] ) ;
  }
  delete[] devices ;

  return id ;
}

int OpenCLWrapper::getDeviceIDWithMaxGlobalMemory( const int device_type )
{
  cl_uint count ;

  cl_device_type dev_type = ( device_type == OPENCL_DEVICE_CPU ) ? CL_DEVICE_TYPE_CPU : CL_DEVICE_TYPE_GPU ;

  cl_int err = clGetDeviceIDs( m_platform , dev_type , 0 , NULL, &count ) ;
  if( err != CL_SUCCESS )
  {
    std::cerr << "Could not get the number of devices" << std::endl ;
  }

  int id = -1 ;
  unsigned long max = 0 ;
  cl_device_id * devices = new cl_device_id[ count ] ;
  clGetDeviceIDs( m_platform , dev_type , count , devices , NULL ) ;

  for( int i = 0 ; i < count ; ++i )
  {
    const unsigned long nb = GetGlobalMemoryForDevice( devices[i] ) ;

    if( nb > max )
    {
      max = nb ;
      id = i ;
    }
  }

  for( int i = 0 ; i < count ; ++i )
  {
    clReleaseDevice( devices[ i ] ) ;
  }
  delete[] devices ;

  return id ;

}

int OpenCLWrapper::getNbDevices( const int device_type )
{
  cl_uint count ;
  cl_device_type dev_type = ( device_type == OPENCL_DEVICE_CPU ) ? CL_DEVICE_TYPE_CPU : CL_DEVICE_TYPE_GPU ;
  cl_int err = clGetDeviceIDs( m_platform , dev_type , 0 , NULL, &count ) ;
  return count ;
}



} // namespace MVS