#ifndef _OPENMVG_MVS_DEPTH_MAP_COMPUTATION_PARAMETERS_HPP_
#define _OPENMVG_MVS_DEPTH_MAP_COMPUTATION_PARAMETERS_HPP_

#include <string>

namespace MVS
{

/**
 * @brief Propagation Schemes 
 */
enum PropagationScheme
{
  PROPAGATION_SCHEME_FULL,     // Using 20 neighbors,
  PROPAGATION_SCHEME_SPEED,    // Using 8 neighbors,
  PROPAGATION_SCHEME_ASYMETRIC // Using 8 neighbors choosen from sampling 8 regions.
};

std::string to_string( const PropagationScheme& pscheme );

/**
* @brief Where are the pixel cost computed
*/
enum cost_topology_type
{
  COST_COMPUTE_FULL, // Compute cost for all pixels in the 2d grid
  COST_COMPUTE_RED,  // Only 1 over two (red as in gipuma paper)
  COST_COMPUTE_BLACK // Only 1 over two (complement of red, as in gipuma paper)
};

/**
* @brief Kind of metric to use
*/
enum cost_metric
{
  COST_METRIC_NCC,
  COST_METRIC_PM,
  COST_METRIC_CENSUS,
  COST_METRIC_DAISY,
  COST_METRIC_BILATERAL_NCC
};

std::string to_string( const cost_metric& metric );

struct DepthMapComputationParameters
{
  // Default values for some costs
  static const double MAX_COST_NCC;
  static const double MAX_COST_PM;
  static const double MAX_COST_CENSUS;
  static const double MAX_COST_DAISY;
  static const double MAX_COST_BILATERAL_NCC;

  /**
  * @brief Ctr
  * @param scale Scale
  * @param alpha Balance between intensity (0) and gradient (1)
  * @param tau_i Threshold on intensity difference
  * @param tau_g Threshold on gradient difference
  * @param gamma Gaussian factor used to give less weight to far (in intensity) samples
  * @param pScheme Propagation scheme (as defined in gipuma)
  * @param min_view_angle Minimum view angle for camera selection
  * @param max_view_angle Maximum view angle for camera selection
  * @param max_view_selection_nb Maximum number of view for view selection
  * @param nb_image_for_cost Number of image used for cost computation
  * @param base_path Base working path
  * @note min_view_angle in degree
  * @note max_view_angle in degree
  */
  DepthMapComputationParameters( const int         scale,
                                 const cost_metric metric,
                                 // For PM metric :
                                 const double alpha,
                                 const double tau_i,
                                 const double tau_g,
                                 const double gamma,
                                 // For PM propagation scheme
                                 const PropagationScheme pScheme,
                                 // For view selection
                                 const double min_view_angle,
                                 const double max_view_angle,
                                 const int    max_view_selection_nb,
                                 // For multipleview cost computation
                                 const int nb_image_for_cost,
                                 // For convenience only ,
                                 const std::string base_path );

  /**
  * @brief Copy ctr
  * @param src Source
  */
  DepthMapComputationParameters( const DepthMapComputationParameters& src ) = default;

  /**
  * @brief Move ctr
  * @param src Source
  */
  DepthMapComputationParameters( DepthMapComputationParameters&& src ) = default;

  /**
  * @brief Assignment operator
  * @param src Source
  * @return self after asssignement
  */
  DepthMapComputationParameters& operator=( const DepthMapComputationParameters& src ) = default;

  /**
  * @brief Move Assignment operator
  * @param src Source
  * @return self after asssignement
  */
  DepthMapComputationParameters& operator=( DepthMapComputationParameters&& src ) = default;

  /**
  * @brief Get cost metric used to compute pixel score
  */
  cost_metric metric( void ) const;

  /**
  * @brief Given a metric get the corresponding maximum score
  * @param metric the current metric
  * @return the maximum score for the given metric
  */
  static double metricMaxCostValue( const cost_metric metric );

  /**
  * @brief Get scale of the computation( 0 for original size)
  * @return scale of the computation
  */
  int scale( void ) const;

  /**
  * @brief Get balance value between gradient (1) and intensity (0)
  * @return balance value
  */
  double alpha( void ) const;

  /**
  * @brief Threshold value on intensity difference
  * @return Threshold value on intensity difference
  */
  double tauI( void ) const;

