#include "SFOPDebug.hpp"

#include "openMVG/image/image_filtering.hpp"
#include "openMVG/image/image_io.hpp"

#include <fstream>

namespace openMVG
{
namespace features
{
namespace sfop
{

void OutputImage( const image::Image<float>& img, const std::string fileName )
{
  image::Image<float> tmp;
  image::ImageNormalize( img, tmp );

  image::Image<unsigned char> tmp2;
  tmp2 = image::Image<float>( tmp * 255.0f ).cast<unsigned char>();

  image::WriteImage( fileName.c_str(), tmp2 );
}

void DumpImage( const image::Image<float>& img )
{
  std::cout << "w : " << img.Width() << std::endl;
  std::cout << "h : " << img.Height() << std::endl;

  for ( int y = 0; y < img.Height(); ++y )
  {
    for ( int x = 0; x < img.Width(); ++x )
    {
      std::cout << "I[" << y << "," << x << "] = " << img( y, x ) << std::endl;
    }
  }
}

void SaveRawImage( const image::Image<float>& img, const std::string& fileName )
{
  std::ofstream file( fileName );
  if ( !file )
  {
    std::cerr << "COuld not output file " << fileName << std::endl;
    exit( -1 );
  }

  size_t nRow = img.Height(), nCol = img.Width();
  file.write( reinterpret_cast<char*>( &nRow ), sizeof( size_t ) );
  file.write( reinterpret_cast<char*>( &nCol ), sizeof( size_t ) );
  for ( size_t idRow = 0; idRow < nRow; ++idRow )
  {
    for ( size_t idCol = 0; idCol < nCol; ++idCol )
    {
      const float data = img( idRow, idCol );
      file.write( reinterpret_cast<const char*>( &data ), sizeof( float ) );
    }
  }
}

image::Image<float> LoadRawImage( const std::string& filename )
{
  std::ifstream file( filename, std::ios::binary );
  if ( !file )
  {
    std::cerr << "COuld not open file : " << filename << std::endl;
    exit( -1 );
  }

  size_t nRow;
  size_t nCol;
  file.read( reinterpret_cast<char*>( &nRow ), sizeof( size_t ) );
  file.read( reinterpret_cast<char*>( &nCol ), sizeof( size_t ) );

  image::Image<float> res;
  res.resize( nCol, nRow );

  for ( size_t idRow = 0; idRow < nRow; ++idRow )
  {
    for ( size_t idCol = 0; idCol < nCol; ++idCol )
    {
      float data;
      file.read( reinterpret_cast<char*>( &data ), sizeof( float ) );
      res( idRow, idCol ) = data;
    }
  }

  return res;
}

} // namespace sfop
} // namespace features
} // namespace openMVG
