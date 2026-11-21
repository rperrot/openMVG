// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_IMAGE_DESCRIBER_HPP
#define OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_IMAGE_DESCRIBER_HPP

#include <cmath>
#include <memory>
#include <vector>

#include "openMVG/features/feature.hpp"
#include "openMVG/features/image_describer.hpp"
#include "openMVG/features/regions_factory.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_basis.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_descriptor.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_detector.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_thread_pool.hpp"
#include "openMVG/image/image_container.hpp"
#include "openMVG/image/image_resampling.hpp"
#include "openMVG/system/logger.hpp"

namespace openMVG
{
namespace features
{
namespace spectral_sift
{

/**
 * @brief Complete Image Describer implementing Gou Koutaki's Spectral SIFT
 *        Produces standard SIFT_Regions (SIOPointFeature + 128-D unsigned char descriptors)
 *        compatible with OpenMVG matching engines and SfM pipelines.
 */
class Spectral_SIFT_Image_describer : public Image_describer
{
public:
  using Regions_type = SIFT_Regions;

  struct Params
  {
    Params( int                              first_octave     = 0,
            int                              num_octaves      = 6,
            float                            edge_threshold   = 10.0f,
            float                            peak_threshold   = 0.04f,
            int                              degree           = 3,
            float                            sigma_min        = 1.6f,
            float                            sigma_max        = 3.2f,
            float                            energy_threshold = 0.999f,
            ESpectralDescriptorNormalization normalization    = DSP_ROOT_SIFT,
            float                            tikhonov_lambda  = 1e-3f,
            float                            margin_factor    = 1.41421356f,
            float                            dsp_min_scale    = 1.0f / 6.0f,
            float                            dsp_max_scale    = 2.0f,
            int                              dsp_num_scales   = 10 )
        : first_octave_( first_octave ),
          num_octaves_( num_octaves ),
          edge_threshold_( edge_threshold ),
          peak_threshold_( peak_threshold ),
          degree_( degree ),
          sigma_min_( sigma_min ),
          sigma_max_( sigma_max ),
          energy_threshold_( energy_threshold ),
          normalization_( normalization ),
          tikhonov_lambda_( tikhonov_lambda ),
          margin_factor_( margin_factor ),
          dsp_min_scale_( dsp_min_scale ),
          dsp_max_scale_( dsp_max_scale ),
          dsp_num_scales_( dsp_num_scales )
    {
    }

    template <class Archive>
    inline void serialize( Archive& ar );

    // Parameters
    int                              first_octave_;     // 0 for original scale, -1 for doubled image (ULTRA preset)
    int                              num_octaves_;      // Maximum number of octaves
    float                            edge_threshold_;   // Lowe curvature ratio threshold (default: 10.0)
    float                            peak_threshold_;   // Contrast threshold (default: 0.04)
    int                              degree_;           // Polynomial degree: 3 (analytical) or 4 (companion matrix)
    float                            sigma_min_;        // Scale-space range min per octave (default: 1.6)
    float                            sigma_max_;        // Scale-space range max per octave (default: 3.2)
    float                            energy_threshold_; // Target energy conservation ratio (default: 0.999)
    ESpectralDescriptorNormalization normalization_;    // Descriptor normalization / DSP mode (default: DSP_ROOT_SIFT)
    float                            tikhonov_lambda_;  // Tikhonov damping factor for 2D Hessian subpixel solve (default: 1e-3)
    float                            margin_factor_;    // Scale-space expansion margin factor (default: sqrt(2) ~ 1.4142)
    float                            dsp_min_scale_;    // Minimum DSP scale multiplier (default: 1/6)
    float                            dsp_max_scale_;    // Maximum DSP scale multiplier (default: 2.0)
    int                              dsp_num_scales_;   // Number of logarithmic DSP scale samples (default: 10)
  };

  explicit Spectral_SIFT_Image_describer( const Params& params = Params() )
      : Image_describer(),
        params_( params ),
        thread_pool_( std::make_shared<ThreadPool>() )
  {
  }

  ~Spectral_SIFT_Image_describer() override
  {
    if ( thread_pool_ )
    {
      thread_pool_->Shutdown();
    }
  }

