#ifndef OPENMVG_FEATURES_SFOP_SFOP_SLICE_HPP
#define OPENMVG_FEATURES_SFOP_SFOP_SLICE_HPP

#include "openMVG/features/sfop/SFOPFeature.hpp"

#include "openMVG/image/image_container.hpp"

#include <cmath>
#include <iostream>
#include <list>

namespace openMVG
{
namespace features
{
namespace sfop
{

/**
 * @brief One slice of SFOP octave
 */
class SFOPSlice
{
public:
  /**
   * @brief Construct a new SFOPSlice 
   * 
   * @param img       Input image 
   * @param idSlice   Id of the slice (relative to the current octave, from -1 to nbSlice )
   * @param idOctave  Id of the current octave 
   * @param NbSlice   Number of slice in this octave 
   */
  SFOPSlice( const image::Image<float>& img, const int idSlice, const int idOctave, const int NbSlice );

  SFOPSlice( const SFOPSlice& src ) = default;
  SFOPSlice( SFOPSlice&& src )      = default;

  SFOPSlice& operator=( const SFOPSlice& src ) = default;
  SFOPSlice& operator=( SFOPSlice&& src ) = default;

  /**
   * @brief Detect features in this slice using local 3d maxima
   * @param below Below slice
   * @param above Above slice
   */
  void detect( const SFOPSlice& below, const SFOPSlice& above, std::vector<SFOPFeature>& kpts ) const;

  /**
   * @brief Build slice (ie: compute eigenvalue and associated precision)
   * 
   */
  void buildSlice( void );

private:
  /**
   * @brief Compute eigen value 
   * 
   * The scaled smaller eigenvalue lambda2 is defined as:
   * 
   *   lambda2 = M * (trace / 2 - sqrt(trace^2 / 4 - determinant))
   *
   * with trace and determinant being:
   * 
   *   trace = Nxx + Nyy
   *   determinant = Nxx * Nyy - Nxy^2;
   */
  void computeEigenValue( const float                M,
                          const image::Image<float>& Nxx,
                          const image::Image<float>& Nxy,
                          const image::Image<float>& Nyy );

  /**
   * @brief Compute best omega 
   * 
   * @param omega0              Omega for angle = 0 
   * @param omega_60            Omega for angle = 60
   * @param omega_120           Omega for angle = 120 
   * 
   * Best : 
   * S = 1/3 a + 2/3 b
   * 
   * a = w_0 + w_60 + w_120
   * b = sqrt( w_0 . w_0 + w_60 . w_60 + w_120 . w_120 - w_0 . w_60 - w_60 .w_120 - w_0 . w_120 )
   * 
   * @return omega that gives lowest model error   
   */
  static image::Image<float> computeBestOmega( const image::Image<float>& omega0,
                                               const image::Image<float>& omega_60,
                                               const image::Image<float>& omega_120 );

  /**
   * @brief Compute precision of the model 
   * 
   * prec = scale * lambda2 / omega 
   * 
   * @param scale Scale applied to the result 
   * @param omega bestOmega of the model
   */
  void computePrecision( const float scale, const image::Image<float>& omega );

  /// Input image used to build the slice
  const image::Image<float>& m_base_img;

  /// Id of the current slice
  int m_id_slice;
  /// Id of the current octave
  int m_id_octave;
  /// Number of slice in this octave
  int m_nb_slice;

  /// smallest eigen value of the structure tensor
  image::Image<float> m_lambda2;

  /// Precision
  image::Image<float> m_precision;
};

} // namespace sfop
} // namespace features
} // namespace openMVG

#endif
