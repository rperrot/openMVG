// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_SFM_METHOD_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_SFM_METHOD_HH_

#include <string>

namespace openMVG_gui
{

enum SfMMethod
{
  SFM_METHOD_INCREMENTAL ,
  SFM_METHOD_GLOBAL
} ;

/**
 * @brief Helper function (only for debug purpose)
 * Get a string for the given parameter 
 * @param meth The parameter to convert to string 
 * @return string corresponding to the parameter
 */
std::string to_string( const SfMMethod meth ) ;

} // namespace openMVG_gui

#endif