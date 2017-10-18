// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_BOUNDING_SPHERE_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_BOUNDING_SPHERE_HH_

#include "openMVG/numeric/numeric.h"

#include <iostream>

namespace openMVG_gui
{

/**
* @brief compute bounding sphere of a given point list
* @param pts List of points
* @param[out] center of the sphere
* @param[out] rad radius of the sphere
* @note This is an implementation of the Ritter algorithm
*/
void computeBoundingSphere( const std::vector< openMVG::Vec3 > & pts ,
                            openMVG::Vec3 & center ,
                            double & rad ) ;

/**
* @brief Compute bounding box of a set of points
* @param pts List of points
* @param[out] min Minimum on each coords
* @param[out] max Maximum on each coords
*/
void computeBoundingBox( const std::vector< openMVG::Vec3 > & pts ,
                         openMVG::Vec3 & min ,
                         openMVG::Vec3 & max ) ;

} // namespace openMVG_gui

#endif