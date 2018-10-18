#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_UTIL_FILE_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_UTIL_FILE_HH_

#include <string>

namespace openMVG_gui
{

/**
  * @brief Merge two files 
  * 
  * @param path1  Path of the first file 
  * @param path2  Path of the second file 
  * @return true  If success
  * @return false If failure
  */
bool mergeFiles( const std::string &path1, const std::string &path2, const std::string &outPath );
} // namespace openMVG_gui

#endif