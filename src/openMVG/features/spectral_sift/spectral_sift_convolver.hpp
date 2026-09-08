// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_CONVOLVER_HPP
#define OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_CONVOLVER_HPP

#include <complex>
#include <vector>

#include <Eigen/Core>
#include <unsupported/Eigen/FFT>

#include "openMVG/image/image_container.hpp"

namespace openMVG
{
namespace features
{
namespace spectral_sift
{

using MatrixXcf = Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic>;

/**
 * @brief Class managing 2D circular/linear convolution in the frequency domain using Eigen::FFT (KissFFT backend).
 *        Caches the 2D FFT transforms of the spatial basis filters to achieve fast convolution across octaves.
 */
class SpectralConvolver
{
public:
  SpectralConvolver();

  /**
   * @brief Initialize or update cached filter FFTs for a given image dimension (W x H).
   *        Zero-pads small kernels (L x L) to (W x H) with center placed at (0, 0) via wrap-around,
   *        and precalculates their 2D forward FFTs.
   * @param width Image width (columns)
   * @param height Image height (rows)
   * @param spatial_kernels Vector of K 2D spatial kernels (each of odd size L x L)
   */
  void Init( int                                 width,
             int                                 height,
             const std::vector<Eigen::MatrixXf>& spatial_kernels );

  /**
   * @brief Convolve input image with all cached basis filters simultaneously.
   *        Computes the 2D FFT of the input image once, then for each basis multiplies in frequency
   *        and performs the 2D inverse FFT.
   * @param input_image Input image of size (width x height)
   * @param[out] output_convolutions Vector of K convolved images
   */
  void ConvolveAll( const image::Image<float>&        input_image,
                    std::vector<image::Image<float>>& output_convolutions );

  /**
   * @brief Convolve input image with a single cached basis filter.
   * @param input_image Input image of size (width x height)
   * @param kernel_idx Index of the kernel in [0, K-1]
   * @param[out] output_image Resulting convolved image
   */
  void Convolve( const image::Image<float>& input_image,
                 int                        kernel_idx,
                 image::Image<float>&       output_image );

  /**
   * @brief 2D Forward FFT of a real matrix to a complex spectrum
   */
  void ForwardFFT2D( const Eigen::MatrixXf& in, MatrixXcf& out );

  /**
   * @brief 2D Inverse FFT of a complex spectrum to a real matrix
   */
  void InverseFFT2D( const MatrixXcf& in, Eigen::MatrixXf& out );

  int GetCachedWidth() const
  {
    return cached_width_;
  }
  int GetCachedHeight() const
  {
    return cached_height_;
  }
  int GetNumKernels() const
  {
    return static_cast<int>( cached_filter_ffts_.size() );
  }

private:
  int                    cached_width_;
  int                    cached_height_;
  std::vector<MatrixXcf> cached_filter_ffts_;
  Eigen::FFT<float>      fft_;
};

} // namespace spectral_sift
} // namespace features
} // namespace openMVG

#endif // OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_CONVOLVER_HPP
