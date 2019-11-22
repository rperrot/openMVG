#include "Camera.hpp"
#include "DepthMap.hpp"
#include "DepthMapComputationCPU.hpp"
#include "DepthMapComputationCommon.hpp"
#include "DepthMapComputationOpenCL.hpp"
#include "DepthMapComputationParameters.hpp"
#include "Image.hpp"
#include "MatchingCost.hpp"
#include "Util.hpp"

#include "openMVG/sfm/sfm.hpp"

#include "third_party/cmdLine/cmdLine.h"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <chrono>
#include <iostream>
#include <random>

#define MULTISCALE
//#define USE_OPENCL
#define EXPORT_INTERMEDIATE_RESULT

#ifdef EXPORT_INTERMEDIATE_RESULT
// Init
static inline std::string GetInitDepthName( const int camera_id, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_init_depth.png";
  return str.str();
}

static inline std::string GetInitPlyName( const int camera_id, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_init_model.ply";
  return str.str();
}

static inline std::string GetInitCostName( const int camera_id, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_init_cost.png";
  return str.str();
}

static inline std::string GetInitNormalName( const int camera_id, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_init_normal.png";
  return str.str();
}

// Propagation
static inline std::string GetPropagationDepthName( const int camera_id, const int iteration, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_propagation_" << iteration << "_depth.png";
  return str.str();
}

static inline std::string GetPropagationPlyName( const int camera_id, const int iteration, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_propagation_" << iteration << "_model.ply";
  return str.str();
}

static inline std::string GetPropagationCostName( const int camera_id, const int iteration, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_propagation_" << iteration << "_cost.png";
  return str.str();
}

static inline std::string GetPropagationNormalName( const int camera_id, const int iteration, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_propagation_" << iteration << "_normal.png";
  return str.str();
}

// Refine
static inline std::string GetRefinementDepthName( const int camera_id, const int iteration, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_refine_" << iteration << "_depth.png";
  return str.str();
}

static inline std::string GetRefinementPlyName( const int camera_id, const int iteration, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_refine_" << iteration << "_model.ply";
  return str.str();
}

static inline std::string GetRefinementCostName( const int camera_id, const int iteration, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_refine_" << iteration << "_cost.png";
  return str.str();
}

static inline std::string GetRefinementNormalName( const int camera_id, const int iteration, const int scale )
{
  std::stringstream str;
  str << "[cam_" << camera_id << "][scale_" << scale << "]_refine_" << iteration << "_normal.png";
  return str.str();
}

static inline std::string GetFinalDepthName( const int id_cam )
{
  std::stringstream str;
  str << "[cam_" << id_cam << "]_final_depth.png";
  return str.str();
}

static inline std::string GetFinalPlyName( const int id_cam )
{
  std::stringstream str;
  str << "[cam_" << id_cam << "]_final_model.ply";
  return str.str();
}

static inline std::string GetFinalNormalName( const int id_cam )
{
  std::stringstream str;
  str << "[cam_" << id_cam << "]_final_normal.png";
  return str.str();
}

static inline std::string GetFinalCostName( const int id_cam )
{
  std::stringstream str;
  str << "[cam_" << id_cam << "]_final_cost.png";
  return str.str();
}

#endif

