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
 * @brief Descriptor normalization / pooling mode for Spectral SIFT
 */
enum ESpectralDescriptorNormalization
{
  LOWE_SIFT     = 0, // Single-scale, L2 + clip(0.2) + L2 * 512 (Lowe 2004)
  ROOT_SIFT     = 1, // Single-scale, L2 + clip(0.2) + L1 + sqrt * 512 (Arandjelovic & Zisserman 2012)
  DSP_SIFT      = 2, // Multi-scale Domain-Size Pooling + L2 (Dong & Soatto 2015)
  DSP_ROOT_SIFT = 3  // Multi-scale Domain-Size Pooling + RootSIFT (Dong & Soatto 2015)
};

inline bool IsDSPEnabled( ESpectralDescriptorNormalization norm )
{
  return norm == DSP_SIFT || norm == DSP_ROOT_SIFT;
}

inline bool IsRootSIFTEnabled( ESpectralDescriptorNormalization norm )
{
  return norm == ROOT_SIFT || norm == DSP_ROOT_SIFT;
}

/**
 * @brief Cached nominal scale-space gradients and FFT for a single octave
 */
struct OctaveNominalCache
{
  image::Image<float>              octave_image;
  MatrixXcf                        octave_fft;
  std::vector<image::Image<float>> Gx_nom;
  std::vector<image::Image<float>> Gy_nom;
  image::Image<float>              smoothed_at_2sigma;
  bool                             has_smoothed = false;
  bool                             valid        = false;
};

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
    Params( ESpectralDescriptorNormalization normalization     = DSP_ROOT_SIFT,
            float                            orientation_scale = 1.5f,
            float                            percentage_keep   = 0.8f,
            int                              nb_smoothing_step = 6,
            float                            clip_value        = 0.2f,
            float                            sigma_min         = 1.6f,
            float                            sigma_max         = 3.2f,
            int                              degree            = 3,
            float                            energy_threshold  = 0.999f,
            float                            margin_factor     = 1.41421356f,
            float                            dsp_min_scale     = 1.0f / 6.0f,
            float                            dsp_max_scale     = 2.0f,
            int                              dsp_num_scales    = 10 )
        : normalization_( normalization ),
          orientation_scale_( orientation_scale ),
          percentage_keep_( percentage_keep ),
          nb_smoothing_step_( nb_smoothing_step ),
          clip_value_( clip_value ),
          sigma_min_( sigma_min ),
          sigma_max_( sigma_max ),
          degree_( degree ),
          energy_threshold_( energy_threshold ),
          margin_factor_( margin_factor ),
          dsp_min_scale_( dsp_min_scale ),
          dsp_max_scale_( dsp_max_scale ),
          dsp_num_scales_( dsp_num_scales )
    {
    }

    ESpectralDescriptorNormalization normalization_;
    float                            orientation_scale_;
    float                            percentage_keep_;
    int                              nb_smoothing_step_;
    float                            clip_value_;
    float                            sigma_min_;
    float                            sigma_max_;
    int                              degree_;
    float                            energy_threshold_;
    // Expansion factor applied to [sigma_min, sigma_max] when generating Gaussian spectral basis
    float                            margin_factor_;
    float                            dsp_min_scale_;
    float                            dsp_max_scale_;
    int                              dsp_num_scales_;
  };

  SpectralSIFTDescriptorExtractor();
  explicit SpectralSIFTDescriptorExtractor( const Params& params );

  void SetThreadPool( ThreadPool* pool )
  {
    pool_ = pool;
    convolver_.SetThreadPool( pool );
    convolver_low_.SetThreadPool( pool );
    convolver_coarse_.SetThreadPool( pool );
  }

  /**
   * @brief Initialize Gaussian spectral basis, auxiliary DSP bases, and canonical sampling grid
   */
  bool Init();

  /**
   * @brief Compute nominal Gaussian basis gradients and optional smoothed image at 2*sigma_min
   * @param[in,out] cache Cache populated with FFT, nominal gradients, and optional smoothed image
   * @param compute_smoothed_at_2sigma Whether to reconstruct Gaussian smoothed image at 2*sigma_min
   * @param use_coarse_convolver Whether to use dedicated coarse-resolution FFT cache
   */
  bool PrepareOctaveNominal( OctaveNominalCache& cache,
                             bool                compute_smoothed_at_2sigma,
                             bool                use_coarse_convolver = false );

  /**
   * @brief Compute orientations and 128-D descriptors using precomputed nominal octave caches
   */
  void DescribeOctaveWithCache( const OctaveNominalCache&              curr_cache,
                                const OctaveNominalCache*              next_cache,
                                int                                    octave_idx,
                                const std::vector<SpectralKeypoint>&   input_keypoints,
                                std::vector<SpectralOrientedKeypoint>& output_keypoints );

  /**
   * @brief Compute orientations and 128-D descriptors for a list of detected keypoints in an octave
   * @param octave_image Floating point grayscale image of the octave
   * @param octave_idx Octave index
   * @param input_keypoints List of detected keypoints (from SpectralSIFTDetector)
   * @param[out] output_keypoints Oriented keypoints with computed 128-D descriptors
   * @param octave_fft Optional pointer to precomputed forward FFT spectrum
   * @param[out] smoothed_image_at_2sigma Optional output pointer to receive Gaussian smoothed image
   *             reconstructed at scale 2 * sigma_min (for exact decimation into the next octave)
   */
  void DescribeOctave( const image::Image<float>&             octave_image,
                       int                                    octave_idx,
                       const std::vector<SpectralKeypoint>&   input_keypoints,
                       std::vector<SpectralOrientedKeypoint>& output_keypoints,
                       const MatrixXcf*                       octave_fft               = nullptr,
                       image::Image<float>*                   smoothed_image_at_2sigma = nullptr );

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
  const std::vector<float>& GetDSPScales() const
  {
    return dsp_scales_;
  }
  float GetNomNextSplit() const
  {
    return nom_next_split_;
  }

