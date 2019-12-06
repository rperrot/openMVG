#include <openMVG/image/image_container.hpp>
#include <openMVG/image/image_convolution.hpp>
#include <openMVG/image/image_drawing.hpp>
#include <openMVG/image/image_io.hpp>
#include <openMVG/image/image_resampling.hpp>
#include <openMVG/numeric/eigen_alias_definition.hpp>

#include "third_party/cmdLine/cmdLine.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace openMVG;
using namespace openMVG::image;

std::vector<Mat> LoadEigenimages( const std::string& folder, const int N, const int LN, const double s1, const double s2 )
{
  std::vector<Mat> res;

  for ( int n = 0; n <= N; ++n )
  {
    std::stringstream filename;
    filename << folder << "/F_" << n << "_[" << LN << "x" << LN << "]_[" << N << "]_[" << s1 << "_" << s2 << "]"
             << ".txt";

    Mat m;
    m.resize( 2 * LN + 1, 2 * LN + 1 );

    std::ifstream file( filename.str() );
    if ( !file )
    {
      std::cerr << "Could not open : " << filename.str() << std::endl;
      res.clear();
      return res;
    }

    for ( size_t idRow = 0; idRow < 2 * LN + 1; ++idRow )
    {
      std::string line;
      if ( !std::getline( file, line ) )
      {
        std::cerr << "File corrupted (File: " << filename.str() << " - Line: " << idRow << ")" << std::endl;
        exit( EXIT_FAILURE );
      }

      std::istringstream str( line );
      for ( size_t idCol = 0; idCol < 2 * LN + 1; ++idCol )
      {
        str >> m( idRow, idCol );
      }
    }
    res.push_back( m );
  }

  return res;
}

Mat LoadEigenCoeffs( const std::string& folder, const int N, const double s1, const double s2 )
{
  Mat res;

  res.resize( N + 1, N + 1 );

  std::stringstream filename;
  filename << folder << "/a_" << N << "_[" << s1 << "_" << s2 << "]"
           << ".txt";

  std::ifstream file( filename.str() );
  if ( !file.good() )
  {
    std::cerr << "Could not load " << filename.str() << std::endl;
    exit( EXIT_FAILURE );
  }

  for ( size_t row = 0; row <= N; ++row )
  {
    std::string line;
    if ( !std::getline( file, line ) )
    {
      std::cerr << "File corrupted (File: " << filename.str() << " - Line: " << row << ")" << std::endl;
      exit( EXIT_FAILURE );
    }
    std::istringstream str( line );

    for ( size_t col = 0; col <= N; ++col )
    {
      str >> res( row, col );
    }
  }

  return res;
}

struct Keypoint
{
  int    _row;
  int    _col;
  double _x;
  double _y;
  double _s;
};

double sLog( const int row, const int col, const double s, const double s2, const double s3, const std::vector<Image<float>>& coefs )
{
  return coefs[ 0 ]( row, col ) +
         coefs[ 1 ]( row, col ) * s +
         coefs[ 2 ]( row, col ) * s2 +
         coefs[ 3 ]( row, col ) * s3;
}

