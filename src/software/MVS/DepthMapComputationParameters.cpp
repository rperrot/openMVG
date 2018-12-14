#include "DepthMapComputationParameters.hpp"

#include "Image.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <limits>
#include <sstream>

namespace MVS
{

const double DepthMapComputationParameters::MAX_COST_NCC           = 2.0;
const double DepthMapComputationParameters::MAX_COST_PM            = 10e6;
const double DepthMapComputationParameters::MAX_COST_CENSUS        = 2.0; //10e4 ;
const double DepthMapComputationParameters::MAX_COST_DAISY         = 1;
const double DepthMapComputationParameters::MAX_COST_BILATERAL_NCC = 2.0;

std::string to_string( const PropagationScheme& pscheme )
{
  std::stringstream str;

  switch ( pscheme )
  {
  case PROPAGATION_SCHEME_ASYMETRIC:
  {
    str << "asymetric";
    break;
  }
  case PROPAGATION_SCHEME_FULL:
  {
    str << "full";
    break;
  }
  case PROPAGATION_SCHEME_SPEED:
  {
    str << "speed";
    break;
  }
  }

  return str.str();
}

std::string to_string( const cost_metric& metric )
{
  std::stringstream str;

  switch ( metric )
  {
  case COST_METRIC_NCC:
  {
    str << "ncc";
    break;
  }
  case COST_METRIC_PM:
  {
    str << "patch-match";
    break;
  }
  case COST_METRIC_DAISY:
  {
    str << "daisy";
    break;
  }
  case COST_METRIC_CENSUS:
  {
    str << "census";
    break;
  }
  case COST_METRIC_BILATERAL_NCC:
  {
    str << "bilateral-ncc";
  }
  }

  return str.str();
}

/**
* @brief Ctr
* @param scale Scale
* @param alpha Balance between intensity (0) and gradient (1)
* @param tau_i Threshold on intensity difference
* @param tau_g Threshold on gradient difference
* @param gamma Gaussian factor used to give less weight to far (in intensity) samples
* @param min_view_angle Minimum view angle for camera selection
* @param max_view_angle Maximum view angle for camera selection
* @param max_view_selection_nb Maximum number of view for view selection
* @param nb_image_for_cost Number of image used for cost computation
* @param base_path Base working path
* @note min_view_angle in degree
* @note max_view_angle in degree
*/
DepthMapComputationParameters::DepthMapComputationParameters( const int         scale,
                                                              const cost_metric metric,
                                                              const double      alpha,
                                                              const double      tau_i,
                                                              const double      tau_g,
                                                              const double      gamma,
                                                              // For PM propagation scheme
                                                              const PropagationScheme pScheme,
                                                              const double            min_view_angle,
                                                              const double            max_view_angle,
                                                              const int               max_view_selection_nb,
                                                              const int               nb_image_for_cost,
                                                              const std::string       base_path )
    : m_scale( scale ),
      m_metric( metric ),
      m_alpha( alpha ),
      m_tau_i( tau_i ),
      m_tau_g( tau_g ),
      m_gamma( gamma ),
      m_p_scheme( pScheme ),
      m_minimum_view_angle( min_view_angle ),
      m_maximum_view_angle( max_view_angle ),
      m_maximum_view_nb( max_view_selection_nb ),
      m_nb_image_for_cost_computation( nb_image_for_cost ),
      m_base_path( base_path )
{
}

/**
* @brief Get cost metric used to compute pixel score
*/
cost_metric DepthMapComputationParameters::metric( void ) const
{
  return m_metric;
}

/**
* @brief Given a metric get the corresponding maximum score
* @param metric the current metric
* @return the maximum score for the given metric
*/
double DepthMapComputationParameters::metricMaxCostValue( const cost_metric metric )
{
  switch ( metric )
  {
  case COST_METRIC_NCC:
  {
    return MAX_COST_NCC;
  }
  case COST_METRIC_PM:
  {
    return MAX_COST_PM;
  }
  case COST_METRIC_CENSUS:
  {
    return MAX_COST_CENSUS;
  }
  case COST_METRIC_DAISY:
  {
    return MAX_COST_DAISY;
  }
  case COST_METRIC_BILATERAL_NCC:
  {
    return MAX_COST_BILATERAL_NCC;
  }
  }
  return std::numeric_limits<double>::max();
}

/**
* @brief Get scale of the computation( 0 for original size)
* @return scale of the computation
*/
int DepthMapComputationParameters::scale( void ) const
{
  return m_scale;
}

/**
* @brief Get balance value between gradient (1) and intensity (0)
* @return balance value
*/
double DepthMapComputationParameters::alpha( void ) const
{
  return m_alpha;
}

/**
* @brief Threshold value on intensity difference
* @return Threshold value on intensity difference
*/
double DepthMapComputationParameters::tauI( void ) const
{
  return m_tau_i;
}

/**
* @brief Threshold value on gradient difference
* @return Threshold value on gradient difference
*/
double DepthMapComputationParameters::tauG( void ) const
{
  return m_tau_g;
}

/**
* @brief Gaussian factor used to weight the samples
* @return gaussian factor
*/
double DepthMapComputationParameters::gamma( void ) const
{
  return m_gamma;
}

/**
 * @brief Get Propagation scheme
 * @return Propagation Scheme
 */
PropagationScheme DepthMapComputationParameters::propagationScheme( void ) const
{
  return m_p_scheme;
}

/**
* @brief Get minimum view angle for view selection
* @return Minimum view angle for view selection
* @note Angle in degree
*/
double DepthMapComputationParameters::minimumViewAngle( void ) const
{
  return m_minimum_view_angle;
}

/**
* @brief Get maximum view angle for view selection
* @return Maximum view angle for view selection
* @note Angle in degree
*/
double DepthMapComputationParameters::maximumViewAngle( void ) const
{
  return m_maximum_view_angle;
}

/**
* @brief Get Number of view for view selection
* @return Maximum number of view used for view selection
*/
int DepthMapComputationParameters::nbMaximumViewSelection( void ) const
{
  return m_maximum_view_nb;
}

/**
* @brief Number of camera used for matching cost
* @param
*/
int DepthMapComputationParameters::nbMultiViewImageForCost( void ) const
{
  return m_nb_image_for_cost_computation;
}

/**
* @brief Get working directory
* @return working directory
*/
std::string DepthMapComputationParameters::workingDirectory( void ) const
{
  return m_base_path;
}

/**
* @brief Get depth directory in the working directory
* @return depth directory (containing all cameras)
*/
std::string DepthMapComputationParameters::depthDirectory( void ) const
{
  const std::string base = workingDirectory();
  return stlplus::create_filespec( base, "depth" );
}

/**
* @brief Get Camera directory given it's ID
* @param ID of the camera to retrieve
* @return Data directory of the i-th camera
*/
std::string DepthMapComputationParameters::getCameraDirectory( const int id ) const
{
  std::stringstream str;
  str << "cam_" << id;
  const std::string depth_path = depthDirectory();

  return stlplus::create_filespec( depth_path, str.str() );
}

std::string DepthMapComputationParameters::getColorPath( const int id ) const
{
  const std::string camera_path = getCameraDirectory( id );
  std::stringstream str;

  str << "color_" << m_scale << ".bin";

  return stlplus::create_filespec( camera_path, str.str() );
}

std::string DepthMapComputationParameters::getGrayscalePath( const int id ) const
{
  const std::string camera_path = getCameraDirectory( id );
  std::stringstream str;

  str << "grayscale_" << m_scale << ".bin";

  return stlplus::create_filespec( camera_path, str.str() );
}

std::string DepthMapComputationParameters::getGradientPath( const int id ) const
{
  const std::string camera_path = getCameraDirectory( id );
  std::stringstream str;

  str << "gradient_" << m_scale << ".bin";

  return stlplus::create_filespec( camera_path, str.str() );
}

/**
* @brief Get census image path
* @param id Id of the image to get
* @return Path of the census image for the given camera
*/
std::string DepthMapComputationParameters::getCensusPath( const int id ) const
{
  const std::string camera_path = getCameraDirectory( id );
  std::stringstream str;

  str << "census_" << m_scale << ".bin";

  return stlplus::create_filespec( camera_path, str.str() );
}

std::string DepthMapComputationParameters::getCameraPath( const int id ) const
{
  const std::string camera_path = getCameraDirectory( id );
  std::stringstream str;

  str << "cam_" << m_scale << ".bin";

  return stlplus::create_filespec( camera_path, str.str() );
}

std::string DepthMapComputationParameters::getDepthPath( const int id ) const
{
  const std::string camera_path = getCameraDirectory( id );
  std::stringstream str;

  str << "dm_" << m_scale << ".bin";

  return stlplus::create_filespec( camera_path, str.str() );
}

/**
 * @brief Get output model path
 * @return Path of the output model
 */
std::string DepthMapComputationParameters::getModelDirectory( void ) const
{
  const std::string base = workingDirectory();
  return stlplus::create_filespec( base, "model" );
}

} // namespace MVS