/**
* @brief Create directory structure for the project
* @param cams The list of input cameras
* @param params the computation parameters
*/
void PrepareOutputDirectory( const std::vector<MVS::Camera>&           cams,
                             const MVS::DepthMapComputationParameters& params )
{
  std::cout << "Preparing output directory" << std::endl;
  const std::string outDirPath = params.workingDirectory();
  if ( !stlplus::is_folder( outDirPath ) )
  {
    stlplus::folder_create( outDirPath );
    if ( !stlplus::is_folder( outDirPath ) )
    {
      std::cerr << "Could not create output directory" << std::endl;
      exit( EXIT_FAILURE );
    }
  }

  // Create depth folder
  const std::string depth_folder = params.depthDirectory();
  if ( !stlplus::is_folder( depth_folder ) )
  {
    stlplus::folder_create( depth_folder );
    if ( !stlplus::is_folder( depth_folder ) )
    {
      std::cerr << "Could not create depth folder" << std::endl;
      exit( EXIT_FAILURE );
    }
  }

  // create model folder
  const std::string model_folder = params.getModelDirectory();
  if ( !stlplus::is_folder( model_folder ) )
  {
    stlplus::folder_create( model_folder );
    if ( !stlplus::is_folder( model_folder ) )
    {
      std::cerr << "Could not create model directory" << std::endl;
      exit( EXIT_FAILURE );
    }
  }

  // Create cameras directories inside model folder
  for ( size_t id_cam = 0; id_cam < cams.size(); ++id_cam )
  {
    std::stringstream str;
    str << "cam_" << id_cam;
    const std::string cam_folder = params.getCameraDirectory( id_cam );
    if ( !stlplus::is_folder( cam_folder ) )
    {
      stlplus::folder_create( cam_folder );
      if ( !stlplus::is_folder( cam_folder ) )
      {
        std::cerr << "Could not create cam_" << id_cam << std::endl;
        exit( EXIT_FAILURE );
      }
    }

    const MVS::ImageLoadType load_type = MVS::ComputeLoadType( params.metric() );

    // Create images and save it
    MVS::Image cur_img( cams[ id_cam ].m_img_path, params.scale(), cams[ id_cam ].m_intrinsic, load_type );
    // Grayscale path
    const std::string color_path     = params.getColorPath( id_cam );
    const std::string grayscale_path = params.getGrayscalePath( id_cam );
    const std::string gradient_path  = params.getGradientPath( id_cam );
    const std::string census_path    = params.getCensusPath( id_cam );

    cur_img.save( color_path, grayscale_path, gradient_path, census_path, load_type );
  }
  std::cout << "Preparation done" << std::endl;
}

