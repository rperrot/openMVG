#include "SFOPImageDescriber.hpp"

#include "openMVG/features/sfop/SFOP.hpp"
#include "openMVG/image/image_container.hpp"

#include "openMVG/features/sift/sift_DescriptorExtractor.hpp"

#include <algorithm>

namespace openMVG
{
namespace features
{
namespace sfop
{

SFOPImageDescriber::SFOPImageDescriber( const SFOPParams& params )
    : m_params( params )
{
}

/**
 * @brief Use a preset to control the number of detected regions
 * @param preset The preset configuration
 * @return True if configuration succeed.
 */
bool SFOPImageDescriber::Set_configuration_preset( EDESCRIBER_PRESET preset )
{
  switch ( preset )
  {
  case EDESCRIBER_PRESET::NORMAL_PRESET:
  {
    break;
  }
  case EDESCRIBER_PRESET::HIGH_PRESET:
  {
    m_params.setNoiseStandardDeviation( m_params.noiseStandardDeviation() / 10.f );
    break;
  }
  case EDESCRIBER_PRESET::ULTRA_PRESET:
  {
    m_params.setNoiseStandardDeviation( m_params.noiseStandardDeviation() / 100.f );
    break;
  }
  default:
    return false;
  }

  return true;
}

// Comparison function used to sort features using octave, then slices
static bool ascendingOctaveSliceFeatures( const SFOPFeature& f1, const SFOPFeature& f2 )
{
  if ( f1.m_id_octave < f2.m_id_octave )
  {
    return true;
  }
  else if ( f1.m_id_octave > f2.m_id_octave )
  {
    return false;
  }
  else
  {
    // Same octave
    return f1.m_id_slice < f2.m_id_slice;
  }
}

/**
  * @brief Detect regions on the image and compute their attributes
  (description)
  * @param image Image.
  * @param mask 8-bit gray image for keypoint filtering (optional).
    Non-zero values depict the region of interest.
  * @return The detected regions and attributes
  */
std::unique_ptr<Regions> SFOPImageDescriber::Describe( const image::Image<unsigned char>& image,
                                                       const image::Image<unsigned char>* mask )
{
  // Convert image to float and uses values in range [0;1]
  image::Image<float> input;
  input = image.GetMat().cast<float>() / 255.f;

  // Build SFOP engine wrt to the current settings
  SFOP sfopEngine( m_params );

  // Detect keypoints
  std::vector<SFOPFeature> kpts;
  sfopEngine.detect( input, kpts );

  // Feature masking (remove keypoints if they are masked)
  if ( mask )
  {
    kpts.erase( std::remove_if( kpts.begin(),
                                kpts.end(),
                                [&]( const SFOPFeature& pt ) {
                                  return ( *mask )( pt.m_y, pt.m_x ) == 0;
                                } ),
                kpts.end() );
  }

  // Describe remaining points using SIFT
  // 1. Sort them in order to get per octave/slices features
  std::sort( kpts.begin(), kpts.end(), ascendingOctaveSliceFeatures );

  int  cur_octave  = -1;
  int  cur_slice   = -1;
  int  prev_octave = 0;
  auto it          = kpts.begin();

  auto regions = std::unique_ptr<Regions_type>( new Regions_type );

  openMVG::image::Image<float> baseImg = input;
  while ( it != kpts.end() )
  {
    // Convert current keypoints to sift keypoints
    std::vector<openMVG::features::sift::Keypoint> sKpts;

    cur_octave = it->m_id_octave;
    cur_slice  = it->m_id_slice;

    // Get all keypoints that share the same slice
    while ( it != kpts.end() )
    {
      if ( it->m_id_octave != cur_octave ||
           it->m_id_slice != cur_slice )
      {
        break;
      }

      openMVG::features::sift::Keypoint kp;
      kp.x     = it->m_x;
      kp.y     = it->m_y;
      kp.sigma = it->m_sigma;
      kp.s     = 0;

      sKpts.emplace_back( kp );

      ++it;
    }

    if ( sKpts.size() > 0 )
    {
      // 1 Build slice
      // 1.1 Decimate base image
      while ( prev_octave != cur_octave )
      {
        openMVG::image::Image<float> nextImg;
        image::ImageGaussianFilter( baseImg, 0.5, nextImg );
        image::ImageDecimate( nextImg, baseImg );

        ++prev_octave;
      }
      // 1.2 Smooth image
      const float octave_sigma = std::pow( 2.0, cur_octave );
      const float sigma_slice  = std::pow( 2.0, cur_octave + cur_slice / m_params.nbSlice() ) / octave_sigma;

      image::Image<float> slice;
      image::ImageGaussianFilter( baseImg, sigma_slice, slice );

      sift::Sift_DescriptorExtractor descExtractor;
      descExtractor( slice, octave_sigma, sKpts );

      for ( const auto itKpt : sKpts )
      {
        Descriptor<unsigned char, 128> descriptor;
        descriptor << ( itKpt.descr.cast<unsigned char>() );
        regions->Descriptors().emplace_back( descriptor );
        regions->Features().emplace_back( itKpt.x, itKpt.y, itKpt.sigma, itKpt.theta );
      }
    }
  }

  return regions;
}

/// Allocate regions depending of the Image_describer
std::unique_ptr<Regions> SFOPImageDescriber::Allocate() const
{
  return std::unique_ptr<Regions_type>( new Regions_type );
}

} // namespace sfop
} // namespace features
} // namespace openMVG
