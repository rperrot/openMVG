#include "ShaderProgram.hh"

#include <QOpenGLFunctions>


namespace openMVG_gui
{

/**
* @brief Ctr
* @param vertexPath Path of the vertex shader file
* @param fragmentPath Path of the fragment shader file
*/
ShaderProgram::ShaderProgram( const std::string & vertexPath , const std::string fragmentPath )
  : m_vert_shader( vertexPath , SHADER_TYPE_VERTEX ) ,
    m_frag_shader( fragmentPath , SHADER_TYPE_FRAGMENT ) ,
    m_pgm( 0 )
{
  linkProgram() ;
  enable() ;
  populateActiveUniformsAndAtttributes() ;
}


/**
* @brief dtr
*/
ShaderProgram::~ShaderProgram( void )
{
  if( m_pgm > 0 )
  {
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->glDeleteProgram( m_pgm ) ;
  }
}

/**
* @brief Indicate if shader program is valid (shaders are ok and pgm linked with success)
* @retval true if ok
* @retval false if something goes wrong
*/
ShaderProgram::operator bool() const
{
  return m_pgm > 0 ;
}

/**
* @brief Enable this shader
*/
void ShaderProgram::enable( void )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUseProgram( m_pgm ) ;
}

/**
* @brief Disable this shader
*/
void ShaderProgram::disable( void )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUseProgram( 0 ) ;
}

/**
* @brief Get id of the shader program
* @retval 0 if shader program is invalid
* @return id of the program
*/
GLuint ShaderProgram::getID( void ) const
{
  return m_pgm ;
}

/**
* @brief Set uniform for 1-variable
* @param name Name of the uniform
* @param x value to pass
*/
void ShaderProgram::setUniform( const std::string & name , const int x )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniform1i( loc , x ) ;
}

/**
* @brief Set uniform for 1-variable
* @param loc Location of the uniform
* @param x value to pass
*/
void ShaderProgram::setUniform( const GLint loc , const int x )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUniform1i( loc , x ) ;
}

/**
* @brief Set uniform for 1-variable
* @param name Name of the uniform
* @param x value to pass
*/
void ShaderProgram::setUniform( const std::string & name , const float x )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniform1f( loc , x ) ;
}

/**
* @brief Set uniform for 1-variable
* @param loc Location of the uniform
* @param x value to pass
*/
void ShaderProgram::setUniform( const GLint loc , const float x )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUniform1f( loc , x ) ;
}

/**
* @brief Set uniform for 2-variable
* @param name Name of the uniform
* @param x value to pass
* @param y value to pass
*/
void ShaderProgram::setUniform( const std::string & name , const int x , const int y )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniform2i( loc , x , y ) ;
}

/**
* @brief Set uniform for 2-variable
* @param loc Location of the uniform
* @param x value to pass
* @param y value to pass
*/
void ShaderProgram::setUniform( const GLint loc , const int x , const int y )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUniform2i( loc , x , y ) ;
}

/**
* @brief Set uniform for 2-variable
* @param name Name of the uniform
* @param x value to pass
* @param y value to pass
*/
void ShaderProgram::setUniform( const std::string & name , const float x , const float y )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniform2f( loc , x , y ) ;
}

/**
* @brief Set uniform for 2-variable
* @param loc Location of the uniform
* @param x value to pass
* @param y value to pass
*/
void ShaderProgram::setUniform( const GLint loc , const float x , const float y )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUniform2f( loc , x , y ) ;
}

/**
* @brief Set uniform for 2-vector
* @param name Name of the uniform
* @param v value to pass
*/
void ShaderProgram::setUniform( const std::string & name , const openMVG::Vec2 & v )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniform2f( loc , v[0] , v[1] ) ;
}

/**
* @brief Set uniform for 2-vector
* @param loc Location of the uniform
* @param v value to pass
*/
void ShaderProgram::setUniform( const GLint loc , const openMVG::Vec2 & v )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUniform2f( loc , v[0] , v[1] ) ;
}


/**
* @brief Set uniform for 3-variable
* @param name Name of the uniform
* @param x value to pass
* @param y value to pass
* @param z value to pass
*/
void ShaderProgram::setUniform( const std::string & name , const int x , const int y , const int z )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniform3i( loc , x , y , z ) ;
}

/**
* @brief Set uniform for 3-variable
* @param loc Location of the variable
* @param x value to pass
* @param y value to pass
* @param z value to pass
*/
void ShaderProgram::setUniform( const GLint loc , const int x , const int y , const int z )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUniform3i( loc , x , y , z ) ;
}

