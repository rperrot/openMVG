#ifndef _OPENMVG_MVS_DEPTH_MAP_COMPUTATION_CPU_HPP_
#define _OPENMVG_MVS_DEPTH_MAP_COMPUTATION_CPU_HPP_

#include "Camera.hpp"
#include "CostMetricFunctors.hpp"
#include "DepthMap.hpp"
#include "DepthMapComputationParameters.hpp"
#include "Image.hpp"

namespace MVS
{

/**
* @brief compute cost between two images
* @param[out] cost Cost for all pixels
* @param planes The planes per pixel
* @param reference_cam First view camera
* @param other_cam Second view camera
* @param stereo_rig Stereo parameters (Rotation, translation) motion from first to second view
* @param image_ref Image data of the first view
* @param image_other Image data of the second view
* @param params Computation parameters
* @param scale Optionnal scale of the computation (if not specified , used the user specified resolution)
*/
void ComputeImagePairCost( openMVG::image::Image<double>&                 cost,
                           const openMVG::image::Image<openMVG::Vec4>&    planes,
                           const Camera&                                  reference_cam,
                           const Camera&                                  other_cam,
                           const std::pair<openMVG::Mat3, openMVG::Vec3>& stereo_rig,
                           const Image&                                   image_ref,
                           const Image&                                   image_other,
                           const DepthMapComputationParameters&           params,
                           std::shared_ptr<AbstractCostMetric>            cost_metric,
                           const int                                      scale = -1 );

/**
* @brief Compute cost (of all pixels) using all neighboring images
* @param[out] Final cost value for all pixels
* @param planes The planes parameters for each pixel
* @param reference_cam The reference view camera
* @param cams Array of all cameras
* @param stereo_rig Array of all stereo rigs between reference and it's neighboring
* @param image_ref Image data of the reference view
* @param neigh_imgs Neighboring images
* @param params Computation parameters
* @param scale Optionnal scale of the computation (if not specified , used the user specified resolution)
*/
void ComputeMultipleViewCost( openMVG::image::Image<double>&                              cost,
                              const openMVG::image::Image<openMVG::Vec4>&                 planes,
                              const Camera&                                               reference_cam,
                              const std::vector<Camera>&                                  cams,
                              const std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>>& stereo_rig,
                              const Image&                                                image_ref,
                              const std::vector<Image>&                                   neigh_imgs,
                              const DepthMapComputationParameters&                        params,
                              const int                                                   scale = -1 );

/**
* @brief Compute multiple view cost for specified pixel
* @param id_row Y-coordinate of the pixel
* @param id_col X-coordinate of the pixel
* @param cur_normal Normal of the plane
* @param cur_d Plane parameter
* @param cams Array of all neighboring cameras
* @param stereo_rig Stereo motion wrt all neighboring cameras
* @param image_ref Image data of the reference view
* @param neigh_imgs Image data of all the neighbors of the camera
* @param params Computation parameters
* @return Multiple view cost for the specified pixel
* @param scale Optionnal scale of the computation (if not specified , used the user specified resolution)
*/
double ComputeMultiViewCost( const int id_row, const int id_col,
                             const openMVG::Vec3&                                        cur_normal, // Normal parameter
                             const double&                                               cur_d,      // Plane d
                             const Camera&                                               reference_cam,
                             const std::vector<Camera>&                                  cams,
                             const std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>>& stereo_rig,
                             const Image&                                                image_ref,
                             const std::vector<Image>&                                   neigh_imgs,
                             const DepthMapComputationParameters&                        params,
                             std::vector<std::shared_ptr<AbstractCostMetric>>            cost_metrics,
                             const int                                                   scale = -1 );

/**
* @brief Compute multiple hypothesis cost matrix for a specified pixel 
* @param[out] res result of the computation 
* @param id_row Y-coordinate of the pixel
* @param id_col X-coordinate of the pixel
* @param hypothesis Hypotheses for which we want to compute cost
* @param cams Array of all neighboring cameras
* @param stereo_rig Stereo motion wrt all neighboring cameras
* @param image_ref Image data of the reference view
* @param neigh_imgs Image data of all the neighbors of the camera
* @param params Computation parameters
* @param cos_metrics Metrics for cost computation pair 
* @return Multiple view cost for the specified pixel
* @param scale Optionnal scale of the computation (if not specified , used the user specified resolution)
*
* @note result matrix has n-hypt rows and n-cams cols 
*/
void ComputeMultiViewCostMatrix( openMVG::Mat&                                               res,
                                 const int                                                   id_row,
                                 const int                                                   id_col,
                                 const std::vector<openMVG::Vec4>&                           hypotheses, // Plane normal, Plane d
                                 const Camera&                                               reference_cam,
                                 const std::vector<Camera>&                                  cams,
                                 const std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>>& stereo_rig,
                                 const Image&                                                image_ref,
                                 const std::vector<Image>&                                   neigh_imgs,
                                 const DepthMapComputationParameters&                        params,
                                 std::vector<std::shared_ptr<AbstractCostMetric>>            cost_metrics,
                                 const int                                                   scale = -1 );

/**
* @brief Compute initial cost at a specific scale
* @param map The depth map of the reference image
* @param reference_cam The reference view camera
* @param cams Array of neighboring cameras
* @param stereo_rig Array of all motions between reference and it's neighbors
* @param image_ref Reference image
* @param neigh_imgs Neighboring images
* @param params Computation parameters
* @param scale Scale of the computation
*/
void ComputeCost( DepthMap&                                                   map,
                  const Camera&                                               reference_cam,
                  const std::vector<Camera>&                                  cams,
                  const std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>>& stereo_rig,
                  const Image&                                                image_ref,
                  const std::vector<Image>&                                   neigh_imgs,
                  const DepthMapComputationParameters&                        params,
                  const int                                                   scale = -1 );

/**
* @brief Perform propagation using Red or Black scheme at specific scale
* @param[in,out] map The depth map to optimize
* @param id_start 0 if propagate Red , 1 if propagate Black
* @param cam Reference camera
* @param cams Neighboring cameras
* @param stereo_rig Array of motion between reference and it's neighbors
* @param image_ref Image data of the reference view
* @param params neigh_imgs Neighboring images
* @param params Computation parameters
* @param scale Scale of the computation
*/
void Propagate( DepthMap& map, const int id_start, const Camera& cam, const std::vector<Camera>& cams, const std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>>& stereo_rig, const Image& image_ref, const std::vector<Image>& neigh_imgs, const DepthMapComputationParameters& params, const int scale = -1 );

/**
* @brief Perform plane refinement at specific scale
* @param map Depth map to refine
* @param cam Reference camera
* @param cams Array of all neighboring cameras
* @param stereo_rig Array of motion between reference and its neighbors
* @param image_ref Image data of the reference view
* @param params Computation parameters
*/
void Refinement( DepthMap&                                                   map,
                 const Camera&                                               cam,
                 const std::vector<Camera>&                                  cams,
                 const std::vector<std::pair<openMVG::Mat3, openMVG::Vec3>>& stereo_rig,
                 const Image&                                                image_ref,
                 const std::vector<Image>&                                   neigh_imgs,
                 const DepthMapComputationParameters&                        params,
                 const int                                                   scale = -1 );

} // namespace MVS

#endif