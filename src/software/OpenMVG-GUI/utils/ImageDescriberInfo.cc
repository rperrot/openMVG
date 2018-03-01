// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ImageDescriberInfo.hh"

#include "openMVG/features/akaze/image_describer_akaze.hpp"
#include "openMVG/features/akaze/image_describer_akaze_io.hpp"

#include <cereal/archives/json.hpp>

namespace openMVG_gui
{
/**
* @brief Query an image describer if it's a binary one
* @retval true if it's a binary describer
* @retval false if it's a scalar describer
*/
bool isBinaryDescriber( const std::shared_ptr< openMVG::features::Image_describer > desc )
{
  std::shared_ptr<openMVG::features::AKAZE_Image_describer> akaze = std::dynamic_pointer_cast<openMVG::features::AKAZE_Image_describer>( desc ) ;
  if( akaze.use_count() > 0 )
  {
    return akaze->params().eAkazeDescriptor_ == openMVG::features::AKAZE_MLDB ;
  }
  else
  {
    return false ;
  }
}

/**
* @brief Query an image describer if it's a binary one
* @param describerPath Path of the describer
* @retval true if it's a binary describer
* @retval false if it's a scalar describer
*/
bool isBinaryDescriber( const std::string & describerPath )
{
  std::ifstream stream( describerPath ) ;
  if ( stream.is_open() )
  {
    std::shared_ptr<openMVG::features::Image_describer> image_describer ;
    cereal::JSONInputArchive archive( stream );
    archive( cereal::make_nvp( "image_describer", image_describer ) );

    return isBinaryDescriber( image_describer ) ;
  }


  return false ;
}

} // namespace openMVG_gui
