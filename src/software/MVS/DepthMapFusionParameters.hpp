#ifndef _OPENMVG_MVS_DEPTH_MAP_FUSION_COMPUTATION_PARAMETERS_HPP_
#define _OPENMVG_MVS_DEPTH_MAP_FUSION_COMPUTATION_PARAMETERS_HPP_

#include <string>

namespace MVS
{

class DepthMapFusionComputationParameters
{
  public:

    /**
     * @brief Parameters for computation of the depth map
     * @param working_path Working path containing all input data
     * @param scale Scale of the computation
     * @param fe delta depth to accept a point
     * @param fang delta angle between normals to accept a point (in degree)
     * @param fcomp minimum number of images for a point to be considered as valid
     */
    DepthMapFusionComputationParameters( const std::string &working_path,
                                         const int          scale,
                                         const double       fe    = 0.1,
                                         const double       fang  = 30.0,
                                         const int          fcomp = 3 );

    /**
     * @brief Copy ctr
     * @param src Source
     */
    DepthMapFusionComputationParameters( const DepthMapFusionComputationParameters &src ) = default;

    /**
     * @brief Move ctr
     * @param src Source
     */
    DepthMapFusionComputationParameters( DepthMapFusionComputationParameters &&src ) = default;

    /**
     * @brief Assignment operator
     * @param src Source
     * @return Self after Assignment
     */
    DepthMapFusionComputationParameters &operator=( const DepthMapFusionComputationParameters &src ) = default;

    /**
     * @brief Move Assignment operator
     * @param src Source
     * @return Self after Assignment
     */
    DepthMapFusionComputationParameters &operator=( DepthMapFusionComputationParameters &&src ) = default;

    /**
     * @brief Scale of the computation
     * @return scale
     */
    int scale( void ) const;

    /**
     * @brief Get working path
     * @return working path
     */
    std::string workingDirectory( void ) const;

    /**
     * @brief Get directory containing all depth maps
     * @return View directory
     */
    std::string getDepthDirectory( void ) const;

    /**
     * @brief Get directory of the i-th camera path
     * @param id_cam Id of the camera to get
     * @return Camera path
     */
    std::string getCameraDirectory( const int id_cam ) const;

    /**
     * @brief Get path to the camera file
     * @param id_cam Id of the camera to get
     * @return Camera path for given camera
     */
    std::string getCameraPath( const int id_cam ) const;

    /**
     * @brief Get path to the camera grayscale file
     * @param id_cam Id of the camera to get
     * @return Grayscale path for given camera
     */
    std::string getGrayscalePath( const int id_cam ) const;

    /**
     * @brief Get path to the camera color file
     * @param id_cam Id of the camera to get
     * @return Color path for given camera
     */
    std::string getColorPath( const int id_cam ) const;

    /**
     * @brief Get path to the camera depth map file
     * @param id_cam Id of the camera to get
     * @return Depth path for given camera
     */
    std::string getDepthPath( const int id_cam ) const;

    /**
     * @brief Get path to the filtered camera depth map file
     * @param id_cam Id of the camera to get
     * @return Filtered depth path for the given camera
     */
    std::string getFilteredDepthPath( const int id_cam ) const;

    /**
     * @brief Get output model directory
     * @return Path of the output directory
     */
    std::string getModelDirectory( void ) const;

    /**
     * @brief Get model name
     * @return model name for the current scale
     */
    std::string getModelPath( void ) const;

    /**
     * @brief Get depth threshold for two depth to be considered as equal
     * @return threshold
     */
    double depthThreshold( void ) const;

    /**
     * @brief Get angle threshold for two normals to be considered as equal
     * @return threshold
     */
    double angleThreshold( void ) const;

    /**
     * @brief Get number of consistent view for a pixel to be considered as valid
     * @return threshold
     */
    int nbMinimumView( void ) const;

    /**
     * @brief Get depth sigma threshold
     * @return depth threshold
     */
    double getSigma( void ) const;

    /**
     * @brief Set depth sigma threshold
     * @param sig depth threshold
     */
    void setSigma( const double sig );

    /**
    * @brief Indicate if Wolff filtering is used
    * @retval true if Wolff filtering is used
    * @retval false if Wolff filtering is not used
    */
    bool useWolff( void ) const ;

    /**
    * @brief set usage of Wolff filtring
    * @param use True to set Wolff filtering
    */
    void setUseWolff( const bool use ) ;

  private:
    int m_scale;

    double m_fe;
    double m_fang;
    double m_fcomp;

    // Wolff
    bool m_use_wolff ;
    double m_sigma;

    std::string m_base_path;
};

} // namespace MVS

#endif