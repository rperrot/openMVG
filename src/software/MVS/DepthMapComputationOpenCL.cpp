#include "DepthMapComputationOpenCL.hpp"

#include "DepthMapComputationCommon.hpp"
#include "Util.hpp"


#include <random>

namespace MVS
{

/**
* @brief Compute Multiple view cost of an image using OpenCL
* @param outCost Output array buffer
* @param reference_cam Camera of the reference view
* @param neigh_imgs Neighboring image
* @param cams Neighboring cameras
* @param stereo_rig Array of motion between reference and it's neighbors
* @param params Computation parameters
* @param Ip OpenCL grayscale/color image of the reference view
* @param Gp OpenCL gradient buffer of the reference view
* @param Kinv OpenCL buffer of the inverse of the intrinsic matrix
* @param planes_n OpenCL buffer of the planes normal
* @param planes_d OpenCL buffer of the planes parameters
* @param wrapper OpenCL helper object
* @param cost_kernel_full OpenCL kernel used to compute the full matching cost
* @param cost_kernel_red OpenCL kernel used to compute partial matching cost (using red scheme)
* @param cost_kernel_black OpenCL kernel used to compute partial matching cost (using black scheme)
* @param append_cost_kernel OpenCL kernel used to append matching cost to all costs array
* @param sort_and_store_cost_kernel OpenCL kernel used to sort costs and compute final cost value
* @param compute_size Size of the computation
* @param delta_plane Plane displacement between pixel position and used plane
* @param c_type Topology of the cost computation (full, red or black)
* @param _all_costs (optional) OpenCL buffer used to store intermediate cost (nb_neighbor*sizeof(image))
* @return Current cost value
* @note outCost should be allocated at correct size before calling this function
*/
cl_mem ComputeCost( cl_mem & outCost ,
                    const Camera & reference_cam ,
                    const std::vector< Image > & neigh_imgs ,
                    const std::vector< Camera > & cams ,
                    const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                    const DepthMapComputationParameters & params ,
                    const int scale ,
                    cl_mem & Ip , cl_mem & Gp , cl_mem & CensusP , cl_mem & Kinv , cl_mem & planes_n , cl_mem & planes_d ,
                    OpenCLWrapper & wrapper ,
                    cl_kernel & cost_kernel_full ,
                    cl_kernel & cost_kernel_red ,
                    cl_kernel & cost_kernel_black ,
                    cl_kernel & append_cost_kernel ,
                    cl_kernel & sort_and_store_cost_kernel ,
                    const std::pair< int , int > & compute_size ,
                    const openMVG::Vec2i & delta_plane ,
                    const cost_topology_type c_type ,
                    cl_mem * _all_costs  )
{
  // Get computation constants
  const cl_int nb_total_neighbors = reference_cam.m_view_neighbors.size() ;
  const cl_int in_width = compute_size.first ;
  const cl_int in_height = compute_size.second ;
  const cl_int nb_pixel = in_width * in_height ;
  const int size[] = { in_width , in_height } ;


  // The costs -> All costs
  cl_mem all_costs ;
  if( ! _all_costs )
  {
    all_costs = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) * reference_cam.m_view_neighbors.size() ,
                                      OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;
  }
  else
  {
    all_costs = *_all_costs ;
  }

  // Select appropriate kernel
  cl_kernel cost_kernel ;
  cl_mem delta_plane_idx ;
  bool has_delta_plane = false ;
  bool has_gradient = false ;
  bool has_census = false ;
  switch( c_type )
  {
    case COST_COMPUTE_RED :
    {
      cost_kernel = cost_kernel_red ;
      has_delta_plane = true ;
      delta_plane_idx = wrapper.createBuffer( delta_plane ) ;
      break ;
    }
    case COST_COMPUTE_BLACK :
    {
      cost_kernel = cost_kernel_black ;
      has_delta_plane = true ;
      delta_plane_idx = wrapper.createBuffer( delta_plane ) ;
      break ;
    }
    case COST_COMPUTE_FULL :
    {
      cost_kernel = cost_kernel_full ;
      has_delta_plane = false ;
      break ;
    }
  }

  cl_float max_cost = DepthMapComputationParameters::metricMaxCostValue( params.metric() ) ;
  switch( params.metric() )
  {
    case COST_METRIC_NCC :
    {
      has_gradient = false ;
      has_census = false ;
      break ;
    }
    case COST_METRIC_PM :
    {
      has_gradient = true ;
      has_census = false ;
      break ;
    }
    case COST_METRIC_CENSUS :
    {
      has_gradient = false ;
      has_census = true ;
    }
    case COST_METRIC_DAISY:
    {
      has_gradient = false ; 
      has_census = false ; 
    }
  }


  // Compute cost for each neighbor
  for( size_t id_cam = 0 ; id_cam < reference_cam.m_view_neighbors.size() ; ++id_cam )
  {
    // Get info of the other image
    const int id_neigh = reference_cam.m_view_neighbors[ id_cam ] ;
    const Camera & cur_cam = cams[ id_neigh ] ;
    const std::pair< openMVG::Mat3 , openMVG::Vec3 > & cur_rig = stereo_rig[ id_cam ] ;
    const Image & other_img = neigh_imgs[ id_cam ] ;

    cl_mem Iq = wrapper.createImage( other_img.intensity() , OpenCLWrapper::OPENCL_IMAGE_READ_ONLY ) ;
    cl_mem Gq ;
    cl_mem CensusQ ;
    if( has_gradient )
    {
      Gq = wrapper.createBuffer( other_img.gradient() ) ;
    }
    if( has_census )
    {
      CensusQ = wrapper.createBuffer( other_img.census() ) ;
    }

    cl_mem Kother = wrapper.createBuffer( ( scale == -1 ) ? cur_cam.m_K : cur_cam.m_K_scaled[ scale ] ) ;
    cl_mem R      = wrapper.createBuffer( cur_rig.first ) ;
    cl_mem t      = wrapper.createBuffer( cur_rig.second ) ;


    int pad = 0 ;

    // Pass the arguments to the kernel
    if( has_delta_plane )
    {
      clSetKernelArg( cost_kernel , 0 , sizeof( cl_mem ) , & delta_plane_idx ) ;
      pad += 1 ;
    }

    clSetKernelArg( cost_kernel , 0 + pad , sizeof( cl_mem ) , & Ip ) ;
    clSetKernelArg( cost_kernel , 1 + pad , sizeof( cl_mem ) , & Iq ) ;

    if( has_gradient )
    {
      clSetKernelArg( cost_kernel , 2 + pad , sizeof( cl_mem ) , & Gp ) ;
      clSetKernelArg( cost_kernel , 3 + pad , sizeof( cl_mem ) , & Gq ) ;
      pad += 2 ;
    }
    if( has_census )
    {
      clSetKernelArg( cost_kernel , 2 + pad , sizeof( cl_mem ) , & CensusP ) ;
      clSetKernelArg( cost_kernel , 3 + pad , sizeof( cl_mem ) , & CensusQ ) ;
      pad += 2 ;
    }

    clSetKernelArg( cost_kernel , 2 + pad , sizeof( cl_mem ) , & planes_n ) ;
    clSetKernelArg( cost_kernel , 3 + pad , sizeof( cl_mem ) , & planes_d ) ;

    clSetKernelArg( cost_kernel , 4 + pad , sizeof( cl_mem ) , & R ) ;
    clSetKernelArg( cost_kernel , 5 + pad , sizeof( cl_mem ) , & t ) ;
    clSetKernelArg( cost_kernel , 6 + pad , sizeof( cl_mem ) , & Kinv ) ;
    clSetKernelArg( cost_kernel , 7 + pad , sizeof( cl_mem ) , & Kother ) ;

    clSetKernelArg( cost_kernel , 8 + pad , sizeof( cl_mem ) , & outCost ) ;


    // Run the kernel
    wrapper.runKernel2d( cost_kernel , size ) ;

    // 2 - accumulate costs
    clSetKernelArg( append_cost_kernel , 0 , sizeof( cl_mem ) , &all_costs ) ;
    clSetKernelArg( append_cost_kernel , 1 , sizeof( cl_mem ) , &outCost ) ;
    const cl_int id = id_cam ;
    clSetKernelArg( append_cost_kernel , 2 , sizeof( cl_int ) , &id ) ;
    clSetKernelArg( append_cost_kernel , 3 , sizeof( cl_int ) , &nb_total_neighbors ) ;
    clSetKernelArg( append_cost_kernel , 4 , sizeof( cl_int ) , &in_width ) ;
    clSetKernelArg( append_cost_kernel , 5 , sizeof( cl_int ) , &in_height ) ;
    clSetKernelArg( append_cost_kernel , 6 , sizeof( cl_float ) , &max_cost ) ;

    wrapper.runKernel2d( append_cost_kernel , size ) ;

    // Clear memory of current view
    wrapper.clearMemory( Iq ) ;
    if( has_gradient )
    {
      wrapper.clearMemory( Gq ) ;
    }
    if( has_census )
    {
      wrapper.clearMemory( CensusQ ) ;
    }
    wrapper.clearMemory( Kother ) ;
    wrapper.clearMemory( R ) ;
    wrapper.clearMemory( t ) ;
  }

