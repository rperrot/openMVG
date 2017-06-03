#ifndef _CAMERA_GIZMO_HH_
#define _CAMERA_GIZMO_HH_

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
                 const double size = 1.0 ) ;

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

  private:

    openMVG::geometry::Pose3 m_pose ;
    double m_size ;

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