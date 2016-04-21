#include "software/SfM/SfMInitImageListingHelper.hpp"


#include "third_party/progress/progress.hpp"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include "openMVG/exif/exif_IO_EasyExif.hpp"
#include "openMVG/exif/sensor_width_database/ParseDatabase.hpp"
#include "openMVG/image/image.hpp"
#include "openMVG/stl/split.hpp"


#include <vector>
#include <iostream>
#include <sstream>
#include <memory>

using namespace openMVG ;
using namespace openMVG::image ;
using namespace openMVG::exif ;
using namespace openMVG::cameras ;
using namespace openMVG::sfm ;

namespace software
{
namespace SfM
{
/// Check that Kmatrix is a string like "f;0;ppx;0;f;ppy;0;0;1"
/// With f,ppx,ppy as valid numerical value
bool checkIntrinsicStringValidity( const std::string & Kmatrix, double & focal, double & ppx, double & ppy )
{
  std::vector<std::string> vec_str;
  stl::split( Kmatrix, ';', vec_str );
  if ( vec_str.size() != 9 )
  {
    std::cerr << "\n Missing ';' character" << std::endl;
    return false;
  }
  // Check that all K matrix value are valid numbers
  for ( size_t i = 0; i < vec_str.size(); ++i )
  {
    double readvalue = 0.0;
    std::stringstream ss;
    ss.str( vec_str[i] );
    if ( ! ( ss >> readvalue ) )
    {
      std::cerr << "\n Used an invalid not a number character" << std::endl;
      return false;
    }
    if ( i == 0 )
    {
      focal = readvalue;
    }
    if ( i == 2 )
    {
      ppx = readvalue;
    }
    if ( i == 5 )
    {
      ppy = readvalue;
    }
  }
  return true;
}


/**
* @brief Fill SfM data using a root path and a list of image name
* @param rootPath Input image root path
* @param imageNames List of image names
* @param error_report_stream Error report stream
* @param show_progress_bar Show progress bar
*/
bool FillSfMImageData( openMVG::sfm::SfM_Data & data ,
                       const std::string & sImageDir ,
                       const std::vector< std::string > & vec_image ,
                       std::ostringstream & error_report_stream ,
                       const EINTRINSIC e_User_camera_model ,
                       const std::vector<Datasheet> & vec_database ,
                       const double focal_pixels ,
                       const std::string & sKmatrix ,
                       const bool show_progress_bar )
{

  Views & views = data.views;
  Intrinsics & intrinsics = data.intrinsics;


  C_Progress_display my_progress_bar( vec_image.size(),
                                      std::cout, "\n- Image listing -\n" );

  for ( std::vector<std::string>::const_iterator iter_image = vec_image.begin();
        iter_image != vec_image.end();
        ++iter_image )
  {
    // Read meta data to fill camera parameter (w,h,focal,ppx,ppy) fields.
    double width , height , ppx , ppy , focal ;

    width = height = ppx = ppy = focal = -1.0;

    const std::string sImageFilename = stlplus::create_filespec( sImageDir, *iter_image );
    const std::string sImFilenamePart = stlplus::filename_part( sImageFilename );

    // Test if the image format is supported:
    if ( openMVG::image::GetFormat( sImageFilename.c_str() ) == openMVG::image::Unknown )
    {
      error_report_stream
          << sImFilenamePart << ": Unkown image file format." << "\n";
      continue; // image cannot be opened
    }

    if( sImFilenamePart.find( "mask.png" ) != std::string::npos
        || sImFilenamePart.find( "_mask.png" ) != std::string::npos )
    {
      error_report_stream
          << sImFilenamePart << " is a mask image" << "\n";
      continue;
    }

    ImageHeader imgHeader;
    if ( !openMVG::image::ReadImageHeader( sImageFilename.c_str(), &imgHeader ) )
    {
      continue;  // image cannot be read
    }

    width = imgHeader.width;
    height = imgHeader.height;
    ppx = width / 2.0;
    ppy = height / 2.0;

    std::unique_ptr<Exif_IO> exifReader( new Exif_IO_EasyExif );
    exifReader->open( sImageFilename );

    const bool bHaveValidExifMetadata =
      exifReader->doesHaveExifInfo()
      && !exifReader->getModel().empty();

    // Consider the case where the focal is provided manually
    if ( !bHaveValidExifMetadata || focal_pixels != -1 )
    {
      if ( sKmatrix.size() > 0 ) // Known user calibration K matrix
      {
        if ( !checkIntrinsicStringValidity( sKmatrix, focal, ppx, ppy ) )
        {
          focal = -1.0;
        }
      }
      else // User provided focal length value
        if ( focal_pixels != -1 )
        {
          focal = focal_pixels;
        }
    }
    else // If image contains meta data
    {
      const std::string sCamModel = exifReader->getModel();

      // Handle case where focal length is equal to 0
      if ( exifReader->getFocal() == 0.0f )
      {
        error_report_stream
            << stlplus::basename_part( sImageFilename ) << ": Focal length is missing." << "\n";
        focal = -1.0;
      }
      else
        // Create the image entry in the list file
      {
        Datasheet datasheet;
        if ( getInfo( sCamModel, vec_database, datasheet ) )
        {
          // The camera model was found in the database so we can compute it's approximated focal length
          const double ccdw = datasheet.sensorSize_;
          focal = std::max ( width, height ) * exifReader->getFocal() / ccdw;
        }
        else
        {
          error_report_stream
              << stlplus::basename_part( sImageFilename )
              << "\" model \"" << sCamModel << "\" doesn't exist in the database" << "\n"
              << "Please consider add your camera model and sensor width in the database." << "\n";
        }
      }
    }

    // Build intrinsic parameter related to the view
    std::shared_ptr<IntrinsicBase> intrinsic ( nullptr );

    if ( focal > 0 && ppx > 0 && ppy > 0 && width > 0 && height > 0 )
    {
      // Create the desired camera type
      switch( e_User_camera_model )
      {
        case PINHOLE_CAMERA:
          intrinsic = std::make_shared<Pinhole_Intrinsic>
                      ( width, height, focal, ppx, ppy );
          break;
        case PINHOLE_CAMERA_RADIAL1:
          intrinsic = std::make_shared<Pinhole_Intrinsic_Radial_K1>
                      ( width, height, focal, ppx, ppy, 0.0 ); // setup no distortion as initial guess
          break;
        case PINHOLE_CAMERA_RADIAL3:
          intrinsic = std::make_shared<Pinhole_Intrinsic_Radial_K3>
                      ( width, height, focal, ppx, ppy, 0.0, 0.0, 0.0 ); // setup no distortion as initial guess
          break;
        case PINHOLE_CAMERA_BROWN:
          intrinsic = std::make_shared<Pinhole_Intrinsic_Brown_T2>
                      ( width, height, focal, ppx, ppy, 0.0, 0.0, 0.0, 0.0, 0.0 ); // setup no distortion as initial guess
          break;
        case PINHOLE_CAMERA_FISHEYE:
          intrinsic = std::make_shared<Pinhole_Intrinsic_Fisheye>
                      ( width, height, focal, ppx, ppy, 0.0, 0.0, 0.0, 0.0 ); // setup no distortion as initial guess
          break;
        default:
          std::cerr << "Error: unknown camera model: " << ( int ) e_User_camera_model << std::endl;
          return false ;
      }
    }

    // Build the view corresponding to the image
    View v( *iter_image, views.size(), views.size(), views.size(), width, height );

    // Add intrinsic related to the image (if any)
    if ( intrinsic == NULL )
    {
      //Since the view have invalid intrinsic data
      // (export the view, with an invalid intrinsic field value)
      v.id_intrinsic = UndefinedIndexT;
    }
    else
    {
      // Add the defined intrinsic to the sfm_container
      intrinsics[v.id_intrinsic] = intrinsic;
    }

    // Add the view to the sfm_container
    views[v.id_view] = std::make_shared<View>( v );

    if( show_progress_bar )
    {
      ++my_progress_bar ;
    }
  }
  return true ;
}


} // namespace SfM
} // namespace software