  // Sort elements and compute final cost
  const cl_int k = params.nbMultiViewImageForCost() ;
  clSetKernelArg( sort_and_store_cost_kernel , 0 , sizeof( cl_mem ) , &outCost ) ;
  clSetKernelArg( sort_and_store_cost_kernel , 1 , sizeof( cl_mem ) , &all_costs ) ;
  clSetKernelArg( sort_and_store_cost_kernel , 2 , sizeof( cl_int ) , &nb_total_neighbors ) ;
  clSetKernelArg( sort_and_store_cost_kernel , 3 , sizeof( cl_int ) , &k ) ;
  clSetKernelArg( sort_and_store_cost_kernel , 4 , sizeof( cl_int ) , &in_width ) ;
  clSetKernelArg( sort_and_store_cost_kernel , 5 , sizeof( cl_int ) , &in_height ) ;
  clSetKernelArg( sort_and_store_cost_kernel , 6 , sizeof( cl_float ) , &max_cost ) ;
  wrapper.runKernel2d( sort_and_store_cost_kernel , size ) ;

  // Clear temporary memory buffer
  if( !_all_costs )
  {
    wrapper.clearMemory( all_costs ) ;
  }
  if( has_delta_plane )
  {
    wrapper.clearMemory( delta_plane_idx ) ;
  }

  return outCost ;
}

/**
* @brief Compute cost of full depth map using OpenCL
* @param map Depth map
* @param reference_cam Reference camera
* @param cams Neighboring cameras
* @param stereo_rig Array of motion between reference and neighboring images
* @param image_ref Reference image data
* @param params Computation parameters
* @param wrapper OpenCL helper object
* @param cost_kernel Kernel used to compute cost
* @param append_cost_kernel Kernel used to append current cost to the all cost array
* @param sort_and_store_cost_kernel Kernel used to sort costs and compute final value
*/
void ComputeCost( DepthMap & map ,
                  const Camera & reference_cam ,
                  const std::vector< Camera > & cams ,
                  const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                  const Image & image_ref ,
                  const DepthMapComputationParameters & params ,
                  OpenCLWrapper & wrapper ,
                  cl_kernel cost_kernel ,
                  cl_kernel append_cost_kernel ,
                  cl_kernel sort_and_store_cost_kernel )
{
  // Load images for the neighbors
  const cl_int in_width = image_ref.width() ;
  const cl_int in_height = image_ref.height() ;
  const cl_int nb_pixel = in_width * in_height ;
  const std::pair<int, int> compute_size = std::make_pair( in_width , in_height ) ;

  // Load neighbor images
  const ImageLoadType load_type = ComputeLoadType( params.metric() ) ;
  const std::vector< Image > neigh_imgs = LoadNeighborImages( reference_cam , params , load_type ) ;

  // Get the initial images
  cl_mem Ip = wrapper.createImage( image_ref.intensity() , OpenCLWrapper::OPENCL_IMAGE_READ_ONLY ) ;
  cl_mem Gp ;
  cl_mem CensusP ;
  if( params.metric() == COST_METRIC_PM )
  {
    Gp = wrapper.createBuffer( image_ref.gradient() ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    CensusP = wrapper.createBuffer( image_ref.census() ) ;
  }
  cl_mem Kinv = wrapper.createBuffer( openMVG::Mat3( reference_cam.m_K.inverse() ) ) ;

  // Split plane parameters into two buffers
  cl_float * tmp_planes_n = new cl_float[ nb_pixel * 3 ] ;
  cl_float * tmp_planes_d = new cl_float[ nb_pixel ] ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      const int idx = y * map.width() + x ;
      const openMVG::Vec4 & cur = map.plane( y , x ) ;
      tmp_planes_n[ 3 * idx ]     = static_cast<cl_float>( cur[0] ) ;
      tmp_planes_n[ 3 * idx + 1 ] = static_cast<cl_float>( cur[1] ) ;
      tmp_planes_n[ 3 * idx + 2 ] = static_cast<cl_float>( cur[2] ) ;

      tmp_planes_d[ idx ]         = static_cast<cl_float>( cur[3] ) ;
    }
  }

  cl_mem planes_n = wrapper.createBuffer( nb_pixel * 3 * sizeof( cl_float ) ,
                                          OpenCLWrapper::OPENCL_BUFFER_READ_ONLY , tmp_planes_n ) ;
  cl_mem planes_d = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) ,
                                          OpenCLWrapper::OPENCL_BUFFER_READ_ONLY , tmp_planes_d ) ;
  delete[] tmp_planes_n ;
  delete[] tmp_planes_d ;

  // Now compute cost
  cl_mem outCost = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;
  ComputeCost( outCost , reference_cam , neigh_imgs , cams , stereo_rig , params , -1 ,
               Ip , Gp , CensusP , Kinv , planes_n , planes_d , wrapper ,
               cost_kernel , cost_kernel , cost_kernel ,
               append_cost_kernel , sort_and_store_cost_kernel ,
               compute_size ) ;

  wrapper.clearMemory( Ip ) ;
  if( params.metric() == COST_METRIC_PM )
  {
    wrapper.clearMemory( Gp ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    wrapper.clearMemory( CensusP ) ;
  }
  wrapper.clearMemory( planes_d ) ;
  wrapper.clearMemory( planes_n ) ;
  wrapper.clearMemory( Kinv ) ;

  // Copy back cost
  cl_float * cost = new cl_float[ in_width * in_height ] ;
  wrapper.copyDeviceToHost( outCost , cost ) ;

  wrapper.clearMemory( outCost ) ;

  // Store back cost to depth map
  int index = 0 ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      map.cost( y , x , cost[ index ] ) ;
      ++index ;
    }
  }
  // Clear all memory
  delete[] cost ;
}

