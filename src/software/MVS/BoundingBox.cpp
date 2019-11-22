#include "BoundingBox.hpp"

#include <limits>

namespace MVS 
{
/**
 * @brief Initialize bounding box 
 * 
 * @param min Minimum values around all axes 
 * @param max Maximum values around all axes 
 */
void InitBBox( openMVG::Vec3& min,
               openMVG::Vec3& max )
{
  min = openMVG::Vec3( std::numeric_limits<double>::max(),
                       std::numeric_limits<double>::max(),
                       std::numeric_limits<double>::max() );
  max = openMVG::Vec3( -std::numeric_limits<double>::max(),
                       -std::numeric_limits<double>::max(),
                       -std::numeric_limits<double>::max() );
}

/**
 * @brief Update bounding box with a new point.
 * 
 * @param min Minimum value around all axes.
 * @param max Maximum value around all axes. 
 * @param pt  Point to update.
 */
void UpdateBBox( openMVG::Vec3& min, openMVG::Vec3& max, const openMVG::Vec3& pt )
{
  min[ 0 ] = std::min( pt[ 0 ], min[ 0 ] );
  min[ 1 ] = std::min( pt[ 1 ], min[ 1 ] );
  min[ 2 ] = std::min( pt[ 2 ], min[ 2 ] );

  max[ 0 ] = std::max( pt[ 0 ], max[ 0 ] );
  max[ 1 ] = std::max( pt[ 1 ], max[ 1 ] );
  max[ 2 ] = std::max( pt[ 2 ], max[ 2 ] );
}


}