  /**
  * @brief Threshold value on gradient difference
  * @return Threshold value on gradient difference
  */
  double tauG( void ) const;

  /**
  * @brief Gaussian factor used to weight the samples
  * @return gaussian factor
  */
  double gamma( void ) const;

  /**
   * @brief Get Propagation scheme
   * @return Propagation Scheme
   */
  PropagationScheme propagationScheme( void ) const;

  /**
  * @brief Get minimum view angle for view selection
  * @return Minimum view angle for view selection
  * @note Angle in degree
  */
  double minimumViewAngle( void ) const;

  /**
  * @brief Get maximum view angle for view selection
  * @return Maximum view angle for view selection
  * @note Angle in degree
  */
  double maximumViewAngle( void ) const;

  /**
  * @brief Get Number of view for view selection
  * @return Maximum number of view used for view selection
  */
  int nbMaximumViewSelection( void ) const;

  /**
  * @brief Number of camera used for matching cost
  * @param
  */
  int nbMultiViewImageForCost( void ) const;

  /**
  * @brief Get working directory
  * @return working directory
  */
  std::string workingDirectory( void ) const;

  /**
  * @brief Get depth directory in the working directory
  * @return depth directory (containing all cameras)
  */
  std::string depthDirectory( void ) const;

  /**
  * @brief Get Camera directory given it's ID
  * @param ID of the camera to retrieve
  * @return Data directory of the i-th camera
  */
  std::string getCameraDirectory( const int id ) const;

  /**
  * @brief Get color image path
  * @param id Id of the image to get
  * @return Path of the color image for the given camera
  */
  std::string getColorPath( const int id ) const;

  /**
  * @brief Get grayscale image path
  * @param id Id of the image to get
  * @return Path of the grayscale image for the given camera
  */
  std::string getGrayscalePath( const int id ) const;

  /**
  * @brief Get gradient image path
  * @param id Id of the image to get
  * @return Path of the gradient image for the given camera
  */
  std::string getGradientPath( const int id ) const;

  /**
  * @brief Get census image path
  * @param id Id of the image to get
  * @return Path of the census image for the given camera
  */
  std::string getCensusPath( const int id ) const;

  /**
  * @brief Get camera path
  * @param id Id of the camera to get
  * @return Path of the camera file for the given camera
  */
  std::string getCameraPath( const int id ) const;

  /**
  * @brief Get depth image path
  * @param id Id of the depth map to get
  * @return Path of the depth image for the given camera
  */
  std::string getDepthPath( const int id ) const;

  /**
   * @brief Get output model directory
   * @return Path of the output directory
   */
  std::string getModelDirectory( void ) const;

  /**
   * @brief Set the Iteration Id 
   * 
   * @param id New iteration ID
   */
  void setIterationId( const int id );

  /**
   * @brief Get the Iteration Id 
   * 
   * @return the current iteration ID
   */
  int getIterationId( void ) const;

  /**
   * @brief Set the Use Joint View Selection mode (AMHMVS)
   * 
   * @param use Enable/diable joint view selection for unified cost 
   */
  void setUseJointViewSelection( const bool use );

  /**
   * @brief Indicate if Joint view selection must be used 
   * 
   * @return true   Use joint view selection for unified cost 
   * @return false  Use Gipuma multiple view cost 
   */
  bool useJointViewSelection( void ) const;

  int m_iteration; // ID of the iteration

  /// Scale of the computation
  int m_scale;

  /// Metric used for cost computation
  cost_metric m_metric;

  /// Cost balance between intensity and gradient
  double m_alpha;

  /// threshold on intensity difference
  double m_tau_i;

  /// threshold on gradient difference
  double m_tau_g;

  /// Gaussian factor to weight samples
  double m_gamma;

  /// Propagation scheme
  PropagationScheme m_p_scheme;

  /// Minimum angle for view selection (in degree)
  double m_minimum_view_angle;

  /// Maximum angle for view selection (in degree)
  double m_maximum_view_angle;

  /// Maximum number of selected view for depth map computation
  int m_maximum_view_nb;

  /// Number of image used for cost computation
  int m_nb_image_for_cost_computation;

  /// Use cost matrix with multiple view selection
  bool m_use_joint_view_selection;

  /// base working directory
  std::string m_base_path;
};

} // namespace MVS

#endif