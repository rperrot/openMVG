#ifndef OPENMVG_FEATURES_SFOP_SFOP_IMAGE_DESCRIBER_HPP
#define OPENMVG_FEATURES_SFOP_SFOP_IMAGE_DESCRIBER_HPP

#include "openMVG/features/image_describer.hpp"
#include "openMVG/features/regions_factory.hpp"

#include "openMVG/features/sfop/SFOPParams.hpp"

namespace openMVG
{
namespace features
{
namespace sfop
{

class SFOPImageDescriber : public Image_describer
{
public:
  using Regions_type = SIFT_Regions;

  SFOPImageDescriber( const SFOPParams& params = SFOPParams() );

  /**
   * @brief Use a preset to control the number of detected regions
   * @param preset The preset configuration
   * @return True if configuration succeed.
   */
  bool Set_configuration_preset( EDESCRIBER_PRESET preset ) override;

  /**
    * @brief Detect regions on the image and compute their attributes
    (description)
    * @param image Image.
    * @param mask 8-bit gray image for keypoint filtering (optional).
      Non-zero values depict the region of interest.
    * @return The detected regions and attributes
    */
  virtual std::unique_ptr<Regions> Describe( const image::Image<unsigned char>& image,
                                             const image::Image<unsigned char>* mask = nullptr ) override;

  /// Allocate regions depending of the Image_describer
  std::unique_ptr<Regions> Allocate() const override;

  template <class Archive>
  void serialize( Archive& ar );

private:
  SFOPParams m_params;
};

} // namespace sfop
} // namespace features
} // namespace openMVG

#endif