#ifndef _OPENMVG_MVS_DEPTH_MAP_COMPUTATION_OPENCL_HPP_
#define _OPENMVG_MVS_DEPTH_MAP_COMPUTATION_OPENCL_HPP_

#include "Camera.hpp"
#include "DepthMap.hpp"
#include "Image.hpp"
#include "OpenCLWrapper.hpp"

#include <utility>

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
                      cl_mem & Ip , cl_mem & Gp , cl_mem & Kinv , cl_mem & planes_n , cl_mem & planes_d ,
                      OpenCLWrapper & wrapper ,
                      cl_kernel & cost_kernel_full ,
                      cl_kernel & cost_kernel_red ,
                      cl_kernel & cost_kernel_black ,
                      cl_kernel & append_cost_kernel ,
                      cl_kernel & sort_and_store_cost_kernel ,
                      const std::pair< int , int > & compute_size ,
                      const openMVG::Vec2i & delta_plane = openMVG::Vec2i( 0 , 0 ) ,
                      const cost_topology_type c_type = COST_COMPUTE_FULL ,
                      cl_mem * _all_costs = nullptr ) ;


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
                    cl_kernel sort_and_store_cost_kernel ) ;


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
                  cl_kernel & compute_pixel_depth_kernel ) ; // Compute pixel depth value


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
                   cl_kernel & compute_new_plane_kernel ) ; // Compute new planes

}

#endif