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