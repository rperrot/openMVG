#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_OBJECTS_GRID_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_OBJECTS_GRID_HH_

#include "RenderableObject.hh"

namespace openMVG_gui
{

/**
* @brief class managing grid in background of the 3d view
* grid is a 2d plane in z = 0
* grid is given by a number of x_lines, y_lines
*/
class Grid : public RenderableObject
{
  public:

    /**
    * @brief Ctr
    * @param nb_element in x
    * @param nb_element in y
    * @note the function ensure nb_x and nb_y are odd
    * @note colors are in range [0;1]^3
    */
    Grid( std::shared_ptr<ShaderProgram> pgm ,
          const int nb_x = 151 ,
          const int nb_y = 151 ,
          const openMVG::Vec3 col_axis_x = openMVG::Vec3( 1.0 , 0.0 , 0.0 ) ,
          const openMVG::Vec3 col_axis_y = openMVG::Vec3( 0.0 , 1.0 , 0.0 ) ,
          const openMVG::Vec3 col_standard_lines = openMVG::Vec3( 0.75 , 0.75 , 0.75 ) ,
          const openMVG::Vec3 col_major_lines    = openMVG::Vec3( 0.40 , 0.40 , 0.40 ) ) ;

    ~Grid();

    /**
    * @brief Prepare object before rendering (ie: create buffers, prepare data)
    * @note openGL context must be active when calling this function
    */
    void prepare( void ) override ;

    /**
    * @brief Draw code for the object
    */
    void draw( void ) const override;

  private:

    // CPU
    int m_nb_x ;
    int m_nb_y ;
    openMVG::Vec3 m_color_x ;
    openMVG::Vec3 m_color_y ;
    openMVG::Vec3 m_color_standard ;
    openMVG::Vec3 m_color_major ;

    // OpenGL
    GLsizei m_nb_vert ;
    GLuint m_vao ;
    GLuint m_vbo ;

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} ;

} // namespace openMVG_gui

#endif