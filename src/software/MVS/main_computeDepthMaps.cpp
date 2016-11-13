#include "Camera.hpp"
#include "DepthMap.hpp"
#include "DepthMapComputationCPU.hpp"
#include "DepthMapComputationOpenCL.hpp"
#include "DepthMapComputationParameters.hpp"
#include "Image.hpp"
#include "MatchingCost.hpp"
#include "Util.hpp"

#include "openMVG/sfm/sfm.hpp"
#include "third_party/cmdLine/cmdLine.h"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <iostream>
#include <random>

#define USE_OPENCL
#define EXPORT_INTERMEDIATE_RESULT




/**
* @brief Create directory structure for the project
* @param cams The list of input cameras
* @param params the computation parameters
*/
void PrepareOutputDirectory( const std::vector< MVS::Camera > & cams ,
                             const MVS::DepthMapComputationParameters & params )
{
  const std::string outDirPath = params.WorkingDirectory() ;
  if( ! stlplus::is_folder( outDirPath ) )
  {
    stlplus::folder_create( outDirPath ) ;
    if( ! stlplus::is_folder( outDirPath ) )
    {
      std::cerr << "Could not create output directory" << std::endl ;
      exit( EXIT_FAILURE ) ;
    }
  }

  // Create depth folder
  const std::string depth_folder = params.DepthDirectory() ;
  if( ! stlplus::is_folder( depth_folder ) )
  {
    stlplus::folder_create( depth_folder ) ;
    if( ! stlplus::is_folder( depth_folder ) )
    {
      std::cerr << "Could not create depth folder" << std::endl ;
      exit( EXIT_FAILURE ) ;
    }
  }

  for( size_t id_cam = 0 ; id_cam < cams.size() ; ++id_cam )
  {
    std::stringstream str ;
    str << "cam_" << id_cam ;
    const std::string cam_folder = params.GetCameraDirectory( id_cam ) ;
    if( ! stlplus::is_folder( cam_folder ) )
    {
      stlplus::folder_create( cam_folder ) ;
      if( ! stlplus::is_folder( cam_folder ) )
      {
        std::cerr << "Could not create cam_" << id_cam << std::endl ;
        exit( EXIT_FAILURE ) ;
      }
    }

    // Create images and save it
    MVS::Image cur_img( cams[ id_cam ].m_img_path , params.Scale() , cams[ id_cam ].m_intrinsic ) ;
    // Grayscale path
    const std::string color_path     = params.GetColorPath( id_cam ) ;
    const std::string grayscale_path = params.GetGrayscalePath( id_cam ) ;
    const std::string gradient_path  = params.GetGradientPath( id_cam ) ;

    cur_img.Save( color_path , grayscale_path , gradient_path ) ;
  }
}