  bool Set_configuration_preset( EDESCRIBER_PRESET preset ) override
  {
    switch ( preset )
    {
      case NORMAL_PRESET:
        params_.peak_threshold_ = 0.04f;
        params_.first_octave_   = 0;
        break;
      case HIGH_PRESET:
        params_.peak_threshold_ = 0.01f;
        params_.first_octave_   = 0;
        break;
      case ULTRA_PRESET:
        params_.peak_threshold_ = 0.01f;
        params_.first_octave_   = -1;
        break;
      default:
        OPENMVG_LOG_ERROR << "[Spectral SIFT] Invalid preset configuration";
        return false;
    }
    detector_.reset();
    descriptor_extractor_.reset();
    return true;
  }

  std::unique_ptr<Regions> Allocate() const override
  {
    return std::unique_ptr<Regions_type>( new Regions_type );
  }

  std::unique_ptr<Regions> Describe(
      const image::Image<unsigned char>& image,
      const image::Image<unsigned char>* mask = nullptr ) override
  {
    auto regions = std::unique_ptr<Regions_type>( new Regions_type );

    if ( image.size() == 0 )
    {
      return regions;
    }

    // Convert input image to float [0, 1]
    const image::Image<float> If( image.GetMat().cast<float>() / 255.0f );

    // Initialize or reuse detector and descriptor extractor
    if ( !detector_ )
    {
      SpectralSIFTDetector::Params det_params(
          params_.peak_threshold_,
          params_.edge_threshold_,
          params_.sigma_min_,
          params_.sigma_max_,
          params_.degree_,
          params_.energy_threshold_,
          params_.tikhonov_lambda_,
          params_.margin_factor_ );
      detector_.reset( new SpectralSIFTDetector( det_params ) );
      detector_->SetThreadPool( thread_pool_.get() );
      detector_->Init();
    }

    if ( !descriptor_extractor_ )
    {
      SpectralSIFTDescriptorExtractor::Params desc_params(
          params_.normalization_,
          1.5f, // orientation_scale
          0.8f, // percentage_keep
          6,    // nb_smoothing_step
          0.2f, // clip_value
          params_.sigma_min_,
          params_.sigma_max_,
          params_.degree_,
          params_.energy_threshold_,
          params_.margin_factor_,
          params_.dsp_min_scale_,
          params_.dsp_max_scale_,
          params_.dsp_num_scales_ );
      descriptor_extractor_.reset( new SpectralSIFTDescriptorExtractor( desc_params ) );
      descriptor_extractor_->SetThreadPool( thread_pool_.get() );
      if ( !descriptor_extractor_->Init() )
      {
        return regions;
      }
    }

    // Octave pyramid processing with 2-octave sliding nominal cache
    OctaveNominalCache curr_cache;
    if ( params_.first_octave_ == -1 )
    {
      image::ImageUpsample( If, curr_cache.octave_image );
    }
    else
    {
      curr_cache.octave_image = If;
    }

    std::vector<SpectralOrientedKeypoint> all_keypoints;
    const int                             border       = 25; // Safe border for smallest octave size
    const bool                            use_dsp      = IsDSPEnabled( params_.normalization_ );
    const int                             L_nom        = descriptor_extractor_->GetBasis().GetKernelSize();
    const float                           max_dsp_mult = descriptor_extractor_->GetDSPScales().empty()
                                                             ? params_.dsp_max_scale_
                                                             : descriptor_extractor_->GetDSPScales().back();

    for ( int o = params_.first_octave_; o < params_.first_octave_ + params_.num_octaves_; ++o )
    {
      if ( curr_cache.octave_image.Width() < 2 * border + 1 || curr_cache.octave_image.Height() < 2 * border + 1 )
      {
        break;
      }

      // Compute forward 2D FFT of current octave image once (or reuse from sliding cache)
      std::vector<SpectralKeypoint> oct_keypoints;
      detector_->DetectOctave( curr_cache.octave_image, o, oct_keypoints, &curr_cache.octave_fft );

      const bool is_last_octave = ( o == params_.first_octave_ + params_.num_octaves_ - 1 );
      const bool next_is_last   = ( o + 1 == params_.first_octave_ + params_.num_octaves_ - 1 );
      const int  next_w         = curr_cache.octave_image.Width() / 2;
      const int  next_h         = curr_cache.octave_image.Height() / 2;

      bool need_coarse_for_curr = false;
      if ( use_dsp && !oct_keypoints.empty() && next_w >= L_nom && next_h >= L_nom )
      {
        float max_kp_sigma = oct_keypoints[ 0 ].sigma;
        for ( size_t i = 1; i < oct_keypoints.size(); ++i )
        {
          max_kp_sigma = std::max( max_kp_sigma, oct_keypoints[ i ].sigma );
        }
        if ( max_kp_sigma * max_dsp_mult > descriptor_extractor_->GetNomNextSplit() )
        {
          need_coarse_for_curr = true;
        }
      }

      const bool need_smoothed = !is_last_octave || need_coarse_for_curr;
      const bool need_curr_nom = !oct_keypoints.empty() || need_smoothed;

      if ( need_curr_nom && ( !curr_cache.valid || ( need_smoothed && !curr_cache.has_smoothed ) ) )
      {
        descriptor_extractor_->PrepareOctaveNominal( curr_cache, need_smoothed, false );
      }

      OctaveNominalCache next_cache;
      if ( need_smoothed && curr_cache.has_smoothed )
      {
        image::ImageDecimate( curr_cache.smoothed_at_2sigma, next_cache.octave_image );
        if ( need_coarse_for_curr &&
             next_cache.octave_image.Width() >= L_nom &&
             next_cache.octave_image.Height() >= L_nom )
        {
          descriptor_extractor_->PrepareOctaveNominal( next_cache, !next_is_last || use_dsp, true );
        }
      }

      if ( !oct_keypoints.empty() )
      {
        std::vector<SpectralOrientedKeypoint> oct_described;
        descriptor_extractor_->DescribeOctaveWithCache(
            curr_cache,
            next_cache.valid ? &next_cache : nullptr,
            o,
            oct_keypoints,
            oct_described );

        // Map coordinates back to original image scale.
        // Because ImageDecimate samples exactly (2i, 2j), there is zero subpixel phase shift across octaves:
        // x_orig = x_octave * 2^o
        const float octave_scale = std::pow( 2.0f, static_cast<float>( o ) );
        for ( auto& kp : oct_described )
        {
          kp.x *= octave_scale;
          kp.y *= octave_scale;
          kp.sigma *= octave_scale;
          all_keypoints.emplace_back( std::move( kp ) );
        }
      }

      if ( !is_last_octave )
      {
        curr_cache = std::move( next_cache );
      }
    }

    // Filter by mask and populate Regions
    regions->Features().reserve( all_keypoints.size() );
    regions->Descriptors().reserve( all_keypoints.size() );

    for ( const auto& kp : all_keypoints )
    {
      if ( mask )
      {
        const int mx = static_cast<int>( std::round( kp.x ) );
        const int my = static_cast<int>( std::round( kp.y ) );
        if ( mx < 0 || mx >= mask->Width() || my < 0 || my >= mask->Height() )
        {
          continue;
        }
        if ( ( *mask )( my, mx ) == 0 )
        {
          continue;
        }
      }

      // Add feature: x, y, scale, orientation
      regions->Features().emplace_back( kp.x, kp.y, kp.sigma, kp.theta );

      // Convert 128-D float vector to Descriptor<unsigned char, 128>
      Descriptor<unsigned char, 128> desc;
      for ( int i = 0; i < 128; ++i )
      {
        desc[ i ] = static_cast<unsigned char>(
            std::max( 0.0f, std::min( 255.0f, std::round( kp.descr[ i ] ) ) ) );
      }
      regions->Descriptors().emplace_back( desc );
    }

    return regions;
  }

  const Params& GetParams() const
  {
    return params_;
  }
  Params& ChangeParams()
  {
    detector_.reset();
    descriptor_extractor_.reset();
    return params_;
  }

  template <class Archive>
  inline void serialize( Archive& ar );

private:
  Params                                           params_;
  std::shared_ptr<ThreadPool>                      thread_pool_;
  std::unique_ptr<SpectralSIFTDetector>            detector_;
  std::unique_ptr<SpectralSIFTDescriptorExtractor> descriptor_extractor_;
};

} // namespace spectral_sift
} // namespace features
} // namespace openMVG

#endif // OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_IMAGE_DESCRIBER_HPP
