// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_PLY_LOADER_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_PLY_LOADER_HH_

#include "openMVG/numeric/numeric.h"

#include <string>
#include <vector>

namespace openMVG_gui
{

/**
* @brief Load a ply file
* @param path Input path
* @param[out] pos Position
* @param[out] col Color
*/
void LoadPly( const std::string & path ,
              std::vector< openMVG::Vec3 > & pos ,
              std::vector< openMVG::Vec3 > & col );


} // namespace openMVG_gui

#endif