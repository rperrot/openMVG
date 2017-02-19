// Copyright (c) 2017 Romuald Perrot.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "testing/testing.h"

#include "openMVG/image/image.hpp"
#include "openMVG/features/saddle/saddle_detector.hpp"

#include "third_party/vectorGraphics/svgDrawer.hpp"


using namespace openMVG ;
using namespace openMVG::image ; 
using namespace openMVG::features ; 
using namespace svg ; 

TEST( saddle , sinboard )
{
  // Generate a sinus board 
  Image<unsigned char> board ; 

  int w = 1024 ;
  int h = 1024 ; 

  int b_w = 6 ; 

  board.resize( w , h ) ; 
  for( int id_row = 0 ; id_row < h ; ++id_row )
  {
    for( int id_col = 0 ; id_col < w ; ++id_col )
    {
      board( id_row , id_col ) = ( 1.0 + std::sin( static_cast<double>(id_row) / ( static_cast<double>(b_w) * M_PI ) ) * 
                                         std::sin( static_cast<double>(id_col) / ( static_cast<double>(b_w) * M_PI ) ) ) / 2.0 * 255 ; 
    }
  }
  WriteImage( "board.png" , board ) ; 

  // Detect points 
  const int nb_level = 6 ;
  const double scale_factor = 1.3 ;
  const int epsilon = 3 ; 
  SaddleDetector detector( nb_level , scale_factor , epsilon ) ; 
  std::vector<SIOPointFeature> pts ; 
  detector.detect( board , pts ) ; 

  std::cout << "Nb points : " << pts.size() << std::endl ;

  // Save points 
  svgDrawer svgStream( board.Width(), board.Height());
  svgStream.drawImage( "board.png" , board.Width(), board.Height());
  for (size_t i = 0; i < pts.size(); ++i) {
    const SIOPointFeature & pt = pts[i];
    svgStream.drawCircle(pt.x(), pt.y(), 2.0 * pt.scale() , svgStyle().stroke("yellow", 2.0));
  }
  const std::string out_filename = "Saddle_Features.svg";
  std::ofstream svgFile( out_filename.c_str() );
  svgFile << svgStream.closeSvgFile().str();
  svgFile.close();
}

/* ************************************************************************* */
int main() { TestResult tr; return TestRegistry::runAllTests(tr);}
/* ************************************************************************* */
