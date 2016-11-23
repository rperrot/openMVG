#include "DepthMapComputationParameters.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <limits>
#include <sstream>

namespace MVS
{

  const double DepthMapComputationParameters::MAX_COST_NCC = 2.0 ;
  const double DepthMapComputationParameters::MAX_COST_PM = 10e6 ;


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
  DepthMapComputationParameters::DepthMapComputationParameters( const int scale ,
      const cost_metric metric ,
      const double alpha ,
      const double tau_i ,
      const double tau_g ,
      const double gamma ,
      const double min_view_angle ,
      const double max_view_angle ,
      const int max_view_selection_nb ,
      const int nb_image_for_cost ,
      const std::string base_path )
    : m_scale( scale ) ,
      m_metric( metric ) ,
      m_alpha( alpha ) ,
      m_tau_i( tau_i ) ,
      m_tau_g( tau_g ) ,
      m_gamma( gamma ) ,
      m_minimum_view_angle( min_view_angle ) ,
      m_maximum_view_angle( max_view_angle ) ,
      m_maximum_view_nb( max_view_selection_nb ) ,
      m_nb_image_for_cost_computation( nb_image_for_cost ) ,
      m_base_path( base_path )
  {

  }


  /**
  * @brief Get cost metric used to compute pixel score
  */
  cost_metric DepthMapComputationParameters::Metric( void ) const
  {
    return m_metric ;
  }

  /**
  * @brief Given a metric get the corresponding maximum score
  * @param metric the current metric
  * @return the maximum score for the given metric
  */
  double DepthMapComputationParameters::MetricMaxCostValue( const cost_metric metric )
  {
    switch( metric )
    {
      case COST_METRIC_NCC :
      {
        return MAX_COST_NCC ;
      }
      case COST_METRIC_PM:
      {
        return MAX_COST_PM ;
      }
    }
    return std::numeric_limits<double>::max() ;
  }


  /**
  * @brief Get scale of the computation( 0 for original size)
  * @return scale of the computation
  */
  int DepthMapComputationParameters::Scale( void ) const
  {
    return m_scale ;
  }

  /**
  * @brief Get balance value between gradient (1) and intensity (0)
  * @return balance value
  */
  double DepthMapComputationParameters::Alpha( void ) const
  {
    return m_alpha ;
  }

  /**
  * @brief Threshold value on intensity difference
  * @return Threshold value on intensity difference
  */
  double DepthMapComputationParameters::TauI( void ) const
  {
    return m_tau_i ;
  }

  /**
  * @brief Threshold value on gradient difference
  * @return Threshold value on gradient difference
  */
  double DepthMapComputationParameters::TauG( void ) const
  {
    return m_tau_g ;
  }

  /**
  * @brief Gaussian factor used to weight the samples
  * @return gaussian factor
  */
  double DepthMapComputationParameters::Gamma( void ) const
  {
    return m_gamma ;
  }


  /**
  * @brief Get minimum view angle for view selection
  * @return Minimum view angle for view selection
  * @note Angle in degree
  */
  double DepthMapComputationParameters::MinimumViewAngle( void ) const
  {
    return m_minimum_view_angle ;
  }

  /**
  * @brief Get maximum view angle for view selection
  * @return Maximum view angle for view selection
  * @note Angle in degree
  */
  double DepthMapComputationParameters::MaximumViewAngle( void ) const
  {
    return m_maximum_view_angle ;
  }

  /**
  * @brief Get Number of view for view selection
  * @return Maximum number of view used for view selection
  */
  int DepthMapComputationParameters::NbMaximumViewSelection( void ) const
  {
    return m_maximum_view_nb ;
  }

  /**
  * @brief Number of camera used for matching cost
  * @param
  */
  int DepthMapComputationParameters::NbMultiViewImageForCost( void ) const
  {
    return m_nb_image_for_cost_computation ;
  }

  /**
  * @brief Get working directory
  * @return working directory
  */
  std::string DepthMapComputationParameters::WorkingDirectory( void ) const
  {
    return m_base_path ;
  }

  /**
  * @brief Get depth directory in the working directory
  * @return depth directory (containing all cameras)
  */
  std::string DepthMapComputationParameters::DepthDirectory( void ) const
  {
    const std::string base = WorkingDirectory() ;
    return stlplus::create_filespec( base , "depth" ) ;
  }

  /**
  * @brief Get Camera directory given it's ID
  * @param ID of the camera to retrieve
  * @return Data directory of the i-th camera
  */
  std::string DepthMapComputationParameters::GetCameraDirectory( const int id ) const
  {
    std::stringstream str ;
    str << "cam_" << id ;
    const std::string depth_path = DepthDirectory();

    return stlplus::create_filespec( depth_path , str.str() ) ;
  }

  std::string DepthMapComputationParameters::GetColorPath( const int id ) const
  {
    const std::string camera_path = GetCameraDirectory( id ) ;
    std::stringstream str ;

    str << "color_" << m_scale << ".bin" ;

    return stlplus::create_filespec( camera_path , str.str() ) ;
  }


  std::string DepthMapComputationParameters::GetGrayscalePath( const int id ) const
  {
    const std::string camera_path = GetCameraDirectory( id ) ;
    std::stringstream str ;

    str << "grayscale_" << m_scale << ".bin" ;

    return stlplus::create_filespec( camera_path , str.str() ) ;
  }

  std::string DepthMapComputationParameters::GetGradientPath( const int id ) const
  {
    const std::string camera_path = GetCameraDirectory( id ) ;
    std::stringstream str ;

    str << "gradient_" << m_scale << ".bin" ;

    return stlplus::create_filespec( camera_path , str.str() ) ;
  }

  std::string DepthMapComputationParameters::GetCameraPath( const int id ) const
  {
    const std::string camera_path = GetCameraDirectory( id ) ;
    std::stringstream str ;

    str << "cam_" << m_scale << ".bin" ;

    return stlplus::create_filespec( camera_path , str.str() ) ;
  }

  std::string DepthMapComputationParameters::GetDepthPath( const int id ) const
  {
    const std::string camera_path = GetCameraDirectory( id ) ;
    std::stringstream str ;

    str << "dm_" << m_scale << ".bin" ;

    return stlplus::create_filespec( camera_path , str.str() ) ;
  }

  /**
   * @brief Get output model path
   * @return Path of the output model
   */
  std::string DepthMapComputationParameters::GetModelDirectory( void ) const
  {
    const std::string base = WorkingDirectory() ;
    return stlplus::create_filespec( base , "model" ) ;
  }


}