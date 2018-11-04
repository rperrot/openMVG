#ifndef OPENMVG_FEATURES_SFOP_SFOP_DEBUG_HPP
#define OPENMVG_FEATURES_SFOP_SFOP_DEBUG_HPP

#include "openMVG/image/image_container.hpp"

#include <string>

namespace openMVG
{
namespace features
{
namespace sfop
{

void DumpImage( const image::Image<float>& img );

void OutputImage( const image::Image<float>& img, const std::string fileName );

void SaveRawImage( const image::Image<float>& img, const std::string& fileName );

image::Image<float> LoadRawImage( const std::string& filename );

} // namespace sfop
} // namespace features
} // namespace openMVG

#endif