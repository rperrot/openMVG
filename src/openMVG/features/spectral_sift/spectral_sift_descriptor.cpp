// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/spectral_sift/spectral_sift_descriptor.hpp"
#include "openMVG/image/image_resampling.hpp"
#include "openMVG/system/logger.hpp"

#include <algorithm>
#include <array>
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

namespace
{

using RowMatrix = image::Image<float>::Base;

void ComputeCentralDiffGradients( const image::Image<float>& src,
                                  image::Image<float>&       gx,
                                  image::Image<float>&       gy )
{
  const int W = src.Width();
  const int H = src.Height();
  gx.resize( W, H );
  gy.resize( W, H );
  gx.fill( 0.0f );
  gy.fill( 0.0f );

  if ( H > 2 && W > 2 )
  {
    static_cast<RowMatrix&>( gx ).block( 1, 1, H - 2, W - 2 ) =
        0.5f * ( static_cast<const RowMatrix&>( src ).block( 1, 2, H - 2, W - 2 ) -
                 static_cast<const RowMatrix&>( src ).block( 1, 0, H - 2, W - 2 ) );

    static_cast<RowMatrix&>( gy ).block( 1, 1, H - 2, W - 2 ) =
        0.5f * ( static_cast<const RowMatrix&>( src ).block( 2, 1, H - 2, W - 2 ) -
                 static_cast<const RowMatrix&>( src ).block( 0, 1, H - 2, W - 2 ) );
  }
}

void EvalClampedWeights( const SpectralBasis& basis,
                         float                sigma,
                         std::vector<float>&  weights )
{
  const int   K = basis.GetK();
  const float t = std::max( -1.0f, std::min( 1.0f, basis.ScaleToT( sigma ) ) );
  weights.resize( K );
  for ( int k = 0; k < K; ++k )
  {
    weights[ k ] = basis.EvaluatePolynomial( k, t );
  }
}

inline bool SampleSingleGradientBilinear( const image::Image<float>& gx_im,
                                          const image::Image<float>& gy_im,
                                          float                      x,
                                          float                      y,
                                          float&                     gx_out,
                                          float&                     gy_out )
{
  const int W = gx_im.Width();
  const int H = gx_im.Height();
  // Exclude the 1-pixel zero border produced by central differences
  if ( x < 1.0f || x >= static_cast<float>( W - 2 ) ||
       y < 1.0f || y >= static_cast<float>( H - 2 ) )
  {
    return false;
  }

  const int   x0  = static_cast<int>( std::floor( x ) );
  const int   y0  = static_cast<int>( std::floor( y ) );
  const float fx  = x - static_cast<float>( x0 );
  const float fy  = y - static_cast<float>( y0 );
  const float w00 = ( 1.0f - fx ) * ( 1.0f - fy );
  const float w10 = fx * ( 1.0f - fy );
  const float w01 = ( 1.0f - fx ) * fy;
  const float w11 = fx * fy;

  gx_out = w00 * gx_im( y0, x0 ) + w10 * gx_im( y0, x0 + 1 ) +
           w01 * gx_im( y0 + 1, x0 ) + w11 * gx_im( y0 + 1, x0 + 1 );
  gy_out = w00 * gy_im( y0, x0 ) + w10 * gy_im( y0, x0 + 1 ) +
           w01 * gy_im( y0 + 1, x0 ) + w11 * gy_im( y0 + 1, x0 + 1 );
  return true;
}

inline bool SampleBandGradientsBilinear( const std::vector<image::Image<float>>& Gx,
                                         const std::vector<image::Image<float>>& Gy,
                                         const float*                            weights,
                                         int                                     K,
                                         float                                   x,
                                         float                                   y,
                                         float&                                  gx_out,
                                         float&                                  gy_out )
{
  if ( Gx.empty() || Gy.empty() )
  {
    return false;
  }

  const int W = Gx[ 0 ].Width();
  const int H = Gx[ 0 ].Height();
  if ( x < 1.0f || x >= static_cast<float>( W - 2 ) ||
       y < 1.0f || y >= static_cast<float>( H - 2 ) )
  {
    return false;
  }

  const int   x0  = static_cast<int>( std::floor( x ) );
  const int   y0  = static_cast<int>( std::floor( y ) );
  const float fx  = x - static_cast<float>( x0 );
  const float fy  = y - static_cast<float>( y0 );
  const float w00 = ( 1.0f - fx ) * ( 1.0f - fy );
  const float w10 = fx * ( 1.0f - fy );
  const float w01 = ( 1.0f - fx ) * fy;
  const float w11 = fx * fy;

  float gx = 0.0f;
  float gy = 0.0f;
  for ( int k = 0; k < K; ++k )
  {
    const float wk = weights[ k ];
    if ( wk == 0.0f )
    {
      continue;
    }
    const auto& gx_k = Gx[ k ];
    const auto& gy_k = Gy[ k ];
    gx += wk * ( w00 * gx_k( y0, x0 ) + w10 * gx_k( y0, x0 + 1 ) +
                 w01 * gx_k( y0 + 1, x0 ) + w11 * gx_k( y0 + 1, x0 + 1 ) );
    gy += wk * ( w00 * gy_k( y0, x0 ) + w10 * gy_k( y0, x0 + 1 ) +
                 w01 * gy_k( y0 + 1, x0 ) + w11 * gy_k( y0 + 1, x0 + 1 ) );
  }

  gx_out = gx;
  gy_out = gy;
  return true;
}

void ReconstructSmoothedAt2Sigma( const SpectralBasis&                    basis_gauss,
                                  float                                   sigma_min,
                                  const std::vector<image::Image<float>>& G_images,
                                  int                                     W,
                                  int                                     H,
                                  image::Image<float>&                    smoothed )
{
  smoothed.resize( W, H );
  static_cast<RowMatrix&>( smoothed ).setZero();
  const int   K_gauss  = basis_gauss.GetK();
  const float t_2sigma = std::max( -1.0f, std::min( 1.0f, basis_gauss.ScaleToT( 2.0f * sigma_min ) ) );
  for ( int k = 0; k < K_gauss; ++k )
  {
    const float w = basis_gauss.EvaluatePolynomial( k, t_2sigma );
    if ( w != 0.0f )
    {
      static_cast<RowMatrix&>( smoothed ) += w * static_cast<const RowMatrix&>( G_images[ k ] );
    }
  }
}

} // namespace