/**
* @brief Compute depth map for a specified camera
* @param cam Reference camera
* @param cams Neighboring cameras
* @param params Computation parameters
* @param image_ref Reference image data
* @param out_path Path where the computed depth map should be saved
*/
void ComputeDepthMap( MVS::Camera & cam ,
                      const std::vector< MVS::Camera > & cams ,
                      const MVS::DepthMapComputationParameters & params ,
                      const MVS::Image & image_ref ,
                      const std::string & out_path )
{
  // Build openCL object
  MVS::OpenCLWrapper clWObject( MVS::OpenCLWrapper::OPENCL_DEVICE_GPU ) ;
  std::string cl_kernel_path   = std::string( MVS_BUILD_DIR ) + std::string( "/opencl_kernels.cl" ) ;
  cl_program cl_pgm            = clWObject.CreateProgramFromSource( MVS::GetFileContent( cl_kernel_path ) ) ;
  cl_kernel krn_cost_pm        = clWObject.GetKernelFromName( cl_pgm , "compute_pixel_cost_PM" ) ;
  cl_kernel krn_cost_ncc       = clWObject.GetKernelFromName( cl_pgm , "compute_pixel_cost_NCC" ) ;
  cl_kernel krn_sum_kernel     = clWObject.GetKernelFromName( cl_pgm , "store_costs" ) ;
  cl_kernel krn_sort_n_store   = clWObject.GetKernelFromName( cl_pgm , "sort_and_store_costs" ) ;
  cl_kernel krn_cost_ncc_red   = clWObject.GetKernelFromName( cl_pgm , "compute_pixel_cost_NCC_red" ) ;
  cl_kernel krn_cost_ncc_black = clWObject.GetKernelFromName( cl_pgm , "compute_pixel_cost_NCC_black" ) ;
  cl_kernel krn_cost_pm_red    = clWObject.GetKernelFromName( cl_pgm , "compute_pixel_cost_PM_red" ) ;
  cl_kernel krn_cost_pm_black  = clWObject.GetKernelFromName( cl_pgm , "compute_pixel_cost_PM_black" ) ;
  cl_kernel krn_update_planes  = clWObject.GetKernelFromName( cl_pgm , "update_plane_wrt_cost" ) ;
  cl_kernel krn_compute_depth  = clWObject.GetKernelFromName( cl_pgm , "compute_pixel_depth" ) ;
  cl_kernel krn_update_planes2 = clWObject.GetKernelFromName( cl_pgm , "update_plane_wrt_cost2" ) ;
  cl_kernel krn_compute_planes = clWObject.GetKernelFromName( cl_pgm , "compute_new_plane" ) ;

  cl_kernel krn_cost_full ;
  cl_kernel krn_cost_red ;
  cl_kernel krn_cost_black ;

  double MAX_COST = MVS::DepthMapComputationParameters::MAX_COST_NCC ;
  switch( params.Metric() )
  {
    case MVS::COST_METRIC_NCC :
    {
      krn_cost_full  = krn_cost_ncc ;
      krn_cost_red   = krn_cost_ncc_red ;
      krn_cost_black = krn_cost_ncc_black ;
      MAX_COST = MVS::DepthMapComputationParameters::MAX_COST_NCC ;
      break ;
    }
    case MVS::COST_METRIC_PM :
    {
      krn_cost_full  = krn_cost_pm ;
      krn_cost_red   = krn_cost_pm_red ;
      krn_cost_black = krn_cost_pm_black ;
      MAX_COST = MVS::DepthMapComputationParameters::MAX_COST_PM ;
      break ;
    }
  }

  // Compute relative motion between current camera and it's neighbors
  std::vector<std::pair< openMVG::Mat3 , openMVG::Vec3 >> StereoRIG ;

  for( size_t id_neigh = 0 ; id_neigh < cam.m_view_neighbors.size() ; ++id_neigh )
  {
    StereoRIG.push_back( MVS::RelativeMotion( cam , cams[cam.m_view_neighbors[id_neigh] ] ) );
  }

  // Initialize depth map
  MVS::DepthMap map( cam.m_cam_dims.second , cam.m_cam_dims.first ) ;
  const double min_disparity = cam.DepthDisparityConversion( cam.m_max_depth * 1.2 ) ;
  const double max_disparity = cam.DepthDisparityConversion( cam.m_min_depth * 0.8 ) ;

  map.RandomizePlanes( cam , min_disparity , max_disparity ) ; // Add 30% of the range
  map.SetGroundTruthDepth( cam , params ) ;

  auto start_time = std::chrono::high_resolution_clock::now() ;
  // Compute initial cost
#ifdef USE_OPENCL
  ComputeCost( map , cam , cams , StereoRIG , image_ref , params , clWObject , krn_cost_full , krn_sum_kernel , krn_sort_n_store ) ;
#else
  ComputeCost( map , cam , cams , StereoRIG , image_ref , params ) ;
#endif
  auto end_time = std::chrono::high_resolution_clock::now() ;

  std::cout << "Initial cost time : " << chrono::duration_cast<chrono::milliseconds>( end_time - start_time ).count() << " ms " << std::endl ;

#ifdef EXPORT_INTERMEDIATE_RESULT
  map.ExportToGrayscale( "init.png" ) ;
  map.ExportToPly( "init.ply" , cam , MAX_COST / 10.0 ) ;
  map.ExportCost( "init_cost.png" ) ;
  map.ExportNormal( "init_normal.png" ) ;
#endif

  int nb_iteration = 8 ;
  for( int id_iteration = 0 ; id_iteration < nb_iteration ; ++id_iteration )
  {
    std::stringstream str , str2 , strply , strplyspa , strcost , strcostref , strnor , strnorref ;

    // 1st : Propagation
    start_time = std::chrono::high_resolution_clock::now() ;
#ifdef USE_OPENCL
    // Red
    Propagate( map , 0 , cam , cams , StereoRIG , image_ref , params , clWObject ,
               krn_cost_red , krn_cost_black , krn_sum_kernel ,
               krn_sort_n_store , krn_update_planes , krn_compute_depth ) ;
    // Black
    Propagate( map , 1 , cam , cams , StereoRIG , image_ref , params , clWObject ,
               krn_cost_red , krn_cost_black , krn_sum_kernel ,
               krn_sort_n_store , krn_update_planes , krn_compute_depth ) ;
#else
    // Red
    Propagate( map , 0 , cam , cams , StereoRIG , image_ref , params ) ;
    // Black
    Propagate( map , 1 , cam , cams , StereoRIG , image_ref , params ) ;
#endif

    end_time = std::chrono::high_resolution_clock::now() ;
    std::cout << "Propagation time : " << chrono::duration_cast<chrono::milliseconds>( end_time - start_time ).count() << " ms " << std::endl ;

#ifdef EXPORT_INTERMEDIATE_RESULT
    str << "iter_" << id_iteration << ".png" ;
    strplyspa << "iter_" << id_iteration << ".ply" ;
    strcost << "iter_" << id_iteration << "_cost.png" ;
    strnor << "iter_" << id_iteration << "_nor.png" ;
    map.ExportToGrayscale( str.str() );
    map.ExportToPly( strplyspa.str() , cam , MAX_COST / 10.0 ) ;
    map.ExportCost( strcost.str() ) ;
    map.ExportNormal( strnor.str() ) ;
#endif

    // Make perturbation and update cost
    start_time = std::chrono::high_resolution_clock::now() ;
#ifdef USE_OPENCL
    Refinement( map , cam , cams , StereoRIG , image_ref , params ,
                clWObject , krn_cost_full , krn_sum_kernel , krn_sort_n_store , krn_update_planes2 , krn_compute_planes ) ;
#else

    Refinement( map , cam , cams , StereoRIG , image_ref , params ) ;
#endif
    end_time = std::chrono::high_resolution_clock::now() ;

    std::cout << "Refinement time : " << chrono::duration_cast<chrono::milliseconds>( end_time - start_time ).count() << " ms " << std::endl ;


#ifdef EXPORT_INTERMEDIATE_RESULT
    str2 << "iter_" << id_iteration << "_ref.png" ;
    strply << "iter_" << id_iteration << "_ref.ply" ;
    strcostref << "iter_" << id_iteration << "_cost_ref.png" ;
    strnorref << "iter_" << id_iteration << "_nor_ref.png" ;
    map.ExportToGrayscale( str2.str() ) ;
    map.ExportToPly( strply.str() , cam , MAX_COST / 10.0 ) ;
    map.ExportCost( strcostref.str() ) ;
    map.ExportNormal( strnorref.str() ) ;
#endif
  }

  // Now save the depth map
  map.Save( out_path ) ;
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

int main( int argc , char ** argv )
{
  CmdLine cmd;

  std::string sSfM_Data_Filename ;
  std::string sOutDir = "";
  std::string sCostMetric = "";

  int kScale = 1 ;         // Divide image by 2^scale
  double kTauCol  = 10.0 ; // threshold on color
  double kTauGrad = 2.0 ;  // threshold on gradient
  double kAlpha   = 0.9 ;  // Balance between color and gradient (90% on grad ; 10% on color)
  double kGamma   = 10.0 ; // Gaussian factor used to weight far samples
  bool kForceOverwrite = false ; //
  double kMinAngleSelection = 5.0 ; // Minimum view angle for elements
  double kMaxAngleSelection = 60.0 ; // Maximum view angle for elements
  int kMaxViewSelectionNb = 9 ; // Maximum of neighbors for view selection
  int kMaxViewPerCost = 3 ; //

  MVS::cost_metric kMetric = MVS::COST_METRIC_PM ;

  cmd.add( make_option( 'i', sSfM_Data_Filename, "input_file" ) ) ;
  cmd.add( make_option( 'o', sOutDir, "outdir" ) ) ;
  cmd.add( make_option( 'c', sCostMetric , "metric" ) ) ;
  cmd.add( make_option( 's', kScale , "scale" ) ) ;
  cmd.add( make_option( 'a', kAlpha , "alpha" ) ) ;
  cmd.add( make_option( 't', kTauCol , "thresholdIntensity" ) ) ;
  cmd.add( make_option( 'g', kTauGrad , "thresholdGradient" ) ) ;
  cmd.add( make_option( 'f', kForceOverwrite , "forceOverwrite" ) ) ;
  cmd.add( make_option( 'm', kMinAngleSelection , "minAngleSelection" ) );
  cmd.add( make_option( 'm', kMaxAngleSelection , "maxAngleSelection" ) ) ;
  cmd.add( make_option( 's', kMaxViewSelectionNb , "maxViewSelectionNb" ) ) ;
  cmd.add( make_option( 'k', kMaxViewPerCost , "maxImageForCost" ) ) ;
  cmd.add( make_option( 'y', kGamma , "gamma" ) ) ;

  cmd.process( argc, argv );

  if( sCostMetric == "ncc" || sCostMetric == "NCC" )
  {
    kMetric = MVS::COST_METRIC_NCC ;
  }
  else if( sCostMetric == "pm" || sCostMetric == "PM" )
  {
    kMetric = MVS::COST_METRIC_PM ;
  }
  else
  {
    std::cerr << "Unknown metric (PM of NCC are the only vlid choices)" << std::endl ;
    std::cerr << "Switch back to PM metric " << std::endl ;
  }

  std::cout << "You called " << std::endl ;
  std::cout << "input                   : " << sSfM_Data_Filename << std::endl ;
  std::cout << "outdir                  : " << sOutDir << std::endl ;
  std::cout << "scale                   : " << kScale << std::endl ;
  std::cout << "metric                  : " << ( ( kMetric == MVS::COST_METRIC_PM ) ? "PM " : "NCC" ) << std::endl ;
  std::cout << "alpha                   : " << kAlpha << std::endl ;
  std::cout << "Tau I                   : " << kTauCol << std::endl ;
  std::cout << "Tau G                   : " << kTauGrad << std::endl ;
  std::cout << "Gamma                   : " << kGamma << std::endl ;
  std::cout << "Min angle               : " << kMinAngleSelection << std::endl ;
  std::cout << "Max angle               : " << kMaxAngleSelection << std::endl ;
  std::cout << "Max neighbor (S)        : " << kMaxViewSelectionNb << std::endl ;
  std::cout << "Max view for cost (K)   : " << kMaxViewPerCost << std::endl ;

  MVS::DepthMapComputationParameters params( kScale , kMetric , kAlpha , kTauCol , kTauGrad , kGamma , kMinAngleSelection , kMaxAngleSelection , kMaxViewSelectionNb , kMaxViewPerCost , sOutDir ) ;

  // Load the SfM data
  openMVG::sfm::SfM_Data sfm_data ;
  if ( !Load( sfm_data, sSfM_Data_Filename, openMVG::sfm::ESfM_Data( openMVG::sfm::ALL ) ) )
  {
    std::cerr << std::endl ;
    std::cerr << "The input SfM_Data file \"" << sSfM_Data_Filename << "\" cannot be read." << std::endl;
    return EXIT_FAILURE;
  }

  // Load the cameras from sfm_data
  std::vector< MVS::Camera > cams  = MVS::LoadCameras( sfm_data , params ) ;

  for( size_t id_cam = 0 ; id_cam < cams.size() ; ++id_cam )
  {
    std::cout << "Cam[" << id_cam << "]" << "d -> " << cams[id_cam].m_min_depth << " , " << cams[id_cam].m_max_depth << std::endl ;
  }

  // Prepare the output folder
  PrepareOutputDirectory( cams , params ) ;

  // Compute all depth maps
  for( size_t id_cam = 0 ; id_cam < cams.size() ; ++id_cam )
  {
    // Get path for current objets
    const std::string cur_depth_path = params.GetDepthPath( id_cam ) ;
    const std::string cur_cam_path   = params.GetCameraPath( id_cam ) ;
    const std::string color_path     = params.GetColorPath( id_cam ) ;
    const std::string grayscale_path = params.GetGrayscalePath( id_cam ) ;
    const std::string gradient_path  = params.GetGradientPath( id_cam ) ;

    cams[ id_cam ].Save( cur_cam_path ) ;

    if( ! stlplus::file_exists( cur_depth_path ) || kForceOverwrite )
    {
      std::cout << "Compute Depth for camera : " << id_cam << std::endl ;
      const MVS::Image cur_image( color_path , grayscale_path , gradient_path ) ;
      ComputeDepthMap( cams[ id_cam ] , cams , params , cur_image , cur_depth_path ) ;
    }
    else
    {
      std::cout << "Skipping depth map for cam " << id_cam << " : already computed" << std::endl ;
    }
  }

  return EXIT_SUCCESS ;
}