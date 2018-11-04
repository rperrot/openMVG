#ifndef OPENMVG_FEATURES_SFOP_SFOP_IMAGE_DESCRIBER_IO_HPP
#define OPENMVG_FEATURES_SFOP_SFOP_IMAGE_DESCRIBER_IO_HPP

#include "openMVG/features/sfop/SFOPImageDescriber.hpp"
#include "openMVG/features/sfop/SFOPParams.hpp"

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>

template <class Archive>
inline void openMVG::features::sfop::SFOPParams::serialize( Archive& ar )
{
  ar( cereal::make_nvp( "num_octaves", m_nb_octave ),
      cereal::make_nvp( "num_scales", m_nb_slice ),
      cereal::make_nvp( "noise_sd", m_s ),
      cereal::make_nvp( "lambda_weight", m_lambda_w ),
      cereal::make_nvp( "precision_threshold", m_precision_th ) );
}

template <class Archive>
inline void openMVG::features::sfop::SFOPImageDescriber::serialize( Archive& ar )
{
  ar( cereal::make_nvp( "params", m_params ) );
}

CEREAL_REGISTER_TYPE_WITH_NAME( openMVG::features::sfop::SFOPImageDescriber, "SFOP_Image_describer" );
CEREAL_REGISTER_POLYMORPHIC_RELATION( openMVG::features::Image_describer, openMVG::features::sfop::SFOPImageDescriber );

#endif