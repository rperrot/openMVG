// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_OBJECTS_CAMERA_GIZMO_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_OBJECTS_CAMERA_GIZMO_HH_

#include "openMVG/geometry/pose3.hpp"

#include "RenderableObject.hh"
#include "SelectableObject.hh"

namespace openMVG_gui
{

/**
* @brief Class holding a simple camera gizmo
* @note Gizmo is a contained in a square of size x size x size
*/
class CameraGizmo : public RenderableObject , public SelectableObject
{
  public:

    /**
    * @brief Ctr
    * @param shad Shader to use for the rendering of the gizmo
    * @param pose Current pose corresponding to the camera
    * @param size Size of the gizmo
    */
    CameraGizmo( std::shared_ptr<ShaderProgram> shad ,
                 const openMVG::geometry::Pose3 & pose ,
                 const bool is_spherical = true ,
                 const double size = 1.0 ) ;

    /**
    * @brief Destructor
    */
    virtual ~CameraGizmo( void ) ;

    /**
    * @brief Get current size of the gizmo
    * @return current size of the gizmo
    */
    double size( void ) const ;

    /**
    * set size of the gizmo
    * @param size (size of the gizmo)
    */
    void setSize( const double size ) ;

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
    * @brief Get model matrix
    * @return current model matrix (in left hand system)
    */
    openMVG::Mat4 modelMat( void ) const override ;

    /**
    * @brief destroy all openGL data (if any present)
    */
    void destroyGLData( void ) override ;


  private:

    openMVG::geometry::Pose3 m_pose ;
    double m_size ;
    bool m_is_spherical ;

    // OpenGL
    GLsizei m_nb_vert ;
    GLuint m_vao ;
    GLuint m_vbo ;

    GLsizei m_nb_vert_selection ;
    GLuint m_vao_selection ;
    GLuint m_vbo_selection ;
} ;

} // namespace openMVG_gui

#endif