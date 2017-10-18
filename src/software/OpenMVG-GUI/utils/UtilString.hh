// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_UTILS_UTIL_STRING_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_UTILS_UTIL_STRING_HH_

#include <string>

namespace openMVG_gui
{

/**
* @brief get full content of a file into a string
* @param path of the file
* @return string content of the file
* @note if file could not be read, return an empty string
*/
std::string fileContent( const std::string & path ) ;


} // namespace openMVG_gui

#endif