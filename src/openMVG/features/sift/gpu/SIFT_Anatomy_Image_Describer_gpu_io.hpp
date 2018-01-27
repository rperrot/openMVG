// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2015 Pierre MOULON.
// Copyright (c) 2018 Romuald PERROT

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_SIFT_SIFT_ANATOMY_IMAGE_DESCRIBER_GPU_IO_HPP
#define OPENMVG_FEATURES_SIFT_SIFT_ANATOMY_IMAGE_DESCRIBER_GPU_IO_HPP

#include "openMVG/features/sift/SIFT_Anatomy_Image_Describer_gpu.hpp"

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>

template<class Archive>
inline void openMVG::features::gpu::SIFT_Anatomy_Image_describerGPU::Params::serialize( Archive & ar )
{
  ar(
    cereal::make_nvp( "first_octave", first_octave_ ),
    cereal::make_nvp( "num_octaves", num_octaves_ ),
    cereal::make_nvp( "num_scales", num_scales_ ),
    cereal::make_nvp( "edge_threshold", edge_threshold_ ),
    cereal::make_nvp( "peak_threshold", peak_threshold_ ),
    cereal::make_nvp( "root_sift", root_sift_ ) );
}


template<class Archive>
inline void openMVG::features::gpu::SIFT_Anatomy_Image_describerGPU::serialize( Archive & ar )
{
  ar( cereal::make_nvp( "params", params_ ) );
}


CEREAL_REGISTER_TYPE_WITH_NAME( openMVG::features::gpu::SIFT_Anatomy_Image_describerGPU, "SIFT_Anatomy_Image_describerGPU" );
CEREAL_REGISTER_POLYMORPHIC_RELATION( openMVG::features::Image_describer, openMVG::features::gpu::SIFT_Anatomy_Image_describerGPU )

#endif // OPENMVG_FEATURES_SIFT_SIFT_ANATOMY_IMAGE_DESCRIBER_GPU_IO_HPP