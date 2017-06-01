#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_SHADER_PROGRAM_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_SHADER_PROGRAM_HH_

#include "Shader.hh"

#include "openMVG/numeric/numeric.h"

#include <map>

namespace openMVG_gui
{

/**
* @brief Class holding a full shader program
*/
class ShaderProgram
{
  public:

    /**
    * @brief Ctr
    * @param vertexPath Path of the vertex shader file
    * @param fragmentPath Path of the fragment shader file
    */
    ShaderProgram( const std::string & vertexPath , const std::string fragmentPath ) ;


    /**
    * @brief dtr
    */
    ~ShaderProgram( void ) ;

    /**
    * @brief Indicate if shader program is valid (shaders are ok and pgm linked with success)
    * @retval true if ok
    * @retval false if something goes wrong
    */
    operator bool() const ;

    /**
    * @brief Enable this shader
    */
    void enable( void ) ;

    /**
    * @brief Disable this shader
    */
    void disable( void ) ;

    /**
    * @brief Get id of the shader program
    * @retval 0 if shader program is invalid
    * @return id of the program
    */
    GLuint getID( void ) const ;

    /**
    * @brief Set uniform for 1-variable
    * @param name Name of the uniform
    * @param x value to pass
    */
    void setUniform( const std::string & name , const int x ) ;

    /**
    * @brief Set uniform for 1-variable
    * @param loc Location of the uniform
    * @param x value to pass
    */
    void setUniform( const GLint loc , const int x ) ;

    /**
    * @brief Set uniform for 1-variable
    * @param name Name of the uniform
    * @param x value to pass
    */
    void setUniform( const std::string & name , const float x ) ;

    /**
    * @brief Set uniform for 1-variable
    * @param loc Location of the uniform
    * @param x value to pass
    */
    void setUniform( const GLint loc , const float x ) ;

    /**
    * @brief Set uniform for 2-variable
    * @param name Name of the uniform
    * @param x value to pass
    * @param y value to pass
    */
    void setUniform( const std::string & name , const int x , const int y ) ;

    /**
    * @brief Set uniform for 2-variable
    * @param loc Location of the uniform
    * @param x value to pass
    * @param y value to pass
    */
    void setUniform( const GLint loc , const int x , const int y ) ;

    /**
    * @brief Set uniform for 2-variable
    * @param name Name of the uniform
    * @param x value to pass
    * @param y value to pass
    */
    void setUniform( const std::string & name , const float x , const float y ) ;

    /**
    * @brief Set uniform for 2-variable
    * @param loc Location of the uniform
    * @param x value to pass
    * @param y value to pass
    */
    void setUniform( const GLint loc , const float x , const float y ) ;

    /**
    * @brief Set uniform for 2-vector
    * @param name Name of the uniform
    * @param v value to pass
    */
    void setUniform( const std::string & name , const openMVG::Vec2 & v ) ;

    /**
    * @brief Set uniform for 2-vector
    * @param loc Location of the uniform
    * @param v value to pass
    */
    void setUniform( const GLint loc , const openMVG::Vec2 & v ) ;

    /**
    * @brief Set uniform for 3-variable
    * @param name Name of the uniform
    * @param x value to pass
    * @param y value to pass
    * @param z value to pass
    */
    void setUniform( const std::string & name , const int x , const int y , const int z ) ;

    /**
    * @brief Set uniform for 3-variable
    * @param loc Location of the variable
    * @param x value to pass
    * @param y value to pass
    * @param z value to pass
    */
    void setUniform( const GLint loc , const int x , const int y , const int z ) ;

    /**
    * @brief Set uniform for 3-variable
    * @param name Name of the uniform
    * @param x value to pass
    * @param y value to pass
    * @param z value to pass
    */
    void setUniform( const std::string & name , const float x , const float y , const float z ) ;

    /**
    * @brief Set uniform for 3-variable
    * @param loc Location of the variable
    * @param x value to pass
    * @param y value to pass
    * @param z value to pass
    */
    void setUniform( const GLint loc , const float x , const float y , const float z ) ;