/**
* @brief Set uniform for 3-variable
* @param name Name of the uniform
* @param x value to pass
* @param y value to pass
* @param z value to pass
*/
void ShaderProgram::setUniform( const std::string & name , const float x , const float y , const float z )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniform3f( loc , x , y , z ) ;
}

/**
* @brief Set uniform for 3-variable
* @param loc Location of the variable
* @param x value to pass
* @param y value to pass
* @param z value to pass
*/
void ShaderProgram::setUniform( const GLint loc , const float x , const float y , const float z )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUniform3f( loc , x , y , z ) ;
}

/**
* @brief Set uniform for 3-vector
* @param name Name of the uniform
* @param v Value to pass
*/
void ShaderProgram::setUniform( const std::string & name , const openMVG::Vec3 & v )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniform3f( loc , v[0] , v[1] , v[2] ) ;
}

/**
* @brief Set uniform for 3-vector
* @param loc Location of the vector
* @param v Value to pass
*/
void ShaderProgram::setUniform( const GLint loc , const openMVG::Vec3 & v )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUniform3f( loc , v[0] , v[1] , v[2] ) ;
}

/**
* @brief Set uniform for 4-variable
* @param name Name of the uniform
* @param x value to pass
* @param y value to pass
* @param z value to pass
* @param w value to pass
*/
void ShaderProgram::setUniform( const std::string & name , const int x , const int y , const int z , const int w )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniform4i( loc , x , y , z , w ) ;
}

/**
* @brief Set uniform for 4-variable
* @param loc Location of the uniform
* @param x value to pass
* @param y value to pass
* @param z value to pass
* @param w value to pass
*/
void ShaderProgram::setUniform( const GLint loc , const int x , const int y , const int z , const int w )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUniform4i( loc , x , y , z , w ) ;
}

/**
* @brief Set uniform for 4-variable
* @param name Name of the uniform
* @param x value to pass
* @param y value to pass
* @param z value to pass
* @param w value to pass
*/
void ShaderProgram::setUniform( const std::string & name , const float x , const float y , const float z , const float w )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniform4f( loc , x , y , z , w ) ;
}

/**
* @brief Set uniform for 4-variable
* @param loc Location of the uniform
* @param x value to pass
* @param y value to pass
* @param z value to pass
* @param w value to pass
*/
void ShaderProgram::setUniform( const GLint loc , const float x , const float y , const float z , const float w )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUniform4f( loc , x , y , z , w ) ;
}

/**
* @brief Set uniform for 4-vector
* @param name Name of the uniform
* @param v value to pass
*/
void ShaderProgram::setUniform( const std::string & name , const openMVG::Vec4 & v )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniform4f( loc , v[0] , v[1] , v[2] , v[3] ) ;
}

/**
* @brief Set uniform for 4-vector
* @param loc Location of the uniform
* @param v Value to pass
*/
void ShaderProgram::setUniform( const GLint loc , const openMVG::Vec4 & v )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  glFuncs->glUniform4f( loc , v[0] , v[1] , v[2] , v[3] ) ;
}

/**
* @brief set uniform for a 4x4 matrix
* @param name Name of the uniform
* @param m Matrix to pass
*/
void ShaderProgram::setUniform( const std::string & name , const openMVG::Mat4 & m , bool transpose )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const Eigen::Matrix4f tmp_m = m.cast<float>() ;
  const GLfloat tmp[] =
  {
    tmp_m( 0 , 0 ) , tmp_m( 0 , 1 ) , tmp_m( 0 , 2 ) , tmp_m( 0 , 3 ) ,
    tmp_m( 1 , 0 ) , tmp_m( 1 , 1 ) , tmp_m( 1 , 2 ) , tmp_m( 1 , 3 ) ,
    tmp_m( 2 , 0 ) , tmp_m( 2 , 1 ) , tmp_m( 2 , 2 ) , tmp_m( 2 , 3 ) ,
    tmp_m( 3 , 0 ) , tmp_m( 3 , 1 ) , tmp_m( 3 , 2 ) , tmp_m( 3 , 3 )
  } ;
  const GLint loc = m_active_uniform_loc[ name ] ;
  glFuncs->glUniformMatrix4fv( loc , 1 , transpose , tmp ) ;
}

