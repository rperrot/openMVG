#ifndef _OPENMVG_MVS_CAMERA_HPP_
#define _OPENMVG_MVS_CAMERA_HPP_

#include "DepthMapComputationParameters.hpp"

#include "openMVG/numeric/numeric.h"
#include "openMVG/sfm/sfm_data.hpp"

#include <random>
#include <utility>

namespace MVS
{
struct Camera
{
  /**
  * @brief Ctr
  */
  Camera( void );

  /**
  * @brief Load camera from serialization file
  * @param path Path where the data are
  */
  Camera( const std::string& path );

  Camera( const Camera& cam ) = default;
  Camera( Camera&& cam )      = default;

  Camera& operator=( const Camera& src ) = default;
  Camera& operator=( Camera&& src ) = default;

  /**
  * @brief Save binary data to a path
  * @param path Path where data is saved
  * @retval true if save is OK
  * @retval false if save has an error
  */
  bool save( const std::string& path );

  /**
  * @brief Load binary data to a path
  * @param path Path where data is saved
  * @retval true if load is OK
  * @retval false if load has an error
  */
  bool load( const std::string& path );

  /**
   * @brief Project a point on the camera.
   * 
   * @param X       3d point (in world coordinate).
   * @param scale   Scale of the computation.
   * 
   * @return (2d) Position on the image plane of the camera. 
   */
  openMVG::Vec2 project( const openMVG::Vec3& X, const int scale ) const;

  /**
  * @brief Get 3d point for a 2d position and it's depth
  * @param x postion in x value
  * @param y position in y value
  * @param depth Depth value
  * @return 3d point at given pixel postion at given depth
  */
  openMVG::Vec3 unProject( const double x, const double y, const double depth, const int scale ) const;

  /**
  * @brief Get 3d point for a 2d position and it's depth
  * @param x postion in x value
  * @param y position in y value
  * @param depth Depth value
  * @return 3d point at given pixel postion at given depth
  * @note this function is like UnProject but works considering camera at origin
  */
  openMVG::Vec3 unProjectLocal( const double x, const double y, const double depth, const int scale ) const;

  /**
   * @brief Transform normal in local frame to normal in global frame 
   * 
   * @return openMVG::Vec3 
   */
  openMVG::Vec3 unProject( const openMVG::Vec3& n, const int scale ) const;

  /**
  * @brief Convert disparity and depth
  * @param d Depth/disparity
  * @return Disparity/Depth
  */
  double depthDisparityConversion( const double d, const int scale ) const;

  /**
  * @brief Convert disparity and depth
  * @param d Depth/Disparity
  * @param baseline Camera baseline
  * @return Disparity/Depth
  */
  double depthDisparityConversion( const double d, const double baseline, const int scale ) const;

  /**
  * @brief Get depth value for given 3d point
  * @param pt Point (in camera frame)
  * @return Depth for given point 
  */
  double localDepth( const openMVG::Vec3& pt, const int scale ) const;

  /**
   * @brief Get depth value 
   * 
   * @param pt        3d point (in global frame)
   * @param scale     Scale of the computation
   * @return double   Depth of the point from the camera 
   */
  double depth( const openMVG::Vec3& pt, const int scale ) const;

  /**
  * @brief Get a 3d point at depth=1
  * @param x pixel x-position
  * @param y pixel y-position
  * @return 3d point width depth = 1
  */
  openMVG::Vec3 get3dPoint( const double x, const double y, const int scale ) const;

  /**
  * @brief Get a view direction through a pixel
  * @param x pixel x-position
  * @param y pixel y-position
  * @return View direction passing through a pixel
  */
  openMVG::Vec3 getViewVector( const double x, const double y, const int scale ) const;

  /**
  * @brief Get intrinsic matrix at a specified scale
  * @param scale The requested scale
  * @return The intrinsic at specified scale
  */
  openMVG::Mat3 getK( const int scale );

  /**
   * @brief Generate a random plane to be viewed from the camera 
   * 
   * @param pl      The new plane 
   * @param id_row  Row 
   * @param id_col  Column
   * @param scale   Scale of the camera 
   * 
   * @return depth used to generate the plane   
   */
  double randomPlane( openMVG::Vec4& pl, const int id_row, const int id_col, const int scale ) const;

  // Intrinsic (at user specific scale)
  openMVG::Mat3 m_K;
  openMVG::Mat3 m_K_inv;

  // Intrinsic with scale
  std::vector<openMVG::Mat3> m_K_scaled;
  std::vector<openMVG::Mat3> m_K_inv_scaled;

  // Rotation
  openMVG::Mat3 m_R;
  // Translation
  openMVG::Vec3 m_t;
  // Center
  openMVG::Vec3 m_C;

  // Projection matrix
  openMVG::Mat34              m_P;
  std::vector<openMVG::Mat34> m_P_scaled;

  // Inverse of rotational part of Projection matrix
  openMVG::Mat3              m_M_inv;
  std::vector<openMVG::Mat3> m_M_inv_scaled;

  // source image
  std::string m_img_path;

  // Intrinsic
  openMVG::cameras::IntrinsicBase* m_intrinsic;

  // Dimensions (width,height) of image
  std::pair<int, int> m_cam_dims;

  // (2D position and 3d point)
  std::vector<std::pair<openMVG::Vec2, openMVG::Vec3>> m_ground_truth;

  // random generators/distrib used to generate planes
  mutable std::mt19937_64 m_rng;

  // Minimum depth value
  double m_min_depth;
  // Maximum depth value
  double m_max_depth;

  /**
  * @brief Compute Ray (origin, direction), given a 2d pixel
  */
  std::pair<openMVG::Vec3, openMVG::Vec3> getRay( const openMVG::Vec2& x, const int scale ) const;

