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
      cereal::make_nvp( "energy_threshold", energy_threshold_ ),
      cereal::make_nvp( "root_sift", root_sift_ ) );
}

template <class Archive>
inline void openMVG::features::spectral_sift::Spectral_SIFT_Image_describer::serialize( Archive& ar )
{
  ar( cereal::make_nvp( "params", params_ ) );
}

CEREAL_REGISTER_TYPE_WITH_NAME( openMVG::features::spectral_sift::Spectral_SIFT_Image_describer, "Spectral_SIFT_Image_describer" );
CEREAL_REGISTER_POLYMORPHIC_RELATION( openMVG::features::Image_describer, openMVG::features::spectral_sift::Spectral_SIFT_Image_describer )

#endif // OPENMVG_FEATURES_SPECTRAL_SIFT_SPECTRAL_SIFT_IMAGE_DESCRIBER_IO_HPP