/**
* @brief Compute cost of full depth map using OpenCL at specified scale
* @param map Depth map
* @param reference_cam Reference camera
* @param cams Neighboring cameras
* @param stereo_rig Array of motion between reference and neighboring images
* @param image_ref Reference image data
* @param params Computation parameters
* @param wrapper OpenCL helper object
* @param cost_kernel Kernel used to compute cost
* @param append_cost_kernel Kernel used to append current cost to the all cost array
* @param sort_and_store_cost_kernel Kernel used to sort costs and compute final value
*/
void ComputeCost( DepthMap & map ,
                  const Camera & reference_cam ,
                  const std::vector< Camera > & cams ,
                  const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                  const Image & image_ref ,
                  const std::vector< Image > & neigh_imgs ,
                  const DepthMapComputationParameters & params ,
                  const int scale ,
                  OpenCLWrapper & wrapper ,
                  cl_kernel cost_kernel ,
                  cl_kernel append_cost_kernel ,
                  cl_kernel sort_and_store_cost_kernel )
{
  // Load images for the neighbors
  const cl_int in_width = image_ref.width() ;
  const cl_int in_height = image_ref.height() ;
  const cl_int nb_pixel = in_width * in_height ;
  const std::pair<int, int> compute_size = std::make_pair( in_width , in_height ) ;

  // Get the initial images
  cl_mem Ip = wrapper.createImage( image_ref.intensity() , OpenCLWrapper::OPENCL_IMAGE_READ_ONLY ) ;
  cl_mem Gp ;
  cl_mem CensusP ;
  if( params.metric() == COST_METRIC_PM )
  {
    Gp = wrapper.createBuffer( image_ref.gradient() ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    CensusP = wrapper.createBuffer( image_ref.census() ) ;
  }
  cl_mem Kinv = wrapper.createBuffer( openMVG::Mat3( reference_cam.m_K_inv_scaled[ scale ] ) ) ;

  // Split plane parameters into two buffers
  cl_float * tmp_planes_n = new cl_float[ nb_pixel * 3 ] ;
  cl_float * tmp_planes_d = new cl_float[ nb_pixel ] ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      const int idx = y * map.width() + x ;
      const openMVG::Vec4 & cur = map.plane( y , x ) ;
      tmp_planes_n[ 3 * idx ]     = static_cast<cl_float>( cur[0] ) ;
      tmp_planes_n[ 3 * idx + 1 ] = static_cast<cl_float>( cur[1] ) ;
      tmp_planes_n[ 3 * idx + 2 ] = static_cast<cl_float>( cur[2] ) ;

      tmp_planes_d[ idx ]         = static_cast<cl_float>( cur[3] ) ;
    }
  }

  cl_mem planes_n = wrapper.createBuffer( nb_pixel * 3 * sizeof( cl_float ) ,
                                          OpenCLWrapper::OPENCL_BUFFER_READ_ONLY , tmp_planes_n ) ;
  cl_mem planes_d = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) ,
                                          OpenCLWrapper::OPENCL_BUFFER_READ_ONLY , tmp_planes_d ) ;
  delete[] tmp_planes_n ;
  delete[] tmp_planes_d ;

  // Now compute cost
  cl_mem outCost = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;
  ComputeCost( outCost , reference_cam , neigh_imgs , cams , stereo_rig , params , scale ,
               Ip , Gp , CensusP , Kinv , planes_n , planes_d , wrapper ,
               cost_kernel , cost_kernel , cost_kernel ,
               append_cost_kernel , sort_and_store_cost_kernel ,
               compute_size ) ;

  wrapper.clearMemory( Ip ) ;
  if( params.metric() == COST_METRIC_PM )
  {
    wrapper.clearMemory( Gp ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    wrapper.clearMemory( CensusP ) ;
  }
  wrapper.clearMemory( planes_d ) ;
  wrapper.clearMemory( planes_n ) ;
  wrapper.clearMemory( Kinv ) ;

  // Copy back cost
  cl_float * cost = new cl_float[ in_width * in_height ] ;
  wrapper.copyDeviceToHost( outCost , cost ) ;

  wrapper.clearMemory( outCost ) ;

  // Store back cost to depth map
  int index = 0 ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      map.cost( y , x , cost[ index ] ) ;
      ++index ;
    }
  }
  // Clear all memory
  delete[] cost ;
}