// @return true if point is a local_min or a local_max
bool isLocalMinMax( const int                        row,         // pixel_y
                    const int                        col,         // pixel_x
                    const double                     p0,          // Point at center
                    const double                     s,           // scale
                    const double                     s2,          // scale * scale
                    const double                     s3,          // scale * scale * scale
                    const bool                       is_bright,   // Indicate if point must be a local min (true) or a local max(false)
                    const std::vector<Image<float>>& sLogImages ) // Image used to compute sLog at given point
{
  const double delta_scale = 0.04; // Values used to compute a delta in scale (to filter point wrt to scale space)

  if ( is_bright )
  {
    // Must be a local min
    for ( int ds = -1; ds <= 1; ++ds )
    {
      const double cur_s  = s + ds * delta_scale;
      const double cur_s2 = cur_s * cur_s;
      const double cur_s3 = cur_s * cur_s2;

      // clang-format off
      const bool fail = ( sLog( row - 1, col - 1, cur_s, cur_s2, cur_s3, sLogImages ) < p0 ) ||
                        ( sLog( row - 1, col    , cur_s, cur_s2, cur_s3, sLogImages ) < p0 ) ||
                        ( sLog( row - 1, col + 1, cur_s, cur_s2, cur_s3, sLogImages ) < p0 ) ||
                        ( sLog( row    , col - 1, cur_s, cur_s2, cur_s3, sLogImages ) < p0 ) ||
                        ( sLog( row    , col    , cur_s, cur_s2, cur_s3, sLogImages ) < p0 ) ||
                        ( sLog( row    , col + 1, cur_s, cur_s2, cur_s3, sLogImages ) < p0 ) ||
                        ( sLog( row + 1, col - 1, cur_s, cur_s2, cur_s3, sLogImages ) < p0 ) ||
                        ( sLog( row + 1, col    , cur_s, cur_s2, cur_s3, sLogImages ) < p0 ) ||
                        ( sLog( row + 1, col + 1, cur_s, cur_s2, cur_s3, sLogImages ) < p0 ) ;
      if( fail )
      {
        /*
        std::cout << "Fail because not a local min" << std::endl; 
        std::cout << "Center value: " << p0 << std::endl; 
        std::cout << "Scale was   : " << cur_s << " - " << s << std::endl; 

        std::cout << sLog( row - 1, col - 1, cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row - 1, col    , cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row - 1, col + 1, cur_s, cur_s2, cur_s3, sLogImages ) << std::endl; 
        std::cout << sLog( row , col - 1, cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row , col    , cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row , col + 1, cur_s, cur_s2, cur_s3, sLogImages ) << std::endl; 
        std::cout << sLog( row + 1, col - 1, cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row + 1, col    , cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row + 1, col + 1, cur_s, cur_s2, cur_s3, sLogImages ) << std::endl; 
        */

        return false;
      }
      // clang-format on
    }
  }
  else
  {
    // Must be a local max
    for ( int ds = -1; ds <= 1; ++ds )
    {
      const double cur_s  = s + ds * delta_scale;
      const double cur_s2 = cur_s * cur_s;
      const double cur_s3 = cur_s * cur_s2;

      // clang-format off
      const bool fail = ( sLog( row - 1, col - 1, cur_s, cur_s2, cur_s3, sLogImages ) > p0 ) ||
                        ( sLog( row - 1, col    , cur_s, cur_s2, cur_s3, sLogImages ) > p0 ) ||
                        ( sLog( row - 1, col + 1, cur_s, cur_s2, cur_s3, sLogImages ) > p0 ) ||
                        ( sLog( row    , col - 1, cur_s, cur_s2, cur_s3, sLogImages ) > p0 ) ||
                        ( sLog( row    , col    , cur_s, cur_s2, cur_s3, sLogImages ) > p0 ) ||
                        ( sLog( row    , col + 1, cur_s, cur_s2, cur_s3, sLogImages ) > p0 ) ||
                        ( sLog( row + 1, col - 1, cur_s, cur_s2, cur_s3, sLogImages ) > p0 ) ||
                        ( sLog( row + 1, col    , cur_s, cur_s2, cur_s3, sLogImages ) > p0 ) ||
                        ( sLog( row + 1, col + 1, cur_s, cur_s2, cur_s3, sLogImages ) > p0 ) ;
      if( fail )
      {
        /*
        std::cout << "Fail because not a local max" << std::endl; 
        std::cout << "Center value: " << p0 << std::endl; 
        std::cout << "Scale was   : " << cur_s << " - " << s << std::endl; 

        std::cout << sLog( row - 1, col - 1, cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row - 1, col    , cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row - 1, col + 1, cur_s, cur_s2, cur_s3, sLogImages ) << std::endl; 
        std::cout << sLog( row , col - 1, cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row , col    , cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row , col + 1, cur_s, cur_s2, cur_s3, sLogImages ) << std::endl; 
        std::cout << sLog( row + 1, col - 1, cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row + 1, col    , cur_s, cur_s2, cur_s3, sLogImages ) << " " << sLog( row + 1, col + 1, cur_s, cur_s2, cur_s3, sLogImages ) << std::endl; 
        */

        return false ;
      }
      // clang-format on
    }
  }
  return true;
}

