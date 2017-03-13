// Copyright (c) 2016 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_CAMERAS_CAMERAS_COMMON_COMMAND_LINE_HELPER_HPP
#define OPENMVG_CAMERAS_CAMERAS_COMMON_COMMAND_LINE_HELPER_HPP

#include "openMVG/cameras/Camera_Common.hpp"

#include <string>

namespace openMVG
{
namespace cameras
{

  // Allow to initialize an object cameras::Intrinsic_Parameter_Type BA from
  // a string and delimiters('|')
  //
  cameras::Intrinsic_Parameter_Type StringTo_Intrinsic_Parameter_Type(
      const std::string &rhs );

} // namespace cameras
} // namespace openMVG

#endif // OPENMVG_CAMERAS_CAMERAS_COMMON_COMMAND_LINE_HELPER_HPP