/**
* @brief Perform propagation using OpenCL
*/
void Propagate( DepthMap & map , const int id_start ,
                Camera & reference_cam ,
                const std::vector< Camera > & cams ,
                const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                const Image & image_ref ,
                const DepthMapComputationParameters & params ,
                OpenCLWrapper & wrapper ,
                cl_kernel & kernel_red , // Compute cost
                cl_kernel & kernel_black , // Compute cost
                cl_kernel & append_cost_kernel , // Append cost
                cl_kernel & sort_and_store_cost_kernel , // Sort and compute real cost
                cl_kernel & update_plane_wrt_cost_kernel , // Update planes based on cost
                cl_kernel & compute_pixel_depth_kernel ) // Compute pixel depth value
{
  const bool do_red = id_start == 0 ;
  const cl_int nb_total_neighbors = reference_cam.m_view_neighbors.size() ;
  const cl_int in_width  = map.width() ;
  const cl_int in_height = map.height() ;
  const cl_int nb_pixel = in_width * in_height ;
  const int size[] = { in_width , in_height } ;
  const std::pair< int , int > compute_size = std::make_pair( in_width , in_height ) ;

  // Load images for the neighbors
  const ImageLoadType load_type = ComputeLoadType( params.metric() );
  const std::vector< Image > neigh_imgs = LoadNeighborImages( reference_cam , params , load_type ) ;

  // (x,y)
  /*
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   | X |   | X |   |   |   |   |
   *   |   |   |   | X |   | X |   | X |   |   |   |
   *   | X |   | X |   | X | O | X |   | X |   | X |
   *   |   |   |   | X |   | X |   | X |   |   |   |
   *   |   |   |   |   | X |   | X |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   */
  const int neighs_idx[20][2] =
  {
    {0, -5} ,

    {0, -3} ,

    { -1, -2} ,
    {1, -2} ,

    { -2, -1} ,
    {0, -1} ,
    {2, -1} ,

    { -5, 0} ,
    { -3, 0} ,
    { -1, 0} ,
    {1, 0} ,
    {3, 0} ,
    {5, 0} ,

    { -2, 1} ,
    {0, 1} ,
    {2, 1} ,

    { -1, 2} ,
    {1, 2} ,

    {0, 3} ,

    {0, 5}
  } ;

  // Get the initial images
  cl_mem Ip = wrapper.createImage( image_ref.intensity() , OpenCLWrapper::OPENCL_IMAGE_READ_ONLY ) ;
  cl_mem Gp ;
  cl_mem CensusP ;
  if( params.metric() == COST_METRIC_PM )
  {
    Gp = wrapper.createBuffer( image_ref.gradient() ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    CensusP = wrapper.createBuffer( image_ref.census() ) ;
  }
  cl_mem Kinv = wrapper.createBuffer( openMVG::Mat3( reference_cam.m_K.inverse() ) ) ;

  // Copy current cost to the best
  cl_float * cur_cost_f = new cl_float[ nb_pixel ] ;
  cl_float * cur_plane_n = new cl_float[ 3 * nb_pixel ] ;
  cl_float * cur_plane_d = new cl_float[ nb_pixel ] ;

  size_t index = 0 ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      cur_cost_f[ index ] = map.cost( y , x ) ;

      const openMVG::Vec4 & cur_plane = map.plane( y , x ) ;

      cur_plane_n[ 3 * index ]     = cur_plane[ 0 ] ;
      cur_plane_n[ 3 * index + 1 ] = cur_plane[ 1 ] ;
      cur_plane_n[ 3 * index + 2 ] = cur_plane[ 2 ] ;

      cur_plane_d[ index ] = cur_plane[ 3 ] ;
      ++index ;
    }
  }

  cl_mem best_cost = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_cost_f ) ;
  cl_mem cur_cost = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_cost_f ) ;
  cl_mem best_plane_n = wrapper.createBuffer( 3 * nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_plane_n ) ;
  cl_mem best_plane_d = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_plane_d ) ;

  // The costs -> All costs
  cl_mem all_costs = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) * reference_cam.m_view_neighbors.size() ,
                     OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;

  delete[] cur_cost_f ;
  delete[] cur_plane_n ;
  delete[] cur_plane_d ;

  //  cl_kernel cost_kernel_ncc = (do_red) ? kernel_ncc_red : kernel_ncc_black ;
  cost_topology_type c_type = ( do_red ) ? COST_COMPUTE_RED : COST_COMPUTE_BLACK ;

  for( int id_plane = 0 ; id_plane < 20 ; ++id_plane )
  {
    // Compute and update cost using delta position
    const openMVG::Vec2i delta_plane( neighs_idx[ id_plane ][0] , neighs_idx[ id_plane][ 1 ]  ) ;
    cl_mem delta_plane_idx = wrapper.createBuffer( delta_plane ) ;

    ComputeCost( cur_cost , reference_cam , neigh_imgs , cams , stereo_rig , params , -1 ,
                 Ip , Gp , CensusP , Kinv , best_plane_n , best_plane_d , wrapper ,
                 kernel_red /* unused */ , kernel_red , kernel_black ,
                 append_cost_kernel , sort_and_store_cost_kernel ,
                 compute_size , delta_plane , c_type , &all_costs ) ;

    // Update best cost and planes
    clSetKernelArg( update_plane_wrt_cost_kernel , 0 , sizeof( cl_mem ) , &best_plane_n ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 1 , sizeof( cl_mem ) , &best_plane_d ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 2 , sizeof( cl_mem ) , &delta_plane_idx ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 3 , sizeof( cl_mem ) , &best_cost ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 4 , sizeof( cl_mem ) , &cur_cost ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 5 , sizeof( cl_int ) , &in_width ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 6 , sizeof( cl_int ) , &in_height ) ;
    wrapper.runKernel2d( update_plane_wrt_cost_kernel , size ) ;

    // Clear local memory
    wrapper.clearMemory( delta_plane_idx ) ;
  }
  wrapper.clearMemory( cur_cost ) ;
  wrapper.clearMemory( all_costs ) ;
  wrapper.clearMemory( Ip ) ;
  if( params.metric() == COST_METRIC_PM )
  {
    wrapper.clearMemory( Gp ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    wrapper.clearMemory( CensusP ) ;
  }


  // Compute depth kernel
  cl_mem K = wrapper.createBuffer( reference_cam.m_K ) ;
  cl_mem best_depth = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;

  clSetKernelArg( compute_pixel_depth_kernel , 0 , sizeof( cl_mem ) , &best_plane_n ) ;
  clSetKernelArg( compute_pixel_depth_kernel , 1 , sizeof( cl_mem ) , &best_plane_d ) ;
  clSetKernelArg( compute_pixel_depth_kernel , 2 , sizeof( cl_mem ) , &K ) ;
  clSetKernelArg( compute_pixel_depth_kernel , 3 , sizeof( cl_int ) , &in_width ) ;
  clSetKernelArg( compute_pixel_depth_kernel , 4 , sizeof( cl_int ) , &in_height ) ;
  clSetKernelArg( compute_pixel_depth_kernel , 5 , sizeof( cl_mem ) , &best_depth ) ;
  wrapper.runKernel2d( compute_pixel_depth_kernel , size ) ;

  wrapper.clearMemory( K ) ;

  float * b_cost = new float[ nb_pixel ] ;
  float * b_plane_n = new float[ nb_pixel * 3 ] ;
  float * b_plane_d = new float[ nb_pixel ] ;
  float * b_depth   = new float[ nb_pixel ] ;

  wrapper.copyDeviceToHost( best_cost , b_cost ) ;
  wrapper.copyDeviceToHost( best_plane_n , b_plane_n ) ;
  wrapper.copyDeviceToHost( best_plane_d , b_plane_d ) ;
  wrapper.copyDeviceToHost( best_depth , b_depth ) ;

  wrapper.clearMemory( best_cost ) ;
  wrapper.clearMemory( best_plane_n ) ;
  wrapper.clearMemory( best_plane_d ) ;
  wrapper.clearMemory( best_depth ) ;

  // Store back cost and plane index
  index = 0 ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      const openMVG::Vec4 plane( b_plane_n[ 3 * index ] , b_plane_n[ 3 * index + 1 ] , b_plane_n[ 3 * index + 2 ] , b_plane_d[ index ] ) ;

      if( ( do_red && ( ( ( y % 2 == 0 ) && ( x % 2 == 0 ) ) || ( ( y % 2 == 1 ) && ( x % 2 == 1 ) ) ) ) ||
          ( !do_red && ( ( ( y % 2 == 0 ) && ( x % 2 == 1 ) ) || ( ( y % 2 == 1 ) && ( x % 2 == 0 ) ) ) ) )
      {
        map.cost( y , x , b_cost[ index ] ) ;
        map.plane( y , x , plane ) ;
        map.depth( y , x , Clamp( static_cast<double>( b_depth[ index ] ) , reference_cam.m_min_depth * 0.8 , reference_cam.m_max_depth * 1.2 ) ) ;
      }

      ++index ;
    }
  }

  delete[] b_cost ;
  delete[] b_plane_n ;
  delete[] b_plane_d ;
  delete[] b_depth ;
}

