// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/spectral_sift/spectral_sift_convolver.hpp"

#include <algorithm>

namespace openMVG
{
namespace features
{
namespace spectral_sift
{

SpectralConvolver::SpectralConvolver()
    : cached_width_( 0 ),
      cached_height_( 0 ),
      pool_( nullptr )
{
}

void SpectralConvolver::EnsureScratch( int H, int W )
{
  ThreadPool* pool        = pool_ ? pool_ : &ThreadPool::GlobalInstance();
  const int   num_threads = std::max( 1, static_cast<int>( pool->NumThreads() ) );
  const int   max_dim     = std::max( H, W );

  if ( static_cast<int>( thread_scratch_.size() ) < num_threads )
  {
    thread_scratch_.resize( num_threads );
  }

  for ( auto& s : thread_scratch_ )
  {
    if ( static_cast<int>( s.r_buf.size() ) < max_dim )
    {
      s.r_buf.resize( max_dim );
      s.c_buf.resize( max_dim );
    }
  }
}

void SpectralConvolver::ForwardFFT2D( const Eigen::MatrixXf& in,
                                      Eigen::MatrixXcf&      out )
{
  const int H = static_cast<int>( in.rows() );
  const int W = static_cast<int>( in.cols() );
  out.resize( H, W );

  MatrixXcf row_fft( H, W );

  EnsureScratch( H, W );
  ThreadPool* pool = pool_ ? pool_ : &ThreadPool::GlobalInstance();

  // 1) 1D FFT along rows
  ParallelForWithId(
      0, H,
      [&]( int r, int tid ) {
        auto& scratch = thread_scratch_[ tid ];
        for ( int c = 0; c < W; ++c )
        {
          scratch.r_buf[ c ] = in( r, c );
        }
        scratch.fft.fwd( scratch.c_buf.data(), scratch.r_buf.data(), W );
        for ( int c = 0; c < W; ++c )
        {
          row_fft( r, c ) = scratch.c_buf[ c ];
        }
      },
      pool );

  // 2) 1D FFT along columns
  ParallelForWithId(
      0, W,
      [&]( int c, int tid ) {
        auto& scratch = thread_scratch_[ tid ];
        scratch.fft.fwd( &out( 0, c ), &row_fft( 0, c ), H );
      },
      pool );
}

void SpectralConvolver::ForwardFFT2D( const RowMatrixXf& in,
                                      Eigen::MatrixXcf&  out )
{
  const int H = static_cast<int>( in.rows() );
  const int W = static_cast<int>( in.cols() );
  out.resize( H, W );

  MatrixXcf row_fft( H, W );

  EnsureScratch( H, W );
  ThreadPool* pool = pool_ ? pool_ : &ThreadPool::GlobalInstance();

  // 1) 1D FFT along rows (in is RowMajor, so &in(r, 0) is contiguous!)
  ParallelForWithId(
      0, H,
      [&]( int r, int tid ) {
        auto& scratch = thread_scratch_[ tid ];
        scratch.fft.fwd( scratch.c_buf.data(), &in( r, 0 ), W );
        for ( int c = 0; c < W; ++c )
        {
          row_fft( r, c ) = scratch.c_buf[ c ];
        }
      },
      pool );

  // 2) 1D FFT along columns
  ParallelForWithId(
      0, W,
      [&]( int c, int tid ) {
        auto& scratch = thread_scratch_[ tid ];
        scratch.fft.fwd( &out( 0, c ), &row_fft( 0, c ), H );
      },
      pool );
}

void SpectralConvolver::InverseFFT2D( const MatrixXcf& in, Eigen::MatrixXf& out )
{
  const int H = static_cast<int>( in.rows() );
  const int W = static_cast<int>( in.cols() );
  out.resize( H, W );

  MatrixXcf col_ifft( H, W );

  EnsureScratch( H, W );
  ThreadPool* pool = pool_ ? pool_ : &ThreadPool::GlobalInstance();

  // 1) 1D Inverse FFT along columns
  ParallelForWithId(
      0, W,
      [&]( int c, int tid ) {
        auto& scratch = thread_scratch_[ tid ];
        scratch.fft.inv( &col_ifft( 0, c ), &in( 0, c ), H );
      },
      pool );

  // 2) 1D Inverse FFT along rows
  ParallelForWithId(
      0, H,
      [&]( int r, int tid ) {
        auto& scratch = thread_scratch_[ tid ];
        for ( int c = 0; c < W; ++c )
        {
          scratch.c_buf[ c ] = col_ifft( r, c );
        }
        scratch.fft.inv( scratch.r_buf.data(), scratch.c_buf.data(), W );
        for ( int c = 0; c < W; ++c )
        {
          out( r, c ) = scratch.r_buf[ c ];
        }
      },
      pool );
}

void SpectralConvolver::InverseFFT2D( const MatrixXcf& in, RowMatrixXf& out )
{
  const int H = static_cast<int>( in.rows() );
  const int W = static_cast<int>( in.cols() );
  out.resize( H, W );

  MatrixXcf col_ifft( H, W );

  EnsureScratch( H, W );
  ThreadPool* pool = pool_ ? pool_ : &ThreadPool::GlobalInstance();

  // 1) 1D Inverse FFT along columns
  ParallelForWithId(
      0, W,
      [&]( int c, int tid ) {
        auto& scratch = thread_scratch_[ tid ];
        scratch.fft.inv( &col_ifft( 0, c ), &in( 0, c ), H );
      },
      pool );

  // 2) 1D Inverse FFT along rows (out is RowMajor, so &out(r, 0) is contiguous!)
  ParallelForWithId(
      0, H,
      [&]( int r, int tid ) {
        auto& scratch = thread_scratch_[ tid ];
        for ( int c = 0; c < W; ++c )
        {
          scratch.c_buf[ c ] = col_ifft( r, c );
        }
        scratch.fft.inv( &out( r, 0 ), scratch.c_buf.data(), W );
      },
      pool );
}

void SpectralConvolver::Init( int                                 width,
                              int                                 height,
                              const std::vector<Eigen::MatrixXf>& spatial_kernels )
{
  cached_width_  = width;
  cached_height_ = height;
  cached_filter_ffts_.clear();
  cached_filter_ffts_.resize( spatial_kernels.size() );

  EnsureScratch( height, width );

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

void SpectralConvolver::ComputeImageFFT( const image::Image<float>& input_image,
                                         MatrixXcf&                 image_fft )
{
  ForwardFFT2D( static_cast<const RowMatrixXf&>( input_image ), image_fft );
}

void SpectralConvolver::ConvolveAllFromFFT( const MatrixXcf&                 input_fft,
                                            std::vector<image::Image<float>>& output_convolutions )
{
  const int    H           = static_cast<int>( input_fft.rows() );
  const int    W           = static_cast<int>( input_fft.cols() );
  const size_t num_kernels = cached_filter_ffts_.size();

  output_convolutions.resize( num_kernels );

  for ( size_t k = 0; k < num_kernels; ++k )
  {
    MatrixXcf prod = input_fft.cwiseProduct( cached_filter_ffts_[ k ] );

    output_convolutions[ k ].resize( W, H );
    InverseFFT2D( prod, static_cast<RowMatrixXf&>( output_convolutions[ k ] ) );
  }
}

void SpectralConvolver::ConvolveAll( const image::Image<float>&        input_image,
                                     std::vector<image::Image<float>>& output_convolutions )
{
  MatrixXcf input_fft;
  ComputeImageFFT( input_image, input_fft );
  ConvolveAllFromFFT( input_fft, output_convolutions );
}

void SpectralConvolver::Convolve( const image::Image<float>& input_image,
                                  int                        kernel_idx,
                                  image::Image<float>&       output_image )
{
  MatrixXcf input_fft;
  ComputeImageFFT( input_image, input_fft );

  MatrixXcf prod = input_fft.cwiseProduct( cached_filter_ffts_[ kernel_idx ] );

  output_image.resize( input_image.Width(), input_image.Height() );
  InverseFFT2D( prod, static_cast<RowMatrixXf&>( output_image ) );
}

} // namespace spectral_sift
} // namespace features
} // namespace openMVG
