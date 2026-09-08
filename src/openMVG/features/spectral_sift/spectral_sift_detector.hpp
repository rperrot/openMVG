// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_DETECTOR_HPP
#define OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_DETECTOR_HPP

#include <memory>
#include <vector>

#include "openMVG/features/spectral_sift/spectral_sift_basis.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_convolver.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_thread_pool.hpp"
#include "openMVG/image/image_container.hpp"

namespace openMVG
{
namespace features
{
namespace spectral_sift
{

/**
 * @brief Detected keypoint in continuous scale-space
 */
struct SpectralKeypoint
{
  float x;          // Subpixel x coordinate in octave coordinate system
  float y;          // Subpixel y coordinate in octave coordinate system
  float sigma;      // Continuous scale coordinate in octave coordinate system
  float val;        // Operator response at extremum
  int   octave;     // Octave index
  int   i;          // Discrete x coordinate
  int   j;          // Discrete y coordinate
  float edge_score; // Ratio of principal curvatures (trace^2 / det)
};

/**
 * @brief Scale-space keypoint detector using polynomial representations (Gou Koutaki, CVPR 2014)
 */
class SpectralSIFTDetector
{
public:
  struct Params
  {
    Params( float peak_threshold   = 0.04f,
            float edge_threshold   = 10.0f,
            float sigma_min        = 1.6f,
            float sigma_max        = 3.2f,
            int   degree           = 3,
            float energy_threshold = 0.999f )
        : peak_threshold_( peak_threshold ),
          edge_threshold_( edge_threshold ),
          sigma_min_( sigma_min ),
          sigma_max_( sigma_max ),
          degree_( degree ),
          energy_threshold_( energy_threshold )
    {
    }

    float peak_threshold_;
    float edge_threshold_;
    float sigma_min_;
    float sigma_max_;
    int   degree_;
    float energy_threshold_;
  };

  SpectralSIFTDetector();
  explicit SpectralSIFTDetector( const Params& params );

  void SetThreadPool( ThreadPool* pool )
  {
    pool_ = pool;
    convolver_.SetThreadPool( pool );
  }

  /**
   * @brief Initialize detector basis filters and FFT convolver
   */
  bool Init();

  /**
   * @brief Detect keypoints in a single octave image
   * @param octave_image Floating point grayscale image of the current octave
   * @param octave_idx Octave index
   * @param[out] keypoints List of detected keypoints with subpixel coordinates and continuous scales
   * @param octave_fft Optional pointer to precomputed forward FFT or output container
   */
  void DetectOctave( const image::Image<float>&     octave_image,
                     int                            octave_idx,
                     std::vector<SpectralKeypoint>& keypoints,
                     MatrixXcf*                     octave_fft = nullptr );

  const SpectralBasis& GetBasis() const
  {
    return basis_slog_;
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
  SpectralBasis     basis_slog_;
  SpectralConvolver convolver_;
  ThreadPool*       pool_;
};

} // namespace spectral_sift
} // namespace features
} // namespace openMVG

#endif // OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_DETECTOR_HPP
