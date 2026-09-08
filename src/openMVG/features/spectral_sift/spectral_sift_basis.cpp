// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/spectral_sift/spectral_sift_basis.hpp"
#include "openMVG/system/logger.hpp"

#include <Eigen/QR>
#include <Eigen/SVD>

#include <cereal/archives/json.hpp>

#include <fstream>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace openMVG
{
namespace features
{
namespace spectral_sift
{

Eigen::MatrixXf SpectralBasis::ReconstructFilter( float sigma ) const
{
  Eigen::MatrixXf filter = Eigen::MatrixXf::Zero( kernel_size_, kernel_size_ );
  const float     t      = ScaleToT( sigma );
  for ( int k = 0; k < K_; ++k )
  {
    const float weight = EvaluatePolynomial( k, t );
    filter += weight * spatial_bases_[ k ];
  }
  return filter;
}

Eigen::MatrixXf GenerateScaleNormalizedLoG( int kernel_size, float sigma )
{
  Eigen::MatrixXf kernel( kernel_size, kernel_size );
  const int       radius      = kernel_size / 2;
  const float     s2          = sigma * sigma;
  const float     two_s2      = 2.0f * s2;
  const float     norm_factor = -1.0f / ( static_cast<float>( M_PI ) * s2 );

  double sum = 0.0;
  for ( int y = 0; y < kernel_size; ++y )
  {
    const float dy  = static_cast<float>( y - radius );
    const float dy2 = dy * dy;
    for ( int x = 0; x < kernel_size; ++x )
    {
      const float dx  = static_cast<float>( x - radius );
      const float r2  = dx * dx + dy2;
      const float val = norm_factor * ( 1.0f - r2 / two_s2 ) * std::exp( -r2 / two_s2 );
      kernel( y, x )  = val;
      sum += val;
    }
  }

  // Ensure discrete zero-sum (zero DC component) for scale-space invariance
  const float mean_val = static_cast<float>( sum / ( kernel_size * kernel_size ) );
  kernel.array() -= mean_val;

  return kernel;
}

Eigen::MatrixXf GenerateGaussian( int kernel_size, float sigma )
{
  Eigen::MatrixXf kernel( kernel_size, kernel_size );
  const int       radius      = kernel_size / 2;
  const float     s2          = sigma * sigma;
  const float     two_s2      = 2.0f * s2;
  const float     norm_factor = 1.0f / ( 2.0f * static_cast<float>( M_PI ) * s2 );

  double sum = 0.0;
  for ( int y = 0; y < kernel_size; ++y )
  {
    const float dy  = static_cast<float>( y - radius );
    const float dy2 = dy * dy;
    for ( int x = 0; x < kernel_size; ++x )
    {
      const float dx  = static_cast<float>( x - radius );
      const float r2  = dx * dx + dy2;
      const float val = norm_factor * std::exp( -r2 / two_s2 );
      kernel( y, x )  = val;
      sum += val;
    }
  }

  // Normalize discrete Gaussian so that sum equals 1
  if ( sum > 0.0 )
  {
    kernel.array() /= static_cast<float>( sum );
  }

  return kernel;
}

bool ComputeSpectralBasis( float               sigma_min,
                           float               sigma_max,
                           int                 degree,
                           float               energy_threshold,
                           ESpectralFilterType filter_type,
                           SpectralBasis&      basis,
                           int                 num_scale_samples )
{
  if ( sigma_min <= 0.0f || sigma_max <= sigma_min )
  {
    OPENMVG_LOG_ERROR << "[Spectral SIFT] Invalid scale range: [" << sigma_min << ", " << sigma_max << "]";
    return false;
  }

  // Enforce degree 3 (analytical) or 4 (Eigen companion solver)
  if ( degree != 3 && degree != 4 )
  {
    OPENMVG_LOG_ERROR << "[Spectral SIFT] Invalid degree: " << degree << ". Only degrees 3 and 4 are permitted.";
    return false;
  }

  if ( energy_threshold <= 0.0f || energy_threshold > 1.0f )
  {
    OPENMVG_LOG_ERROR << "[Spectral SIFT] Invalid energy threshold: " << energy_threshold;
    return false;
  }

  if ( num_scale_samples < degree + 1 )
  {
    num_scale_samples = 100;
  }

  // Determine kernel dimension L = 2 * R + 1
  const int radius      = static_cast<int>( std::ceil( 3.5f * sigma_max ) );
  const int kernel_size = 2 * radius + 1;
  const int num_pixels  = kernel_size * kernel_size;

  // Build matrix A (num_pixels x num_scale_samples)
  Eigen::MatrixXf    A( num_pixels, num_scale_samples );
  std::vector<float> sigmas( num_scale_samples );
  std::vector<float> t_vals( num_scale_samples );

  const float delta_sigma = ( sigma_max - sigma_min ) / static_cast<float>( num_scale_samples - 1 );
  for ( int j = 0; j < num_scale_samples; ++j )
  {
    const float s = sigma_min + static_cast<float>( j ) * delta_sigma;
    sigmas[ j ]   = s;
    t_vals[ j ]   = 2.0f * ( s - sigma_min ) / ( sigma_max - sigma_min ) - 1.0f;

    Eigen::MatrixXf f = ( filter_type == FILTER_SLOG )
                            ? GenerateScaleNormalizedLoG( kernel_size, s )
                            : GenerateGaussian( kernel_size, s );

    // Flatten filter row by row into column j of A
    for ( int y = 0; y < kernel_size; ++y )
    {
      for ( int x = 0; x < kernel_size; ++x )
      {
        A( y * kernel_size + x, j ) = f( y, x );
      }
    }
  }

  // Perform Thin SVD using Divide and Conquer BDCSVD
  Eigen::BDCSVD<Eigen::MatrixXf> svd( A, Eigen::ComputeThinU | Eigen::ComputeThinV );
  const Eigen::VectorXf&         S = svd.singularValues();
  const Eigen::MatrixXf&         U = svd.matrixU();
  const Eigen::MatrixXf&         V = svd.matrixV();

  // Compute cumulative energy ratio
  const float total_energy = S.squaredNorm();
  float       cum_energy   = 0.0f;
  int         K            = 0;
  for ( int i = 0; i < S.size(); ++i )
  {
    cum_energy += S( i ) * S( i );
    if ( ( cum_energy / total_energy ) >= energy_threshold )
    {
      K = i + 1;
      break;
    }
  }
  if ( K == 0 )
  {
    K = static_cast<int>( S.size() );
  }

  const float achieved_energy = cum_energy / total_energy;

  // Extract K spatial basis filters
  std::vector<Eigen::MatrixXf> spatial_bases;
  spatial_bases.reserve( K );
  for ( int k = 0; k < K; ++k )
  {
    Eigen::MatrixXf Bk( kernel_size, kernel_size );
    for ( int y = 0; y < kernel_size; ++y )
    {
      for ( int x = 0; x < kernel_size; ++x )
      {
        Bk( y, x ) = U( y * kernel_size + x, k );
      }
    }
    spatial_bases.emplace_back( std::move( Bk ) );
  }

  // Fit polynomial w_k(t) of degree d to discrete weights S(k) * V(j, k)
  // Vandermonde matrix on normalized variable t in [-1, 1]
  const int       num_coeffs = degree + 1;
  Eigen::MatrixXf X( num_scale_samples, num_coeffs );
  for ( int j = 0; j < num_scale_samples; ++j )
  {
    const float t  = t_vals[ j ];
    float       tp = 1.0f;
    for ( int p = 0; p <= degree; ++p )
    {
      X( j, p ) = tp;
      tp *= t;
    }
  }

  Eigen::ColPivHouseholderQR<Eigen::MatrixXf> qr( X );
  Eigen::MatrixXf                             poly_coeffs( K, num_coeffs );

  for ( int k = 0; k < K; ++k )
  {
    const Eigen::VectorXf W_k = S( k ) * V.col( k );
    const Eigen::VectorXf c_k = qr.solve( W_k );
    poly_coeffs.row( k )      = c_k.transpose();
  }

  // Populate output basis object
  basis.SetFilterType( filter_type );
  basis.SetSigmaRange( sigma_min, sigma_max );
  basis.SetDegree( degree );
  basis.SetEnergyRatioThreshold( energy_threshold );
  basis.SetEnergyRatioAchieved( achieved_energy );
  basis.SetK( K );
  basis.SetKernelSize( kernel_size );
  basis.SetSpatialBases( spatial_bases );
  basis.SetPolyCoeffs( poly_coeffs );

  return true;
}

bool SaveSpectralBasis( const std::string& filename, const SpectralBasis& basis )
{
  std::ofstream ofs( filename );
  if ( !ofs.is_open() )
  {
    OPENMVG_LOG_ERROR << "[Spectral SIFT] Failed to open file for writing: " << filename;
    return false;
  }
  try
  {
    cereal::JSONOutputArchive archive( ofs );
    archive( cereal::make_nvp( "spectral_basis", basis ) );
  }
  catch ( const std::exception& e )
  {
    OPENMVG_LOG_ERROR << "[Spectral SIFT] Cereal serialization failed: " << e.what();
    return false;
  }
  return true;
}

bool LoadSpectralBasis( const std::string& filename, SpectralBasis& basis )
{
  std::ifstream ifs( filename );
  if ( !ifs.is_open() )
  {
    OPENMVG_LOG_ERROR << "[Spectral SIFT] Failed to open file for reading: " << filename;
    return false;
  }
  try
  {
    cereal::JSONInputArchive archive( ifs );
    archive( cereal::make_nvp( "spectral_basis", basis ) );
  }
  catch ( const std::exception& e )
  {
    OPENMVG_LOG_ERROR << "[Spectral SIFT] Cereal deserialization failed: " << e.what();
    return false;
  }
  return true;
}

} // namespace spectral_sift
} // namespace features
} // namespace openMVG
