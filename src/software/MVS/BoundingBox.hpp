#ifndef _OPENMVG_MVS_BOUNDING_BOX_HPP_
#define _OPENMVG_MVS_BOUNDING_BOX_HPP_

#include "openMVG/numeric/numeric.h"

namespace MVS
{

/**
 * @brief Initialize bounding box 
 * 
 * @param min Minimum values around all axes 
 * @param max Maximum values around all axes 
 */
void InitBBox( openMVG::Vec3& min,
               openMVG::Vec3& max );

/**
 * @brief Update bounding box with a new point.
 * 
 * @param min Minimum value around all axes.
 * @param max Maximum value around all axes. 
 * @param pt  Point to update.
 */
void UpdateBBox( openMVG::Vec3& min, openMVG::Vec3& max, const openMVG::Vec3& pt );

} // namespace MVS

#endif