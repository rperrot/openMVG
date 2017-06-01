#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_PLANE_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_PLANE_HH_

#include "openMVG/numeric/numeric.h"

namespace openMVG_gui
{
/**
* @brief class holding a 3d hyperplane
*/
class Plane
{
  public:

    /**
    * @brief Constructor
    * @param Plane with a specified normal passing through a point
    */
    Plane( const openMVG::Vec3 & n , const openMVG::Vec3 & pt ) ;

    /**
    * @brief Intersection distance of plane and a line
    * @param a First point of the line
    * @param b Second point of the line
    * @return Distance of the intersection point between line and plane
    * @note distance of the intersection is wrt to a
    */
    double intersectionDistance( const openMVG::Vec3 & a , const openMVG::Vec3 & b ) const ;

    /**
    * @brief Compute Intersection point of a plane and a line
    * @param a First point of the line
    * @param b Second point of the line
    * @return Intersection point of the line and the plane
    */
    openMVG::Vec3 intersectionPosition( const openMVG::Vec3 & a , const openMVG::Vec3 & b ) const ;

  private:

    // normal
    openMVG::Vec3 m_n ;
    // Position
    openMVG::Vec3 m_p ;
} ;
}

#endif