  // baseline
  double              m_min_baseline;
  double              m_max_baseline;
  std::vector<double> m_baseline;
  double              m_mean_baseline;

  // List of neighbors cameras
  std::vector<int> m_view_neighbors;
};

/*
 * @param K Input intrinsic matrix
 * @brief scale factor ( 1 -> No change, else / 2^scale )
 * @param K input intrinsic
 * @param scale Scale factor
 * @return Scaled intrinsic
 */
openMVG::Mat3 ScaleK( const openMVG::Mat3& K, const int scale );

/*
* @brief Compute stereo rig motion, assuming cam1 is the reference
* @param cam1 First camera
* @param cam2 Second camera
* @return Stereo rig from first to second (rotation,translation)
*/
std::pair<openMVG::Mat3, openMVG::Vec3> RelativeMotion( const Camera& cam1, const Camera& cam2 );

/**
* Extract camera informations from sfm_data
* @param sfm_data Input data
* @param params Parameters
*/
std::vector<Camera> LoadCameras( const openMVG::sfm::SfM_Data& sfm_data, const DepthMapComputationParameters& params );

/**
* @brief Compute homography induced by a given stereo rig and a plane
* @param R Rotation from reference (at origin) and the second camera
* @param t Translation vector (from origin to second)
* @param K Intrinsic matrix of origin
* @param Kp Intrinsic matrix of second
* @param n Plane normal
* @param d depth (relative to the origin)
* @return Homography
*/
/*openMVG::Mat3 HomographyTransformation( const openMVG::Mat3& R,
                                        const openMVG::Vec3& t,
                                        const openMVG::Mat3& K,
                                        const openMVG::Mat3& Kp,
                                        const openMVG::Vec3& n,
                                        const double         d );
*/

/**
* @brief Compute homography induced by a given stereo rig and a plane
* @param R Rotation from reference (at origin) and the second camera
* @param t Translation vector (from origin to second)
* @param cam_ref Camera (origin)
* @param cam_other Camera (second)
* @param pl Plane (normal and parameter)
* @param scale Scale of the computation
* @return Homography
*/
openMVG::Mat3 HomographyTransformation( const openMVG::Mat3& R,
                                        const openMVG::Vec3& t,
                                        const MVS::Camera&   cam_ref,
                                        const MVS::Camera&   cam_other,
                                        const openMVG::Vec4& pl,
                                        const int            scale );

/*
* @brief Compute depth value for plane a a specified position
* @param cam Camera
* @param n Plane normal
* @param d Plane parameter
* @param x Pixel x position
* @param y Pixel y position
* @return depth
*/
static inline double DepthFromPlane( const MVS::Camera&   cam,
                                     const openMVG::Vec3& n,
                                     const double         d,
                                     const double         x,
                                     const double         y,
                                     const int            scale )
{
  const openMVG::Mat3& K = ( scale == -1 ) ? cam.m_K : cam.m_K_scaled[ scale ];

  const double fx    = K( 0, 0 );
  const double fy    = K( 1, 1 );
  const double alpha = fx / fy;

  const double u = K( 0, 2 );
  const double v = K( 1, 2 );

  return -d * fx / ( n[ 0 ] * ( x - u ) + n[ 1 ] * ( y - v ) * alpha + n[ 2 ] * fx );
}

/**
* @brief Compute plane parameter
* @param cam Camera
* @param id_row pixel x position
* @param id_col pixel y position
* @param depth Depth value
* @param n Normal
* @return Plane parameter
*/
static inline double GetPlaneD( const MVS::Camera&   cam,
                                const int            id_row,
                                const int            id_col,
                                const double         depth,
                                const openMVG::Vec3& n,
                                const int            scale )
{
  const openMVG::Mat34& P     = ( scale == -1 ) ? cam.m_P : cam.m_P_scaled[ scale ];
  const openMVG::Mat3&  M_inv = ( scale == -1 ) ? cam.m_M_inv : cam.m_M_inv_scaled[ scale ];

  openMVG::Vec3 pt;
  pt[ 0 ] = depth * id_col - P( 0, 3 );
  pt[ 1 ] = depth * id_row - P( 1, 3 );
  pt[ 2 ] = depth - P( 2, 3 );

  openMVG::Vec3 ptX = M_inv * pt;

  return -ptX.dot( n );
}

/**
* @brief Compute depth value at specified pixel position using a plane
* @param plane Plane paramater
* @param id_row Y-coordinate of the pixel
* @param id_col X-coordinate of the pixel
* @param cam Camera
* @return depth value at specified pixel
* @note This computes intersection between ray through the pixel and the plane, then get final depth
*/
double ComputeDepth( const openMVG::Vec4& plane, const int id_row, const int id_col, const Camera& cam, const int scale );

/**
 * @brief Propagate depth from a pixel "from" to an other one named "to" 
 * 
 * @param d_from        Depth at pixel from 
 * @param n_from        Normal at pixel from 
 * @param id_row_from   Y-coord of the pixel from
 * @param id_col_from   X-coord of the pixel from 
 * @param id_row_to     Y-coord of the pixel to 
 * @param id_col_to     X-coord of the pixel to 
 * @param cam           Camera 
 * @param scale         Scale of the camera 
 * 
 * @return depth at pixel to 
 */
double PropagateDepth( const double         d_from,
                       const openMVG::Vec3& n_from,
                       const int            id_row_from,
                       const int            id_col_from,
                       const int            id_row_to,
                       const int            id_col_to,
                       const Camera&        cam,
                       const int            scale );

} // namespace MVS

#endif