SpectralSIFTDescriptorExtractor::SpectralSIFTDescriptorExtractor()
    : params_( Params() ),
      is_initialized_( false ),
      has_low_basis_( false ),
      convolver_nom_ready_( false ),
      convolver_low_ready_( false ),
      convolver_coarse_ready_( false ),
      pool_( nullptr ),
      low_nom_split_( 1.27f ),
      nom_next_split_( 4.20f )
{
}

SpectralSIFTDescriptorExtractor::SpectralSIFTDescriptorExtractor( const Params& params )
    : params_( params ),
      is_initialized_( false ),
      has_low_basis_( false ),
      convolver_nom_ready_( false ),
      convolver_low_ready_( false ),
      convolver_coarse_ready_( false ),
      pool_( nullptr ),
      low_nom_split_( 1.27f ),
      nom_next_split_( 4.20f )
{
}

bool SpectralSIFTDescriptorExtractor::Init()
{
  if ( is_initialized_ )
  {
    return true;
  }

  convolver_nom_ready_    = false;
  convolver_low_ready_    = false;
  convolver_coarse_ready_ = false;

  if ( params_.sigma_min_ <= 0.0f || params_.sigma_max_ <= params_.sigma_min_ || params_.margin_factor_ < 1.0f )
  {
    OPENMVG_LOG_ERROR << "[SpectralSIFTDescriptorExtractor] Invalid scale parameters.";
    return false;
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

  const float two_smin = 2.0f * params_.sigma_min_;
  nom_next_split_      = std::min( s_max * 0.93f, std::hypot( two_smin, 2.0f * s_min * 1.2f ) );
  if ( nom_next_split_ <= s_min )
  {
    nom_next_split_ = s_max;
  }

  has_low_basis_ = false;
  dsp_scales_.clear();
  canonical_grid_.clear();

  if ( IsDSPEnabled( params_.normalization_ ) )
  {
    if ( params_.dsp_min_scale_ <= 0.0f ||
         params_.dsp_max_scale_ < params_.dsp_min_scale_ ||
         params_.dsp_num_scales_ < 1 )
    {
      OPENMVG_LOG_ERROR << "[SpectralSIFTDescriptorExtractor] Invalid DSP parameters.";
      return false;
    }

    // Precompute logarithmic DSP scale multipliers
    dsp_scales_.resize( static_cast<size_t>( params_.dsp_num_scales_ ) );
    if ( params_.dsp_num_scales_ == 1 )
    {
      dsp_scales_[ 0 ] = ( params_.dsp_min_scale_ == params_.dsp_max_scale_ ) ? params_.dsp_min_scale_ : 1.0f;
    }
    else
    {
      const float log_min  = std::log( params_.dsp_min_scale_ );
      const float log_max  = std::log( params_.dsp_max_scale_ );
      const float log_step = ( log_max - log_min ) / static_cast<float>( params_.dsp_num_scales_ - 1 );
      for ( int i = 0; i < params_.dsp_num_scales_; ++i )
      {
        dsp_scales_[ i ] = std::exp( log_min + static_cast<float>( i ) * log_step );
      }
      dsp_scales_.front() = params_.dsp_min_scale_;
      dsp_scales_.back()  = params_.dsp_max_scale_;
    }

    // Precompute 20x20 canonical descriptor grid over u, v in (-1, 4)
    constexpr int   kGridSize = 20;
    constexpr float kStep     = 5.0f / static_cast<float>( kGridSize );
    canonical_grid_.reserve( kGridSize * kGridSize );
    for ( int iy = 0; iy < kGridSize; ++iy )
    {
      const float v     = -1.0f + ( static_cast<float>( iy ) + 0.5f ) * kStep;
      const float y_rot = v - 1.5f;
      const int   v0    = static_cast<int>( std::floor( v ) );
      const float dv    = v - static_cast<float>( v0 );

      for ( int ix = 0; ix < kGridSize; ++ix )
      {
        const float u     = -1.0f + ( static_cast<float>( ix ) + 0.5f ) * kStep;
        const float x_rot = u - 1.5f;
        const int   u0    = static_cast<int>( std::floor( u ) );
        const float du    = u - static_cast<float>( u0 );

        CanonicalSample sample;
        sample.x_rot     = x_rot;
        sample.y_rot     = y_rot;
        sample.w_spatial = std::exp( -( x_rot * x_rot + y_rot * y_rot ) * ( 1.0f / 8.0f ) );
        sample.u0        = u0;
        sample.v0        = v0;
        sample.wu[ 0 ]   = 1.0f - du;
        sample.wu[ 1 ]   = du;
        sample.wv[ 0 ]   = 1.0f - dv;
        sample.wv[ 1 ]   = dv;
        canonical_grid_.push_back( sample );
      }
    }

    // Build low-scale Gaussian band [0.50, ~1.41] when DSP reaches below nominal s_min
    if ( params_.dsp_min_scale_ * params_.sigma_min_ < s_min && s_min > 0.50f )
    {
      const float low_min = 0.50f;
      const float low_max = std::max( low_min * 1.5f, std::min( s_min * 1.25f, 4.0f * low_min ) );
      if ( !ComputeSpectralBasis( low_min,
                                  low_max,
                                  params_.degree_,
                                  params_.energy_threshold_,
                                  FILTER_GAUSSIAN,
                                  basis_low_ ) )
      {
        OPENMVG_LOG_ERROR << "[SpectralSIFTDescriptorExtractor] Failed to compute low-scale Gaussian spectral basis.";
        return false;
      }
      has_low_basis_ = true;
      low_nom_split_ = 0.5f * ( s_min + std::min( low_max, s_max ) );
    }
  }

  is_initialized_ = true;
  return true;
}

bool SpectralSIFTDescriptorExtractor::PrepareOctaveNominal( OctaveNominalCache& cache,
                                                            bool                compute_smoothed_at_2sigma,
                                                            bool                use_coarse_convolver )
{
  if ( !Init() )
  {
    return false;
  }

  const int W = cache.octave_image.Width();
  const int H = cache.octave_image.Height();
  if ( W <= 0 || H <= 0 )
  {
    return false;
  }

  const int K_gauss = basis_gauss_.GetK();

  SpectralConvolver& conv       = use_coarse_convolver ? convolver_coarse_ : convolver_;
  bool&              conv_ready = use_coarse_convolver ? convolver_coarse_ready_ : convolver_nom_ready_;

  // If nominal gradients are already computed, only reconstruct smoothed_at_2sigma if newly requested
  if ( !cache.valid || static_cast<int>( cache.Gx_nom.size() ) != K_gauss ||
       cache.Gx_nom[ 0 ].Width() != W || cache.Gx_nom[ 0 ].Height() != H )
  {
    if ( !conv_ready || conv.GetCachedWidth() != W || conv.GetCachedHeight() != H ||
         conv.GetNumKernels() != K_gauss )
    {
      conv.Init( W, H, basis_gauss_.GetSpatialBases() );
      conv_ready = true;
    }

    if ( cache.octave_fft.rows() != H || cache.octave_fft.cols() != W )
    {
      conv.ComputeImageFFT( cache.octave_image, cache.octave_fft );
    }

    std::vector<image::Image<float>> G_images;
    conv.ConvolveAllFromFFT( cache.octave_fft, G_images );

    if ( compute_smoothed_at_2sigma )
    {
      ReconstructSmoothedAt2Sigma( basis_gauss_, params_.sigma_min_, G_images, W, H, cache.smoothed_at_2sigma );
      cache.has_smoothed = true;
    }

    cache.Gx_nom.resize( K_gauss );
    cache.Gy_nom.resize( K_gauss );
    for ( int k = 0; k < K_gauss; ++k )
    {
      ComputeCentralDiffGradients( G_images[ k ], cache.Gx_nom[ k ], cache.Gy_nom[ k ] );
    }

    cache.valid = true;
  }
  else if ( compute_smoothed_at_2sigma && !cache.has_smoothed )
  {
    if ( !conv_ready || conv.GetCachedWidth() != W || conv.GetCachedHeight() != H ||
         conv.GetNumKernels() != K_gauss )
    {
      conv.Init( W, H, basis_gauss_.GetSpatialBases() );
      conv_ready = true;
    }
    if ( cache.octave_fft.rows() != H || cache.octave_fft.cols() != W )
    {
      conv.ComputeImageFFT( cache.octave_image, cache.octave_fft );
    }

    std::vector<image::Image<float>> G_images;
    conv.ConvolveAllFromFFT( cache.octave_fft, G_images );
    ReconstructSmoothedAt2Sigma( basis_gauss_, params_.sigma_min_, G_images, W, H, cache.smoothed_at_2sigma );
    cache.has_smoothed = true;
  }

  return true;
}

void SpectralSIFTDescriptorExtractor::DescribeOctaveWithCache( const OctaveNominalCache&              curr_cache,
                                                               const OctaveNominalCache*              next_cache,
                                                               int                                    octave_idx,
                                                               const std::vector<SpectralKeypoint>&   input_keypoints,
                                                               std::vector<SpectralOrientedKeypoint>& output_keypoints )
{
  if ( input_keypoints.empty() || !curr_cache.valid )
  {
    return;
  }

  if ( !Init() )
  {
    return;
  }

  const int   num_kps     = static_cast<int>( input_keypoints.size() );
  ThreadPool* pool        = pool_ ? pool_ : &ThreadPool::GlobalInstance();
  const int   num_threads = std::max( 1, static_cast<int>( pool->NumThreads() ) );

  std::vector<std::vector<SpectralOrientedKeypoint>> thread_kps( num_threads );

  if ( !IsDSPEnabled( params_.normalization_ ) )
  {
    ParallelForWithId(
        0, num_kps,
        [&]( int i, int tid ) {
          const auto&        kp = input_keypoints[ i ];
          std::vector<float> orientations;
          ComputeOrientations( curr_cache.Gx_nom, curr_cache.Gy_nom, kp, orientations );

          for ( const float theta : orientations )
          {
            SpectralOrientedKeypoint okp;
            okp.x      = kp.x;
            okp.y      = kp.y;
            okp.sigma  = kp.sigma;
            okp.theta  = theta;
            okp.octave = octave_idx;
            okp.val    = kp.val;

            ComputeDescriptor( curr_cache.Gx_nom, curr_cache.Gy_nom, kp, theta, okp.descr );
            thread_kps[ tid ].emplace_back( std::move( okp ) );
          }
        },
        pool );
  }
  else
  {
    const int W = curr_cache.octave_image.Width();
    const int H = curr_cache.octave_image.Height();

    float min_kp_sigma = input_keypoints[ 0 ].sigma;
    float max_kp_sigma = input_keypoints[ 0 ].sigma;
    for ( int i = 1; i < num_kps; ++i )
    {
      min_kp_sigma = std::min( min_kp_sigma, input_keypoints[ i ].sigma );
      max_kp_sigma = std::max( max_kp_sigma, input_keypoints[ i ].sigma );
    }

    const float min_sigma_req = min_kp_sigma * dsp_scales_.front();
    const float max_sigma_req = max_kp_sigma * dsp_scales_.back();

    image::Image<float> Gx_raw;
    image::Image<float> Gy_raw;
    const bool          need_raw = ( min_sigma_req < 0.50f );
    if ( need_raw )
    {
      ComputeCentralDiffGradients( curr_cache.octave_image, Gx_raw, Gy_raw );
    }

    std::vector<image::Image<float>> Gx_low;
    std::vector<image::Image<float>> Gy_low;
    if ( has_low_basis_ && min_sigma_req <= low_nom_split_ )
    {
      const int K_low = basis_low_.GetK();
      if ( !convolver_low_ready_ || convolver_low_.GetCachedWidth() != W ||
           convolver_low_.GetCachedHeight() != H || convolver_low_.GetNumKernels() != K_low )
      {
        convolver_low_.Init( W, H, basis_low_.GetSpatialBases() );
        convolver_low_ready_ = true;
      }

      std::vector<image::Image<float>> G_low;
      convolver_low_.ConvolveAllFromFFT( curr_cache.octave_fft, G_low );

      Gx_low.resize( K_low );
      Gy_low.resize( K_low );
      for ( int k = 0; k < K_low; ++k )
      {
        ComputeCentralDiffGradients( G_low[ k ], Gx_low[ k ], Gy_low[ k ] );
      }
    }

    const int                 L_nom        = basis_gauss_.GetKernelSize();
    const OctaveNominalCache* p_next       = nullptr;
    if ( next_cache != nullptr && next_cache->valid &&
         next_cache->octave_image.Width() >= L_nom &&
         next_cache->octave_image.Height() >= L_nom )
    {
      p_next = next_cache;
    }

    // Optional level-2 decimated cascade (W/4 x H/4) if user sets dsp_max_scale > 3.0
    const float        sigma_ceil_1 = std::hypot( 2.0f * params_.sigma_min_, 2.0f * basis_gauss_.GetSigmaMax() );
    OctaveNominalCache level2_cache;
    if ( max_sigma_req > sigma_ceil_1 && p_next != nullptr && p_next->has_smoothed )
    {
      const int l2_w = p_next->octave_image.Width() / 2;
      const int l2_h = p_next->octave_image.Height() / 2;
      if ( l2_w >= L_nom && l2_h >= L_nom )
      {
        image::ImageDecimate( p_next->smoothed_at_2sigma, level2_cache.octave_image );
        SpectralConvolver conv_l2;
        conv_l2.SetThreadPool( pool );
        conv_l2.Init( l2_w, l2_h, basis_gauss_.GetSpatialBases() );
        conv_l2.ComputeImageFFT( level2_cache.octave_image, level2_cache.octave_fft );

        std::vector<image::Image<float>> G_l2;
        conv_l2.ConvolveAllFromFFT( level2_cache.octave_fft, G_l2 );
        const int K_nom = basis_gauss_.GetK();
        level2_cache.Gx_nom.resize( K_nom );
        level2_cache.Gy_nom.resize( K_nom );
        for ( int k = 0; k < K_nom; ++k )
        {
          ComputeCentralDiffGradients( G_l2[ k ], level2_cache.Gx_nom[ k ], level2_cache.Gy_nom[ k ] );
        }
        level2_cache.valid = true;
      }
    }

    const image::Image<float>*              p_gx_raw = need_raw ? &Gx_raw : nullptr;
    const image::Image<float>*              p_gy_raw = need_raw ? &Gy_raw : nullptr;
    const std::vector<image::Image<float>>* p_gx_low = Gx_low.empty() ? nullptr : &Gx_low;
    const std::vector<image::Image<float>>* p_gy_low = Gy_low.empty() ? nullptr : &Gy_low;
    const OctaveNominalCache*               p_level2 = level2_cache.valid ? &level2_cache : nullptr;

    ParallelForWithId(
        0, num_kps,
        [&]( int i, int tid ) {
          const auto&        kp = input_keypoints[ i ];
          std::vector<float> orientations;
          ComputeOrientations( curr_cache.Gx_nom, curr_cache.Gy_nom, kp, orientations );

          for ( const float theta : orientations )
          {
            SpectralOrientedKeypoint okp;
            okp.x      = kp.x;
            okp.y      = kp.y;
            okp.sigma  = kp.sigma;
            okp.theta  = theta;
            okp.octave = octave_idx;
            okp.val    = kp.val;

            ComputeDescriptorDSP( curr_cache,
                                  p_gx_raw,
                                  p_gy_raw,
                                  p_gx_low,
                                  p_gy_low,
                                  p_next,
                                  p_level2,
                                  kp,
                                  theta,
                                  okp.descr );
            thread_kps[ tid ].emplace_back( std::move( okp ) );
          }
        },
        pool );
  }

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

  const int W = octave_image.Width();
  const int H = octave_image.Height();

  bool need_coarse = false;
  if ( IsDSPEnabled( params_.normalization_ ) && !input_keypoints.empty() )
  {
    float max_kp_sigma = input_keypoints[ 0 ].sigma;
    for ( size_t i = 1; i < input_keypoints.size(); ++i )
    {
      max_kp_sigma = std::max( max_kp_sigma, input_keypoints[ i ].sigma );
    }
    const int L_nom = basis_gauss_.GetKernelSize();
    if ( max_kp_sigma * dsp_scales_.back() > nom_next_split_ &&
         ( W / 2 ) >= L_nom && ( H / 2 ) >= L_nom )
    {
      need_coarse = true;
    }
  }

  const bool need_smoothed = ( smoothed_image_at_2sigma != nullptr ) || need_coarse;

  OctaveNominalCache curr_cache;
  curr_cache.octave_image = octave_image;
  if ( octave_fft != nullptr && octave_fft->rows() == H && octave_fft->cols() == W )
  {
    curr_cache.octave_fft = *octave_fft;
  }

  if ( !PrepareOctaveNominal( curr_cache, need_smoothed, false ) )
  {
    return;
  }

  if ( smoothed_image_at_2sigma != nullptr )
  {
    *smoothed_image_at_2sigma = curr_cache.smoothed_at_2sigma;
  }

  if ( input_keypoints.empty() )
  {
    return;
  }

  OctaveNominalCache next_cache;
  if ( need_coarse && curr_cache.has_smoothed )
  {
    image::ImageDecimate( curr_cache.smoothed_at_2sigma, next_cache.octave_image );
    const float sigma_ceil_1  = std::hypot( 2.0f * params_.sigma_min_, 2.0f * basis_gauss_.GetSigmaMax() );
    float       max_kp_sigma  = input_keypoints[ 0 ].sigma;
    for ( size_t i = 1; i < input_keypoints.size(); ++i )
    {
      max_kp_sigma = std::max( max_kp_sigma, input_keypoints[ i ].sigma );
    }
    const bool need_l2_smooth = ( max_kp_sigma * dsp_scales_.back() > sigma_ceil_1 );
    PrepareOctaveNominal( next_cache, need_l2_smooth, true );
  }

  DescribeOctaveWithCache( curr_cache,
                           next_cache.valid ? &next_cache : nullptr,
                           octave_idx,
                           input_keypoints,
                           output_keypoints );
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

  const float        t = std::max( -1.0f, std::min( 1.0f, basis_gauss_.ScaleToT( sigma ) ) );
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

  const float        t = std::max( -1.0f, std::min( 1.0f, basis_gauss_.ScaleToT( sigma ) ) );
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

  NormalizeDescriptor( descr );
}

void SpectralSIFTDescriptorExtractor::NormalizeDescriptor( Eigen::Matrix<float, 128, 1>& descr ) const
{
  float norm2 = descr.norm();
  if ( norm2 < 1e-7f )
  {
    descr.setZero();
    return;
  }
  descr /= norm2;

  // Threshold large values at clip_value (0.2)
  for ( int i = 0; i < 128; ++i )
  {
    descr[ i ] = std::min( descr[ i ], params_.clip_value_ );
  }

  if ( IsRootSIFTEnabled( params_.normalization_ ) )
  {
    // Root-SIFT: L1 normalize, square root, scale to 512
    const float norm1 = descr.lpNorm<1>() + 1e-7f;
    descr             = 512.0f * ( descr.cwiseMax( 0.0f ) / norm1 ).array().sqrt();
  }
  else
  {
    // Standard SIFT: L2 re-normalize, scale to 512
    norm2 = descr.norm() + 1e-7f;
    descr = 512.0f * ( descr / norm2 ).array();
  }
}

void SpectralSIFTDescriptorExtractor::ComputeDescriptorDSP( const OctaveNominalCache&               curr_cache,
                                                             const image::Image<float>*              Gx_raw,
                                                             const image::Image<float>*              Gy_raw,
                                                             const std::vector<image::Image<float>>* Gx_low,
                                                             const std::vector<image::Image<float>>* Gy_low,
                                                             const OctaveNominalCache*               next_cache,
                                                             const OctaveNominalCache*               level2_cache,
                                                             const SpectralKeypoint&                 kp,
                                                             float                                   orientation,
                                                             Eigen::Matrix<float, 128, 1>&           descr ) const
{
  descr.setZero();

  if ( dsp_scales_.empty() || canonical_grid_.empty() )
  {
    return;
  }

  const float cos_t              = std::cos( orientation );
  const float sin_t              = std::sin( orientation );
  const float two_pi             = 2.0f * static_cast<float>( M_PI );
  const float inv_angle_bin_size = 8.0f / two_pi;
  const float two_smin           = 2.0f * params_.sigma_min_;
  const float two_smin_sq        = two_smin * two_smin;
  const float l2_pre_sq          = 5.0f * two_smin_sq; // sigma^2 = (2*smin)^2 + (4*smin)^2

  const bool         low_available = has_low_basis_ && Gx_low != nullptr && Gy_low != nullptr && !Gx_low->empty();
  std::vector<float> weights;
  std::vector<float> weights_low05;
  if ( low_available )
  {
    EvalClampedWeights( basis_low_, 0.50f, weights_low05 );
  }

  enum class BandMode
  {
    RAW,
    BLEND_RAW_LOW,
    BAND
  };

  for ( const float alpha : dsp_scales_ )
  {
    const float sigma_m    = alpha * kp.sigma;
    const float cell_width = 3.0f * sigma_m;

    BandMode                                mode        = BandMode::BAND;
    float                                   w_low       = 0.0f;
    float                                   coord_scale = 1.0f;
    float                                   grad_scale  = 1.0f;
    const std::vector<image::Image<float>>* Gx_band     = &curr_cache.Gx_nom;
    const std::vector<image::Image<float>>* Gy_band     = &curr_cache.Gy_nom;

    if ( sigma_m < 0.50f && Gx_raw != nullptr && Gy_raw != nullptr )
    {
      w_low = ( sigma_m > 1e-4f )
                  ? std::min( 1.0f, std::max( 0.0f, std::exp( -0.5f / ( sigma_m * sigma_m ) + 2.0f ) ) )
                  : 0.0f;
      if ( !low_available || w_low < 1e-3f )
      {
        mode = BandMode::RAW;
      }
      else
      {
        mode = BandMode::BLEND_RAW_LOW;
      }
    }
    else if ( low_available && sigma_m <= low_nom_split_ )
    {
      EvalClampedWeights( basis_low_, sigma_m, weights );
      Gx_band     = Gx_low;
      Gy_band     = Gy_low;
      coord_scale = 1.0f;
      grad_scale  = 1.0f;
    }
    else if ( sigma_m <= nom_next_split_ || next_cache == nullptr )
    {
      EvalClampedWeights( basis_gauss_, sigma_m, weights );
      Gx_band     = &curr_cache.Gx_nom;
      Gy_band     = &curr_cache.Gy_nom;
      coord_scale = 1.0f;
      grad_scale  = 1.0f;
    }
    else
    {
      const float s_rem = 0.5f * std::sqrt( std::max( 0.0f, sigma_m * sigma_m - two_smin_sq ) );
      if ( s_rem <= basis_gauss_.GetSigmaMax() || level2_cache == nullptr )
      {
        EvalClampedWeights( basis_gauss_, s_rem, weights );
        Gx_band     = &next_cache->Gx_nom;
        Gy_band     = &next_cache->Gy_nom;
        coord_scale = 0.5f;
        grad_scale  = 0.5f;
      }
      else
      {
        const float s_rem2 = 0.25f * std::sqrt( std::max( 0.0f, sigma_m * sigma_m - l2_pre_sq ) );
        EvalClampedWeights( basis_gauss_, s_rem2, weights );
        Gx_band     = &level2_cache->Gx_nom;
        Gy_band     = &level2_cache->Gy_nom;
        coord_scale = 0.25f;
        grad_scale  = 0.25f;
      }
    }

    const int K_band = static_cast<int>( weights.size() );
    const int K_low  = static_cast<int>( weights_low05.size() );

    for ( const CanonicalSample& s : canonical_grid_ )
    {
      const float px_f = kp.x + ( cos_t * s.x_rot - sin_t * s.y_rot ) * cell_width;
      const float py_f = kp.y + ( sin_t * s.x_rot + cos_t * s.y_rot ) * cell_width;

      float gx = 0.0f;
      float gy = 0.0f;
      bool  ok = false;

      if ( mode == BandMode::RAW )
      {
        ok = SampleSingleGradientBilinear( *Gx_raw, *Gy_raw, px_f, py_f, gx, gy );
      }
      else if ( mode == BandMode::BLEND_RAW_LOW )
      {
        float rx = 0.0f;
        float ry = 0.0f;
        float lx = 0.0f;
        float ly = 0.0f;
        if ( SampleSingleGradientBilinear( *Gx_raw, *Gy_raw, px_f, py_f, rx, ry ) &&
             SampleBandGradientsBilinear( *Gx_low, *Gy_low, weights_low05.data(), K_low, px_f, py_f, lx, ly ) )
        {
          const float w_raw = 1.0f - w_low;
          gx                = w_raw * rx + w_low * lx;
          gy                = w_raw * ry + w_low * ly;
          ok                = true;
        }
      }
      else
      {
        ok = SampleBandGradientsBilinear(
            *Gx_band, *Gy_band, weights.data(), K_band, px_f * coord_scale, py_f * coord_scale, gx, gy );
      }

      if ( !ok )
      {
        continue;
      }

      const float weighted_mag = sigma_m * grad_scale * std::hypot( gx, gy ) * s.w_spatial;

      float diff_theta = std::atan2( gy, gx ) - orientation;
      while ( diff_theta < 0.0f )
        diff_theta += two_pi;
      while ( diff_theta >= two_pi )
        diff_theta -= two_pi;

      const float o       = diff_theta * inv_angle_bin_size;
      const float o_floor = std::floor( o );
      const int   o0      = static_cast<int>( o_floor ) & 7;
      const int   o1      = ( o0 + 1 ) & 7;
      const float do_val  = o - o_floor;
      const float wo0     = 1.0f - do_val;
      const float wo1     = do_val;

      for ( int iu = 0; iu <= 1; ++iu )
      {
        const int cell_u = s.u0 + iu;
        if ( cell_u < 0 || cell_u >= 4 )
        {
          continue;
        }
        const float w_u = weighted_mag * s.wu[ iu ];

        for ( int iv = 0; iv <= 1; ++iv )
        {
          const int cell_v = s.v0 + iv;
          if ( cell_v < 0 || cell_v >= 4 )
          {
            continue;
          }
          const float w_uv = w_u * s.wv[ iv ];
          const int   base = ( cell_v * 4 + cell_u ) * 8;
          descr[ base + o0 ] += w_uv * wo0;
          descr[ base + o1 ] += w_uv * wo1;
        }
      }
    }
  }

  descr /= static_cast<float>( dsp_scales_.size() );
  NormalizeDescriptor( descr );
}

} // namespace spectral_sift
} // namespace features
} // namespace openMVG
