// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/spectral_sift/spectral_sift_convolver.hpp"

#ifdef OPENMVG_USE_OPENMP
#include <omp.h>
#endif

namespace openMVG
{
namespace features
{
namespace spectral_sift
{

SpectralConvolver::SpectralConvolver()
    : cached_width_( 0 ),
      cached_height_( 0 )
{
}

void SpectralConvolver::ForwardFFT2D( const Eigen::MatrixXf& in,
                                      Eigen::MatrixXcf&      out )
{
  const int H = static_cast<int>( in.rows() );
  const int W = static_cast<int>( in.cols() );
  out.resize( H, W );

  MatrixXcf row_fft( H, W );

#ifdef OPENMVG_USE_OPENMP
#pragma omp parallel
  {
    Eigen::FFT<float> thread_fft;
#pragma omp for
    for ( int r = 0; r < H; ++r )
    {
      std::vector<std::complex<float>> row_c( W );
      std::vector<float>               row_r( W );
      for ( int c = 0; c < W; ++c )
      {
        row_r[ c ] = in( r, c );
      }
      thread_fft.fwd( row_c, row_r );
      for ( int c = 0; c < W; ++c )
      {
        row_fft( r, c ) = row_c[ c ];
      }
    }
  }
#else
  for ( int r = 0; r < H; ++r )
  {
    std::vector<std::complex<float>> row_c( W );
    std::vector<float>               row_r( W );
    for ( int c = 0; c < W; ++c )
    {
      row_r[ c ] = in( r, c );
    }
    fft_.fwd( row_c, row_r );
    for ( int c = 0; c < W; ++c )
    {
      row_fft( r, c ) = row_c[ c ];
    }
  }
#endif

#ifdef OPENMVG_USE_OPENMP
#pragma omp parallel
  {
    Eigen::FFT<float> thread_fft;
#pragma omp for
    for ( int c = 0; c < W; ++c )
    {
      std::vector<std::complex<float>> col_in( H );
      std::vector<std::complex<float>> col_out( H );
      for ( int r = 0; r < H; ++r )
      {
        col_in[ r ] = row_fft( r, c );
      }
      thread_fft.fwd( col_out, col_in );
      for ( int r = 0; r < H; ++r )
      {
        out( r, c ) = col_out[ r ];
      }
    }
  }
#else
  for ( int c = 0; c < W; ++c )
  {
    std::vector<std::complex<float>> col_in( H );
    std::vector<std::complex<float>> col_out( H );
    for ( int r = 0; r < H; ++r )
    {
      col_in[ r ] = row_fft( r, c );
    }
    fft_.fwd( col_out, col_in );
    for ( int r = 0; r < H; ++r )
    {
      out( r, c ) = col_out[ r ];
    }
  }
#endif
}

void SpectralConvolver::InverseFFT2D( const MatrixXcf& in, Eigen::MatrixXf& out )
{
  const int H = static_cast<int>( in.rows() );
  const int W = static_cast<int>( in.cols() );
  out.resize( H, W );

  MatrixXcf col_ifft( H, W );

#ifdef OPENMVG_USE_OPENMP
#pragma omp parallel
  {
    Eigen::FFT<float> thread_fft;
#pragma omp for
    for ( int c = 0; c < W; ++c )
    {
      std::vector<std::complex<float>> col_in( H );
      std::vector<std::complex<float>> col_out( H );
      for ( int r = 0; r < H; ++r )
      {
        col_in[ r ] = in( r, c );
      }
      thread_fft.inv( col_out, col_in );
      for ( int r = 0; r < H; ++r )
      {
        col_ifft( r, c ) = col_out[ r ];
      }
    }
  }
#else
  for ( int c = 0; c < W; ++c )
  {
    std::vector<std::complex<float>> col_in( H );
    std::vector<std::complex<float>> col_out( H );
    for ( int r = 0; r < H; ++r )
    {
      col_in[ r ] = in( r, c );
    }
    fft_.inv( col_out, col_in );
    for ( int r = 0; r < H; ++r )
    {
      col_ifft( r, c ) = col_out[ r ];
    }
  }
#endif

#ifdef OPENMVG_USE_OPENMP
#pragma omp parallel
  {
    Eigen::FFT<float> thread_fft;
#pragma omp for
    for ( int r = 0; r < H; ++r )
    {
      std::vector<std::complex<float>> row_in( W );
      std::vector<float>               row_out( W );
      for ( int c = 0; c < W; ++c )
      {
        row_in[ c ] = col_ifft( r, c );
      }
      thread_fft.inv( row_out, row_in );
      for ( int c = 0; c < W; ++c )
      {
        out( r, c ) = row_out[ c ];
      }
    }
  }
#else
  for ( int r = 0; r < H; ++r )
  {
    std::vector<std::complex<float>> row_in( W );
    std::vector<float>               row_out( W );
    for ( int c = 0; c < W; ++c )
    {
      row_in[ c ] = col_ifft( r, c );
    }
    fft_.inv( row_out, row_in );
    for ( int c = 0; c < W; ++c )
    {
      out( r, c ) = row_out[ c ];
    }
  }
#endif
}

void SpectralConvolver::Init( int width, int height, const std::vector<Eigen::MatrixXf>& spatial_kernels )
{
  cached_width_  = width;
  cached_height_ = height;
  cached_filter_ffts_.clear();
  cached_filter_ffts_.resize( spatial_kernels.size() );

  for ( size_t k = 0; k < spatial_kernels.size(); ++k )
  {
    const Eigen::MatrixXf& kernel = spatial_kernels[ k ];
    const int              L      = static_cast<int>( kernel.rows() );
    const int              radius = L / 2;

    Eigen::MatrixXf padded = Eigen::MatrixXf::Zero( height, width );
    for ( int y = 0; y < L; ++y )
    {
      const int dy     = y - radius;
      const int dest_y = ( dy + height ) % height;
      for ( int x = 0; x < L; ++x )
      {
        const int dx             = x - radius;
        const int dest_x         = ( dx + width ) % width;
        padded( dest_y, dest_x ) = kernel( y, x );
      }
    }

    ForwardFFT2D( padded, cached_filter_ffts_[ k ] );
  }
}

void SpectralConvolver::ConvolveAll( const image::Image<float>&        input_image,
                                     std::vector<image::Image<float>>& output_convolutions )
{
  const int    W           = input_image.Width();
  const int    H           = input_image.Height();
  const size_t num_kernels = cached_filter_ffts_.size();

  output_convolutions.resize( num_kernels );

  // Compute forward 2D FFT of the input image once
  MatrixXcf input_fft;
  ForwardFFT2D( input_image.GetMat(), input_fft );

  // For each kernel, multiply in frequency domain and apply inverse 2D FFT
#ifdef OPENMVG_USE_OPENMP
#pragma omp parallel for
#endif
  for ( int k = 0; k < static_cast<int>( num_kernels ); ++k )
  {
    MatrixXcf prod( H, W );
    for ( int r = 0; r < H; ++r )
    {
      for ( int c = 0; c < W; ++c )
      {
        prod( r, c ) = input_fft( r, c ) * cached_filter_ffts_[ k ]( r, c );
      }
    }

    Eigen::MatrixXf result( H, W );
    InverseFFT2D( prod, result );

    output_convolutions[ k ] = image::Image<float>( result );
  }
}

void SpectralConvolver::Convolve( const image::Image<float>& input_image,
                                  int                        kernel_idx,
                                  image::Image<float>&       output_image )
{
  const int W = input_image.Width();
  const int H = input_image.Height();

  MatrixXcf input_fft;
  ForwardFFT2D( input_image.GetMat(), input_fft );

  MatrixXcf prod( H, W );
  for ( int r = 0; r < H; ++r )
  {
    for ( int c = 0; c < W; ++c )
    {
      prod( r, c ) = input_fft( r, c ) * cached_filter_ffts_[ kernel_idx ]( r, c );
    }
  }

  Eigen::MatrixXf result( H, W );
  InverseFFT2D( prod, result );
  output_image = image::Image<float>( result );
}

} // namespace spectral_sift
} // namespace features
} // namespace openMVG
