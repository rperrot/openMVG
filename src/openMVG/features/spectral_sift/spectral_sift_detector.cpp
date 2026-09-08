// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/spectral_sift/spectral_sift_detector.hpp"
#include "openMVG/system/logger.hpp"

#include <unsupported/Eigen/Polynomials>

#include <algorithm>
#include <cmath>

#ifdef OPENMVG_USE_OPENMP
#include <omp.h>
#endif

namespace openMVG
{
namespace features
{
namespace spectral_sift
{

SpectralSIFTDetector::SpectralSIFTDetector()
    : params_( Params() ), is_initialized_( false )
{
}

SpectralSIFTDetector::SpectralSIFTDetector( const Params& params )
    : params_( params ), is_initialized_( false )
{
}

bool SpectralSIFTDetector::Init()
{
  if ( is_initialized_ )
  {
    return true;
  }

  if ( !ComputeSpectralBasis( params_.sigma_min_,
                              params_.sigma_max_,
                              params_.degree_,
                              params_.energy_threshold_,
                              FILTER_SLOG,
                              basis_slog_ ) )
  {
    OPENMVG_LOG_ERROR << "[SpectralSIFTDetector] Failed to compute sLoG spectral basis.";
    return false;
  }

  is_initialized_ = true;
  return true;
}

void SpectralSIFTDetector::DetectOctave( const image::Image<float>&     octave_image,
                                         int                            octave_idx,
                                         std::vector<SpectralKeypoint>& keypoints )
{
  if ( !Init() )
  {
    return;
  }

  const int W          = octave_image.Width();
  const int H          = octave_image.Height();
  const int K          = basis_slog_.GetK();
  const int degree     = basis_slog_.GetDegree();
  const int num_coeffs = degree + 1;
  const int border     = basis_slog_.GetKernelSize() / 2 + 2;

  if ( W < 2 * border + 1 || H < 2 * border + 1 )
  {
    return;
  }

  // Ensure convolver cache is configured for current octave dimensions
  if ( convolver_.GetCachedWidth() != W || convolver_.GetCachedHeight() != H )
  {
    convolver_.Init( W, H, basis_slog_.GetSpatialBases() );
  }

  // Convolve octave image with the K basis filters
  std::vector<image::Image<float>> C_images;
  convolver_.ConvolveAll( octave_image, C_images );

  // Precalculate the (degree + 1) coefficient images:
  // A_p(y, x) = sum_{k=0}^{K-1} c_{k, p} * C_k(y, x)
  const Eigen::MatrixXf&           poly_coeffs = basis_slog_.GetPolyCoeffs();
  std::vector<image::Image<float>> A_images( num_coeffs );
  for ( int p = 0; p < num_coeffs; ++p )
  {
    A_images[ p ].resize( W, H );
    A_images[ p ].fill( 0.0f );
  }

#ifdef OPENMVG_USE_OPENMP
#pragma omp parallel for
#endif
  for ( int y = 0; y < H; ++y )
  {
    for ( int x = 0; x < W; ++x )
    {
      for ( int p = 0; p < num_coeffs; ++p )
      {
        float val = 0.0f;
        for ( int k = 0; k < K; ++k )
        {
          val += poly_coeffs( k, p ) * C_images[ k ]( y, x );
        }
        A_images[ p ]( y, x ) = val;
      }
    }
  }

  // Coarse-to-fine candidate selection:
  // Evaluate 5 discrete scale layers across [-1, 1]
  const int                        num_discrete_slices               = 5;
  const float                      discrete_t[ num_discrete_slices ] = { -1.0f, -0.5f, 0.0f, 0.5f, 1.0f };
  std::vector<image::Image<float>> S_slices( num_discrete_slices );
  for ( int s = 0; s < num_discrete_slices; ++s )
  {
    S_slices[ s ].resize( W, H );
    const float t = discrete_t[ s ];

    std::vector<float> tp( num_coeffs );
    float              cur_t = 1.0f;
    for ( int p = 0; p < num_coeffs; ++p )
    {
      tp[ p ] = cur_t;
      cur_t *= t;
    }

#ifdef OPENMVG_USE_OPENMP
#pragma omp parallel for
#endif
    for ( int y = 0; y < H; ++y )
    {
      for ( int x = 0; x < W; ++x )
      {
        float val = 0.0f;
        for ( int p = 0; p < num_coeffs; ++p )
        {
          val += A_images[ p ]( y, x ) * tp[ p ];
        }
        S_slices[ s ]( y, x ) = val;
      }
    }
  }

  // Pre-filter candidates via 3D discrete 3x3x3 local extrema on the 3 central slices (1, 2, 3)
  const float coarse_threshold = 0.8f * params_.peak_threshold_;
  struct Candidate
  {
    int x;
    int y;
    int s;
  };
  std::vector<Candidate> candidates;

  for ( int s = 1; s <= 3; ++s )
  {
    const image::Image<float>& prev_slice = S_slices[ s - 1 ];
    const image::Image<float>& curr_slice = S_slices[ s ];
    const image::Image<float>& next_slice = S_slices[ s + 1 ];

    for ( int y = border; y < H - border; ++y )
    {
      for ( int x = border; x < W - border; ++x )
      {
        const float val = curr_slice( y, x );
        if ( std::abs( val ) < coarse_threshold )
        {
          continue;
        }

        bool is_max = true;
        bool is_min = true;

        if ( val > 0.0f )
        {
          is_min = false;
          // Check 26 neighbors for maximum
          for ( int dy = -1; dy <= 1 && is_max; ++dy )
          {
            for ( int dx = -1; dx <= 1 && is_max; ++dx )
            {
              if ( prev_slice( y + dy, x + dx ) >= val )
              {
                is_max = false;
                break;
              }
              if ( next_slice( y + dy, x + dx ) >= val )
              {
                is_max = false;
                break;
              }
              if ( ( dx != 0 || dy != 0 ) && curr_slice( y + dy, x + dx ) >= val )
              {
                is_max = false;
                break;
              }
            }
          }
        }
        else
        {
          is_max = false;
          // Check 26 neighbors for minimum
          for ( int dy = -1; dy <= 1 && is_min; ++dy )
          {
            for ( int dx = -1; dx <= 1 && is_min; ++dx )
            {
              if ( prev_slice( y + dy, x + dx ) <= val )
              {
                is_min = false;
                break;
              }
              if ( next_slice( y + dy, x + dx ) <= val )
              {
                is_min = false;
                break;
              }
              if ( ( dx != 0 || dy != 0 ) && curr_slice( y + dy, x + dx ) <= val )
              {
                is_min = false;
                break;
              }
            }
          }
        }

        if ( is_max || is_min )
        {
          candidates.push_back( { x, y, s } );
        }
      }
    }
  }

  // For each candidate, solve the continuous scale polynomial derivative P'(t) = 0
  const float edge_ratio_threshold =
      ( params_.edge_threshold_ + 1.0f ) * ( params_.edge_threshold_ + 1.0f ) / params_.edge_threshold_;

  for ( const auto& cand : candidates )
  {
    const int x = cand.x;
    const int y = cand.y;

    std::vector<float> roots;

    if ( degree == 3 )
    {
      // P(t)  = A0 + A1*t + A2*t^2 + A3*t^3
      // P'(t) = A1 + 2*A2*t + 3*A3*t^2 = a*t^2 + b*t + c = 0
      const float c = A_images[ 1 ]( y, x );
      const float b = 2.0f * A_images[ 2 ]( y, x );
      const float a = 3.0f * A_images[ 3 ]( y, x );

      if ( std::abs( a ) < 1e-8f )
      {
        if ( std::abs( b ) > 1e-8f )
        {
          const float root = -c / b;
          if ( root >= -1.0f && root <= 1.0f )
          {
            roots.push_back( root );
          }
        }
      }
      else
      {
        const float delta = b * b - 4.0f * a * c;
        if ( delta >= 0.0f )
        {
          const float sqrt_delta = std::sqrt( delta );
          const float r1         = ( -b + sqrt_delta ) / ( 2.0f * a );
          const float r2         = ( -b - sqrt_delta ) / ( 2.0f * a );

          if ( r1 >= -1.0f && r1 <= 1.0f )
            roots.push_back( r1 );
          if ( r2 >= -1.0f && r2 <= 1.0f && std::abs( r2 - r1 ) > 1e-4f )
            roots.push_back( r2 );
        }
      }
    }
    else if ( degree == 4 )
    {
      // P'(t) = A1 + 2*A2*t + 3*A3*t^2 + 4*A4*t^3
      Eigen::Vector4f coeffs_deriv;
      coeffs_deriv[ 0 ] = A_images[ 1 ]( y, x );
      coeffs_deriv[ 1 ] = 2.0f * A_images[ 2 ]( y, x );
      coeffs_deriv[ 2 ] = 3.0f * A_images[ 3 ]( y, x );
      coeffs_deriv[ 3 ] = 4.0f * A_images[ 4 ]( y, x );

      Eigen::PolynomialSolver<float, 3> solver;
      solver.compute( coeffs_deriv );

      const auto& poly_roots = solver.roots();
      for ( int i = 0; i < poly_roots.size(); ++i )
      {
        if ( std::abs( poly_roots[ i ].imag() ) < 1e-5f )
        {
          const float r = poly_roots[ i ].real();
          if ( r >= -1.0f && r <= 1.0f )
          {
            roots.push_back( r );
          }
        }
      }
    }

    // Process each valid root
    for ( const float t_exact : roots )
    {
      // Verify extremum: evaluate P(t) and P''(t)
      float P_val    = 0.0f;
      float P_deriv2 = 0.0f;

      float tp = 1.0f;
      for ( int p = 0; p < num_coeffs; ++p )
      {
        P_val += A_images[ p ]( y, x ) * tp;
        if ( p >= 2 )
        {
          P_deriv2 += static_cast<float>( p * ( p - 1 ) ) * A_images[ p ]( y, x ) * std::pow( t_exact, p - 2 );
        }
        tp *= t_exact;
      }

      // Check contrast threshold
      if ( std::abs( P_val ) < params_.peak_threshold_ )
      {
        continue;
      }

      // Check extremum curvature: maximum if P > 0 and P'' < 0, minimum if P < 0 and P'' > 0
      if ( ( P_val > 0.0f && P_deriv2 >= 0.0f ) || ( P_val < 0.0f && P_deriv2 <= 0.0f ) )
      {
        continue;
      }

      // Continuous scale
      const float sigma_exact = basis_slog_.TToScale( t_exact );

      // Subpixel 2D Taylor refinement on 3x3 local patch at continuous scale t_exact
      float V[ 3 ][ 3 ];
      for ( int dy = -1; dy <= 1; ++dy )
      {
        for ( int dx = -1; dx <= 1; ++dx )
        {
          float v     = 0.0f;
          float cur_t = 1.0f;
          for ( int p = 0; p < num_coeffs; ++p )
          {
            v += A_images[ p ]( y + dy, x + dx ) * cur_t;
            cur_t *= t_exact;
          }
          V[ dy + 1 ][ dx + 1 ] = v;
        }
      }

      // Spatial 2D derivatives by central differences
      const float Dx  = ( V[ 1 ][ 2 ] - V[ 1 ][ 0 ] ) * 0.5f;
      const float Dy  = ( V[ 2 ][ 1 ] - V[ 0 ][ 1 ] ) * 0.5f;
      const float Dxx = V[ 1 ][ 2 ] - 2.0f * V[ 1 ][ 1 ] + V[ 1 ][ 0 ];
      const float Dyy = V[ 2 ][ 1 ] - 2.0f * V[ 1 ][ 1 ] + V[ 0 ][ 1 ];
      const float Dxy = ( V[ 2 ][ 2 ] - V[ 2 ][ 0 ] - V[ 0 ][ 2 ] + V[ 0 ][ 0 ] ) * 0.25f;

      // 2D Hessian analysis
      const float trace = Dxx + Dyy;
      const float det   = Dxx * Dyy - Dxy * Dxy;

      // Discard unstable / saddle points
      if ( det <= 0.0f )
      {
        continue;
      }

      // Edge rejection (Lowe curvature ratio)
      const float edge_score = ( trace * trace ) / det;
      if ( edge_score > edge_ratio_threshold )
      {
        continue;
      }

      // Solve 2x2 system H * delta = -J
      const float delta_x = ( -Dx * Dyy + Dy * Dxy ) / det;
      const float delta_y = ( -Dy * Dxx + Dx * Dxy ) / det;

      if ( std::abs( delta_x ) > 0.5f || std::abs( delta_y ) > 0.5f )
      {
        continue;
      }

      // Refined response value
      const float refined_val = V[ 1 ][ 1 ] + 0.5f * ( Dx * delta_x + Dy * delta_y );
      if ( std::abs( refined_val ) < params_.peak_threshold_ )
      {
        continue;
      }

      // Valid keypoint
      SpectralKeypoint kp;
      kp.x          = static_cast<float>( x ) + delta_x;
      kp.y          = static_cast<float>( y ) + delta_y;
      kp.sigma      = sigma_exact;
      kp.val        = refined_val;
      kp.octave     = octave_idx;
      kp.i          = x;
      kp.j          = y;
      kp.edge_score = edge_score;

      keypoints.emplace_back( kp );
    }
  }
}

} // namespace spectral_sift
} // namespace features
} // namespace openMVG