private:
  struct CanonicalSample
  {
    float x_rot;
    float y_rot;
    float w_spatial;
    int   u0;
    int   v0;
    float wu[ 2 ];
    float wv[ 2 ];
  };

  Params                       params_;
  bool                         is_initialized_;
  bool                         has_low_basis_;
  bool                         convolver_nom_ready_;
  bool                         convolver_low_ready_;
  bool                         convolver_coarse_ready_;
  SpectralBasis                basis_gauss_;
  SpectralBasis                basis_low_;
  SpectralConvolver            convolver_;
  SpectralConvolver            convolver_low_;
  SpectralConvolver            convolver_coarse_;
  ThreadPool*                  pool_;
  std::vector<float>           dsp_scales_;
  std::vector<CanonicalSample> canonical_grid_;
  float                        low_nom_split_;
  float                        nom_next_split_;

  /**
   * @brief Assign dominant orientation(s) to a keypoint
   */
  void ComputeOrientations( const std::vector<image::Image<float>>& Gx_images,
                            const std::vector<image::Image<float>>& Gy_images,
                            const SpectralKeypoint&                 kp,
                            std::vector<float>&                     orientations ) const;

  /**
   * @brief Apply final L2 clipping and Lowe or RootSIFT normalization
   */
  void NormalizeDescriptor( Eigen::Matrix<float, 128, 1>& descr ) const;

  /**
   * @brief Compute the single-scale 128-D descriptor vector for an oriented keypoint
   */
  void ComputeDescriptor( const std::vector<image::Image<float>>& Gx_images,
                          const std::vector<image::Image<float>>& Gy_images,
                          const SpectralKeypoint&                 kp,
                          float                                   orientation,
                          Eigen::Matrix<float, 128, 1>&           descr ) const;

  /**
   * @brief Compute the Domain-Size Pooled 128-D descriptor vector across logarithmic scales
   */
  void ComputeDescriptorDSP( const OctaveNominalCache&               curr_cache,
                             const image::Image<float>*              Gx_raw,
                             const image::Image<float>*              Gy_raw,
                             const std::vector<image::Image<float>>* Gx_low,
                             const std::vector<image::Image<float>>* Gy_low,
                             const OctaveNominalCache*               next_cache,
                             const OctaveNominalCache*               level2_cache,
                             const SpectralKeypoint&                 kp,
                             float                                   orientation,
                             Eigen::Matrix<float, 128, 1>&           descr ) const;
};

} // namespace spectral_sift
} // namespace features
} // namespace openMVG

#endif // OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_DESCRIPTOR_HPP