/**
* @brief Perform propagation using OpenCL at specified scale
*/
void Propagate( DepthMap & map , const int id_start ,
                Camera & reference_cam ,
                const std::vector< Camera > & cams ,
                const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                const Image & image_ref ,
                const std::vector< Image > & neigh_imgs ,
                const DepthMapComputationParameters & params ,
                const int scale ,
                OpenCLWrapper & wrapper ,
                cl_kernel & kernel_red , // Compute cost
                cl_kernel & kernel_black , // Compute cost
                cl_kernel & append_cost_kernel , // Append cost
                cl_kernel & sort_and_store_cost_kernel , // Sort and compute real cost
                cl_kernel & update_plane_wrt_cost_kernel , // Update planes based on cost
                cl_kernel & compute_pixel_depth_kernel )  // Compute pixel depth value
{
  const bool do_red = id_start == 0 ;
  const cl_int nb_total_neighbors = reference_cam.m_view_neighbors.size() ;
  const cl_int in_width  = map.width() ;
  const cl_int in_height = map.height() ;
  const cl_int nb_pixel = in_width * in_height ;
  const int size[] = { in_width , in_height } ;
  const std::pair< int , int > compute_size = std::make_pair( in_width , in_height ) ;


  // (x,y)
  /*
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   | X |   | X |   |   |   |   |
   *   |   |   |   | X |   | X |   | X |   |   |   |
   *   | X |   | X |   | X | O | X |   | X |   | X |
   *   |   |   |   | X |   | X |   | X |   |   |   |
   *   |   |   |   |   | X |   | X |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   */
  const int neighs_idx[20][2] =
  {
    {0, -5} ,

    {0, -3} ,

    { -1, -2} ,
    {1, -2} ,

    { -2, -1} ,
    {0, -1} ,
    {2, -1} ,

    { -5, 0} ,
    { -3, 0} ,
    { -1, 0} ,
    {1, 0} ,
    {3, 0} ,
    {5, 0} ,

    { -2, 1} ,
    {0, 1} ,
    {2, 1} ,

    { -1, 2} ,
    {1, 2} ,

    {0, 3} ,

    {0, 5}
  } ;

  // Get the initial images
  cl_mem Ip = wrapper.createImage( image_ref.intensity() , OpenCLWrapper::OPENCL_IMAGE_READ_ONLY ) ;
  cl_mem Gp ;
  cl_mem CensusP ;
  if( params.metric() == COST_METRIC_PM )
  {
    Gp = wrapper.createBuffer( image_ref.gradient() ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    CensusP = wrapper.createBuffer( image_ref.census() ) ;
  }
  cl_mem Kinv = wrapper.createBuffer( openMVG::Mat3( reference_cam.m_K_inv_scaled[ scale ] ) ) ;

  // Copy current cost to the best
  cl_float * cur_cost_f = new cl_float[ nb_pixel ] ;
  cl_float * cur_plane_n = new cl_float[ 3 * nb_pixel ] ;
  cl_float * cur_plane_d = new cl_float[ nb_pixel ] ;

  size_t index = 0 ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      cur_cost_f[ index ] = map.cost( y , x ) ;

      const openMVG::Vec4 & cur_plane = map.plane( y , x ) ;

      cur_plane_n[ 3 * index ]     = cur_plane[ 0 ] ;
      cur_plane_n[ 3 * index + 1 ] = cur_plane[ 1 ] ;
      cur_plane_n[ 3 * index + 2 ] = cur_plane[ 2 ] ;

      cur_plane_d[ index ] = cur_plane[ 3 ] ;
      ++index ;
    }
  }

  cl_mem best_cost = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_cost_f ) ;
  cl_mem cur_cost = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_cost_f ) ;
  cl_mem best_plane_n = wrapper.createBuffer( 3 * nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_plane_n ) ;
  cl_mem best_plane_d = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_plane_d ) ;

  // The costs -> All costs
  cl_mem all_costs = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) * reference_cam.m_view_neighbors.size() ,
                     OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;

  delete[] cur_cost_f ;
  delete[] cur_plane_n ;
  delete[] cur_plane_d ;

  //  cl_kernel cost_kernel_ncc = (do_red) ? kernel_ncc_red : kernel_ncc_black ;
  cost_topology_type c_type = ( do_red ) ? COST_COMPUTE_RED : COST_COMPUTE_BLACK ;

  for( int id_plane = 0 ; id_plane < 20 ; ++id_plane )
  {
    // Compute and update cost using delta position
    const openMVG::Vec2i delta_plane( neighs_idx[ id_plane ][0] , neighs_idx[ id_plane][ 1 ]  ) ;
    cl_mem delta_plane_idx = wrapper.createBuffer( delta_plane ) ;

    ComputeCost( cur_cost , reference_cam , neigh_imgs , cams , stereo_rig , params , scale ,
                 Ip , Gp , CensusP , Kinv , best_plane_n , best_plane_d , wrapper ,
                 kernel_red /* unused */ , kernel_red , kernel_black ,
                 append_cost_kernel , sort_and_store_cost_kernel ,
                 compute_size , delta_plane , c_type , &all_costs ) ;

    // Update best cost and planes
    clSetKernelArg( update_plane_wrt_cost_kernel , 0 , sizeof( cl_mem ) , &best_plane_n ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 1 , sizeof( cl_mem ) , &best_plane_d ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 2 , sizeof( cl_mem ) , &delta_plane_idx ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 3 , sizeof( cl_mem ) , &best_cost ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 4 , sizeof( cl_mem ) , &cur_cost ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 5 , sizeof( cl_int ) , &in_width ) ;
    clSetKernelArg( update_plane_wrt_cost_kernel , 6 , sizeof( cl_int ) , &in_height ) ;
    wrapper.runKernel2d( update_plane_wrt_cost_kernel , size ) ;

    // Clear local memory
    wrapper.clearMemory( delta_plane_idx ) ;
  }
  wrapper.clearMemory( cur_cost ) ;
  wrapper.clearMemory( all_costs ) ;
  wrapper.clearMemory( Ip ) ;
  if( params.metric() == COST_METRIC_PM )
  {
    wrapper.clearMemory( Gp ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    wrapper.clearMemory( CensusP ) ;
  }


  // Compute depth kernel
  cl_mem K = wrapper.createBuffer( reference_cam.m_K_scaled[ scale ] ) ;
  cl_mem best_depth = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;

  clSetKernelArg( compute_pixel_depth_kernel , 0 , sizeof( cl_mem ) , &best_plane_n ) ;
  clSetKernelArg( compute_pixel_depth_kernel , 1 , sizeof( cl_mem ) , &best_plane_d ) ;
  clSetKernelArg( compute_pixel_depth_kernel , 2 , sizeof( cl_mem ) , &K ) ;
  clSetKernelArg( compute_pixel_depth_kernel , 3 , sizeof( cl_int ) , &in_width ) ;
  clSetKernelArg( compute_pixel_depth_kernel , 4 , sizeof( cl_int ) , &in_height ) ;
  clSetKernelArg( compute_pixel_depth_kernel , 5 , sizeof( cl_mem ) , &best_depth ) ;
  wrapper.runKernel2d( compute_pixel_depth_kernel , size ) ;

  wrapper.clearMemory( K ) ;

  float * b_cost = new float[ nb_pixel ] ;
  float * b_plane_n = new float[ nb_pixel * 3 ] ;
  float * b_plane_d = new float[ nb_pixel ] ;
  float * b_depth   = new float[ nb_pixel ] ;

  wrapper.copyDeviceToHost( best_cost , b_cost ) ;
  wrapper.copyDeviceToHost( best_plane_n , b_plane_n ) ;
  wrapper.copyDeviceToHost( best_plane_d , b_plane_d ) ;
  wrapper.copyDeviceToHost( best_depth , b_depth ) ;

  wrapper.clearMemory( best_cost ) ;
  wrapper.clearMemory( best_plane_n ) ;
  wrapper.clearMemory( best_plane_d ) ;
  wrapper.clearMemory( best_depth ) ;

  // Store back cost and plane index
  index = 0 ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      const openMVG::Vec4 plane( b_plane_n[ 3 * index ] , b_plane_n[ 3 * index + 1 ] , b_plane_n[ 3 * index + 2 ] , b_plane_d[ index ] ) ;

      if( ( do_red && ( ( ( y % 2 == 0 ) && ( x % 2 == 0 ) ) || ( ( y % 2 == 1 ) && ( x % 2 == 1 ) ) ) ) ||
          ( !do_red && ( ( ( y % 2 == 0 ) && ( x % 2 == 1 ) ) || ( ( y % 2 == 1 ) && ( x % 2 == 0 ) ) ) ) )
      {
        map.cost( y , x , b_cost[ index ] ) ;
        map.plane( y , x , plane ) ;
        map.depth( y , x , Clamp( static_cast<double>( b_depth[ index ] ) , reference_cam.m_min_depth * 0.8 , reference_cam.m_max_depth * 1.2 ) ) ;
      }

      ++index ;
    }
  }

  delete[] b_cost ;
  delete[] b_plane_n ;
  delete[] b_plane_d ;
  delete[] b_depth ;
}