// Indicate if point is an edge using hessian
bool isEdge( const int                        row,
             const int                        col,
             const double                     p5,
             const double                     s,
             const double                     s2,
             const double                     s3,
             const std::vector<Image<float>>& sLogImages )
{
  /*
  p1 p2 p3
  p4 p5 p6
  p7 p8 p9
  */

  // clang-format off
  const double p1 = sLog( row - 1, col - 1, s, s2, s3, sLogImages );
  const double p2 = sLog( row - 1, col    , s, s2, s3, sLogImages );
  const double p3 = sLog( row - 1, col + 1, s, s2, s3, sLogImages );

  const double p4 = sLog( row    , col - 1, s, s2, s3, sLogImages );
  const double p6 = sLog( row    , col + 1, s, s2, s3, sLogImages );
  
  const double p7 = sLog( row + 1, col - 1, s, s2, s3, sLogImages );
  const double p8 = sLog( row + 1, col    , s, s2, s3, sLogImages );
  const double p9 = sLog( row + 1, col + 1, s, s2, s3, sLogImages );
  // clang-format on

  const double dXX = p6 + p4 - 2.0 * p5;
  const double dYY = p2 + p8 - 2.0 * p5;
  const double dXY = ( ( p9 - p3 ) - ( p7 - p1 ) ) / 4.0;

  const double tr  = dXX + dYY;
  const double det = dXX * dYY - dXY * dXY;

  const double edge_th_param = 10.0;
  const double edge_th       = ( edge_th_param + 1.0 ) * ( edge_th_param + 1.0 ) / edge_th_param;
  const double edge_response = tr * tr / det;

  if ( det <= 0.0 )
    return true;
  if ( edge_response >= edge_th )
    return true;

  return false; // Maybe a corner or flat region
}

bool PixelRefine( int&                             row,
                  int&                             col,
                  double&                          x,
                  double&                          y,
                  double&                          s,
                  double&                          s2,
                  double                           s3,
                  const std::vector<Image<float>>& sLogImages )
{
  const double p1 = sLog( row - 1, col - 1, s, s2, s3, sLogImages );
  const double p2 = sLog( row - 1, col, s, s2, s3, sLogImages );
  const double p3 = sLog( row - 1, col + 1, s, s2, s3, sLogImages );

  const double p4 = sLog( row, col - 1, s, s2, s3, sLogImages );
  const double p5 = sLog( row, col, s, s2, s3, sLogImages );
  const double p6 = sLog( row, col + 1, s, s2, s3, sLogImages );

  const double p7 = sLog( row + 1, col - 1, s, s2, s3, sLogImages );
  const double p8 = sLog( row + 1, col, s, s2, s3, sLogImages );
  const double p9 = sLog( row + 1, col + 1, s, s2, s3, sLogImages );

  const double dXX = p6 + p4 - 2.0 * p5;
  const double dYY = p2 + p8 - 2.0 * p5;
  const double dXY = ( ( p9 - p3 ) - ( p7 - p1 ) ) / 4.0;

  const double dx = ( p6 - p4 ) / 2.0;
  const double dy = ( p8 - p2 ) / 2.0;

  // Inverse Hessian
  Eigen::Matrix<double, 2, 2> A;
  A << dXX, dXY, dXY, dYY;
  Vec2 b;
  b << -dx, -dy;

  Eigen::FullPivLU<Eigen::Matrix<double, 2, 2>> lu_decomp( A );
  if ( !lu_decomp.isInvertible() )
    return false;

  const Vec2   dst   = lu_decomp.solve( b );
  const double ofstX = dst( 0 );
  const double ofstY = dst( 1 );

  x += ofstX;
  y += ofstY;

  s += ( ofstX * dx + ofstY * dy ) / 2.0;
  s2 = s * s;
  s3 = s * s2;

  return true;
}

