#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_INTRINSIC_REFINER_OPTIONS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_INTRINSIC_REFINER_OPTIONS_HH_

#include "openMVG/cameras/Camera_Common.hpp"

namespace openMVG_gui
{

/**
* @brief list of intrinsics params that need to be refined
*/
enum IntrinsicRefinerOptions : int
{
  INTRINSIC_REFINER_NONE            = 0 ,

  INTRINSIC_REFINER_FOCAL_LENGTH    = 1 << 0 ,
  INTRINSIC_REFINER_PRINCIPAL_POINT = 1 << 1 ,
  INTRINSIC_REFINER_DISTORTION      = 1 << 2 ,

  INTRINSIC_REFINER_FOCAL_LENGTH_PRINCIPAL_POINT = ( 1 << 0 ) | ( 1 << 1 ) ,
  INTRINSIC_REFINER_FOCAL_LENGTH_DISTORTION = ( 1 << 0 ) | ( 1 << 2 ) ,
  INTRINSIC_REFINER_PRINCIPAL_POINT_DISTORTION = ( 1 << 1 ) | ( 1 << 2 ) ,

  INTRINSIC_REFINER_ALL = 1 << 0 | 1 << 1 | 1 << 2
} ;


/**
* @brief perform bitwise OR operation on two options
* @param a first option
* @param b second option
* @return bitwise operation on two options
*/
constexpr IntrinsicRefinerOptions operator|( IntrinsicRefinerOptions a , IntrinsicRefinerOptions b ) ;

/**
* @brief Indicate if a complex option contains another option
* @param option Source option
* @param query Query
* @retval true if Query is fully contained in Source
* @retval if intersection between Query and Source is partial
*/
bool contains( const IntrinsicRefinerOptions& option , const IntrinsicRefinerOptions & query ) ;

/**
* @brief Convert from gui param to openMVG params
*/
openMVG::cameras::Intrinsic_Parameter_Type convert( const IntrinsicRefinerOptions & opt ) ;

} // namespace OpenMVG_gui

#endif