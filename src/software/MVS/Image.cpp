#include "Image.hpp"

#include "openMVG/cameras/Camera_undistort_image.hpp"

#include "openMVG/image/image_io.hpp"
#include "openMVG/image/image_filtering.hpp"
#include "openMVG/image/image_resampling.hpp"

#include "openMVG/numeric/numeric_io_cereal.hpp"
#include "openMVG/image/pixel_types_io_cereal.hpp"

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
    ComputeGradient() ;
  }
  if( ( load & IMAGE_CENSUS ) != 0 )
  {
    ComputeCensus() ;
  }
}

/**
* @brief Load image using both parts
* @param gray_image_path Path of the image
* @param gradient_image_path Path of the image
*/
Image::Image( const std::string & color_image_path , const std::string & gray_image_path , const std::string & gradient_image_path , const std::string & census_path , const ImageLoadType & load )
{
  if( ! Load( color_image_path , gray_image_path , gradient_image_path , census_path , load ) )
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
unsigned char Image::Intensity( const int id_row , const int id_col ) const
{
  return m_grayscale.coeffRef( id_row , id_col ) ;
}

/**
* @brief Get intensity at specified position
* @param pos Position ( id_y , id_x )
* @return intensity at specified position
*/
unsigned char Image::Intensity( const openMVG::Vec2i & pos ) const
{
  return Intensity( pos[0] , pos[1] ) ;
}


/**
* @brief Get census bitstring at specified position
* @param id_row Index of the row
* @param id_col Index of the column
* @return census bitstring for corresponding pixel
*/
unsigned long long Image::Census( const int id_row , const int id_col ) const
{
  return m_census( id_row , id_col ) ;
}

/**
* @brief Get gradient magnitude at specified position
* @param id_row Index of the row
* @param id_col Index of the column
* @return Gradient magnitude at specified position
*/
const openMVG::Vec4 & Image::Gradient( const int id_row , const int id_col ) const
{
  return m_gradient.coeffRef( id_row , id_col ) ;
}

/**
* @brief Get gradient magnitude at specified position
* @param pos Position ( id_y , id_x )
* @return intensity at specified position
*/
const openMVG::Vec4 & Image::Gradient( const openMVG::Vec2i & pos ) const
{
  return Gradient( pos[0] , pos[1] ) ;
}

/**
* @brief Indicate if a position is in the image
* @param id_row Index of the row
* @param id_col Index of the column
* @retval true if pixel is in the image
* @retval false if pixel is outside the image
*/
bool Image::Inside( const int id_row , const int id_col ) const
{
  return m_grayscale.Contains( id_row , id_col ) ;
}

/**
* @brief Indicate if a position is in the image
* @param pos Position ( id_y , id_x )
* @retval true if position is inside the image
* @retval false if position is outside the image
*/
bool Image::Inside( const openMVG::Vec2i & pos ) const
{
  return Inside( pos[0] , pos[1] ) ;
}

/**
* @brief Get width of the image
* @return width of the image
*/
unsigned long Image::Width( void ) const
{
  return m_grayscale.Width() ;
}

/**
* @brief Get height of the image
* @return Height of the image
*/
unsigned long Image::Height( void ) const
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
bool Image::Save( const std::string & color_path ,
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
bool Image::Load( const std::string & color_path ,
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

const openMVG::image::Image<unsigned char> & Image::Intensity( void ) const
{
  return m_grayscale ;
}

const openMVG::image::Image<openMVG::Vec4> & Image::Gradient( void ) const
{
  return m_gradient ;
}

/**
* @brief Get Census image
*/
const openMVG::image::Image<unsigned long long> & Image::Census( void ) const
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
    const std::string camera_path    = params.GetCameraDirectory( real_id ) ;
    const std::string color_path     = params.GetColorPath( real_id ) ;
    const std::string grayscale_path = params.GetGrayscalePath( real_id ) ;
    const std::string gradient_path  = params.GetGradientPath( real_id ) ;
    const std::string census_path    = params.GetCensusPath( real_id ) ;

    neigh_imgs.push_back( Image( color_path , grayscale_path , gradient_path , census_path , load ) ) ;
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
    neigh_imgs.push_back( Image( img_path , scale , neigh_cam.m_intrinsic , load ) ) ;
  }

  return neigh_imgs ;
}

/**
* @brief Compute Census transform
*/
void Image::ComputeCensus( void )
{
  m_census.resize( m_grayscale.Width() , m_grayscale.Height() , true , 0ul ) ;

  for( int id_row = 0 ; id_row < m_gradient.Height() ; ++id_row )
  {
    for( int id_col = 0 ; id_col < m_gradient.Width() ; ++id_col )
    {

      // 9x7 window
      const unsigned char c = m_grayscale( id_row , id_col ) ;
      unsigned long long  census = 0 ;
      for( int y = id_row - 4 ; y <= id_row + 4 ; ++y )
      {
        for( int x = id_col - 3 ; x <= id_col + 3 ; ++x )
        {
          if( ! ( x == id_col && y == id_row ) )
          {
            const int val = m_grayscale( y , x ) < c ;

            census <<= 1 ;
            census |= val & 0x1 ;
          }
        }
      }
      m_census( id_row , id_col ) = census ;
    }
  }
}

/**
* @brief Compute Gradient value
*/
void Image::ComputeGradient( void )
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


}