void addPutativeKeypoint( const int                        row,
                          const int                        col,
                          const int                        idOctave,
                          const double                     s,
                          const bool                       is_bright,
                          const std::vector<Image<float>>& sLogImages,
                          std::vector<Keypoint>&           keypoints )
{
  const double s2 = s * s;
  const double s3 = s * s2;

  // Compute sLog at given pixel for the given scale
  const double p0 = sLog( row, col, s, s2, s3, sLogImages );

  const double peak_threshold = 0.07 * 255 * 0.5;
  // Peak threshold
  if ( fabs( p0 ) < peak_threshold )
  {
    return;
  }

  if ( !isLocalMinMax( row, col, p0, s, s2, s3, is_bright, sLogImages ) )
  {
    return;
  }

  int    cur_row = row;
  int    cur_col = col;
  double cur_s   = s;
  double cur_s2  = cur_s * cur_s;
  double cur_s3  = cur_s * cur_s2;
  double x       = col;
  double y       = row;

  // Refinement process.
  for ( int i = 0; i < 5; ++i )
  {
    if ( isEdge( cur_row, cur_col, p0, cur_s, cur_s2, cur_s3, sLogImages ) )
    {
      return;
    }
    else
    {
      if ( !PixelRefine( cur_row, cur_col, x, y, cur_s, cur_s2, cur_s3, sLogImages ) )
      {
        return;
      }

      // Pixel refinement
      Keypoint tmp;

      const double scaleOctave = 1 << idOctave;

      tmp._col = cur_col * scaleOctave;
      tmp._row = cur_row * scaleOctave;
      tmp._s   = s * scaleOctave;
      tmp._x   = x * scaleOctave;
      tmp._y   = y * scaleOctave;
      keypoints.push_back( tmp );

      break;
    }
  }
}

void detect( const Image<float>&     in,
             const int               idOctave,
             const std::vector<Mat>& Fi,
             const Mat&              a_ij,
             const double            s1,
             const double            s2,
             std::vector<Keypoint>&  keypoints )
{
  std::vector<Image<float>> qi;
  qi.resize( Fi.size() );

  // Filter images (ie: compute qi_s)
  std::cout << "Compute qi" << std::endl;
  for ( int i = 0; i < Fi.size(); ++i )
  {
    const Mat& curFilter = Fi[ i ];
    std::cout << "q_" << i << std::endl;
    ImageConvolution( in, curFilter, qi[ i ] );
  }

  // Compute sLog image coeffs (eq (22))
  std::cout << "Compute sLog base images" << std::endl;
  std::vector<Image<float>> coefs;
  for ( int i = 0; i < Fi.size(); ++i )
  {
    Image<float> tmp;
    tmp.resize( qi[ 0 ].Width(), qi[ 0 ].Height() );

    for ( int j = 0; j < Fi.size(); ++j )
    {
      tmp += qi[ j ] * a_ij( j, i );
    }
    coefs.push_back( tmp );
  }

  const double threshold_different_detect = 2.0; // Difference in scale to consider detection scale as different.

  std::cout << "Compute keypoints" << std::endl;
  // Now compute optimal scale
  const int borderSize = 5;
  for ( size_t row = borderSize; row < qi[ 0 ].Height() - borderSize; ++row )
  {
    for ( size_t col = borderSize; col < qi[ 0 ].Width() - borderSize; ++col )
    {
      // Compute derivative of slog.
      // slog       = coef[0] + coef[1] s + coef[2] s^2 + coef[3] s^3
      // d_s( log ) = coef[1] + 2.0 * s * coef[2] + 3.0 * s^2 coef[3]
      //            = c + s * b + s^2 * c
      const double a = 3.0 * coefs[ 3 ]( row, col );
      const double b = 2.0 * coefs[ 2 ]( row, col );
      const double c = coefs[ 1 ]( row, col );

      // solve quadratic form a^2 s + b s + c
      // ie: peak in the scale is when derivative d_s( log )
      const double delta = b * b - 4.0 * a * c;
      if ( delta > 0.0 )
      {
        const double tmp = std::sqrt( delta );
        const double _s1 = ( -b - tmp ) / ( 2.0 * a );
        const double _s2 = ( -b + tmp ) / ( 2.0 * a );

        const double scale1 = std::min( _s1, _s2 );
        const double scale2 = std::max( _s1, _s2 );

        if ( scale1 >= ( s1 + 0.5 ) && scale1 <= ( s2 - 0.5 ) )
        {
          // is_bright indicate if the point is a local min or local max.
          // if second derivative is a positive, it's a local min
          // (the author named it bright point)
          const bool is_bright = ( 2.0 * a * scale1 + b ) > 0.0; // 2nd derivative.
          // Scale is in range
          addPutativeKeypoint( row, col, idOctave, scale1, is_bright, coefs, keypoints );
        }

        if ( scale2 >= ( s1 + 0.5 ) && scale2 <= ( s2 - 0.5 ) && ( scale2 - scale1 ) >= threshold_different_detect )
        {
          const bool is_bright = ( 2.0 * a * scale2 + b ) > 0.0;

          // In original
          addPutativeKeypoint( row, col, idOctave, scale2, is_bright, coefs, keypoints );
        }
      }
    }
  }
}

