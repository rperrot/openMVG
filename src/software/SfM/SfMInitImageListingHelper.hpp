#ifndef _SOFTWARE_SFM_SFM_INIT_IMAGE_LISTING_HELPER_HPP_
#define _SOFTWARE_SFM_SFM_INIT_IMAGE_LISTING_HELPER_HPP_

#include "openMVG/cameras/cameras.hpp"
#include "openMVG/exif/sensor_width_database/datasheet.hpp"
#include "openMVG/sfm/sfm_data.hpp"


#include <string>
#include <vector>
#include <sstream>

namespace software
{
namespace SfM
{

/// Check that Kmatrix is a string like "f;0;ppx;0;f;ppy;0;0;1"
/// With f,ppx,ppy as valid numerical value
bool checkIntrinsicStringValidity( const std::string & Kmatrix, double & focal, double & ppx, double & ppy ) ;


/**
* @brief Fill SfM data using a root path and a list of image name
* @param rootPath Input image root path
* @param imageNames List of image names
* @param error_report_stream Error report stream
* @param e_User_camera_model Desired camera model
* @param vec_database Database of sensor width for existing camera models
* @param focal_pixel Input focal pixel (force to replace input)
* @param sKmatrix Input intrinsic matrix (force to replace input)
* @param show_progress_bar Show progress bar
*/
bool FillSfMImageData( openMVG::sfm::SfM_Data & data ,
                       const std::string & rootPath ,
                       const std::vector< std::string > & imageNames ,
                       std::ostringstream & error_report_stream ,
                       const openMVG::cameras::EINTRINSIC e_User_camera_model ,
                       const std::vector<Datasheet> & vec_database ,
                       const double focal_pixel = -1.0 ,
                       const std::string & sKmatrix = std::string() ,
                       const bool show_progress_bar = false ) ;

} // namespace SfM
} // namespace software

#endif