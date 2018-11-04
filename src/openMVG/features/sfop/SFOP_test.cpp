#include "openMVG/features/regions_factory.hpp"

#include "openMVG/features/sfop/SFOPImageDescriber.hpp"
#include "openMVG/features/sfop/SFOPImageDescriber_io.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/system/timer.hpp"
#include "third_party/vectorGraphics/svgDrawer.hpp"

#include "testing/testing.h"

#include <memory>

using namespace openMVG;
using namespace openMVG::image;
using namespace openMVG::features;
using namespace openMVG::features::sfop;
using namespace openMVG::system;

TEST( SFOP_Keypoint, DetectionAndDescription )
{
  Image<unsigned char> in;

  const std::string png_filename = std::string( THIS_SOURCE_DIR ) + "/../../../openMVG_Samples/imageData/StanfordMobileVisualSearch/Ace_0.png";
  EXPECT_TRUE( ReadImage( png_filename.c_str(), &in ) );

  openMVG::system::Timer timer;

  SFOPImageDescriber sfopDescriber( ( SFOPParams() ) );

  auto          regions = sfopDescriber.Describe( in );
  SIFT_Regions* regs    = dynamic_cast<SIFT_Regions*>( regions.get() );

  std::cout
      << "# Keypoints   : " << regs->Features().size() << std::endl;
  std::cout << "SFOP duration : " << timer.elapsedMs() << "ms" << std::endl;

  //--
  // Export found keypoints as a SVG surface
  // SIFT features drawn as circle and one line for the orientation
  //--
  using namespace svg;
  svgDrawer svgStream( in.Width(), in.Height() );
  svgStream.drawImage( png_filename, in.Width(), in.Height() );
  for ( size_t i = 0; i < regs->Features().size(); ++i )
  {
    const SIOPointFeature& curFeat = regs->Features()[ i ];

    const float x      = curFeat.x();
    const float y      = curFeat.y();
    const float scale  = curFeat.scale();
    const float orient = curFeat.orientation();

    svgStream.drawCircle( x, y, scale, svgStyle().stroke( "yellow", 2.0 ) );
    // Orientation
    svgStream.drawLine( x, y, x + std::cos( orient ) * scale, y + std::sin( orient ) * scale, svgStyle().stroke( "green", 2.0 ) );
  }
  std::string   out_filename = "SFOP_Features.svg";
  std::ofstream svgFile( out_filename.c_str() );
  svgFile << svgStream.closeSvgFile().str();
  svgFile.close();
}

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
