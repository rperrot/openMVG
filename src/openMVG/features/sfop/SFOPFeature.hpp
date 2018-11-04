#ifndef OPENMVG_FEATURES_SFOP_SFOP_FEATURES_HPP
#define OPENMVG_FEATURES_SFOP_SFOP_FEATURES_HPP

namespace openMVG
{
namespace features
{
namespace sfop
{

/**
 * @brief Class defining one SFOP feature
 *
 * It contains the location (x,y), the scale sigma as well as precision and the
 * smaller eigenvalue of the structure tensor.
 */
class SFOPFeature
{
public:
  /// x-coordinate (column)
  float m_x;

  /// y-coordinate (row)
  float m_y;

  /// Size, i.e. integration scale with largest response
  float m_sigma;

  /// Id of the slice
  int m_id_slice;

  /// Id of the octave
  int m_id_octave;

  /// Smaller eigenvalue of the structure tensor
  float m_lambda2;

  /// Precision
  float m_precision;

  /**
   * @brief Constructor
   *
   * @param x         x-coordinate
   * @param y         y-coordinate 
   * @param sigma     Size
   * @param idSlice   Id of the slice 
   * @param idOctave  Id of the octave 
   * @param lambda2   Eigenvalue lambda2
   * @param precision Precision
   */
  SFOPFeature( const float x,
               const float y,
               const float sigma,
               const int   idSlice,
               const int   idOctave,
               const float lambda2,
               const float precision );
};

} // namespace sfop
} // namespace features
} // namespace openMVG

#endif
