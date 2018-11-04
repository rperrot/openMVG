#ifndef OPENMVG_FEATURES_SFOP_SFOP_HPP
#define OPENMVG_FEATURES_SFOP_SFOP_HPP

#include "openMVG/features/sfop/SFOPFeature.hpp"
#include "openMVG/features/sfop/SFOPParams.hpp"
#include "openMVG/image/image_container.hpp"

namespace openMVG
{
namespace features
{
namespace sfop
{

/**
 * @brief Main SFOP class
 * 
 */
class SFOP
{
public:
  SFOP( const SFOPParams& params );

  /**
   * @brief Detect features on a given image 
   * 
   * @param img     Image to use 
   * @param kpts    Keypoints of the image 
   */
  void detect( const image::Image<float>& img, std::vector<SFOPFeature>& kpts ) const;

private:
  /**
    * @brief Perform filtering on the keypoints (wrt to noise model of the image)
    * 
    * @param feats Features to clear 
    */
  void filterFeatures( std::vector<SFOPFeature>& feats, const int width, const int height ) const;

  /**
    * @brief Suppresses less precise features if very close to each other
    *
    * @params feats Features to filter based on distance 
    */
  void nonMaximaSuppression( std::vector<SFOPFeature>& feats ) const;

  SFOPParams m_params;
};

} // namespace sfop
} // namespace features
} // namespace openMVG

#endif