int main( int argc, char** argv )
{

  int         N  = 3;
  int         LN = 33;
  double      s1 = 1.0;
  double      s2 = 5.0;
  std::string sInputImage;

  CmdLine cmd;
  cmd.add( make_option( 'i', sInputImage, "input_image" ) );

  try
  {
    if ( argc == 1 )
      throw std::string( "Invalid command line parameter." );
    cmd.process( argc, argv );
  }
  catch ( const std::string& s )
  {
    std::cerr << s << std::endl;
    std::cerr << "Usage: " << argv[ 0 ] << '\n'
              << "[-i|--input_image] the path to the input image.\n";
    exit( EXIT_FAILURE );
  }

  // Parameters:
  // Input image
  // eigenimages params (N, LN, s1, s2)
  // Output folder

  Image<unsigned char> image;
  ReadImage( sInputImage.c_str(), &image );

  image::Image<float> imageF( image.GetMat().cast<float>() / 255.0f );

  std::vector<Mat> F = LoadEigenimages( ".", N, LN, s1, s2 );
  Mat              a = LoadEigenCoeffs( ".", N, s1, s2 );

  std::vector<Keypoint> keypoints;

  const int border   = 5;
  int       idOctave = 0;
  while ( imageF.Width() > ( LN + border ) &&
          imageF.Height() > ( LN + border ) )
  {
    detect( imageF, idOctave, F, a, s1, s2, keypoints );

    Image<float> tmp;
    ImageDecimate( imageF, tmp );
    imageF = tmp;

    ++idOctave;
  }

  std::cout << "Number of keypoints: " << keypoints.size() << std::endl;

  std::cout << "Producing output image" << std::endl;
  Image<RGBColor> result;
  ReadImage( sInputImage.c_str(), &result );

  // Draw points.
  for ( size_t idPt = 0; idPt < keypoints.size(); ++idPt )
  {
    const Keypoint& cur = keypoints[ idPt ];

    // Center
    DrawCircle( cur._x, cur._y, 1.0, RGBColor( 0, 255, 0 ), &result );
    // Circle
    DrawCircle( cur._x, cur._y, cur._s , RGBColor( 255, 0, 0 ), &result );
  }

  WriteImage( "out.png", result );
}