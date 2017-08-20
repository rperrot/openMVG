#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_CAMERA_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_CAMERA_HH_

#include "openMVG/numeric/numeric.h"

namespace openMVG_gui
{

/**
* @brief Class holding a camera in openGL
* @note it's a projective camera
*/
class Camera
{
  public:

    /**
    * @brief Ctr
    * @param pos Position of the camera
    * @param dest Destination point of the camera
    * @param up Up Vector
    * @param fov Field Of View (in radian)
    * @param near_plane Near plane
    * @param far_plane Far plane
    * @param ortho Indicate if it's an orthographic camera
    */
    Camera( const openMVG::Vec3 & pos ,
            const openMVG::Vec3 & dest ,
            const openMVG::Vec3 & up ,
            const double fov ,
            const double near_plane ,
            const double far_plane ,
            const bool ortho = false ) ;

    /**
    * @brief Get camera position
    * @return current position of the camera
    */
    openMVG::Vec3 position( void ) const ;

    /**
    * @brief Set camera position
    * @param p New position
    */
    void setPosition( const openMVG::Vec3 & p ) ;

    /**
    * @brief Set camera destination
    * @return current camera destination point
    */
    openMVG::Vec3 destination( void ) const ;

    /**
    * @brief Set destination point
    * @param d New destination point
    */
    void setDestination( const openMVG::Vec3 & d ) ;

    /**
    * @brief Get up vector
    * @return Current up vector
    */
    openMVG::Vec3 up( void ) const ;

    /**
    * @brief Set up vector
    * @param u New up vector
    */
    void setUp( const openMVG::Vec3 & u ) ;

    /**
    * @brief get Near plane
    * @return Current near plane
    */
    double nearPlane( void ) const ;

    /**
    * @brief Set near plane
    * @param near New near plane
    */
    void setNearPlane( const double near ) ;

    /**
    * @brief Get Far plane
    * @return Current far plane
    */
    double farPlane( void ) const ;

    /**
    * @brief Set Far plane
    * @param far New far plane
    */
    void setFarPlane( const double far ) ;

    /**
    * @brief Get Field Of View
    * @return FOV
    * @TODO: precise if it's horizontal or vertical FOV
    * @note FOV is in radian
    */
    double fov( void ) const ;
    /**
    * @brief Set Field of View
    * @param fov New FOV
    * @note fov must be defined in radian
    */
    void setFov( const double fov ) ;

    /**
    * @brief Get view matrix for the current parameters
    * @return View matrix
    * @note It should be directly used in openGL (no transpose needed)
    */
    openMVG::Mat4 viewMatrix( void ) const ;

    /**
    * @brief Get projection matrix for the current parameters
    * @param aspect Aspect ratio (w/h)
    * @return Projection matrix
    * @note if m_is_ortho is true return an orthographic projection, if false return a perspective projection
    * @note It should be directly used in openGL (no transpose needed)
    */
    openMVG::Mat4 projMatrix( const double width , const double height ) const ;

    /**
    * @brief center camera to a specified position while keeping orientation frame
    * @param nCenter new center
    * @note this move position and destination
    * @note at final, dest is at new center and position is move to keep direction vector the same
    */
    void centerTo( const openMVG::Vec3 & nCenter ) ;

    /**
    * @brief Set camera to fit a specific sphere
    * @param center Center of the sphere
    * @param rad Radius of the sphere
    */
    void fitBoundingSphere( const openMVG::Vec3 & center , const double rad ) ;

    /**
    * @brief Zoom in (or out )
    * @param factor zooming factor
    */
    void zoom( const double factor ) ;

    /**
    * @brief Move dest and dir from this vector
    */
    void pan( const openMVG::Vec3 & delta ) ;

    /**
    * @brief Move position and update up vector accordingly
    */
    void rotateAroundDestination( const openMVG::Vec3 & axis , const double angle ) ;

    /**
    * @brief Given a screen position, gets it's corresponding 3d point
    * @param pt Point to transform (screen_x,screen_y,depth)
    * @param viewport [start_x,start_y,width,height]
    * @note this is similar to gluUnproject
    */
    openMVG::Vec3 unProject( const openMVG::Vec3 & pt , const double viewport[4] ) const ;


    /**
    * @brief Indicate if it's an orthographic camera
    * @retval true if it's ortho
    * @retval false if not
    */
    bool isOrtho( void ) const ;

    /**
    * @brief Set orthographic mode
    * @param iso orthographic activation mode
    */
    void setOrtho( const bool iso ) ;

    /**
    * @brief serialization load
    */
    template< class Archive>
    void load( Archive & ar ) ;

    /**
    * @brief serialization save
    */
    template< class Archive>
    void save( Archive & ar ) const ;

  private:

    /**
    * @brief Orthonormalize frame
    * ie: reset up vector to make an orthonormal frame
    */
    void orthonormalize( void ) ;

    openMVG::Vec3 m_pos ;
    openMVG::Vec3 m_dest ;
    openMVG::Vec3 m_up ;
    double m_fov ;
    double m_near ;
    double m_far ;

    bool m_is_ortho ;

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} ;

/**
* @brief serialization load
*/
template< class Archive >
void Camera::load( Archive & ar )
{
  ar( m_pos ) ;
  ar( m_dest ) ;
  ar( m_up ) ;
  ar( m_fov ) ;
  ar( m_near ) ;
  ar( m_far ) ;
}

/**
* @brief serialization save
*/
template< class Archive>
void Camera::save( Archive & ar ) const
{
  ar( m_pos ) ;
  ar( m_dest ) ;
  ar( m_up ) ;
  ar( m_fov ) ;
  ar( m_near ) ;
  ar( m_far ) ;
}

} // namespace openMVG_gui

#endif