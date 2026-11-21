// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_IMAGE_DESCRIBER_IO_HPP
#define OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_IMAGE_DESCRIBER_IO_HPP

#include "openMVG/features/spectral_sift/spectral_sift_image_describer.hpp"

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>

template <class Archive>
inline void openMVG::features::spectral_sift::Spectral_SIFT_Image_describer::Params::serialize( Archive& ar )
{
  ar(
      cereal::make_nvp( "first_octave", first_octave_ ),
      cereal::make_nvp( "num_octaves", num_octaves_ ),
      cereal::make_nvp( "edge_threshold", edge_threshold_ ),
      cereal::make_nvp( "peak_threshold", peak_threshold_ ),
      cereal::make_nvp( "degree", degree_ ),
      cereal::make_nvp( "sigma_min", sigma_min_ ),
      cereal::make_nvp( "sigma_max", sigma_max_ ),
      cereal::make_nvp( "energy_threshold", energy_threshold_ ) );

  if ( !Archive::is_loading::value )
  {
    int norm_int = static_cast<int>( normalization_ );
    ar(
        cereal::make_nvp( "normalization", norm_int ),
        cereal::make_nvp( "dsp_min_scale", dsp_min_scale_ ),
        cereal::make_nvp( "dsp_max_scale", dsp_max_scale_ ),
        cereal::make_nvp( "dsp_num_scales", dsp_num_scales_ ) );
  }
  else
  {
    int norm_int = static_cast<int>( DSP_ROOT_SIFT );
    try
    {
      ar( cereal::make_nvp( "normalization", norm_int ) );
      if ( norm_int >= static_cast<int>( LOWE_SIFT ) &&
           norm_int <= static_cast<int>( DSP_ROOT_SIFT ) )
      {
        normalization_ = static_cast<ESpectralDescriptorNormalization>( norm_int );
      }
      else
      {
        normalization_ = DSP_ROOT_SIFT;
      }
      ar(
          cereal::make_nvp( "dsp_min_scale", dsp_min_scale_ ),
          cereal::make_nvp( "dsp_max_scale", dsp_max_scale_ ),
          cereal::make_nvp( "dsp_num_scales", dsp_num_scales_ ) );
    }
    catch ( const cereal::Exception& )
    {
      // Backward compatibility for legacy JSON files storing only "root_sift"
      bool legacy_root_sift = true;
      ar( cereal::make_nvp( "root_sift", legacy_root_sift ) );
      normalization_  = legacy_root_sift ? ROOT_SIFT : LOWE_SIFT;
      dsp_min_scale_  = 1.0f / 6.0f;
      dsp_max_scale_  = 2.0f;
      dsp_num_scales_ = 10;
    }
  }

  ar(
      cereal::make_nvp( "tikhonov_lambda", tikhonov_lambda_ ),
      cereal::make_nvp( "margin_factor", margin_factor_ ) );
}

template <class Archive>
inline void openMVG::features::spectral_sift::Spectral_SIFT_Image_describer::serialize( Archive& ar )
{
  ar( cereal::make_nvp( "params", params_ ) );
}

CEREAL_REGISTER_TYPE_WITH_NAME( openMVG::features::spectral_sift::Spectral_SIFT_Image_describer, "Spectral_SIFT_Image_describer" );
CEREAL_REGISTER_POLYMORPHIC_RELATION( openMVG::features::Image_describer, openMVG::features::spectral_sift::Spectral_SIFT_Image_describer )

#endif // OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_IMAGE_DESCRIBER_IO_HPP
