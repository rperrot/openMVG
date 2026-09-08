// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/spectral_sift/spectral_sift_descriptor.hpp"
#include "openMVG/system/logger.hpp"

#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace openMVG
{
namespace features
{
namespace spectral_sift
{

SpectralSIFTDescriptorExtractor::SpectralSIFTDescriptorExtractor()
    : params_( Params() ),
      is_initialized_( false ),
      pool_( nullptr )
{
}

SpectralSIFTDescriptorExtractor::SpectralSIFTDescriptorExtractor( const Params& params )
    : params_( params ),
      is_initialized_( false ),
      pool_( nullptr )
{
}

bool SpectralSIFTDescriptorExtractor::Init()
{
  if ( is_initialized_ )
  {
    return true;
  }

  // Expand Gaussian spectral basis by margin_factor_ so that the nominal octave range [sigma_min, sigma_max]
  // and scale 2 * sigma_min (for inter-octave decimation) are squarely in the interior of the continuous polynomial.
  const float s_min = params_.sigma_min_ / params_.margin_factor_;
  const float s_max = params_.sigma_max_ * params_.margin_factor_;

  if ( !ComputeSpectralBasis( s_min,
                              s_max,
                              params_.degree_,
                              params_.energy_threshold_,
                              FILTER_GAUSSIAN,
                              basis_gauss_ ) )
  {
    OPENMVG_LOG_ERROR << "[SpectralSIFTDescriptorExtractor] Failed to compute Gaussian spectral basis.";
    return false;
  }

  is_initialized_ = true;
  return true;
}

void SpectralSIFTDescriptorExtractor::DescribeOctave( const image::Image<float>&             octave_image,
                                                      int                                    octave_idx,
                                                      const std::vector<SpectralKeypoint>&   input_keypoints,
                                                      std::vector<SpectralOrientedKeypoint>& output_keypoints,
                                                      const MatrixXcf*                       octave_fft,
                                                      image::Image<float>*                   smoothed_image_at_2sigma )
{
  if ( input_keypoints.empty() && smoothed_image_at_2sigma == nullptr )
  {
    return;
  }

  if ( !Init() )
  {
    return;
  }

  const int W       = octave_image.Width();
  const int H       = octave_image.Height();
  const int K_gauss = basis_gauss_.GetK();

  if ( convolver_.GetCachedWidth() != W || convolver_.GetCachedHeight() != H )
  {
    convolver_.Init( W, H, basis_gauss_.GetSpatialBases() );
  }

  // Convolve octave image with the K Gaussian basis filters
  std::vector<image::Image<float>> G_images;
  if ( octave_fft != nullptr )
  {
    if ( octave_fft->rows() != H || octave_fft->cols() != W )
    {
      convolver_.ComputeImageFFT( octave_image, const_cast<MatrixXcf&>( *octave_fft ) );
    }
    convolver_.ConvolveAllFromFFT( *octave_fft, G_images );
  }
  else
  {
    convolver_.ConvolveAll( octave_image, G_images );
  }

  using RowMatrix = image::Image<float>::Base;

  // Reconstruct smoothed image at sigma = 2 * sigma_min for downsampling the next octave
  if ( smoothed_image_at_2sigma != nullptr )
  {
    smoothed_image_at_2sigma->resize( W, H );
    static_cast<RowMatrix&>( *smoothed_image_at_2sigma ).setZero();
    const float t_2sigma = basis_gauss_.ScaleToT( 2.0f * params_.sigma_min_ );
    for ( int k = 0; k < K_gauss; ++k )
    {
      const float w = basis_gauss_.EvaluatePolynomial( k, t_2sigma );
      if ( w != 0.0f )
      {
        static_cast<RowMatrix&>( *smoothed_image_at_2sigma ) +=
            w * static_cast<const RowMatrix&>( G_images[ k ] );
      }
    }
  }

  if ( input_keypoints.empty() )
  {
    return;
  }

  // Precompute spatial central difference gradients for each basis image using Eigen block operations
  std::vector<image::Image<float>> Gx_images( K_gauss );
  std::vector<image::Image<float>> Gy_images( K_gauss );

  for ( int k = 0; k < K_gauss; ++k )
  {
    Gx_images[ k ].resize( W, H );
    Gy_images[ k ].resize( W, H );
    Gx_images[ k ].fill( 0.0f );
    Gy_images[ k ].fill( 0.0f );

    if ( H > 2 && W > 2 )
    {
      static_cast<RowMatrix&>( Gx_images[ k ] ).block( 1, 1, H - 2, W - 2 ) =
          0.5f * ( static_cast<const RowMatrix&>( G_images[ k ] ).block( 1, 2, H - 2, W - 2 ) -
                   static_cast<const RowMatrix&>( G_images[ k ] ).block( 1, 0, H - 2, W - 2 ) );

      static_cast<RowMatrix&>( Gy_images[ k ] ).block( 1, 1, H - 2, W - 2 ) =
          0.5f * ( static_cast<const RowMatrix&>( G_images[ k ] ).block( 2, 1, H - 2, W - 2 ) -
                   static_cast<const RowMatrix&>( G_images[ k ] ).block( 0, 1, H - 2, W - 2 ) );
    }
  }

  // Multi-threaded computation of orientations and descriptors across keypoints
  const int   num_kps     = static_cast<int>( input_keypoints.size() );
  ThreadPool* pool        = pool_ ? pool_ : &ThreadPool::GlobalInstance();
  const int   num_threads = std::max( 1, static_cast<int>( pool->NumThreads() ) );

  std::vector<std::vector<SpectralOrientedKeypoint>> thread_kps( num_threads );

  ParallelForWithId(
      0, num_kps,
      [&]( int i, int tid ) {
        const auto& kp = input_keypoints[ i ];
        std::vector<float> orientations;
        ComputeOrientations( Gx_images, Gy_images, kp, orientations );

        for ( const float theta : orientations )
        {
          SpectralOrientedKeypoint okp;
          okp.x      = kp.x;
          okp.y      = kp.y;
          okp.sigma  = kp.sigma;
          okp.theta  = theta;
          okp.octave = octave_idx;
          okp.val    = kp.val;

          ComputeDescriptor( Gx_images, Gy_images, kp, theta, okp.descr );
          thread_kps[ tid ].emplace_back( std::move( okp ) );
        }
      },
      pool );

  size_t total_described = 0;
  for ( const auto& kps : thread_kps )
  {
    total_described += kps.size();
  }
  output_keypoints.reserve( output_keypoints.size() + total_described );
  for ( auto& kps : thread_kps )
  {
    for ( auto& kp : kps )
    {
      output_keypoints.emplace_back( std::move( kp ) );
    }
  }
}

void SpectralSIFTDescriptorExtractor::ComputeOrientations( const std::vector<image::Image<float>>& Gx_images,
                                                           const std::vector<image::Image<float>>& Gy_images,
                                                           const SpectralKeypoint&                 kp,
                                                           std::vector<float>&                     orientations ) const
{
  orientations.clear();

  const int W       = Gx_images[ 0 ].Width();
  const int H       = Gx_images[ 0 ].Height();
  const int K_gauss = basis_gauss_.GetK();

  const float sigma                = kp.sigma;
  const float radius_ori           = 3.0f * params_.orientation_scale_ * sigma;
  const int   int_radius           = static_cast<int>( std::ceil( radius_ori ) );
  const float sigma_ori            = params_.orientation_scale_ * sigma;
  const float two_sigma_ori_sq     = 2.0f * sigma_ori * sigma_ori;
  const float inv_two_sigma_ori_sq = 1.0f / two_sigma_ori_sq;

  const float        t = basis_gauss_.ScaleToT( sigma );
  std::vector<float> weights( K_gauss );
  for ( int k = 0; k < K_gauss; ++k )
  {
    weights[ k ] = basis_gauss_.EvaluatePolynomial( k, t );
  }

  const int   num_bins     = 36;
  const float bin_size     = 2.0f * static_cast<float>( M_PI ) / static_cast<float>( num_bins );
  const float inv_bin_size = 1.0f / bin_size;
  std::vector<float> hist( num_bins, 0.0f );

  const int center_x = static_cast<int>( std::round( kp.x ) );
  const int center_y = static_cast<int>( std::round( kp.y ) );

  for ( int dy = -int_radius; dy <= int_radius; ++dy )
  {
    const int py = center_y + dy;
    if ( py < 1 || py >= H - 1 )
      continue;

    const float fdy  = static_cast<float>( py ) - kp.y;
    const float fdy2 = fdy * fdy;

    for ( int dx = -int_radius; dx <= int_radius; ++dx )
    {
      const int px = center_x + dx;
      if ( px < 1 || px >= W - 1 )
        continue;

      const float fdx = static_cast<float>( px ) - kp.x;
      const float r2  = fdx * fdx + fdy2;
      if ( r2 > radius_ori * radius_ori )
        continue;

      // Reconstruct gradients at continuous scale
      float gx = 0.0f;
      float gy = 0.0f;
      for ( int k = 0; k < K_gauss; ++k )
      {
        gx += weights[ k ] * Gx_images[ k ]( py, px );
        gy += weights[ k ] * Gy_images[ k ]( py, px );
      }

      const float mag   = std::hypot( gx, gy );
      float       angle = std::atan2( gy, gx );
      if ( angle < 0.0f )
      {
        angle += 2.0f * static_cast<float>( M_PI );
      }

      const float w   = std::exp( -r2 * inv_two_sigma_ori_sq );
      int         bin = static_cast<int>( std::floor( angle * inv_bin_size ) );
      bin             = std::max( 0, std::min( num_bins - 1, bin ) );

      hist[ bin ] += w * mag;
    }
  }

  // Smooth orientation histogram circularly
  for ( int pass = 0; pass < params_.nb_smoothing_step_; ++pass )
  {
    std::vector<float> smoothed( num_bins );
    for ( int b = 0; b < num_bins; ++b )
    {
      const int prev = ( b + num_bins - 1 ) % num_bins;
      const int next = ( b + 1 ) % num_bins;
      smoothed[ b ]  = 0.25f * hist[ prev ] + 0.5f * hist[ b ] + 0.25f * hist[ next ];
    }
    hist = std::move( smoothed );
  }

  // Find peak
  float max_val = 0.0f;
  for ( int b = 0; b < num_bins; ++b )
  {
    if ( hist[ b ] > max_val )
    {
      max_val = hist[ b ];
    }
  }

  if ( max_val < 1e-7f )
  {
    orientations.push_back( 0.0f );
    return;
  }

  const float peak_threshold = params_.percentage_keep_ * max_val;
  for ( int b = 0; b < num_bins; ++b )
  {
    const int prev = ( b + num_bins - 1 ) % num_bins;
    const int next = ( b + 1 ) % num_bins;

    if ( hist[ b ] >= hist[ prev ] && hist[ b ] >= hist[ next ] && hist[ b ] >= peak_threshold )
    {
      // Parabolic sub-bin peak fitting
      const float denom  = 2.0f * ( hist[ prev ] - 2.0f * hist[ b ] + hist[ next ] );
      float       offset = 0.0f;
      if ( std::abs( denom ) > 1e-7f )
      {
        offset = ( hist[ prev ] - hist[ next ] ) / denom;
      }

      float angle = ( static_cast<float>( b ) + 0.5f + offset ) * bin_size;
      while ( angle < 0.0f )
        angle += 2.0f * static_cast<float>( M_PI );
      while ( angle >= 2.0f * static_cast<float>( M_PI ) )
        angle -= 2.0f * static_cast<float>( M_PI );

      orientations.push_back( angle );
    }
  }

  if ( orientations.empty() )
  {
    orientations.push_back( 0.0f );
  }
}

void SpectralSIFTDescriptorExtractor::ComputeDescriptor( const std::vector<image::Image<float>>& Gx_images,
                                                         const std::vector<image::Image<float>>& Gy_images,
                                                         const SpectralKeypoint&                 kp,
                                                         float                                   orientation,
                                                         Eigen::Matrix<float, 128, 1>&           descr ) const
{
  descr.setZero();

  const int W       = Gx_images[ 0 ].Width();
  const int H       = Gx_images[ 0 ].Height();
  const int K_gauss = basis_gauss_.GetK();

  const float sigma                 = kp.sigma;
  const float cell_width            = 3.0f * sigma;
  const float inv_cell_width        = 1.0f / cell_width;
  const float descr_radius          = 2.0f * std::sqrt( 2.0f ) * 2.0f * cell_width;
  const int   int_radius            = static_cast<int>( std::ceil( descr_radius ) );
  const float inv_angle_bin_size    = 8.0f / ( 2.0f * static_cast<float>( M_PI ) );
  const float inv_two_sigma_grid_sq = 1.0f / 8.0f; // 2.0 * sigma_grid^2 = 2.0 * 2.0^2 = 8.0

  const float        t = basis_gauss_.ScaleToT( sigma );
  std::vector<float> weights( K_gauss );
  for ( int k = 0; k < K_gauss; ++k )
  {
    weights[ k ] = basis_gauss_.EvaluatePolynomial( k, t );
  }

  const float cos_t = std::cos( orientation );
  const float sin_t = std::sin( orientation );

  const int center_x = static_cast<int>( std::round( kp.x ) );
  const int center_y = static_cast<int>( std::round( kp.y ) );

  // 4x4 spatial cells, 8 angle bins = 128 dimensions
  for ( int dy = -int_radius; dy <= int_radius; ++dy )
  {
    const int py = center_y + dy;
    if ( py < 1 || py >= H - 1 )
      continue;

    const float delta_y = static_cast<float>( py ) - kp.y;

    for ( int dx = -int_radius; dx <= int_radius; ++dx )
    {
      const int px = center_x + dx;
      if ( px < 1 || px >= W - 1 )
        continue;

      const float delta_x = static_cast<float>( px ) - kp.x;

      // Rotate offset by -orientation and scale by cell width
      const float x_rot = ( cos_t * delta_x + sin_t * delta_y ) * inv_cell_width;
      const float y_rot = ( -sin_t * delta_x + cos_t * delta_y ) * inv_cell_width;

      // Cell coordinates centered at (1.5, 1.5)
      const float u = x_rot + 1.5f;
      const float v = y_rot + 1.5f;

      if ( u <= -1.0f || u >= 4.0f || v <= -1.0f || v >= 4.0f )
      {
        continue;
      }

      // Reconstruct continuous scale gradients
      float gx = 0.0f;
      float gy = 0.0f;
      for ( int k = 0; k < K_gauss; ++k )
      {
        gx += weights[ k ] * Gx_images[ k ]( py, px );
        gy += weights[ k ] * Gy_images[ k ]( py, px );
      }

      const float mag        = std::hypot( gx, gy );
      float       grad_theta = std::atan2( gy, gx );
      float       diff_theta = grad_theta - orientation;
      while ( diff_theta < 0.0f )
        diff_theta += 2.0f * static_cast<float>( M_PI );
      while ( diff_theta >= 2.0f * static_cast<float>( M_PI ) )
        diff_theta -= 2.0f * static_cast<float>( M_PI );

      const float o = diff_theta * inv_angle_bin_size;

      // Gaussian weighting window on descriptor patch (sigma_grid = 2.0)
      const float w_spatial    = std::exp( -( x_rot * x_rot + y_rot * y_rot ) * inv_two_sigma_grid_sq );
      const float weighted_mag = mag * w_spatial;

      // Trilinear interpolation: u, v, o
      const int u0 = static_cast<int>( std::floor( u ) );
      const int v0 = static_cast<int>( std::floor( v ) );
      const int o0 = static_cast<int>( std::floor( o ) );

      const float du     = u - static_cast<float>( u0 );
      const float dv     = v - static_cast<float>( v0 );
      const float do_val = o - static_cast<float>( o0 );

      for ( int iu = 0; iu <= 1; ++iu )
      {
        const int cell_u = u0 + iu;
        if ( cell_u < 0 || cell_u >= 4 )
          continue;
        const float wu = ( iu == 0 ) ? ( 1.0f - du ) : du;

        for ( int iv = 0; iv <= 1; ++iv )
        {
          const int cell_v = v0 + iv;
          if ( cell_v < 0 || cell_v >= 4 )
            continue;
          const float wv = ( iv == 0 ) ? ( 1.0f - dv ) : dv;

          for ( int io = 0; io <= 1; ++io )
          {
            const int   bin_o = ( o0 + io ) % 8;
            const float wo    = ( io == 0 ) ? ( 1.0f - do_val ) : do_val;

            const int descr_idx = ( cell_v * 4 + cell_u ) * 8 + bin_o;
            descr[ descr_idx ] += weighted_mag * wu * wv * wo;
          }
        }
      }
    }
  }

  // Normalization and thresholding
  float norm2 = descr.norm() + 1e-7f;
  descr /= norm2;

  // Threshold large values at clip_value (0.2)
  for ( int i = 0; i < 128; ++i )
  {
    descr[ i ] = std::min( descr[ i ], params_.clip_value_ );
  }

  if ( params_.root_sift_ )
  {
    // Root-SIFT: L1 normalize, square root, scale to 512
    const float norm1 = descr.lpNorm<1>() + 1e-7f;
    descr             = 512.0f * ( descr / norm1 ).array().sqrt();
  }
  else
  {
    // Standard SIFT: L2 re-normalize, scale to 512
    norm2 = descr.norm() + 1e-7f;
    descr = 512.0f * ( descr / norm2 ).array();
  }
}

} // namespace spectral_sift
} // namespace features
} // namespace openMVG
