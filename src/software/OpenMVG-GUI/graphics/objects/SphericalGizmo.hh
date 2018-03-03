// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_SPHERICAL_GIZMO_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_SPHERICAL_GIZMO_HH_

#include "RenderableObject.hh"

namespace openMVG_gui
{

/**
* @brief class holding a 3d gizmo with 3 axis
*/
class SphericalGizmo: public RenderableObject
{
  public:

    /**
    * @brief Ctr
    * @param pgm Programm shader
    * @param center Center of the gizmo
    * @param radius Radius of the gizmo
    */
    SphericalGizmo( std::shared_ptr<OpenGLContext> ctx ,
                    std::shared_ptr<ShaderProgram> pgm ,
                    const openMVG::Vec3 & center ,
                    const double radius ) ;

    virtual ~SphericalGizmo( void ) ;

    /**
    * @brief Set center of the gizmo
    * @param nCenter New center
    */
    void setCenter( const openMVG::Vec3 & nCenter ) ;

    /**
    * @brief Get center of the gizmo
    * @return current center
    */
    openMVG::Vec3 center( void ) const ;

    /**
    * @brief Set radius of the gizmo
    * @param nRad
    */
    void setRadius( const double nRad ) ;

    /**
    * @brief Get radius of the gizmo
    * @return current radius
    */
    double radius( void ) const ;

    /**
    * @brief Get model matrix
    * @return current model matrix (in left hand system)
    */
    openMVG::Mat4 modelMat( void ) const override ;

    /**
    * @brief Prepare object before rendering (ie: create buffers, prepare data)
    * @note openGL context must be active when calling this function
    */
    void prepare( void ) override ;

    /**
    * @brief Draw code for the object
    */
    void draw( void ) const override;

    /**
    * @brief destroy all openGL data (if any present)
    */
    void destroyGLData( void ) override ;


  private:

    /**
    * @brief Build model matrix
    */
    void buildModelMat( void ) ;

    openMVG::Vec3 m_center ;
    double m_radius ;

    openMVG::Mat4 m_model ;

    // OpenGL
    GLsizei m_nb_vert ;
    GLuint m_vao ;
    GLuint m_vbo ;

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} ;

} // namespace openMVG_gui

#endif