/**
* @brief Compute depth map using a multiple scale approach
* @param cam the camera corresponding to the depth map to compute
* @param cams List of all cameras in the scene
* @param start_scale Scale of the begginging process
* @param out_path Path where to save the depth map
*/
void ComputeMultipleScaleDepthMap( const int                           id_cam,
                                   MVS::Camera&                        cam,
                                   const std::vector<MVS::Camera>&     cams,
                                   MVS::DepthMapComputationParameters& params,
                                   const int                           start_scale, // Starting scale
                                   const std::string&                  out_path )
{
  // Get size of the intermediate images
  std::vector<std::pair<int, int>> imgs_dims;
  for ( int i = 0; i <= start_scale; ++i )
  {
    if ( i < params.scale() )
    {
      imgs_dims.emplace_back( std::make_pair( 0, 0 ) );
    }
    else if ( i == params.scale() )
    {
      imgs_dims.emplace_back( std::make_pair( cam.m_cam_dims.second, cam.m_cam_dims.first ) );
    }
    else
    {
      const std::pair<int, int>& prev = imgs_dims[ i - 1 ];
      imgs_dims.emplace_back( std::make_pair( prev.first / 2, prev.second / 2 ) );
    }
  }

  // Initialize depth map
  MVS::DepthMap map( imgs_dims[ start_scale ].first, imgs_dims[ start_scale ].second );

  map.randomizePlanes( cam, cam.m_min_depth * 0.8, cam.m_max_depth * 1.2, start_scale );
  map.setGroundTruthDepth( cam, params, start_scale );

  // Compute relative motion between current camera and it's neighbors
  std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>> StereoRIG;
  for ( size_t id_neigh = 0; id_neigh < cam.m_view_neighbors.size(); ++id_neigh )
  {
    StereoRIG.emplace_back( MVS::RelativeMotion( cam, cams[ cam.m_view_neighbors[ id_neigh ] ] ) );
  }

  const double             MAX_COST  = MVS::DepthMapComputationParameters::metricMaxCostValue( params.metric() );
  const MVS::ImageLoadType load_type = MVS::ComputeLoadType( params.metric() );

#ifdef USE_OPENCL
  // Build openCL object
  MVS::OpenCLWrapper       clWObject( MVS::OpenCLWrapper::OPENCL_DEVICE_GPU );
  std::vector<std::string> cl_kernel_paths; // /!\ do not change order of the paths
  cl_kernel_paths.push_back( std::string( MVS_BUILD_DIR ) + std::string( "/opencl_common_kernels.cl" ) );
  cl_kernel_paths.push_back( std::string( MVS_BUILD_DIR ) + std::string( "/opencl_metrics_kernels.cl" ) );
  cl_kernel_paths.push_back( std::string( MVS_BUILD_DIR ) + std::string( "/opencl_kernels.cl" ) );

  cl_program cl_pgm                = clWObject.createProgramFromSource( MVS::GetFilesContent( cl_kernel_paths ) );
  cl_kernel  krn_cost_pm           = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_PM" );
  cl_kernel  krn_cost_ncc          = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_NCC" );
  cl_kernel  krn_cost_census       = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_Census" );
  cl_kernel  krn_sum_kernel        = clWObject.getKernelFromName( cl_pgm, "store_costs" );
  cl_kernel  krn_sort_n_store      = clWObject.getKernelFromName( cl_pgm, "sort_and_store_costs" );
  cl_kernel  krn_cost_ncc_red      = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_NCC_red" );
  cl_kernel  krn_cost_ncc_black    = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_NCC_black" );
  cl_kernel  krn_cost_pm_red       = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_PM_red" );
  cl_kernel  krn_cost_pm_black     = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_PM_black" );
  cl_kernel  krn_cost_census_red   = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_Census_red" );
  cl_kernel  krn_cost_census_black = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_Census_black" );
  cl_kernel  krn_update_planes     = clWObject.getKernelFromName( cl_pgm, "update_plane_wrt_cost" );
  cl_kernel  krn_compute_depth     = clWObject.getKernelFromName( cl_pgm, "compute_pixel_depth" );
  cl_kernel  krn_update_planes2    = clWObject.getKernelFromName( cl_pgm, "update_plane_wrt_cost2" );
  cl_kernel  krn_compute_planes    = clWObject.getKernelFromName( cl_pgm, "compute_new_plane" );

  cl_kernel krn_cost_full;
  cl_kernel krn_cost_red;
  cl_kernel krn_cost_black;

  // Get the correct kernels
  switch ( params.metric() )
  {
    case MVS::COST_METRIC_NCC:
    {
      krn_cost_full  = krn_cost_ncc;
      krn_cost_red   = krn_cost_ncc_red;
      krn_cost_black = krn_cost_ncc_black;
      break;
    }
    case MVS::COST_METRIC_PM:
    {
      krn_cost_full  = krn_cost_pm;
      krn_cost_red   = krn_cost_pm_red;
      krn_cost_black = krn_cost_pm_black;
      break;
    }
    case MVS::COST_METRIC_CENSUS:
    {
      krn_cost_full  = krn_cost_census;
      krn_cost_red   = krn_cost_census_red;
      krn_cost_black = krn_cost_census_black;
      break;
    }
  }
#endif

  // One more for the last step
  const int nb_step[] = {4, 3, 3};

  int index = 0;
  for ( int scale = start_scale; scale >= params.scale(); --scale, ++index )
  {
    // Compute depth map at given scale
    std::cout << "Depth map computation at scale : " << scale << std::endl;

    // 0 - Load image and its neighboring images at specified scale
    const MVS::Image                 reference_image = MVS::Image( cam.m_img_path, scale, cam.m_intrinsic, load_type );
    const std::vector<MVS::Image>    neigh_imgs      = LoadNeighborImages( cam, cams, params, scale, load_type );
    const std::vector<MVS::DepthMap> neigh_dms       = LoadNeighborDepthMaps( cam, scale, params );

    // 1 - Compute Initial cost
    auto start_time = std::chrono::high_resolution_clock::now();
#ifdef USE_OPENCL
    ComputeCost( map, cam, cams, StereoRIG, reference_image, neigh_imgs, params, scale, clWObject, krn_cost_full, krn_sum_kernel, krn_sort_n_store );
#else
    ComputeCost( map, cam, cams, StereoRIG, reference_image, neigh_imgs, params, scale );
#endif
    auto end_time = std::chrono::high_resolution_clock::now();

#ifdef EXPORT_INTERMEDIATE_RESULT
    map.exportCost( GetInitCostName( id_cam, scale ) );
    map.exportToGrayscale( GetInitDepthName( id_cam, scale ) );
    map.exportToPly( GetInitPlyName( id_cam, scale ), cam, MAX_COST / 20.0, scale );
    map.exportNormal( GetInitNormalName( id_cam, scale ) );
#endif

    std::cout << " ** Initial cost time : "
              << std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count()
              << " ms "
              << std::endl;

    // 2 - Propagate - Refine
    for ( int id_step = 0; id_step < nb_step[ index ]; ++id_step )
    {
      params.setIterationId( id_step );
      // 2-1 Propagate
      start_time = std::chrono::high_resolution_clock::now();
#ifdef USE_OPENCL
      // Red
      Propagate( map, 0, cam, cams, StereoRIG, reference_image, neigh_imgs, params, scale, clWObject, krn_cost_red, krn_cost_black, krn_sum_kernel, krn_sort_n_store, krn_update_planes, krn_compute_depth );
      // Black
      Propagate( map, 1, cam, cams, StereoRIG, reference_image, neigh_imgs, params, scale, clWObject, krn_cost_red, krn_cost_black, krn_sum_kernel, krn_sort_n_store, krn_update_planes, krn_compute_depth );
#else
      Propagate( map, 0, cam, cams, StereoRIG, reference_image, neigh_imgs, neigh_dms, params, scale );
      Propagate( map, 1, cam, cams, StereoRIG, reference_image, neigh_imgs, neigh_dms, params, scale );

#endif
#ifdef EXPORT_INTERMEDIATE_RESULT
      map.exportCost( GetPropagationCostName( id_cam, id_step, scale ) );
      map.exportToGrayscale( GetPropagationDepthName( id_cam, id_step, scale ) );
      map.exportToPly( GetPropagationPlyName( id_cam, id_step, scale ), cam, MAX_COST / 20.0, params.scale() );
      map.exportNormal( GetPropagationNormalName( id_cam, id_step, scale ) );
#endif

      end_time = std::chrono::high_resolution_clock::now();
      std::cout << " ** Propagation " << id_step << " time : "
                << std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count()
                << " ms "
                << std::endl;

      // 2-2 Refine
      start_time = std::chrono::high_resolution_clock::now();
#ifdef USE_OPENCL
      Refinement( map, cam, cams, StereoRIG, reference_image, neigh_imgs, params, scale, clWObject, krn_cost_full, krn_sum_kernel, krn_sort_n_store, krn_update_planes2, krn_compute_planes );
#else
      Refinement( map, cam, cams, StereoRIG, reference_image, neigh_imgs, neigh_dms, params, scale );
#endif
#ifdef EXPORT_INTERMEDIATE_RESULT
      map.exportCost( GetRefinementCostName( id_cam, id_step, scale ) );
      map.exportToGrayscale( GetRefinementDepthName( id_cam, id_step, scale ) );
      map.exportToPly( GetRefinementPlyName( id_cam, id_step, scale ), cam, MAX_COST / 20.0, scale );
      map.exportNormal( GetRefinementNormalName( id_cam, id_step, scale ) );
#endif

      end_time = std::chrono::high_resolution_clock::now();
      std::cout << " ** Refinement " << id_step << " time : "
                << std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count()
                << " ms "
                << std::endl;
    }

    // 3 - Upscale
    if ( scale != params.scale() )
    {
      map = map.upscale( imgs_dims[ scale - 1 ].first, imgs_dims[ scale - 1 ].second );
    }
  }

  map.filterDepthRange( cam.m_min_depth * 0.81, cam.m_max_depth * 1.19 );

  // Filter at the end
//  map = map.medianFilter( cam, 3, 3, params.scale() );
#ifdef EXPORT_INTERMEDIATE_RESULT
  map.exportCost( GetFinalCostName( id_cam ) );
  map.exportToGrayscale( GetFinalDepthName( id_cam ) );
  map.exportToPly( GetFinalPlyName( id_cam ), cam, MAX_COST / 20.0, params.scale() );
  map.exportNormal( GetFinalNormalName( id_cam ) );
#endif

  // Save the depth map
  map.save( out_path );
}

