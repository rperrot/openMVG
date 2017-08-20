#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_SPHERE_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_SPHERE_HH_

#include "openMVG/numeric/numeric.h"

namespace openMVG_gui
{
/**
* @brief class holding a sphere
*/
class Sphere
{
  public:

    /**
    * @brief Ctr
    * @param center Center of the sphere
    * @param rad Radius of the sphere
    */
    Sphere( const openMVG::Vec3 & center , const double rad ) ;

    /*
    * @brief Compute intersection point (if it exists) with a line
    * @param a First point
    * @param b Second point
    * @param[out] pt Intersection point
    * @retval true if intersection exists
    * @retval false if no intersection
    */
    bool intersect( const openMVG::Vec3 & a , const openMVG::Vec3 & b , openMVG::Vec3 & pt ) const ;

  private:
    openMVG::Vec3 m_center ;
    double m_radius ;
} ;

} // namespace openMVG_gui

#endif