    /**
    * @brief Set uniform for 3-vector
    * @param name Name of the uniform
    * @param v Value to pass
    */
    void setUniform( const std::string & name , const openMVG::Vec3 & v ) ;

    /**
    * @brief Set uniform for 3-vector
    * @param loc Location of the vector
    * @param v Value to pass
    */
    void setUniform( const GLint loc , const openMVG::Vec3 & v ) ;

    /**
    * @brief Set uniform for 4-variable
    * @param name Name of the uniform
    * @param x value to pass
    * @param y value to pass
    * @param z value to pass
    * @param w value to pass
    */
    void setUniform( const std::string & name , const int x , const int y , const int z , const int w ) ;

    /**
    * @brief Set uniform for 4-variable
    * @param loc Location of the uniform
    * @param x value to pass
    * @param y value to pass
    * @param z value to pass
    * @param w value to pass
    */
    void setUniform( const GLint loc , const int x , const int y , const int z , const int w ) ;

    /**
    * @brief Set uniform for 4-variable
    * @param name Name of the uniform
    * @param x value to pass
    * @param y value to pass
    * @param z value to pass
    * @param w value to pass
    */
    void setUniform( const std::string & name , const float x , const float y , const float z , const float w ) ;

    /**
    * @brief Set uniform for 4-variable
    * @param loc Location of the uniform
    * @param x value to pass
    * @param y value to pass
    * @param z value to pass
    * @param w value to pass
    */
    void setUniform( const GLint loc , const float x , const float y , const float z , const float w ) ;

    /**
    * @brief Set uniform for 4-vector
    * @param name Name of the uniform
    * @param v value to pass
    */
    void setUniform( const std::string & name , const openMVG::Vec4 & v  ) ;

    /**
    * @brief Set uniform for 4-vector
    * @param loc Location of the uniform
    * @param v Value to pass
    */
    void setUniform( const GLint loc , const openMVG::Vec4 & v  ) ;

    /**
    * @brief set uniform for a 4x4 matrix
    * @param name Name of the uniform
    * @param m Matrix to pass
    */
    void setUniform( const std::string & name , const openMVG::Mat4 & m , bool transpose = false ) ;

    /**
    * @brief set uniform for a 4x4 matrix
    * @param loc Location of the uniform
    * @param m Matrix to pass
    */
    void setUniform( const GLint loc , const openMVG::Mat4 & m , bool transpose = false ) ;

    /**
    * @brief get attribute location
    * @param name Attribute name
    * @retval -1 if attribute is not present, not active or invalid name
    * @return location of active attribute if valid
    */
    GLint attribLocation( const std::string & name ) const ;

    /**
    * @brief get uniform location
    * @param name Uniform name
    * @retval -1 if uniform is not present, not active or invalid name
    * @return location of active uniform if valid
    */
    GLint uniformLocation( const std::string & name ) const ;

    /**
    * @brief Indicate if a given uniform name is present in the shader 
    * @param name Name of the uniform to test 
    * @retval true if present 
    * @retval false if absent 
    */
    bool hasUniform( const std::string & name ) const ; 

    /**
    * @brief Indicate if a given attribute name is present in the shader 
    * @param name Name of the attribute to test 
    * @retval true if present 
    * @retval false if absent 
    */
    bool hasAttribute( const std::string & name ) const ; 


  private:

    /**
    * @brief Link and get error
    */
    void linkProgram( void ) ;

    /**
    * @brief get list of active uniforms and attributes
    */
    void populateActiveUniformsAndAtttributes( void ) ;



    Shader m_vert_shader ;
    Shader m_frag_shader ;

    std::map< std::string , GLint > m_active_uniform_loc ;
    std::map< std::string , GLint > m_active_attribute_loc ;

    GLuint m_pgm ;
} ;

} // namespace openMVG_gui

#endif