/**
* @brief Compute depth map for a specified camera
* @param cam Reference camera
* @param cams Neighboring cameras
* @param params Computation parameters
* @param image_ref Reference image data
* @param out_path Path where the computed depth map should be saved
*/
void ComputeDepthMap( const int                           id_cam,
                      MVS::Camera&                        cam,
                      const std::vector<MVS::Camera>&     cams,
                      MVS::DepthMapComputationParameters& params,
                      const MVS::Image&                   image_ref,
                      const std::string&                  out_path )
{
  // Build openCL object
#ifdef USE_OPENCL
  MVS::OpenCLWrapper       clWObject( MVS::OpenCLWrapper::OPENCL_DEVICE_GPU );
  std::vector<std::string> cl_kernel_paths; // /!\ do not change order of the paths
  cl_kernel_paths.push_back( std::string( MVS_BUILD_DIR ) + std::string( "/opencl_common_kernels.cl" ) );
  cl_kernel_paths.push_back( std::string( MVS_BUILD_DIR ) + std::string( "/opencl_metrics_kernels.cl" ) );
  cl_kernel_paths.push_back( std::string( MVS_BUILD_DIR ) + std::string( "/opencl_kernels.cl" ) );

  cl_program cl_pgm           = clWObject.createProgramFromSource( MVS::GetFilesContent( cl_kernel_paths ) );
  cl_kernel  krn_sum_kernel   = clWObject.getKernelFromName( cl_pgm, "store_costs" );
  cl_kernel  krn_sort_n_store = clWObject.getKernelFromName( cl_pgm, "sort_and_store_costs" );
  // NCC
  cl_kernel krn_cost_ncc       = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_NCC" );
  cl_kernel krn_cost_ncc_red   = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_NCC_red" );
  cl_kernel krn_cost_ncc_black = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_NCC_black" );
  // Patch match
  cl_kernel krn_cost_pm       = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_PM" );
  cl_kernel krn_cost_pm_red   = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_PM_red" );
  cl_kernel krn_cost_pm_black = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_PM_black" );
  // Census
  cl_kernel krn_cost_census       = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_Census" );
  cl_kernel krn_cost_census_red   = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_Census_red" );
  cl_kernel krn_cost_census_black = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_Census_black" );
  // Bilateral NCC
  cl_kernel krn_cost_bilateral_ncc       = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_Bilateral_NCC" );
  cl_kernel krn_cost_bilateral_ncc_red   = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_Bilateral_NCC_red" );
  cl_kernel krn_cost_bilateral_ncc_black = clWObject.getKernelFromName( cl_pgm, "compute_pixel_cost_Bilateral_NCC_black" );

  cl_kernel krn_update_planes  = clWObject.getKernelFromName( cl_pgm, "update_plane_wrt_cost" );
  cl_kernel krn_compute_depth  = clWObject.getKernelFromName( cl_pgm, "compute_pixel_depth" );
  cl_kernel krn_update_planes2 = clWObject.getKernelFromName( cl_pgm, "update_plane_wrt_cost2" );
  cl_kernel krn_compute_planes = clWObject.getKernelFromName( cl_pgm, "compute_new_plane" );

  cl_kernel krn_cost_full;
  cl_kernel krn_cost_red;
  cl_kernel krn_cost_black;

  // Get the correct kernels
  switch ( params.metric() )
  {
    case MVS::COST_METRIC_NCC:
    {
      krn_cost_full  = krn_cost_ncc;
      krn_cost_red   = krn_cost_ncc_red;
      krn_cost_black = krn_cost_ncc_black;
      break;
    }
    case MVS::COST_METRIC_PM:
    {
      krn_cost_full  = krn_cost_pm;
      krn_cost_red   = krn_cost_pm_red;
      krn_cost_black = krn_cost_pm_black;
      break;
    }
    case MVS::COST_METRIC_CENSUS:
    {
      krn_cost_full  = krn_cost_census;
      krn_cost_red   = krn_cost_census_red;
      krn_cost_black = krn_cost_census_black;
      break;
    }
    case MVS::COST_METRIC_BILATERAL_NCC:
    {
      krn_cost_full  = krn_cost_bilateral_ncc;
      krn_cost_red   = krn_cost_bilateral_ncc_red;
      krn_cost_black = krn_cost_bilateral_ncc_black;
    }
  }
#else
  const MVS::ImageLoadType load_type = ComputeLoadType( params.metric() );
  const std::vector<MVS::Image> neigh_imgs = LoadNeighborImages( cam, params, load_type );
  const std::vector<MVS::DepthMap> neigh_dms = LoadNeighborDepthMaps( cam, params.scale(), params );

#endif

  const double MAX_COST = MVS::DepthMapComputationParameters::metricMaxCostValue( params.metric() );

  // Compute relative motion between current camera and it's neighbors
  std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>> StereoRIG;
  for ( const size_t neighbor_id : cam.m_view_neighbors )
  {
    StereoRIG.emplace_back( MVS::RelativeMotion( cam, cams[ neighbor_id ] ) );
  }

  // Initialize depth map
  MVS::DepthMap map( cam.m_cam_dims.second, cam.m_cam_dims.first );

  map.randomizePlanes( cam, cam.m_min_depth * 0.8, cam.m_max_depth * 1.2, params.scale() );
  map.setGroundTruthDepth( cam, params, params.scale() );

  auto start_time = std::chrono::high_resolution_clock::now();
  // Compute initial cost
#ifdef USE_OPENCL
  ComputeCost( map, cam, cams, StereoRIG, image_ref, params, clWObject, krn_cost_full, krn_sum_kernel, krn_sort_n_store );
#else
  ComputeCost( map, cam, cams, StereoRIG, image_ref, neigh_imgs, params, params.scale() );
#endif
  auto end_time = std::chrono::high_resolution_clock::now();

  std::cout << "Initial cost time : " << std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count() << " ms " << std::endl;

#ifdef EXPORT_INTERMEDIATE_RESULT
  map.exportToGrayscale( GetInitDepthName( id_cam, params.scale() ) );
  map.exportToPly( GetInitPlyName( id_cam, params.scale() ), cam, MAX_COST / 20.0, params.scale() );
  map.exportCost( GetInitCostName( id_cam, params.scale() ) );
  map.exportNormal( GetInitNormalName( id_cam, params.scale() ) );
#endif

  int nb_iteration = 6;
  for ( int id_iteration = 0; id_iteration < nb_iteration; ++id_iteration )
  {
    params.setIterationId( id_iteration );
    std::stringstream str, str2, strply, strplyspa, strcost, strcostref, strnor, strnorref;

    // 1st : Propagation
    start_time = std::chrono::high_resolution_clock::now();
#ifdef USE_OPENCL
    // Red
    Propagate( map, 0, cam, cams, StereoRIG, image_ref, params, clWObject, krn_cost_red, krn_cost_black, krn_sum_kernel, krn_sort_n_store, krn_update_planes, krn_compute_depth );
    // Black
    Propagate( map, 1, cam, cams, StereoRIG, image_ref, params, clWObject, krn_cost_red, krn_cost_black, krn_sum_kernel, krn_sort_n_store, krn_update_planes, krn_compute_depth );
#else
    // Red
    Propagate( map, 0, cam, cams, StereoRIG, image_ref, neigh_imgs, neigh_dms, params, params.scale() );
    // Black
    Propagate( map, 1, cam, cams, StereoRIG, image_ref, neigh_imgs, neigh_dms, params, params.scale() );
#endif

    end_time = std::chrono::high_resolution_clock::now();
    std::cout << "Propagation time : " << std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count() << " ms " << std::endl;

#ifdef EXPORT_INTERMEDIATE_RESULT
    map.exportToGrayscale( GetPropagationDepthName( id_cam, id_iteration, params.scale() ) );
    map.exportToPly( GetPropagationPlyName( id_cam, id_iteration, params.scale() ), cam, MAX_COST / 20.0, params.scale() );
    map.exportCost( GetPropagationCostName( id_cam, id_iteration, params.scale() ) );
    map.exportNormal( GetPropagationNormalName( id_cam, id_iteration, params.scale() ) );
#endif

    // Make perturbation and update cost
    start_time = std::chrono::high_resolution_clock::now();
#ifdef USE_OPENCL
    Refinement( map, cam, cams, StereoRIG, image_ref, params, clWObject, krn_cost_full, krn_sum_kernel, krn_sort_n_store, krn_update_planes2, krn_compute_planes );
#else
    Refinement( map, cam, cams, StereoRIG, image_ref, neigh_imgs, neigh_dms, params, params.scale() );
#endif
    end_time = std::chrono::high_resolution_clock::now();

    std::cout << "Refinement time : " << std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count() << " ms " << std::endl;

#ifdef EXPORT_INTERMEDIATE_RESULT
    map.exportToGrayscale( GetRefinementDepthName( id_cam, id_iteration, params.scale() ) );
    map.exportToPly( GetRefinementPlyName( id_cam, id_iteration, params.scale() ), cam, MAX_COST / 20.0, params.scale() );
    map.exportCost( GetRefinementCostName( id_cam, id_iteration, params.scale() ) );
    map.exportNormal( GetRefinementNormalName( id_cam, id_iteration, params.scale() ) );
#endif
  }

  std::cout << "Post filtering" << std::endl;

  map.filterDepthRange( cam.m_min_depth * 0.81, cam.m_max_depth * 1.19 );

#ifdef EXPORT_INTERMEDIATE_RESULT
  map.exportCost( GetFinalCostName( id_cam ) );
  map.exportToGrayscale( GetFinalDepthName( id_cam ) );
  map.exportToPly( GetFinalPlyName( id_cam ), cam, MAX_COST / 20.0, params.scale() );
  map.exportNormal( GetFinalNormalName( id_cam ) );
#endif

  // Now save the depth map
  map.save( out_path );
}

