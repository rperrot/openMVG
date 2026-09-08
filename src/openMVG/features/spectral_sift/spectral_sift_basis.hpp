// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_BASIS_HPP
#define OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_BASIS_HPP

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include <Eigen/Core>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace openMVG
{
namespace features
{
namespace spectral_sift
{

/**
 * @brief Type of scale-space spectral filter
 */
enum ESpectralFilterType
{
  FILTER_SLOG     = 0, // Scale-normalized Laplacian of Gaussian (sLoG) for detection
  FILTER_GAUSSIAN = 1  // Gaussian filter for continuous patch reconstruction and gradient description
};

/**
 * @brief Serializer helper for Eigen::MatrixXf across Cereal archives
 */
struct SerializableMatrixXf
{
  Eigen::MatrixXf matrix;

  SerializableMatrixXf() = default;
  explicit SerializableMatrixXf( const Eigen::MatrixXf& m )
      : matrix( m )
  {
  }

  template <class Archive>
  void save( Archive& ar ) const
  {
    const int          rows = static_cast<int>( matrix.rows() );
    const int          cols = static_cast<int>( matrix.cols() );
    std::vector<float> data( matrix.data(), matrix.data() + matrix.size() );
    ar( cereal::make_nvp( "rows", rows ),
        cereal::make_nvp( "cols", cols ),
        cereal::make_nvp( "data", data ) );
  }

  template <class Archive>
  void load( Archive& ar )
  {
    int                rows = 0;
    int                cols = 0;
    std::vector<float> data;
    ar( cereal::make_nvp( "rows", rows ),
        cereal::make_nvp( "cols", cols ),
        cereal::make_nvp( "data", data ) );
    matrix.resize( rows, cols );
    if ( !data.empty() && data.size() == static_cast<size_t>( rows * cols ) )
    {
      std::copy( data.begin(), data.end(), matrix.data() );
    }
  }
};

/**
 * @brief Structure storing the decomposed spatial basis filters and polynomial representations
 *        for continuous scale-space processing (Gou Koutaki, CVPR 2014).
 */
class SpectralBasis
{
public:
  SpectralBasis()
      : filter_type_( FILTER_SLOG ),
        sigma_min_( 1.6f ),
        sigma_max_( 3.2f ),
        degree_( 3 ),
        energy_ratio_threshold_( 0.999f ),
        energy_ratio_achieved_( 0.0f ),
        K_( 0 ),
        kernel_size_( 25 )
  {
  }

  // Getters
  ESpectralFilterType GetFilterType() const
  {
    return filter_type_;
  }
  float GetSigmaMin() const
  {
    return sigma_min_;
  }
  float GetSigmaMax() const
  {
    return sigma_max_;
  }
  int GetDegree() const
  {
    return degree_;
  }
  float GetEnergyRatioThreshold() const
  {
    return energy_ratio_threshold_;
  }
  float GetEnergyRatioAchieved() const
  {
    return energy_ratio_achieved_;
  }
  int GetK() const
  {
    return K_;
  }
  int GetKernelSize() const
  {
    return kernel_size_;
  }

  const std::vector<Eigen::MatrixXf>& GetSpatialBases() const
  {
    return spatial_bases_;
  }
  const Eigen::MatrixXf& GetPolyCoeffs() const
  {
    return poly_coeffs_;
  }

  // Setters
  void SetFilterType( ESpectralFilterType type )
  {
    filter_type_ = type;
  }
  void SetSigmaRange( float s_min, float s_max )
  {
    sigma_min_ = s_min;
    sigma_max_ = s_max;
  }
  void SetDegree( int deg )
  {
    degree_ = deg;
  }
  void SetEnergyRatioThreshold( float th )
  {
    energy_ratio_threshold_ = th;
  }
  void SetEnergyRatioAchieved( float ac )
  {
    energy_ratio_achieved_ = ac;
  }
  void SetK( int k )
  {
    K_ = k;
  }
  void SetKernelSize( int size )
  {
    kernel_size_ = size;
  }
  void SetSpatialBases( const std::vector<Eigen::MatrixXf>& bases )
  {
    spatial_bases_ = bases;
  }
  void SetPolyCoeffs( const Eigen::MatrixXf& coeffs )
  {
    poly_coeffs_ = coeffs;
  }

  /**
   * @brief Map a physical scale sigma in [sigma_min, sigma_max] to normalized variable t in [-1, 1]
   */
  inline float ScaleToT( float sigma ) const
  {
    return 2.0f * ( sigma - sigma_min_ ) / ( sigma_max_ - sigma_min_ ) - 1.0f;
  }

  /**
   * @brief Map a normalized variable t in [-1, 1] to physical scale sigma in [sigma_min, sigma_max]
   */
  inline float TToScale( float t ) const
  {
    return sigma_min_ + 0.5f * ( t + 1.0f ) * ( sigma_max_ - sigma_min_ );
  }

  /**
   * @brief Evaluate the k-th weight polynomial w_k(t) = sum_{p=0}^d c_{k, p} * t^p
   * @param k Index of the basis (0 <= k < K_)
   * @param t Normalized scale in [-1, 1]
   */
  inline float EvaluatePolynomial( int k, float t ) const
  {
    float val = 0.0f;
    float tp  = 1.0f;
    for ( int p = 0; p <= degree_; ++p )
    {
      val += poly_coeffs_( k, p ) * tp;
      tp *= t;
    }
    return val;
  }

  /**
   * @brief Evaluate the derivative of the k-th weight polynomial w'_k(t) = sum_{p=1}^d p * c_{k, p} * t^{p-1}
   * @param k Index of the basis (0 <= k < K_)
   * @param t Normalized scale in [-1, 1]
   */
  inline float EvaluateDerivative( int k, float t ) const
  {
    float val = 0.0f;
    float tp  = 1.0f;
    for ( int p = 1; p <= degree_; ++p )
    {
      val += static_cast<float>( p ) * poly_coeffs_( k, p ) * tp;
      tp *= t;
    }
    return val;
  }

  /**
   * @brief Evaluate the second derivative of the k-th weight polynomial w''_k(t) = sum_{p=2}^d p*(p-1) * c_{k, p} * t^{p-2}
   * @param k Index of the basis (0 <= k < K_)
   * @param t Normalized scale in [-1, 1]
   */
  inline float EvaluateSecondDerivative( int k, float t ) const
  {
    float val = 0.0f;
    float tp  = 1.0f;
    for ( int p = 2; p <= degree_; ++p )
    {
      val += static_cast<float>( p * ( p - 1 ) ) * poly_coeffs_( k, p ) * tp;
      tp *= t;
    }
    return val;
  }

  /**
   * @brief Reconstruct the theoretical filter at continuous scale sigma as a linear combination of spatial bases
   * @param sigma Continuous scale
   * @return Approximated 2D filter matrix of size kernel_size_ x kernel_size_
   */
  Eigen::MatrixXf ReconstructFilter( float sigma ) const;

  /**
   * @brief Cereal serialization method
   */
  template <class Archive>
  void serialize( Archive& ar )
  {
    int                               filter_type_int = static_cast<int>( filter_type_ );
    std::vector<SerializableMatrixXf> serializable_bases;
    SerializableMatrixXf              serializable_poly_coeffs;

    if ( !Archive::is_loading::value )
    {
      serializable_bases.reserve( spatial_bases_.size() );
      for ( const auto& b : spatial_bases_ )
      {
        serializable_bases.emplace_back( b );
      }
      serializable_poly_coeffs = SerializableMatrixXf( poly_coeffs_ );
    }

    ar( cereal::make_nvp( "filter_type", filter_type_int ),
        cereal::make_nvp( "sigma_min", sigma_min_ ),
        cereal::make_nvp( "sigma_max", sigma_max_ ),
        cereal::make_nvp( "degree", degree_ ),
        cereal::make_nvp( "energy_ratio_threshold", energy_ratio_threshold_ ),
        cereal::make_nvp( "energy_ratio_achieved", energy_ratio_achieved_ ),
        cereal::make_nvp( "K", K_ ),
        cereal::make_nvp( "kernel_size", kernel_size_ ),
        cereal::make_nvp( "poly_coeffs", serializable_poly_coeffs ),
        cereal::make_nvp( "spatial_bases", serializable_bases ) );

    if ( Archive::is_loading::value )
    {
      filter_type_ = static_cast<ESpectralFilterType>( filter_type_int );
      poly_coeffs_ = serializable_poly_coeffs.matrix;
      spatial_bases_.clear();
      spatial_bases_.reserve( serializable_bases.size() );
      for ( const auto& sb : serializable_bases )
      {
        spatial_bases_.emplace_back( sb.matrix );
      }
    }
  }

private:
  ESpectralFilterType          filter_type_;
  float                        sigma_min_;
  float                        sigma_max_;
  int                          degree_;
  float                        energy_ratio_threshold_;
  float                        energy_ratio_achieved_;
  int                          K_;
  int                          kernel_size_;
  std::vector<Eigen::MatrixXf> spatial_bases_;
  Eigen::MatrixXf              poly_coeffs_;
};

/**
 * @brief Generate a scale-normalized discrete Laplacian of Gaussian (sLoG) 2D kernel:
 *        sLoG(x, y, s) = s^2 * laplacian(g(x, y, s)) = -1/(pi * s^2) * (1 - r^2 / (2 * s^2)) * exp(-r^2 / (2 * s^2))
 * @param kernel_size Dimension of the square kernel (must be odd)
 * @param sigma Scale parameter
 */
Eigen::MatrixXf GenerateScaleNormalizedLoG( int kernel_size, float sigma );

/**
 * @brief Generate a discrete 2D Gaussian kernel:
 *        g(x, y, s) = 1 / (2 * pi * s^2) * exp(-r^2 / (2 * s^2))
 * @param kernel_size Dimension of the square kernel (must be odd)
 * @param sigma Scale parameter
 */
Eigen::MatrixXf GenerateGaussian( int kernel_size, float sigma );

/**
 * @brief Compute the optimal orthogonal spatial bases and polynomial weights using SVD on sampled filters.
 *        Selects the minimum number of bases K to satisfy the target energy conservation ratio.
 * @param sigma_min Minimum scale of the range (e.g. 1.6f)
 * @param sigma_max Maximum scale of the range (e.g. 3.2f)
 * @param degree Polynomial degree (must be 3 or 4)
 * @param energy_threshold Target energy conservation threshold (e.g. 0.999f for 99.9%)
 * @param filter_type FILTER_SLOG or FILTER_GAUSSIAN
 * @param[out] basis The resulting SpectralBasis object
 * @param num_scale_samples Number of discrete scale samples for SVD (default 100)
 * @return True if successful
 */
bool ComputeSpectralBasis( float               sigma_min,
                           float               sigma_max,
                           int                 degree,
                           float               energy_threshold,
                           ESpectralFilterType filter_type,
                           SpectralBasis&      basis,
                           int                 num_scale_samples = 100 );

/**
 * @brief Save a SpectralBasis object to a JSON file using Cereal
 */
bool SaveSpectralBasis( const std::string& filename, const SpectralBasis& basis );

/**
 * @brief Load a SpectralBasis object from a JSON file using Cereal
 */
bool LoadSpectralBasis( const std::string& filename, SpectralBasis& basis );

} // namespace spectral_sift
} // namespace features
} // namespace openMVG

#endif // OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_BASIS_HPP
