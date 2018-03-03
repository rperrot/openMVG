// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017, 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_OBJECTS_GRID_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_OBJECTS_GRID_HH_

#include "RenderableObject.hh"

// fwrd declaration
namespace openMVG
{
namespace sfm
{
struct SfM_Data ;
} // namespace sfm
} // namespace openMVG

namespace openMVG_gui
{

/**
* @brief Class holding a point cloud
*/
class PointCloud : public RenderableObject
{
  public:

    /**
    * @brief Point cloud
    * @param pgm The shader
    * @param pts The point list
    * @param col The colors associated with the points (if no color provide an empty array)
    */
    PointCloud( std::shared_ptr<OpenGLContext> ctx ,
                std::shared_ptr<ShaderProgram> pgm ,
                const std::vector< openMVG::Vec3 > & pts ,
                const std::vector< openMVG::Vec3 > & col ,
                const openMVG::Vec3 defaultColor = openMVG::Vec3( 0.9 , 0.9 , 0.9 ) ) ;

    /**
     * @brief Ctr from SfM_Data
     * @param pgm Shader program
     * @param sfm_data The SfM Data file
     * @param defaultColor The color associated with the points (since sfm_data does not have color inside)
     */
    PointCloud( std::shared_ptr<OpenGLContext> ctx ,
                std::shared_ptr<ShaderProgram> pgm ,
                std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data ,
                const openMVG::Vec3 defaultColor = openMVG::Vec3( 0.9 , 0.9 , 0.9 ) ) ;

    /**
    * @brief Dtr
    */
    virtual ~PointCloud();

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
    std::vector< openMVG::Vec3 > m_pts ;
    std::vector< openMVG::Vec3 > m_col ;
    openMVG::Vec3 m_default_color ;

    // OpenGL
    GLsizei m_nb_vert ;
    GLuint m_vao ;
    GLuint m_vbo ;

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} ;

} // namespace openMVG_gui

#endif