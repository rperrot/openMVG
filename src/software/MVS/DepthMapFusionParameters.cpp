#include "DepthMapFusionParameters.hpp"

#include "openMVG/numeric/numeric.h"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <sstream>


namespace MVS
{
  /**
  * @brief Parameters for computation of the depth map
  * @param fe delta depth to accept a point
  * @param fang delta angle between normals to accept a point (in degree)
  * @param fcomp minimum number of images for a point to be considered as valid
  */
  DepthMapFusionComputationParameters::DepthMapFusionComputationParameters( const std::string & base_path ,
      const int scale ,
      const double fe ,
      const double fang ,
      const int fcomp )
    : m_scale( scale ) ,
      m_fe( fe ) ,
      m_fang( openMVG::D2R( fang ) ) ,
      m_fcomp( fcomp ) ,
      m_base_path( base_path )
  {

  }

  /**
  * @brief Scale of the computation
  * @return scale
  */
  int DepthMapFusionComputationParameters::Scale( void ) const
  {
    return m_scale ;
  }

  /**
  * @brief Get working path
  * @return working path
  */
  std::string DepthMapFusionComputationParameters::WorkingDirectory( void ) const
  {
    return m_base_path ;
  }

  /**
  * @brief Get directory containing all depth maps
  * @return View directory
  */
  std::string DepthMapFusionComputationParameters::GetDepthDirectory( void ) const
  {
    const std::string base = WorkingDirectory() ;
    return stlplus::create_filespec( base , "depth" ) ;
  }

  /**
  * @brief Get directory of the i-th camera path
  * @param id_cam Id of the camera to get
  * @return Camera path
  */
  std::string DepthMapFusionComputationParameters::GetCameraDirectory( const int id_cam ) const
  {
    const std::string depth_dir = GetDepthDirectory();

    std::stringstream str ;
    str << "cam_" << id_cam ;

    return stlplus::create_filespec( depth_dir , str.str() ) ;
  }


  /**
  * @brief Get path to the camera file
  * @param id_cam Id of the camera to get
  * @return Camera path for given camera
  */
  std::string DepthMapFusionComputationParameters::GetCameraPath( const int id_cam ) const
  {
    const std::string cam_dir = GetCameraDirectory( id_cam ) ;

    std::stringstream str ;
    str << "cam_" << m_scale << ".bin" ;

    return stlplus::create_filespec( cam_dir , str.str() ) ;
  }

  /**
  * @brief Get path to the camera grayscale file
  * @param id_cam Id of the camera to get
  * @return Grayscale path for given camera
  */
  std::string DepthMapFusionComputationParameters::GetGrayscalePath( const int id_cam ) const
  {
    const std::string cam_dir = GetCameraDirectory( id_cam ) ;

    std::stringstream str ;
    str << "grayscale_" << m_scale << ".bin" ;

    return stlplus::create_filespec( cam_dir , str.str() ) ;
  }

  /**
  * @brief Get path to the camera color file
  * @param id_cam Id of the camera to get
  * @return Color path for given camera
  */
  std::string DepthMapFusionComputationParameters::GetColorPath( const int id_cam ) const
  {
    const std::string cam_dir = GetCameraDirectory( id_cam ) ;

    std::stringstream str ;
    str << "color_" << m_scale << ".bin" ;

    return stlplus::create_filespec( cam_dir , str.str() ) ;
  }

  /**
  * @brief Get path to the camera depth map file
  * @param id_cam Id of the camera to get
  * @return Depth path for given camera
  */
  std::string DepthMapFusionComputationParameters::GetDepthPath( const int id_cam ) const
  {
    const std::string cam_dir = GetCameraDirectory( id_cam ) ;

    std::stringstream str ;
    str << "dm_" << m_scale << ".bin" ;

    return stlplus::create_filespec( cam_dir , str.str() ) ;
  }

  /**
  * @brief Get path to the filtered camera depth map file
  * @param id_cam Id of the camera to get
  * @reeturn Filtered depth path for the given camera
  */
  std::string DepthMapFusionComputationParameters::GetFilteredDepthPath( const int id_cam ) const
  {
    const std::string cam_dir = GetCameraDirectory( id_cam ) ;

    std::stringstream str ;
    str << "dm_" << m_scale << "_filtered.bin" ;

    return stlplus::create_filespec( cam_dir , str.str() ) ;
  }


  /**
  * @brief Get depth threshold for two depth to be considered as equal
  * @return threshold
  */
  double DepthMapFusionComputationParameters::DepthThreshold( void ) const
  {
    return m_fe ;
  }

  /**
  * @brief Get angle threshold for two normals to be considered as equal
  * @return threshold
  */
  double DepthMapFusionComputationParameters::AngleThreshold( void ) const
  {
    return m_fang ;
  }

  /**
  * @brief Get number of consistent view for a pixel to be considered as valid
  * @return threshold
  */
  int DepthMapFusionComputationParameters::NbMinimumView( void ) const
  {
    return m_fcomp ;
  }
}