#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_DOWNLOAD_FILE_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_DOWNLOAD_FILE_HH_

#include <string>

namespace openMVG_gui
{

/**
 * @brief Download a file from an URL
 *
 * @param base_url    URL of the file to download
 * @param output_path Path where the file will be downloaded
 * @param hash_url    URL of the hash file
 *
 * @retval true if success
 * @retval false if failure
 *
 * @note if no hash URL is given, no hash is computed
 * @note HASH used is SHA256
 */
bool DownloadFile( const std::string &base_url, const std::string &output_path, const std::string &hash_url = "" );

} // namespace openMVG_gui

#endif