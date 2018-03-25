#include "Image.hpp"

#include "openMVG/cameras/Camera_undistort_image.hpp"

#include "openMVG/image/image_io.hpp"
#include "openMVG/image/image_filtering.hpp"
#include "openMVG/image/image_resampling.hpp"
#include "openMVG/image/pixel_types_io_cereal.hpp"

#include "openMVG/numeric/numeric_io_cereal.hpp"

#include <cereal/archives/portable_binary.hpp>

#include <fstream>
#include <utility>

namespace MVS
{

/**
  * @brief load an augmented image from a file
  * @param path Path of the image to load
  * @param scale Scale of the image (0 -> Same size , 1 -> half the size , other -> 1/2^scale)
  */
Image::Image( const std::string & path , const int scale , const openMVG::cameras::IntrinsicBase * intrinsic , ImageLoadType load )
{
  if( ReadImage( path.c_str() , &m_grayscale ) == 0 )
  {
    std::cerr << "Warning : could not load image" << std::endl ;
  }

  openMVG::image::Image< openMVG::image::RGBColor > color_img ;
  if( ! ReadImage( path.c_str() , &color_img ) )
  {
    std::cerr << "Could not load image" << std::endl ;
  }

  // Undistort image
  if ( intrinsic->have_disto() )
  {
    openMVG::image::Image<openMVG::image::RGBColor> img_ud ;
    // Undistort
    UndistortImage( color_img , intrinsic , img_ud , openMVG::image::BLACK );
    color_img = img_ud ;
  }

  const openMVG::image::Sampler2d<openMVG::image::SamplerLinear> sampler;
  openMVG::image::Rescale( color_img , scale , sampler , m_color ) ;

  // Convert to grayscale
  openMVG::image::ConvertPixelType( m_color , &m_grayscale ) ;

  if( ( load & IMAGE_GRADIENT ) != 0 )
  {
    computeGradient() ;
  }
  if( ( load & IMAGE_CENSUS ) != 0 )
  {
    computeCensus() ;
  }
}

/**
* @brief Load image using both parts
* @param gray_image_path Path of the image
* @param gradient_image_path Path of the image
*/
Image::Image( const std::string & color_image_path , const std::string & gray_image_path , const std::string & gradient_image_path , const std::string & census_path , const ImageLoadType & load )
{
  if( ! this->load( color_image_path , gray_image_path , gradient_image_path , census_path , load ) )
  {
    std::cerr << "Warning : could not create image from serialization" << std::endl ;
  }
}


/**
* @brief Get intensity at a specified position
* @param id_row Index of the row
* @param id_col Index of the column
* @return intensity at specified position
*/
unsigned char Image::intensity( const int id_row , const int id_col ) const
{
  return m_grayscale.coeffRef( id_row , id_col ) ;
}

/**
* @brief Get intensity at specified position
* @param pos Position ( id_y , id_x )
* @return intensity at specified position
*/
unsigned char Image::intensity( const openMVG::Vec2i & pos ) const
{
  return intensity( pos[0] , pos[1] ) ;
}


/**
* @brief Get census bitstring at specified position
* @param id_row Index of the row
* @param id_col Index of the column
* @return census bitstring for corresponding pixel
*/
uint64_t Image::census( const int id_row , const int id_col ) const
{
  return m_census.coeffRef( id_row , id_col ) ;
}

/**
* @brief Get gradient magnitude at specified position
* @param id_row Index of the row
* @param id_col Index of the column
* @return Gradient magnitude at specified position
*/
const openMVG::Vec4 & Image::gradient( const int id_row , const int id_col ) const
{
  return m_gradient.coeffRef( id_row , id_col ) ;
}

/**
* @brief Get gradient magnitude at specified position
* @param pos Position ( id_y , id_x )
* @return intensity at specified position
*/
const openMVG::Vec4 & Image::gradient( const openMVG::Vec2i & pos ) const
{
  return gradient( pos[0] , pos[1] ) ;
}

/**
* @brief Indicate if a position is in the image
* @param id_row Index of the row
* @param id_col Index of the column
* @retval true if pixel is in the image
* @retval false if pixel is outside the image
*/
bool Image::inside( const int id_row , const int id_col ) const
{
  return m_grayscale.Contains( id_row , id_col ) ;
}

/**
* @brief Indicate if a position is in the image
* @param pos Position ( id_y , id_x )
* @retval true if position is inside the image
* @retval false if position is outside the image
*/
bool Image::inside( const openMVG::Vec2i & pos ) const
{
  return inside( pos[0] , pos[1] ) ;
}

/**
* @brief Get width of the image
* @return width of the image
*/
unsigned long Image::width( void ) const
{
  return m_grayscale.Width() ;
}

/**
* @brief Get height of the image
* @return Height of the image
*/
unsigned long Image::height( void ) const
{
  return m_grayscale.Height() ;
}

/**
 * @brief Save each files in the corresponding path
 * @param grayscale_path Path for the grayscale image
 * @param gradient_path Path for the gradient image
 * @retval true If succes
 * @retval false If failure
 */
bool Image::save( const std::string & color_path ,
                  const std::string & grayscale_path ,
                  const std::string & gradient_path ,
                  const std::string & census_path ,
                  const ImageLoadType & load ) const
{
  if( ( load & IMAGE_COLOR ) != 0 )
  {
    std::ofstream out_color( color_path , std::ios::binary ) ;
    if( ! out_color )
    {
      return false ;
    }

    cereal::PortableBinaryOutputArchive ar_color( out_color ) ;

    try
    {
      ar_color( m_color ) ;
    }
    catch( ... )
    {
      std::cerr << "Error while trying to serialize-out the color image" << std::endl ;
      return false ;
    }
  }

  // Grayscale
  if( ( load & IMAGE_GRAYSCALE ) != 0 )
  {
    std::ofstream out_gray( grayscale_path , std::ios::binary ) ;
    if( ! out_gray )
    {
      return false ;
    }

    cereal::PortableBinaryOutputArchive ar_gray( out_gray ) ;

    try
    {
      ar_gray( m_grayscale ) ;
    }
    catch( ... )
    {
      std::cerr << "Error while trying to serialize-out the grayscale image" << std::endl ;
      return false ;
    }
  }

  // Gradient
  if( ( load & IMAGE_GRADIENT ) != 0 )
  {
    std::ofstream out_grad( gradient_path , std::ios::binary ) ;
    if( ! out_grad )
    {
      return false ;
    }

    cereal::PortableBinaryOutputArchive ar_grad( out_grad ) ;

    try
    {
      ar_grad( m_gradient ) ;
    }
    catch( ... )
    {
      std::cerr << "Error while trying to serialize-out the gradient image" << std::endl ;
      return false ;
    }
  }

  // Census
  if( ( load & IMAGE_CENSUS ) != 0 )
  {
    std::ofstream out_census( census_path , std::ios::binary ) ;
    if( ! out_census )
    {
      return false ;
    }

    cereal::PortableBinaryOutputArchive ar_census( out_census ) ;

    try
    {
      ar_census( m_census ) ;
    }
    catch( ... )
    {
      std::cerr << "Error while trying to serialize-out the census image" << std::endl ;
      return false ;
    }
  }

  return true ;
}

/**
 * @brief Load each files with the corresponding path
 * @param grayscale_path Path for the grayscale image
 * @param gradient_path Path for the gradient image
 * @retval true If success
 * @retval false If failure
 */
bool Image::load( const std::string & color_path ,
                  const std::string & grayscale_path ,
                  const std::string & gradient_path ,
                  const std::string & census_path ,
                  const ImageLoadType & load )
{
  // Color
  if( ( load & IMAGE_COLOR ) != 0 )
  {
    std::ifstream in_color( color_path , std::ios::binary ) ;
    if( ! in_color )
    {
      std::cerr << "Could not open : " << color_path << std::endl ;
      return false ;
    }

    cereal::PortableBinaryInputArchive ar_color( in_color ) ;

    try
    {
      ar_color( m_color ) ;
    }
    catch( ... )
    {
      std::cerr << "Error while trying to serialize-in color image" << std::endl ;
      return false ;
    }
  }

  // Grayscale
  if( ( load & IMAGE_GRAYSCALE ) != 0 )
  {
    std::ifstream in_gray( grayscale_path , std::ios::binary ) ;
    if( ! in_gray )
    {
      std::cerr << "Could not open : " << grayscale_path << std::endl ;
      return false ;
    }

    cereal::PortableBinaryInputArchive ar_gray( in_gray ) ;

    try
    {
      ar_gray( m_grayscale ) ;
    }
    catch( ... )
    {
      std::cerr << "Error while trying to serialize-in grayscale image" << std::endl ;
      return false ;
    }
  }

  // Gradient
  if( ( load & IMAGE_GRADIENT ) != 0 )
  {
    std::ifstream in_grad( gradient_path , std::ios::binary ) ;
    if( ! in_grad )
    {
      std::cerr << "Could not open : '" << gradient_path << "'" << std::endl ;
      return false ;
    }

    cereal::PortableBinaryInputArchive ar_grad( in_grad ) ;

    try
    {
      ar_grad( m_gradient ) ;
    }
    catch( ... )
    {
      std::cerr << "Error while trying to serialize-in gradient image" << std::endl ;
      return false ;
    }
  }

  // Census
  if( ( load & IMAGE_CENSUS ) != 0 )
  {
    std::ifstream in_census( census_path , std::ios::binary ) ;
    if( ! in_census )
    {
      std::cerr << "Could not open : " << census_path << std::endl ;
      return false ;
    }

    cereal::PortableBinaryInputArchive ar_census( in_census ) ;

    try
    {
      ar_census( m_census ) ;
    }
    catch( ... )
    {
      std::cerr << "Error while trying to serialize-in census image" << std::endl ;
      return false ;
    }
  }

  return true ;
}

/**
 * @brief Used for std::map usage
 */
bool operator<( const Image & imgA , const Image & imgB )
{
  if( imgA.m_color.data() < imgB.m_color.data() )
  {
    return true ;
  }
  else if( imgA.m_color.data() > imgB.m_color.data() )
  {
    return false ;
  }
  else
  {
    // m_color == m_color
    if( imgA.m_grayscale.data() < imgB.m_grayscale.data() )
    {
      return true ;
    }
    else if( imgA.m_grayscale.data() > imgB.m_grayscale.data() )
    {
      return false ;
    }
    else
    {
      // m_color == m_color && m_grayscale == m_grayscale
      if( imgA.m_gradient.data() < imgB.m_gradient.data() )
      {
        return true ;
      }
      else if( imgA.m_gradient.data() > imgB.m_gradient.data() )
      {
        return false ;
      }
      else
      {
        // m_color == m_color && m_grayscale == m_grayscale && m_gradient == m_gradient
        return imgA.m_census.data() < imgB.m_census.data() ;
      }
    }
  }
}


const openMVG::image::Image<unsigned char> & Image::intensity( void ) const
{
  return m_grayscale ;
}

const openMVG::image::Image<openMVG::Vec4> & Image::gradient( void ) const
{
  return m_gradient ;
}

/**
* @brief Get Census image
*/
const openMVG::image::Image<uint64_t> & Image::census( void ) const
{
  return m_census ;
}

/**
* @brief Given a camera, load it's neighboring images
* @param reference_cam The reference camera
* @param params The computation parameters
* @return a vector of neighboring images
*/
std::vector< Image > LoadNeighborImages( const Camera & reference_cam ,
    const DepthMapComputationParameters & params ,
    const ImageLoadType & load )
{
  std::vector< Image > neigh_imgs ;
  for( size_t id_neigh = 0 ; id_neigh < reference_cam.m_view_neighbors.size() ; ++id_neigh )
  {
    const int real_id = reference_cam.m_view_neighbors[ id_neigh ] ;
    const std::string camera_path    = params.getCameraDirectory( real_id ) ;
    const std::string color_path     = params.getColorPath( real_id ) ;
    const std::string grayscale_path = params.getGrayscalePath( real_id ) ;
    const std::string gradient_path  = params.getGradientPath( real_id ) ;
    const std::string census_path    = params.getCensusPath( real_id ) ;

    neigh_imgs.emplace_back( Image( color_path , grayscale_path , gradient_path , census_path , load ) ) ;
  }

  return neigh_imgs ;
}

/**
 * @brief Load neighbor images at a specific scale
 * @param reference_cam Reference camera
 * @param all_cams All cameras
 * @param params The computation parameters
 * @param scale Scale of the requested images
 * @return a vector of neighboring images
 */
std::vector< Image > LoadNeighborImages( const Camera & reference_cam ,
    const std::vector< Camera > & all_cams ,
    const DepthMapComputationParameters & params ,
    const int scale ,
    const ImageLoadType & load )
{
  std::vector< Image > neigh_imgs ;
  for( size_t id_neigh = 0 ; id_neigh < reference_cam.m_view_neighbors.size() ; ++id_neigh )
  {
    const int real_id = reference_cam.m_view_neighbors[ id_neigh ] ;
    const Camera & neigh_cam = all_cams[ real_id ] ;
    const std::string img_path = neigh_cam.m_img_path ;

    // Load image and convert at specific scale
    neigh_imgs.emplace_back( Image( img_path , scale , neigh_cam.m_intrinsic , load ) ) ;
  }

  return neigh_imgs ;
}

#define MEAN_CENSUS_TRANSFORM 1

/**
* @brief Compute Census transform
*/
void Image::computeCensus( void )
{
  m_census.resize( m_grayscale.Width() , m_grayscale.Height() , true , 0ul ) ;

  const int half_h = 3 ;
  const int half_w = 4 ;

  // { y , x } -> { y , x }
  const int star_pattern_idx[24][2][2] =
  {
    { { -2 , -2 } , { -2 , 0 } } ,
    { { -2 , -1 } , { -1 , 1 } } ,
    { { -2 , 0 } , { -1 , -2 } } ,
    { { -2 , 1 } , { -2 , -1 } } ,
    { { -2 , 2 } , { 0 , 2 } } ,

    { { -1 , -2 } , { 1 , -2 } } ,
    { { -1 , -1 } , { 0 , 1 } } ,
    { { -1 , 0 } , { -2 , -2 } } ,
    { { -1 , 1 } , { 1 , 0 } } ,
    { { -1 , 2 } , { 1 , 1 } } ,

    { { 0 , -2 } , { 2 , 1 } } ,
    { { 0 , -1 } , { 2 , -2 } } ,
    { { 0 , 1 } , { -2 , 2 } } ,
    { { 0 , 2 } , { -2 , 1 } } ,

    { { 1 , -2 } , { -1 , -1 } } ,
    { { 1 , -1 } , { -1 , 0 } } ,
    { { 1 , 0 } , { 2 , 2 } } ,
    { { 1 , 1 } , { 0 , -1 } } ,
    { { 1 , 2 } , { -1 , 2 } } ,

    { { 2 , -2 } , { 0 , -2 } } ,
    { { 2 , -1 } , { 2 , 1 } } ,
    { { 2 , 0 } , { 1 , 2 } } ,
    { { 2 , 1 } , { 1 , -1 } } ,
    { { 2 , 2 } , { 2 , 0 } }
  } ;

  for( int id_row = half_h ; id_row < m_gradient.Height() - half_h ; ++id_row )
  {
    for( int id_col = half_w ; id_col < m_gradient.Width() - half_w ; ++id_col )
    {
      // Compute mean inside 9x7 window
#ifdef MEAN_CENSUS_TRANSFORM
      int sum = 0 ;
      int nb = 0 ;
      for( int y = id_row - half_h ; y <= id_row + half_h ; ++y )
      {
        for( int x = id_col - half_w ; x <= id_col + half_w ; ++x )
        {
          sum += m_grayscale( y , x ) ;
          ++nb ;
        }
      }
      const unsigned char c = static_cast<unsigned char>( sum / nb ) ;
#elif defined CENSUS_TRANSFORM
      const unsigned char c = m_grayscale( id_row , id_col ) ;
#elif defined STAR_CENSUS_TRANSFORM

#endif

      uint64_t  census = 0 ;

#ifdef STAR_CENSUS_TRANSFORM
      // Ref : https://www.spiedigitallibrary.org/journals/Optical-Engineering/volume-55/issue-06/063107/Improved-census-transform-for-noise-robust-stereo-matching/10.1117/1.OE.55.6.063107.full?SSO=1
      // use 24-2-1 pattern
      for( int sample = 0 ; sample < 24 ; ++sample )
      {
        const int ipy = id_row + star_pattern_idx[ sample ][ 0 ][ 0 ] ;
        const int ipx = id_col + star_pattern_idx[ sample ][ 0 ][ 1 ] ;

        const int iqy = id_row + star_pattern_idx[ sample ][ 1 ][ 0 ] ;
        const int iqx = id_col + star_pattern_idx[ sample ][ 1 ][ 1 ] ;

        const unsigned char Ip = m_grayscale( ipy , ipy ) ;
        const unsigned char Iq = m_grayscale( iqy , iqx ) ;

        const int val = Ip < Iq ;

        census <<= 1 ;
        census |= val & 0x1 ;
      }

#else
      // Standard census transform and mean census
      // 9x7 window
      for( int y = id_row - half_h ; y <= id_row + half_h ; ++y )
      {
        for( int x = id_col - half_w ; x <= id_col + half_w ; ++x )
        {
          if( ! ( x == id_col && y == id_row ) )
          {
            const int val = m_grayscale( y , x ) < c ;

            census <<= 1 ;
            census |= val & 0x1 ;
          }
        }
      }
#endif
      m_census( id_row , id_col ) = census ;
    }
  }
}

/**
* @brief Compute Gradient value
*/
void Image::computeGradient( void )
{
  openMVG::image::Image< double > in_d ;
  in_d = m_grayscale.GetMat().cast<double>() ;
  openMVG::image::Image< double > Dx , Dy ;

  openMVG::image::ImageScharrXDerivative( in_d , Dx ) ;
  openMVG::image::ImageScharrYDerivative( in_d , Dy ) ;

  m_gradient.resize( m_grayscale.Width() , m_grayscale.Height() , true , openMVG::Vec4( 0 , 0 , 0 , 0 ) ) ;
  for( int y = 0 ; y < m_gradient.Height() ; ++y )
  {
    for( int x = 0 ; x < m_gradient.Width() ; ++x )
    {
      // TODO : compute dxy and dyx
      openMVG::Vec4 grad( Dx( y , x ) , Dy( y , x ) , 0.0 , 0.0 ) ;
      m_gradient( y , x ) = grad ;
    }
  }
}


} // namespace MVS