// -i sfm_data
// -o MVS directory

// For each images
//  -> Compute it's neighborhoud
//  -> Compute it's depth value using PM

// PM
// For all cameras, compute homography between ref and cam I
// -> Use RelativeCameraMotion to compute R and t from ref to I
// -> The homography, given a plane defined by d and n
// -> H = Ki ( R - 1/d n^T ) Kr^{-1}

int main( int argc, char** argv )
{
  CmdLine cmd;

  std::string sSfM_Data_Filename;
  std::string sOutDir     = "";
  std::string sCostMetric = "";

  int                    kScale              = 1;                                 // Divide image by 2^scale
  double                 kTauCol             = 10.0;                              // threshold on color
  double                 kTauGrad            = 2.0;                               // threshold on gradient
  double                 kAlpha              = 0.9;                               // Balance between color and gradient (90% on grad ; 10% on color)
  double                 kGamma              = 10.0;                              // Gaussian factor used to weight far samples
  bool                   kForceOverwrite     = false;                             //
  double                 kMinAngleSelection  = 5.0;                               // Minimum view angle for elements
  double                 kMaxAngleSelection  = 60.0;                              // Maximum view angle for elements
  int                    kMaxViewSelectionNb = 9;                                 // Maximum of neighbors for view selection
  int                    kMaxViewPerCost     = 4;                                 //
  MVS::PropagationScheme kScheme             = MVS::PROPAGATION_SCHEME_ASYMETRIC; //MVS::PROPAGATION_SCHEME_FULL;
  bool                   kUseJointView       = false;                             // false;
  std::string            sPropagationScheme  = "FIXED_FULL";                      // FIXED_SPEED - ASYMETRIC

  MVS::cost_metric kMetric = MVS::COST_METRIC_NCC;

  cmd.add( make_option( 'i', sSfM_Data_Filename, "input_file" ) );
  cmd.add( make_option( 'o', sOutDir, "outdir" ) );
  cmd.add( make_option( 'c', sCostMetric, "metric" ) );
  cmd.add( make_option( 's', kScale, "scale" ) );
  cmd.add( make_option( 'a', kAlpha, "alpha" ) );
  cmd.add( make_option( 't', kTauCol, "thresholdIntensity" ) );
  cmd.add( make_option( 'g', kTauGrad, "thresholdGradient" ) );
  cmd.add( make_option( 'f', kForceOverwrite, "forceOverwrite" ) );
  cmd.add( make_option( 'm', kMinAngleSelection, "minAngleSelection" ) );
  cmd.add( make_option( 'M', kMaxAngleSelection, "maxAngleSelection" ) );
  //  cmd.add( make_option( 's', kMaxViewSelectionNb, "maxViewSelectionNb" ) );
  cmd.add( make_option( 'k', kMaxViewPerCost, "maxImageForCost" ) );
  cmd.add( make_option( 'y', kGamma, "gamma" ) );
  cmd.add( make_option( 'p', sPropagationScheme, "propagationScheme" ) );
  cmd.add( make_option( 'j', kUseJointView, "jointViewSelection" ) );

  cmd.process( argc, argv );

  std::cerr << "metric : " << sCostMetric << std::endl;

  // Cost metric
  if ( MVS::to_lower( sCostMetric ) == "ncc" )
  {
    kMetric = MVS::COST_METRIC_NCC;
  }
  else if ( MVS::to_lower( sCostMetric ) == "pm" )
  {
    kMetric = MVS::COST_METRIC_PM;
  }
  else if ( MVS::to_lower( sCostMetric ) == "census" )
  {
    kMetric = MVS::COST_METRIC_CENSUS;
  }
  else if ( MVS::to_lower( sCostMetric ) == "daisy" )
  {
    kMetric = MVS::COST_METRIC_DAISY;
  }
  else if ( MVS::to_lower( sCostMetric ) == "bilateral_ncc" )
  {
    kMetric = MVS::COST_METRIC_BILATERAL_NCC;
  }
  else
  {
    std::cerr << "Unknown metric" << std::endl;
    std::cerr << "Switch back to NCC metric " << std::endl;
  }

  // Sampling scheme
  if ( MVS::to_lower( sPropagationScheme ) == "fixed_full" )
  {
    kScheme = MVS::PROPAGATION_SCHEME_FULL;
  }
  else if ( MVS::to_lower( sPropagationScheme ) == "fixed_speed" )
  {
    kScheme = MVS::PROPAGATION_SCHEME_SPEED;
  }
  else if ( MVS::to_lower( sPropagationScheme ) == "dynamic_asymetric" )
  {
    kScheme = MVS::PROPAGATION_SCHEME_ASYMETRIC;
  }
  else
  {
    std::cerr << "Unknown propagation scheme" << std::endl;
    std::cerr << "Switch back to DYNAMIC_ASYMETRIC" << std::endl;
  }

  std::cout << "You called " << std::endl;
  std::cout << "input                    : " << sSfM_Data_Filename << std::endl;
  std::cout << "outdir                   : " << sOutDir << std::endl;
  std::cout << "scale                    : " << kScale << std::endl;
  std::cout << "metric                   : " << to_string( kMetric ) << std::endl;
  std::cout << "Propagation scheme       : " << to_string( kScheme ) << std::endl;
  std::cout << "Use joint view selection : " << ( kUseJointView ? "yes" : "no" ) << std::endl;
  std::cout << "alpha                    : " << kAlpha << std::endl;
  std::cout << "Tau I                    : " << kTauCol << std::endl;
  std::cout << "Tau G                    : " << kTauGrad << std::endl;
  std::cout << "Gamma                    : " << kGamma << std::endl;
  std::cout << "Min angle                : " << kMinAngleSelection << std::endl;
  std::cout << "Max angle                : " << kMaxAngleSelection << std::endl;
  std::cout << "Max neighbor (S)         : " << kMaxViewSelectionNb << std::endl;
  std::cout << "Max view for cost (K)    : " << kMaxViewPerCost << std::endl;

  MVS::DepthMapComputationParameters params( kScale, kMetric, kAlpha, kTauCol, kTauGrad, kGamma, kScheme, kMinAngleSelection, kMaxAngleSelection, kMaxViewSelectionNb, kMaxViewPerCost, sOutDir );
  params.setUseJointViewSelection( kUseJointView );

  // Load the SfM data
  openMVG::sfm::SfM_Data sfm_data;
  if ( !Load( sfm_data, sSfM_Data_Filename, openMVG::sfm::ESfM_Data( openMVG::sfm::ALL ) ) )
  {
    std::cerr << std::endl;
    std::cerr << "The input SfM_Data file \"" << sSfM_Data_Filename << "\" cannot be read." << std::endl;
    return EXIT_FAILURE;
  }

  // Load the cameras from sfm_data
  std::vector<MVS::Camera> cams = MVS::LoadCameras( sfm_data, params );

  // Prepare the output folder
  PrepareOutputDirectory( cams, params );

  // Compute all depth maps
  for ( size_t id_cam = 0; id_cam < cams.size(); ++id_cam )
  {
    // Get path for current objets
    const std::string cur_depth_path = params.getDepthPath( id_cam, params.scale() );
    const std::string cur_cam_path   = params.getCameraPath( id_cam );
    const std::string color_path     = params.getColorPath( id_cam );
    const std::string grayscale_path = params.getGrayscalePath( id_cam );
    const std::string gradient_path  = params.getGradientPath( id_cam );
    const std::string census_path    = params.getCensusPath( id_cam );

    cams[ id_cam ].save( cur_cam_path );

    if ( !stlplus::file_exists( cur_depth_path ) || kForceOverwrite )
    {
      std::cout << "Compute Depth for camera : " << id_cam << std::endl;

#ifdef MULTISCALE
      ComputeMultipleScaleDepthMap( id_cam, cams[ id_cam ], cams, params, params.scale() + 2, cur_depth_path );
#else
      const MVS::ImageLoadType load_type = MVS::ComputeLoadType( params.metric() );
      const MVS::Image cur_image( color_path, grayscale_path, gradient_path, census_path, load_type );
      ComputeDepthMap( id_cam, cams[ id_cam ], cams, params, cur_image, cur_depth_path );
#endif
    }
    else
    {
      std::cout << "Skipping depth map for cam " << id_cam << " : already computed" << std::endl;
    }
  }

  return EXIT_SUCCESS;
}