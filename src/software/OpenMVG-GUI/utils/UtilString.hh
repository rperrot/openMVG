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