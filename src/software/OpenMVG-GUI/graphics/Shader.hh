#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_SHADER_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_SHADER_HH_


#include <QOpenGLFunctions>

#include <string>

namespace openMVG_gui
{

/**
* @brief Kind of shader to used
*/
enum ShaderType
{
  SHADER_TYPE_VERTEX ,
  SHADER_TYPE_FRAGMENT ,
  SHADER_TYPE_GEOMETRY
} ;

/**
* @brief Class managing a shader (vertex, fragment, geometry, ...) in openGL
*/
class Shader 
{
  public:

    /**
    * @brief Load a shader from a file
    * @param shaderPath Path
    * @param type shader type
    */
    Shader( const std::string & shaderPath , const ShaderType type ) ;

    /**
    * @brief dtr
    */
    ~Shader( ) ;

    /**
    * @brief get ID of the shader
    * @return Id of the shader
    */
    GLuint getID( void ) ;

    /**
    * @brief get current type of the shader
    * @return type of the shader
    */
    ShaderType getType( void ) ;

    /**
    * @brief Test if shader is valid
    * @retval true if valid
    * @retval false if invalid
    */
    operator bool() const ;

  private:

    void loadAndCompile( const std::string & filePath ) ;

    bool m_valid ;

    ShaderType m_type ;

    // OpenGL ID
    GLuint m_id ;
} ;

} // openMVG_gui

#endif