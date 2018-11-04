#ifndef OPENMVG_FEATURES_SFOP_SFOP_OCTAVE_HPP
#define OPENMVG_FEATURES_SFOP_SFOP_OCTAVE_HPP

#include "openMVG/features/sfop/SFOPFeature.hpp"
#include "openMVG/features/sfop/SFOPSlice.hpp"

#include "openMVG/image/image_container.hpp"

#include <vector>

namespace openMVG
{
namespace features
{
namespace sfop
{
/**
 * @brief Class for holding multiple layers of one octave.
 *
 * The SFOP scale space is devided into octaves, each having the same number of
 * layers and being computed from a downsampled version of the original image.
 * Every octave has half the resolution of its predecessor.
 */
class SFOPOctave
{
public:
  /**
   * @brief Construct a new SFOPOctave 
   * 
   * @param baseImg     First image of the octave 
   * @param idOctave    Id of the octave 
   * @param nb_slice    Number of slice in this octave 
   */
  SFOPOctave( const image::Image<float>& baseImg,
              const int                  idOctave,
              const int                  nb_slice );

  SFOPOctave( const SFOPOctave& src ) = default;
  SFOPOctave( SFOPOctave&& src )      = default;

  SFOPOctave& operator=( const SFOPOctave& src ) = default;
  SFOPOctave& operator=( SFOPOctave&& src ) = default;

  /**
   * @brief Build all internal slices of the octave
   * 
   */
  void buildSlices( void );

  /**
   * @brief Detect features in this octave 
   * 
   * @param kpts set of keypoints 
   */
  void detect( std::vector<SFOPFeature>& kpts ) const;

private:
  /// Number of slice in this octave
  int m_nb_slice;

  /// Index of this octave, starting with 0 being the original image
  int m_id_octave;

  /// Input image
  const image::Image<float>& m_base_img;

  /// All slices of the octave
  std::vector<SFOPSlice> m_slices;
};

} // namespace sfop
} // namespace features
} // namespace openMVG

#endif
