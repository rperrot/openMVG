#ifndef _OPENMVG_MVS_IMAGE_HPP_
#define _OPENMVG_MVS_IMAGE_HPP_

#include "openMVG/image/image.hpp"

#include "Camera.hpp"

namespace MVS
{
  // Class holding a basic image used for MVS reconstruction
  // Contains intensity and gradient (gx,gy,gxy,gyx)
  class Image
  {
    public:

      /**
      * @brief load an augmented image from a file
      * @param path Path of the image to load
      * @param scale Scale of the image (0 -> Same size , 1 -> half the size , other -> 1/2^scale)
      */
      Image( const std::string & path , const int scale , const openMVG::cameras::IntrinsicBase * intrinsic ) ;

      /**
      * @brief Load image using both parts
      * @param gray_image_path Path of the image
      * @param gradient_image_path Path of the image
      */
      Image( const std::string & color_image_path , const std::string & gray_image_path , const std::string & gradient_image_path ) ;

      /**
      * @brief Copy ctr
      * @param src source
      */
      Image( const Image & src ) ;

      /**
      * @brief Move ctr
      * @param src source
      */
      Image( Image && src ) ;

      /**
      * @brief Assignement operator
      * @param src Source
      * @return Self after assignment
      */
      Image & operator=( const Image & src ) ;

      /**
      * @brief Move assignment operator
      * @param src Source
      * @return Self after assignment
      */
      Image & operator=( Image && src ) ;


      /**
      * @brief Get intensity at a specified position
      * @param id_row Index of the row
      * @param id_col Index of the column
      * @return intensity at specified position
      */
      unsigned char Intensity( const int id_row , const int id_col ) const ;

      /**
      * @brief Get intensity at specified position
      * @param pos Position ( id_y , id_x )
      * @return intensity at specified position
      */
      unsigned char Intensity( const openMVG::Vec2i & pos ) const ;

      /**
      * @brief Get reference to the Intensity image
      * @return Reference to the current intensity image
      */
      const openMVG::image::Image<unsigned char> & Intensity( void ) const ;

      /**
      * @brief Get reference to the gradient image
      * @return Reference to the current gradient image
      */
      const openMVG::image::Image<openMVG::Vec4> & Gradient( void ) const ;

      /**
      * @brief Get gradient magnitude at specified position
      * @param id_row Index of the row
      * @param id_col Index of the column
      * @return Gradient magnitude at specified position
      */
      const openMVG::Vec4 & Gradient( const int id_row , const int id_col ) const ;

      /**
      * @brief Get gradient magnitude (dx,dy,dxy,dyx) at specified position
      * @param pos Position ( id_y , id_x )
      * @return intensity at specified position
      */
      const openMVG::Vec4 & Gradient( const openMVG::Vec2i & pos ) const ;

      /**
      * @brief Indicate if a position is in the image
      * @param id_row Index of the row
      * @param id_col Index of the column
      * @retval true if pixel is in the image
      * @retval false if pixel is outside the image
      */
      bool Inside( const int id_row , const int id_col ) const ;

      /**
      * @brief Indicate if a position is in the image
      * @param pos Position ( id_y , id_x )
      * @retval true if position is inside the image
      * @retval false if position is outside the image
      */
      bool Inside( const openMVG::Vec2i & pos ) const ;

      /**
      * @brief Get width of the image
      * @return width of the image
      */
      unsigned long Width( void ) const ;

      /**
      * @brief Get height of the image
      * @return Height of the image
      */
      unsigned long Height( void ) const ;

      /**
      * @brief Save each files in the corresponding path
      * @param grayscale_path Path for the grayscale image
      * @param gradient_path Path for the gradient image
      * @retval true If success
      * @retval false If failure
      */
      bool Save( const std::string & color_path , const std::string & grayscale_path , const std::string & gradient_path ) const ;

      /**
      * @brief Load each files with the corresponding path
      * @param grayscale_path Path for the grayscale image
      * @param gradient_path Path for the gradient image
      * @retval true If success
      * @retval false If failure
      */
      bool Load( const std::string & color_path , const std::string & grayscale_path , const std::string & gradient_path ) ;

    private:

      // Color image
      openMVG::image::Image< openMVG::image::RGBColor > m_color ;
      // Grayscale intensity
      openMVG::image::Image< unsigned char > m_grayscale ;
      // Norm of the gradient
      openMVG::image::Image< openMVG::Vec4 > m_gradient ;
  } ;


  /**
  * @brief Given a camera, load it's neighboring images
  * @param reference_cam The reference camera
  * @param params The computation parameters
  * @return a vector of neighboring images
  */
  std::vector< Image > LoadNeighborImages( const Camera & reference_cam ,
      const DepthMapComputationParameters & params ) ;


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
      const int scale = -1 ) ;
}

#endif