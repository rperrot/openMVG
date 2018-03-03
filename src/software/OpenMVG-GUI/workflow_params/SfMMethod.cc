// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#include "SfMMethod.hh"


namespace openMVG_gui
{

/**
 * @brief Helper function (only for debug purpose)
 * Get a string for the given parameter
 * @param meth The parameter to convert to string
 * @return string corresponding to the parameter
 */
std::string to_string( const SfMMethod meth )
{
  switch( meth )
  {
    case SFM_METHOD_GLOBAL :
    {
      return "SfM_Method global" ;
    }
    case SFM_METHOD_INCREMENTAL :
    {
      return "SfM_Method incremental" ;
    }
  }
  return "" ;
}

} // namespace openMVG_gui
