#include "Shader.hh"

#include "UtilString.hh"

#include <iostream>

namespace openMVG_gui
{
/**
* @brief Load a shader from a file
* @param shaderPath Path
* @param type shader type
*/
Shader::Shader( const std::string & shaderPath , const ShaderType type )
  : m_valid( false ) ,
    m_type( type ) ,
    m_id( 0 )
{
  loadAndCompile( shaderPath ) ;
}

/**
* @brief dtr
*/
Shader::~Shader( )
{
  if( m_valid )
  {
    glDeleteShader( m_id );
  }
}


/**
* @brief get ID of the shader
* @return Id of the shader
*/
GLuint Shader::getID( void )
{
  return m_id ;
}

/**
* @brief get current type of the shader
* @return type of the shader
*/
ShaderType Shader::getType( void )
{
  return m_type ;
}

/**
* @brief Test if shader is valid
* @retval true if valid
* @retval false if invalid
*/
Shader::operator bool() const
{
  return m_valid ;
}

void Shader::loadAndCompile( const std::string & filePath )
{
  m_id = 0 ;
  switch( m_type )
  {
    case SHADER_TYPE_VERTEX :
    {
      m_id = glCreateShader( GL_VERTEX_SHADER ) ;
      break ;
    }
    case SHADER_TYPE_FRAGMENT :
    {
      m_id = glCreateShader( GL_FRAGMENT_SHADER ) ;
      break ;
    }
    case SHADER_TYPE_GEOMETRY :
    {
      m_id = glCreateShader( GL_GEOMETRY_SHADER );
      break ;
    }
  }
  if( ! m_id )
  {
    std::cerr << "Error creating shader" << std::endl ;
    m_valid = false ;
    return ;
    // TODO throw something ?
  }

  // Compile shader
  const std::string content = fileContent( filePath ) ;
  GLint len = content.size() ;
  const char * c_content = content.c_str() ;
  glShaderSource( m_id , 1 , &c_content , &len ) ;
  glCompileShader( m_id ) ;

  // Get error code
  GLint ok ;
  glGetShaderiv( m_id , GL_COMPILE_STATUS, &ok );
  if( ok )
  {
    m_valid = true ;
    return ;
  }
  else
  {
    // Get error message
    GLint log_len = 0;
    glGetShaderiv( m_id , GL_INFO_LOG_LENGTH , &log_len );

    GLchar* log = new GLchar[ log_len ] ;

    glGetShaderInfoLog( m_id , log_len , nullptr , log ) ;

    std::cerr << "Compile log : " << log << std::endl ;

    delete[] log ;
  }
}


} // openMVG_gui
