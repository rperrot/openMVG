#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_IMAGE_DESCRIBER_INFO_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_IMAGE_DESCRIBER_INFO_HH_

#include <memory>
#include <string>

namespace openMVG
{
namespace features
{
class Image_describer ;
} // namespace features
} // namespace openMVG

namespace openMVG_gui
{

// TODO (see with @pmoulon) : does this function may not be deserved to be in Image_describer class ?
// -> This could be helpfull for future describer ?


/**
* @brief Query an image describer if it's a binary one
* @param desc Describer 
* @retval true if it's a binary describer
* @retval false if it's a scalar describer
*/
bool isBinaryDescriber( const std::shared_ptr< openMVG::features::Image_describer > desc ) ;

/**
* @brief Query an image describer if it's a binary one
* @param describerPath Path of the describer
* @retval true if it's a binary describer
* @retval false if it's a scalar describer
*/
bool isBinaryDescriber( const std::string & describerPath ) ; 

} // namespace openMVG_gui

#endif