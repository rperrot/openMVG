#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_AABB_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_AABB_HH_

#include "openMVG/numeric/numeric.h"

namespace openMVG_gui
{

/**
* @brief class managing a standard Axis Aligned Bounding Box
*/
class AABB
{
  public:

    /**
    * @brief ctr
    */
    AABB( void ) ;

    /**
    * @brief ctr around a point
    */
    AABB( const openMVG::Vec3 & pt ) ;

    /**
    * @brief Full ctr
    */
    AABB( const openMVG::Vec3 & min , const openMVG::Vec3 & max ) ;

    /**
    * @brief Copy ctr
    */
    AABB( const AABB & src ) ;

    /**
    * @brief Assignment operator
    */
    AABB & operator=( const AABB & src ) ;

    /**
    * @brief Minimum
    */
    openMVG::Vec3 min( void ) const ;

    /**
    * @brief Maximum
    */
    openMVG::Vec3 max( void ) const ;

    /**
    * @brief Length in each dim
    */
    openMVG::Vec3 extends( void ) const ;

    /**
    * @brief Union of two bounding boxes
    */
    friend AABB makeUnion( const AABB & a , const AABB & b ) ;

    /**
    * @brief Union of a bounding box and a point
    */
    friend AABB makeUnion( const AABB & a , const openMVG::Vec3 & pt ) ;

    /**
    * @brief Union of a bounding box and a point
    */
    friend AABB makeUnion( const openMVG::Vec3 & pt , const AABB & a ) ;

  private:

    openMVG::Vec3 m_min ;
    openMVG::Vec3 m_max ;
} ;

} // namespace openMVG_gui

#endif