/**
* @brief set uniform for a 4x4 matrix
* @param loc Location of the uniform
* @param m Matrix to pass
*/
void ShaderProgram::setUniform( const GLint loc , const openMVG::Mat4 & m , bool transpose )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  const Eigen::Matrix4f tmp_m = m.cast<float>() ;
  const GLfloat tmp[] =
  {
    tmp_m( 0 , 0 ) , tmp_m( 0 , 1 ) , tmp_m( 0 , 2 ) , tmp_m( 0 , 3 ) ,
    tmp_m( 1 , 0 ) , tmp_m( 1 , 1 ) , tmp_m( 1 , 2 ) , tmp_m( 1 , 3 ) ,
    tmp_m( 2 , 0 ) , tmp_m( 2 , 1 ) , tmp_m( 2 , 2 ) , tmp_m( 2 , 3 ) ,
    tmp_m( 3 , 0 ) , tmp_m( 3 , 1 ) , tmp_m( 3 , 2 ) , tmp_m( 3 , 3 )
  } ;
  glFuncs->glUniformMatrix4fv( loc , 1 , transpose , tmp ) ;
}

/**
* @brief get attribute location
* @param name Attribute name
* @retval -1 if attribute is not present, not active or invalid name
* @return location of active attribute if valid
*/
GLint ShaderProgram::attribLocation( const std::string & name ) const
{
  if( m_active_attribute_loc.count( name ) )
  {
    return m_active_attribute_loc.at( name ) ;
  }
  return -1 ;
}

/**
* @brief get uniform location
* @param name Uniform name
* @retval -1 if uniform is not present, not active or invalid name
* @return location of active uniform if valid
*/
GLint ShaderProgram::uniformLocation( const std::string & name ) const
{
  if( m_active_uniform_loc.count( name ) )
  {
    return m_active_uniform_loc.at( name ) ;
  }
  return -1 ;
}

/**
* @brief Indicate if a given uniform name is present in the shader
* @param name Name of the uniform to test
* @retval true if present
* @retval false if absent
*/
bool ShaderProgram::hasUniform( const std::string & name ) const
{
  return m_active_uniform_loc.count( name ) > 0 ;
}

/**
* @brief Indicate if a given attribute name is present in the shader
* @param name Name of the attribute to test
* @retval true if present
* @retval false if absent
*/
bool ShaderProgram::hasAttribute( const std::string & name ) const
{
  return m_active_attribute_loc.count( name ) > 0 ;
}

/**
* @brief Link and get error
*/
void ShaderProgram::linkProgram( void )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  m_pgm = glFuncs->glCreateProgram() ;
  glFuncs->glAttachShader( m_pgm , m_vert_shader.getID() ) ;
  glFuncs->glAttachShader( m_pgm , m_frag_shader.getID() ) ;
  glFuncs->glLinkProgram( m_pgm ) ;

  GLint program_ok;
  glFuncs->glGetProgramiv( m_pgm, GL_LINK_STATUS, &program_ok );
  if ( program_ok != GL_TRUE )
  {
    GLsizei log_length = 0;
    GLchar message[1024];
    glFuncs->glGetProgramInfoLog( m_pgm, 1024, &log_length, message );
    m_pgm = 0 ;
  }
}

/**
* @brief get list of active uniforms and attributes
*/
void ShaderProgram::populateActiveUniformsAndAtttributes( void )
{
  QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
  enable() ;
  GLint nb , maxLen ;
  // Uniforms
  {
    glFuncs->glGetProgramiv( m_pgm , GL_ACTIVE_UNIFORMS, &nb );
    glFuncs->glGetProgramiv( m_pgm , GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen );
    GLchar * u_name = new GLchar[ maxLen ] ;

    for( int id_unif = 0 ; id_unif < nb ; ++id_unif )
    {
      GLint size ;
      GLenum type ;
      glFuncs->glGetActiveUniform( m_pgm , id_unif , maxLen, nullptr , &size, &type, u_name );
      std::string s_name( u_name ) ;

      m_active_uniform_loc[ s_name ] = glFuncs->glGetUniformLocation( m_pgm , u_name ) ;
    }
    delete[] u_name ;
  }

  // Attributes
  {
    glFuncs->glGetProgramiv( m_pgm , GL_ACTIVE_ATTRIBUTES, &nb );
    glFuncs->glGetProgramiv( m_pgm, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLen );
    GLchar * a_name = new GLchar[ maxLen ] ;

    for( int id_attr = 0 ; id_attr < nb ; ++id_attr )
    {
      GLint size ;
      GLenum type ;
      glFuncs->glGetActiveAttrib( m_pgm , id_attr, maxLen, nullptr, &size, &type, a_name );
      std::string s_name( a_name ) ;

      m_active_attribute_loc[ s_name ] = glFuncs->glGetAttribLocation( m_pgm , a_name ) ;
    }
    delete[] a_name ;
  }
}

} // namespace openMVG_gui