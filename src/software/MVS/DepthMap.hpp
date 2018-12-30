#ifndef _OPENMVG_MVS_DEPTH_MAP_HPP_
#define _OPENMVG_MVS_DEPTH_MAP_HPP_

#include "Camera.hpp"

#include "openMVG/image/image_container.hpp"
#include "openMVG/numeric/numeric.h"

#include <string>

namespace MVS
{
/**
* @brief Structure holding a depth map with their associated normal
* @note depth and normals are relative to the current view (they must be back projected to have global frame in the fusion step)
*/
struct DepthMap
{
public:
  /**
    * @brief Load a depth map from a file
    * @param path Path of the depth map to load
    */
  DepthMap( const std::string& path );

  /**
    * @brief Create an empty depth map
    * @param height Height of the image
    * @param width Width of the image
    * @param depth Depth initialization
    * @param nor Normal initialization
    */
  DepthMap( const int height, const int width, const double depth = 1.0, const openMVG::Vec4& pl = openMVG::Vec4( 0, 0, 1, 1 ) );

  /**
    * @brief Copy ctr
    * @param src Source
    */
  DepthMap( const DepthMap& src ) = default;
  /**
    * @brief Move ctr
    * @param src Source
    */
  DepthMap( DepthMap&& src ) = default;

  /**
    * @brief Assignment operator
    * @param src Source
    * @return self after assignement
    */
  DepthMap& operator=( const DepthMap& src ) = default;
  /**
    * @brief Move assignment operator
    * @param src Source
    * @return self after assignement
    */
  DepthMap& operator=( DepthMap&& src ) = default;

  /**
    * @brief Get matching cost at specified position
    * @param id_row Row index
    * @param id_col Column index
    * @return Matching cost at specified position
    */
  double cost( const int id_row, const int id_col ) const;

  /**
    * @brief Get matching cost at specified position
    * @param pos Requested position (y,x)
    * @return Matching cost at specified position
    */
  double cost( const openMVG::Vec2i& pos ) const;

  /**
    * @brief Set matching cost at specified position
    * @param id_row Row index
    * @param id_col Column index
    * @param new_cost New matching cost
    */
  void cost( const int id_row, const int id_col, const double new_cost );

  /**
    * @brief Set matching cost at specified position
    * @param pos Requested position (y,x)
    * @param new_cost New matching cost
    */
  void cost( const openMVG::Vec2i& pos, const double new_cost );

  /**
    * @brief Get depth value at specified position
    * @param id_row Index of the row
    * @param id_col Inex of the column
    * @return depth at specified position
    */
  double depth( const int id_row, const int id_col ) const;

  /**
    * @brief Get depth value at specified position
    * @param pos Requested position (y,x)
    * @return depth at specified position
    */
  double depth( const openMVG::Vec2i& pos ) const;

  /**
    * @brief Set Depth value at specified position
    * @param id_row Row index
    * @param id_col Column index
    * @param new_depth New depth value
    */
  void depth( const int id_row, const int id_col, const double new_depth );

  /**
    * @brief Set Depth value at specified position
    * @param pos Position index (y,x)
    * @param new_depth New depth value
    */
  void depth( const openMVG::Vec2i& pos, const double new_depth );

  /**
    * @brief Apply randomization on normals
    * @param cam    Camera associated with the depth
    * @param d_min  Minimum depth
    * @param d_max  Maximum depth
    * @param scale  Scale of the computation
    */
  void randomizePlanes( const Camera& cam, const double d_min, const double d_max, const int scale );

  /**
    * @brief Get plane at specified position
    * @param id_row Index of the row
    * @param id_col Inex of the column
    * @return plane at specified position
    */
  openMVG::Vec4 plane( const int id_row, const int id_col ) const;

  /**
    * @brief Get plane at specified position
    * @param pos Requested position (y,x)
    * @return plane at specified position
    */
  openMVG::Vec4 plane( const openMVG::Vec2i& pos ) const;

  /**
    * @brief Set plane
    * @param id_row Row index
    * @param id_col Column index
    * @param new_plane The new plane (Xi,n)
    */
  void plane( const int id_row, const int id_col, const openMVG::Vec4& new_plane );

  /**
    * @brief Set plane
    * @param pos Requested position (y,x)
    * @param new_plane The new plane
    */
  void plane( const openMVG::Vec2i& pos, const openMVG::Vec4& new_normal );

  /**
   * @brief Set best view for selected pixel 
   * 
   * @param id_row Row index
   * @param id_col Column index
   * 
   * @param best_view The new best view 
   */
  void bestView( const int id_row, const int id_col, const int best_view );

  /**
   * @brief Get best view for selected pixel 
   * 
   * @param id_row Row index
   * @param id_col Column index
   * 
   * @return the best view for the given pixel 
   */
  int bestView( const int id_row, const int id_col ) const;

  /**
    * @brief Indicate if a point is inside the image
    * @param id_row Row index
    * @param id_col Col index
    * @retval true if the point is inside
    * @retval false if the point is outside
    */
  bool
  inside( const int id_row, const int id_col ) const;

  /**
    * @brief Save depth map to a file
    * @param path Path for the output file
    * @retval true If success
    * @retval false If failure
    */
  bool save( const std::string& path ) const;

  /**
    * @brief Load a depth map from a file
    * @param path Path for the input file
    * @retval true If success
    * @retval false If failure
    */
  bool load( const std::string& path );

  /**
    * @brief Get width of the dm
    * @return width
    */
  int width( void ) const;

  /**
    * @brieg Get height of the dm
    * @return heigth
    */
  int height( void ) const;

  /**
    * @brief Get reference of the plane
    * @return Reference to the planes
    */
  const openMVG::image::Image<openMVG::Vec4>& planes( void ) const;

  /**
    * @brief Export to grayscale depth value
    * @param path Export path
    */
  void exportToGrayscale( const std::string& path ) const;

  /**
    * @brief Export to grayscale cost value
    * @param path Export path
    */
  void exportCost( const std::string& path ) const;

  /**
    * @brief Export to RGB file normal value
    * @param path Export path
    */
  void exportNormal( const std::string& path ) const;

  /**
    * @brief Export depth map as a ply point cloud
    * @param path Path of the file to export
    * @param cam The camera used to compute point position
    * @param cost_threshold Threshold to remove some points (point with cost above are discarted)
    */
  void exportToPly( const std::string& path, const Camera& cam, const double cost_threshold, const int scale );

  /**
    * @brief Set ground truth depth for known points
    * @param cam Camera in which ground truth are known
    */
  void setGroundTruthDepth( const Camera& cam, const DepthMapComputationParameters& params, const int scale );

  /**
    * @brief Scale up depth map
    * @param target_width Target width (may not be half size but half size / 2)
    * @param target_height Target height
    * @return A scaled up depth map
    * @note Unknown Depth values are interpolated using neighboring depth values
    */
  DepthMap upscale( const int target_height, const int target_width ) const;

  DepthMap medianFilter( const Camera& cam, const int x_size, const int y_size, const int scale ) const;

private:
  // Matching cost
  openMVG::image::Image<double> m_cost;

  // Depth value
  openMVG::image::Image<double> m_depth;

  // Plane ( n , d )
  openMVG::image::Image<openMVG::Vec4> m_plane;

  // The current best important view index (relative to the neighbors list)
  openMVG::image::Image<int> m_most_important_view;
};

} // namespace MVS

#endif
