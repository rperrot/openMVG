// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
    Camera( const openMVG::Vec3 & pos = openMVG::Vec3( 0.0 , -3.0 , 3.0 ),
            const openMVG::Vec3 & dest = openMVG::Vec3( 0.0 , 0.0 , 0.0 ) ,
            const openMVG::Vec3 & up = openMVG::Vec3( 0.0 , 0.0 , 1.0 ) ,
            const double fov = openMVG::D2R( 90 ) ,
            const double near_plane = 0.1 ,
            const double far_plane = 10000.0 ,
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

    /**
     * @brief Dump structure to a string (for debug only)
     * @return dump to string 
     */
    std::string dump( void ) const ; 

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
  ar( m_pos[0] , m_pos[1] , m_pos[2] ) ;
  ar( m_dest[0] , m_dest[1] , m_dest[2] ) ;
  ar( m_up[0] , m_up[1] , m_up[2] ) ;
  ar( m_fov ) ;
  ar( m_near ) ;
  ar( m_far ) ;
  ar( m_is_ortho ) ;
}

/**
* @brief serialization save
*/
template< class Archive>
void Camera::save( Archive & ar ) const
{
  ar( m_pos[0] , m_pos[1] , m_pos[2] ) ;
  ar( m_dest[0] , m_dest[1] , m_dest[2] ) ;
  ar( m_up[0] , m_up[1] , m_up[2] ) ;
  ar( m_fov ) ;
  ar( m_near ) ;
  ar( m_far ) ;
  ar( m_is_ortho ) ;
}

} // namespace openMVG_gui

#endif