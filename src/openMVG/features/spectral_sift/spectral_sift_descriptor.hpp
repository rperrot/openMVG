// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_DESCRIPTOR_HPP
#define OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_DESCRIPTOR_HPP

#include <memory>
#include <vector>

#include "openMVG/features/feature.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_basis.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_convolver.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_detector.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_thread_pool.hpp"
#include "openMVG/image/image_container.hpp"

namespace openMVG
{
namespace features
{
namespace spectral_sift
{

/**
 * @brief Fully described oriented keypoint with 128-D descriptor
 */
struct SpectralOrientedKeypoint
{
  float x;      // Subpixel x
  float y;      // Subpixel y
  float sigma;  // Continuous scale
  float theta;  // Dominant orientation in radians [0, 2*pi)
  int   octave; // Octave index
  float val;    // Response value

  Eigen::Matrix<float, 128, 1> descr; // 128-D descriptor vector
};

/**
 * @brief SIFT-compatible descriptor extractor using continuous Gaussian scale-space basis
 */
class SpectralSIFTDescriptorExtractor
{
public:
  struct Params
  {
    Params( bool  root_sift         = true,
            float orientation_scale = 1.5f,
            float percentage_keep   = 0.8f,
            int   nb_smoothing_step = 6,
            float clip_value        = 0.2f,
            float sigma_min         = 1.6f,
            float sigma_max         = 3.2f,
            int   degree            = 3,
            float energy_threshold  = 0.999f )
        : root_sift_( root_sift ),
          orientation_scale_( orientation_scale ),
          percentage_keep_( percentage_keep ),
          nb_smoothing_step_( nb_smoothing_step ),
          clip_value_( clip_value ),
          sigma_min_( sigma_min ),
          sigma_max_( sigma_max ),
          degree_( degree ),
          energy_threshold_( energy_threshold )
    {
    }

    bool  root_sift_;
    float orientation_scale_;
    float percentage_keep_;
    int   nb_smoothing_step_;
    float clip_value_;
    float sigma_min_;
    float sigma_max_;
    int   degree_;
    float energy_threshold_;
  };

  SpectralSIFTDescriptorExtractor();
  explicit SpectralSIFTDescriptorExtractor( const Params& params );

  void SetThreadPool( ThreadPool* pool )
  {
    pool_ = pool;
    convolver_.SetThreadPool( pool );
  }

  /**
   * @brief Initialize Gaussian spectral basis and convolver
   */
  bool Init();

  /**
   * @brief Compute orientations and 128-D descriptors for a list of detected keypoints in an octave
   * @param octave_image Floating point grayscale image of the octave
   * @param octave_idx Octave index
   * @param input_keypoints List of detected keypoints (from SpectralSIFTDetector)
   * @param[out] output_keypoints Oriented keypoints with computed 128-D descriptors
   * @param octave_fft Optional pointer to precomputed forward FFT spectrum
   */
  void DescribeOctave( const image::Image<float>&             octave_image,
                       int                                    octave_idx,
                       const std::vector<SpectralKeypoint>&   input_keypoints,
                       std::vector<SpectralOrientedKeypoint>& output_keypoints,
                       const MatrixXcf*                       octave_fft = nullptr );

  const SpectralBasis& GetBasis() const
  {
    return basis_gauss_;
  }
  const Params& GetParams() const
  {
    return params_;
  }
  void SetParams( const Params& params )
  {
    params_         = params;
    is_initialized_ = false;
  }
  SpectralConvolver& GetConvolver()
  {
    return convolver_;
  }

private:
  Params            params_;
  bool              is_initialized_;
  SpectralBasis     basis_gauss_;
  SpectralConvolver convolver_;
  ThreadPool*       pool_;

  /**
   * @brief Assign dominant orientation(s) to a keypoint
   */
  void ComputeOrientations( const std::vector<image::Image<float>>& Gx_images,
                            const std::vector<image::Image<float>>& Gy_images,
                            const SpectralKeypoint&                 kp,
                            std::vector<float>&                     orientations ) const;

  /**
   * @brief Compute the 128-D descriptor vector for an oriented keypoint
   */
  void ComputeDescriptor( const std::vector<image::Image<float>>& Gx_images,
                          const std::vector<image::Image<float>>& Gy_images,
                          const SpectralKeypoint&                 kp,
                          float                                   orientation,
                          Eigen::Matrix<float, 128, 1>&           descr ) const;
};

} // namespace spectral_sift
} // namespace features
} // namespace openMVG

#endif // OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_DESCRIPTOR_HPP