/**
* @brief Perform refinement using opencl
* @param map Depth map to refine
* @param cam Reference camera
* @param cams Array of all neighboring cameras
* @param stereo_rig Array of motion between reference and its neighbors
* @param image_ref Image data of the reference view
* @param params Computation parameters
* @param wrapper OpenCL helper object
* @param kernel_full Kernel to compute cost
* @param append_cost_kernel Kernel used to append cost to all costs
* @param sort_and_store_cost_kernel Kernel used to sort all costs and compute final cost
* @param update_plane_wrt_cost2_kernel Kernel used to update planes
* @param compute_new_plane_kernel Kernel used to compute planes parameters
*/
void Refinement( DepthMap & map ,
                 Camera & cam ,
                 const std::vector< Camera > & cams ,
                 const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                 const Image & image_ref ,
                 const DepthMapComputationParameters & params ,
                 OpenCLWrapper & wrapper ,
                 cl_kernel & kernel_full , // Compute cost
                 cl_kernel & append_cost_kernel , // Append cost
                 cl_kernel & sort_and_store_cost_kernel , // Sort and compute real cost
                 cl_kernel & update_plane_wrt_cost2_kernel , // Update planes based on cost
                 cl_kernel & compute_new_plane_kernel ) // Compute new planes
{
  // Load images for the neighbors
  const ImageLoadType load_type = ComputeLoadType( params.metric() ) ;
  const std::vector< Image > neigh_imgs = LoadNeighborImages( cam , params , load_type ) ;
  const cl_int in_width  = image_ref.width() ;
  const cl_int in_height = image_ref.height() ;
  const cl_int nb_pixel  = in_width * in_height ;
  const int size[] = { static_cast<int>( in_width ) , static_cast<int>( in_height ) } ;
  const std::pair< int , int > compute_size = std::make_pair( in_width , in_height ) ;
  const openMVG::Vec2i delta_plane( 0 , 0 ) ;
  const cost_topology_type c_type = COST_COMPUTE_FULL ;

  // Initialize RNG
  std::mt19937_64 rng ;
  std::random_device device;
  std::seed_seq seq{device(), device(), device(), device()};
  rng.seed( seq ) ;

  std::uniform_real_distribution<float> distrib_01( 0.0 , 1.0 ) ;

  cl_float * rng_numbers = new cl_float[ 4 * nb_pixel ] ;

  const double min_disparity = cam.depthDisparityConversion( cam.m_max_depth ) ;
  const double max_disparity = cam.depthDisparityConversion( cam.m_min_depth ) ;
  const cl_float min_disparity_f = static_cast<cl_float>( min_disparity ) ;
  const cl_float max_disparity_f = static_cast<cl_float>( max_disparity ) ;

  cl_float theta_max = 60.0f ;
  cl_float cos_theta_max = std::cos( openMVG::D2R( theta_max ) ) ;
  cl_float delta_disparity = ( max_disparity_f - min_disparity_f ) / 2.0f ;

  cl_mem P    = wrapper.createBuffer( cam.m_P ) ;
  cl_mem Minv = wrapper.createBuffer( cam.m_M_inv ) ;
  cl_mem C    = wrapper.createBuffer( cam.m_C ) ;
  cl_mem Kinv = wrapper.createBuffer( openMVG::Mat3( cam.m_K.inverse() ) ) ;

  cl_mem Ip   = wrapper.createImage( image_ref.intensity() , OpenCLWrapper::OPENCL_BUFFER_READ_ONLY ) ;
  cl_mem Gp ;
  cl_mem CensusP ;
  if( params.metric() == COST_METRIC_PM )
  {
    Gp = wrapper.createBuffer( image_ref.gradient() ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    CensusP = wrapper.createBuffer( image_ref.census() );
  }

  cl_float * cur_n = new cl_float[ nb_pixel * 3 ] ;
  cl_float * cur_pl_d = new cl_float[ nb_pixel ] ;
  cl_float * cur_d = new cl_float[ nb_pixel ] ;
  cl_float * base_cost = new cl_float[ nb_pixel ] ;

  size_t index = 0 ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      const openMVG::Vec4 & plane = map.plane( y , x ) ;
      const double depth = map.depth( y , x ) ;
      const double cost = map.cost( y , x ) ;

      cur_n[ 3 * index ]     = plane[0] ;
      cur_n[ 3 * index + 1 ] = plane[1] ;
      cur_n[ 3 * index + 2 ] = plane[2] ;

      cur_pl_d[ index ] = plane[3] ;

      cur_d[ index ] = depth ;

      base_cost[ index ] = cost ;

      ++index ;
    }
  }

  cl_mem cur_plane_n  = wrapper.createBuffer( 3 * nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_n ) ;
  cl_mem cur_plane_d = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_pl_d ) ;
  cl_mem cur_depth   = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_d ) ;
  cl_mem cur_cost    = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , base_cost ) ;

  delete[] base_cost ;
  delete[] cur_n ;
  delete[] cur_pl_d ;
  delete[] cur_d ;

  cl_float fx = cam.m_K( 0 , 0 ) ;
  cl_float baseline = cam.m_mean_baseline ;

  cl_mem out_plane_n = wrapper.createBuffer( 3 * nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;
  cl_mem out_plane_d = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;
  cl_mem out_depth   = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;


  cl_mem cl_rng      = wrapper.createBuffer( 4 * nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;
  cl_mem tmp_cost    = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;
  cl_mem all_costs   = wrapper.createBuffer( cam.m_view_neighbors.size() * nb_pixel * sizeof( cl_float ) ,
                       OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;

  while( delta_disparity > 0.01f )
  {
    // Compute new random numbers
    for( int id_pix = 0 ; id_pix < nb_pixel ; ++id_pix )
    {
      const float u1 = distrib_01( rng ) ;
      const float u2 = distrib_01( rng ) ;
      const float u3 = distrib_01( rng ) ;
      const float u4 = distrib_01( rng ) ;

      rng_numbers[ 4 * id_pix ]     = u1 ;
      rng_numbers[ 4 * id_pix + 1 ] = u2 ;
      rng_numbers[ 4 * id_pix + 2 ] = u3 ;
      rng_numbers[ 4 * id_pix + 3 ] = u4 ;
    }
    wrapper.copyHostToDevice( rng_numbers , cl_rng ) ;

    // Given the rng, computes the new planes
    clSetKernelArg( compute_new_plane_kernel , 0 , sizeof( cl_mem ) , &cl_rng ) ;
    clSetKernelArg( compute_new_plane_kernel , 1 , sizeof( cl_mem ) , &cur_depth ) ;
    clSetKernelArg( compute_new_plane_kernel , 2 , sizeof( cl_mem ) , &cur_plane_n ) ;
    clSetKernelArg( compute_new_plane_kernel , 3 , sizeof( cl_float ) , &min_disparity_f ) ;
    clSetKernelArg( compute_new_plane_kernel , 4 , sizeof( cl_float ) , &max_disparity_f ) ;
    clSetKernelArg( compute_new_plane_kernel , 5 , sizeof( cl_float ) , &delta_disparity ) ;
    clSetKernelArg( compute_new_plane_kernel , 6 , sizeof( cl_float ) , &cos_theta_max ) ;
    clSetKernelArg( compute_new_plane_kernel , 7 , sizeof( cl_mem ) , &P ) ;
    clSetKernelArg( compute_new_plane_kernel , 8 , sizeof( cl_mem ) , &Minv ) ;
    clSetKernelArg( compute_new_plane_kernel , 9 , sizeof( cl_mem ) , &C ) ;
    clSetKernelArg( compute_new_plane_kernel , 10 , sizeof( cl_float ) , &fx ) ;
    clSetKernelArg( compute_new_plane_kernel , 11 , sizeof( cl_float ) , &baseline ) ;
    clSetKernelArg( compute_new_plane_kernel , 12 , sizeof( cl_int ) , &in_width ) ;
    clSetKernelArg( compute_new_plane_kernel , 13 , sizeof( cl_int ) , &in_height ) ;
    clSetKernelArg( compute_new_plane_kernel , 14 , sizeof( cl_mem ) , &out_plane_n ) ;
    clSetKernelArg( compute_new_plane_kernel , 15 , sizeof( cl_mem ) , &out_plane_d ) ;
    clSetKernelArg( compute_new_plane_kernel , 16 , sizeof( cl_mem ) , &out_depth ) ;

    wrapper.runKernel2d( compute_new_plane_kernel , size ) ;

    // Compute cost
    ComputeCost( tmp_cost , cam , neigh_imgs , cams , stereo_rig , params , -1 ,
                 Ip , Gp , CensusP , Kinv , out_plane_n , out_plane_d , wrapper ,
                 kernel_full , kernel_full , kernel_full ,
                 append_cost_kernel , sort_and_store_cost_kernel ,
                 compute_size , delta_plane , c_type , &all_costs ) ;


    // Update planes and cost
    clSetKernelArg( update_plane_wrt_cost2_kernel , 0 , sizeof( cl_mem ) , &cur_plane_n ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 1 , sizeof( cl_mem ) , &cur_plane_d ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 2 , sizeof( cl_mem ) , &cur_depth ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 3 , sizeof( cl_mem ) , &out_plane_n ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 4 , sizeof( cl_mem ) , &out_plane_d ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 5 , sizeof( cl_mem ) , &out_depth ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 6 , sizeof( cl_mem ) , &cur_cost ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 7 , sizeof( cl_mem ) , &tmp_cost ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 8 , sizeof( cl_int ) , &in_width ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 9 , sizeof( cl_int ) , &in_height ) ;

    wrapper.runKernel2d( update_plane_wrt_cost2_kernel , size ) ;

    // Update parameters to compute new plane
    delta_disparity /= 2.f ;
    theta_max /= 2.f ;
    cos_theta_max = std::cos( openMVG::D2R( theta_max ) ) ;
  }
  delete[] rng_numbers ;
  wrapper.clearMemory( all_costs ) ;
  wrapper.clearMemory( tmp_cost ) ;
  wrapper.clearMemory( out_plane_n ) ;
  wrapper.clearMemory( out_plane_d ) ;
  wrapper.clearMemory( out_depth ) ;
  wrapper.clearMemory( Ip );
  if( params.metric() == COST_METRIC_PM )
  {
    wrapper.clearMemory( Gp ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    wrapper.clearMemory( CensusP ) ;
  }
  wrapper.clearMemory( Kinv ) ;
  wrapper.clearMemory( cl_rng ) ;
  wrapper.clearMemory( C ) ;
  wrapper.clearMemory( P ) ;
  wrapper.clearMemory( Minv ) ;

  // Copy back parameters
  cur_n     = new cl_float[ 3 * nb_pixel ] ;
  cur_pl_d  = new cl_float[ nb_pixel ] ;
  cur_d = new cl_float[ nb_pixel ] ;
  base_cost = new cl_float[ nb_pixel ] ;

  wrapper.copyDeviceToHost( cur_plane_n , cur_n ) ;
  wrapper.copyDeviceToHost( cur_plane_d , cur_pl_d ) ;
  wrapper.copyDeviceToHost( cur_depth , cur_d ) ;
  wrapper.copyDeviceToHost( cur_cost , base_cost ) ;

  wrapper.clearMemory( cur_plane_n ) ;
  wrapper.clearMemory( cur_plane_d ) ;
  wrapper.clearMemory( cur_depth ) ;
  wrapper.clearMemory( cur_cost ) ;

  // Store back results inside depth map
  index = 0 ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      openMVG::Vec4 pl( cur_n[ 3 * index ] , cur_n[ 3 * index + 1 ] , cur_n[ 3 * index + 2 ] , cur_pl_d[ index ] );

      map.plane( y , x , pl ) ;
      map.depth( y , x , cur_d[ index ] ) ;
      map.cost( y , x , base_cost[ index ] ) ;

      ++index ;
    }
  }

  delete[] cur_n ;
  delete[] cur_pl_d ;
  delete[] cur_d ;
  delete[] base_cost ;
}

/**
* @brief Perform refinement using opencl
* @param map Depth map to refine
* @param cam Reference camera
* @param cams Array of all neighboring cameras
* @param stereo_rig Array of motion between reference and its neighbors
* @param image_ref Image data of the reference view
* @param params Computation parameters
* @param wrapper OpenCL helper object
* @param kernel_full Kernel to compute cost
* @param append_cost_kernel Kernel used to append cost to all costs
* @param sort_and_store_cost_kernel Kernel used to sort all costs and compute final cost
* @param update_plane_wrt_cost2_kernel Kernel used to update planes
* @param compute_new_plane_kernel Kernel used to compute planes parameters
*/
void Refinement( DepthMap & map ,
                 Camera & cam ,
                 const std::vector< Camera > & cams ,
                 const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                 const Image & image_ref ,
                 const std::vector< Image > & neigh_imgs ,
                 const DepthMapComputationParameters & params ,
                 const int scale ,
                 OpenCLWrapper & wrapper ,
                 cl_kernel & kernel_full , // Compute cost
                 cl_kernel & append_cost_kernel , // Append cost
                 cl_kernel & sort_and_store_cost_kernel , // Sort and compute real cost
                 cl_kernel & update_plane_wrt_cost2_kernel , // Update planes based on cost
                 cl_kernel & compute_new_plane_kernel )  // Compute new planes
{
  // Load images for the neighbors
  const cl_int in_width  = image_ref.width() ;
  const cl_int in_height = image_ref.height() ;
  const cl_int nb_pixel  = in_width * in_height ;
  const int size[] = { static_cast<int>( in_width ) , static_cast<int>( in_height ) } ;
  const std::pair< int , int > compute_size = std::make_pair( in_width , in_height ) ;
  const openMVG::Vec2i delta_plane( 0 , 0 ) ;
  const cost_topology_type c_type = COST_COMPUTE_FULL ;

  // Initialize RNG
  std::mt19937_64 rng ;
  std::random_device device;
  std::seed_seq seq{device(), device(), device(), device()};
  rng.seed( seq ) ;

  std::uniform_real_distribution<float> distrib_01( 0.0 , 1.0 ) ;

  cl_float * rng_numbers = new cl_float[ 4 * nb_pixel ] ;

  const double min_disparity = cam.depthDisparityConversion( cam.m_max_depth , scale ) ;
  const double max_disparity = cam.depthDisparityConversion( cam.m_min_depth , scale ) ;
  const cl_float min_disparity_f = static_cast<cl_float>( min_disparity ) ;
  const cl_float max_disparity_f = static_cast<cl_float>( max_disparity ) ;

  cl_float theta_max =  60.0f ;
  cl_float cos_theta_max = std::cos( openMVG::D2R( theta_max ) ) ;
  cl_float delta_disparity = ( max_disparity_f - min_disparity_f ) / 2.0f ;

  cl_mem P    = wrapper.createBuffer( cam.m_P_scaled[ scale ] ) ;
  cl_mem Minv = wrapper.createBuffer( cam.m_M_inv_scaled[ scale ] ) ;
  cl_mem C    = wrapper.createBuffer( cam.m_C ) ;
  cl_mem Kinv = wrapper.createBuffer( openMVG::Mat3( cam.m_K_inv_scaled[ scale ] ) ) ;

  cl_mem Ip   = wrapper.createImage( image_ref.intensity() , OpenCLWrapper::OPENCL_BUFFER_READ_ONLY ) ;
  cl_mem Gp ;
  cl_mem CensusP ;
  if( params.metric() == COST_METRIC_PM )
  {
    Gp = wrapper.createBuffer( image_ref.gradient() ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    CensusP = wrapper.createBuffer( image_ref.census() ) ;
  }

  cl_float * cur_n = new cl_float[ nb_pixel * 3 ] ;
  cl_float * cur_pl_d = new cl_float[ nb_pixel ] ;
  cl_float * cur_d = new cl_float[ nb_pixel ] ;
  cl_float * base_cost = new cl_float[ nb_pixel ] ;

  size_t index = 0 ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      const openMVG::Vec4 & plane = map.plane( y , x ) ;
      const double depth = map.depth( y , x ) ;
      const double cost = map.cost( y , x ) ;

      cur_n[ 3 * index ]     = plane[0] ;
      cur_n[ 3 * index + 1 ] = plane[1] ;
      cur_n[ 3 * index + 2 ] = plane[2] ;

      cur_pl_d[ index ] = plane[3] ;

      cur_d[ index ] = depth ;

      base_cost[ index ] = cost ;

      ++index ;
    }
  }

  cl_mem cur_plane_n  = wrapper.createBuffer( 3 * nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_n ) ;
  cl_mem cur_plane_d = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_pl_d ) ;
  cl_mem cur_depth   = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , cur_d ) ;
  cl_mem cur_cost    = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE , base_cost ) ;

  delete[] base_cost ;
  delete[] cur_n ;
  delete[] cur_pl_d ;
  delete[] cur_d ;

  cl_float fx = cam.m_K_scaled[ scale ]( 0 , 0 ) ;
  cl_float baseline = cam.m_mean_baseline ;

  cl_mem out_plane_n = wrapper.createBuffer( 3 * nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;
  cl_mem out_plane_d = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;
  cl_mem out_depth   = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;


  cl_mem cl_rng      = wrapper.createBuffer( 4 * nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;
  cl_mem tmp_cost    = wrapper.createBuffer( nb_pixel * sizeof( cl_float ) , OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;
  cl_mem all_costs   = wrapper.createBuffer( cam.m_view_neighbors.size() * nb_pixel * sizeof( cl_float ) ,
                       OpenCLWrapper::OPENCL_BUFFER_READ_WRITE ) ;

  while( delta_disparity > 0.01f )
  {
    // Compute new random numbers
    for( int id_pix = 0 ; id_pix < nb_pixel ; ++id_pix )
    {
      const float u1 = distrib_01( rng ) ;
      const float u2 = distrib_01( rng ) ;
      const float u3 = distrib_01( rng ) ;
      const float u4 = distrib_01( rng ) ;

      rng_numbers[ 4 * id_pix ]     = u1 ;
      rng_numbers[ 4 * id_pix + 1 ] = u2 ;
      rng_numbers[ 4 * id_pix + 2 ] = u3 ;
      rng_numbers[ 4 * id_pix + 3 ] = u4 ;
    }
    wrapper.copyHostToDevice( rng_numbers , cl_rng ) ;

    // Given the rng, computes the new planes
    clSetKernelArg( compute_new_plane_kernel , 0 , sizeof( cl_mem ) , &cl_rng ) ;
    clSetKernelArg( compute_new_plane_kernel , 1 , sizeof( cl_mem ) , &cur_depth ) ;
    clSetKernelArg( compute_new_plane_kernel , 2 , sizeof( cl_mem ) , &cur_plane_n ) ;
    clSetKernelArg( compute_new_plane_kernel , 3 , sizeof( cl_float ) , &min_disparity_f ) ;
    clSetKernelArg( compute_new_plane_kernel , 4 , sizeof( cl_float ) , &max_disparity_f ) ;
    clSetKernelArg( compute_new_plane_kernel , 5 , sizeof( cl_float ) , &delta_disparity ) ;
    clSetKernelArg( compute_new_plane_kernel , 6 , sizeof( cl_float ) , &cos_theta_max ) ;
    clSetKernelArg( compute_new_plane_kernel , 7 , sizeof( cl_mem ) , &P ) ;
    clSetKernelArg( compute_new_plane_kernel , 8 , sizeof( cl_mem ) , &Minv ) ;
    clSetKernelArg( compute_new_plane_kernel , 9 , sizeof( cl_mem ) , &C ) ;
    clSetKernelArg( compute_new_plane_kernel , 10 , sizeof( cl_float ) , &fx ) ;
    clSetKernelArg( compute_new_plane_kernel , 11 , sizeof( cl_float ) , &baseline ) ;
    clSetKernelArg( compute_new_plane_kernel , 12 , sizeof( cl_int ) , &in_width ) ;
    clSetKernelArg( compute_new_plane_kernel , 13 , sizeof( cl_int ) , &in_height ) ;
    clSetKernelArg( compute_new_plane_kernel , 14 , sizeof( cl_mem ) , &out_plane_n ) ;
    clSetKernelArg( compute_new_plane_kernel , 15 , sizeof( cl_mem ) , &out_plane_d ) ;
    clSetKernelArg( compute_new_plane_kernel , 16 , sizeof( cl_mem ) , &out_depth ) ;

    wrapper.runKernel2d( compute_new_plane_kernel , size ) ;

    // Compute cost
    ComputeCost( tmp_cost , cam , neigh_imgs , cams , stereo_rig , params , scale ,
                 Ip , Gp , CensusP , Kinv , out_plane_n , out_plane_d , wrapper ,
                 kernel_full , kernel_full , kernel_full ,
                 append_cost_kernel , sort_and_store_cost_kernel ,
                 compute_size , delta_plane , c_type , &all_costs ) ;


    // Update planes and cost
    clSetKernelArg( update_plane_wrt_cost2_kernel , 0 , sizeof( cl_mem ) , &cur_plane_n ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 1 , sizeof( cl_mem ) , &cur_plane_d ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 2 , sizeof( cl_mem ) , &cur_depth ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 3 , sizeof( cl_mem ) , &out_plane_n ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 4 , sizeof( cl_mem ) , &out_plane_d ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 5 , sizeof( cl_mem ) , &out_depth ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 6 , sizeof( cl_mem ) , &cur_cost ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 7 , sizeof( cl_mem ) , &tmp_cost ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 8 , sizeof( cl_int ) , &in_width ) ;
    clSetKernelArg( update_plane_wrt_cost2_kernel , 9 , sizeof( cl_int ) , &in_height ) ;

    wrapper.runKernel2d( update_plane_wrt_cost2_kernel , size ) ;

    // Update parameters to compute new plane
    delta_disparity /= 2.f ;
    theta_max /= 2.f ;
    cos_theta_max = std::cos( openMVG::D2R( theta_max ) ) ;
  }
  delete[] rng_numbers ;
  wrapper.clearMemory( all_costs ) ;
  wrapper.clearMemory( tmp_cost ) ;
  wrapper.clearMemory( out_plane_n ) ;
  wrapper.clearMemory( out_plane_d ) ;
  wrapper.clearMemory( out_depth ) ;
  wrapper.clearMemory( Ip );
  if( params.metric() == COST_METRIC_PM )
  {
    wrapper.clearMemory( Gp ) ;
  }
  if( params.metric() == COST_METRIC_CENSUS )
  {
    wrapper.clearMemory( CensusP ) ;
  }
  wrapper.clearMemory( Kinv ) ;
  wrapper.clearMemory( cl_rng ) ;
  wrapper.clearMemory( C ) ;
  wrapper.clearMemory( P ) ;
  wrapper.clearMemory( Minv ) ;

  // Copy back parameters
  cur_n     = new cl_float[ 3 * nb_pixel ] ;
  cur_pl_d  = new cl_float[ nb_pixel ] ;
  cur_d = new cl_float[ nb_pixel ] ;
  base_cost = new cl_float[ nb_pixel ] ;

  wrapper.copyDeviceToHost( cur_plane_n , cur_n ) ;
  wrapper.copyDeviceToHost( cur_plane_d , cur_pl_d ) ;
  wrapper.copyDeviceToHost( cur_depth , cur_d ) ;
  wrapper.copyDeviceToHost( cur_cost , base_cost ) ;

  wrapper.clearMemory( cur_plane_n ) ;
  wrapper.clearMemory( cur_plane_d ) ;
  wrapper.clearMemory( cur_depth ) ;
  wrapper.clearMemory( cur_cost ) ;

  // Store back results inside depth map
  index = 0 ;
  for( int y = 0 ; y < map.height() ; ++y )
  {
    for( int x = 0 ; x < map.width() ; ++x )
    {
      openMVG::Vec4 pl( cur_n[ 3 * index ] , cur_n[ 3 * index + 1 ] , cur_n[ 3 * index + 2 ] , cur_pl_d[ index ] );

      map.plane( y , x , pl ) ;
      map.depth( y , x , cur_d[ index ] ) ;
      map.cost( y , x , base_cost[ index ] ) ;

      ++index ;
    }
  }

  delete[] cur_n ;
  delete[] cur_pl_d ;
  delete[] cur_d ;
  delete[] base_cost ;
}

} // namespace MVS