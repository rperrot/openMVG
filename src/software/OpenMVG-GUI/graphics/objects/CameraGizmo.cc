#include "CameraGizmo.hh"

#include <QOpenGLExtraFunctions>

namespace openMVG_gui
{


/**
* @brief Ctr
* @param shad Shader to use for the rendering of the gizmo
* @param pose Current pose corresponding to the camera
* @param size Size of the gizmo
*/
CameraGizmo::CameraGizmo( std::shared_ptr<ShaderProgram> shad ,
                          const openMVG::geometry::Pose3 & pose ,
                          const double size )
  : RenderableObject( shad ) ,
    m_pose( pose ) ,
    m_size( size )
{

}

/**
* @brief Get current size of the gizmo
* @return current size of the gizmo
*/
double CameraGizmo::size( void ) const
{
  return m_size ;
}

/**
*
*/
void CameraGizmo::setSize( const double size )
{
  m_size = size ;
}

/**
* @brief Prepare object before rendering (ie: create buffers, prepare data)
* @note openGL context must be active when calling this function
*/
void CameraGizmo::prepare( void )
{
  if( m_prepared )
  {
    return ;
  }
  QOpenGLExtraFunctions *glFuncs = QOpenGLContext::currentContext()->extraFunctions();

  const int nb_line = 8 ;
  const int nb_vert = 2 * nb_line ;
  const int nb_component_per_vert = 6 ;
  const int nb_total = nb_vert * nb_component_per_vert ;
  m_nb_vert = nb_vert ;

  GLfloat * data = new GLfloat[ nb_total ] ;

  // The square
  int outId = 0 ;
  // 1
  data[ 6 * outId ] = -0.5 ;
  data[ 6 * outId + 1 ] = - 0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  data[ 6 * outId ] = 0.5 ;
  data[ 6 * outId + 1 ] = -0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  // 2
  data[ 6 * outId ] = 0.5 ;
  data[ 6 * outId + 1 ] = -0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  data[ 6 * outId ] = 0.5 ;
  data[ 6 * outId + 1 ] = 0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  // 3
  data[ 6 * outId ] = 0.5 ;
  data[ 6 * outId + 1 ] = 0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  data[ 6 * outId ] = -0.5 ;
  data[ 6 * outId + 1 ] = 0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  // 4
  data[ 6 * outId ] = -0.5 ;
  data[ 6 * outId + 1 ] = 0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  data[ 6 * outId ] = -0.5 ;
  data[ 6 * outId + 1 ] = -0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  // 5
  data[ 6 * outId ] = 0.0 ;
  data[ 6 * outId + 1 ] = 0.0 ;
  data[ 6 * outId + 2 ] = 0.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  data[ 6 * outId ] = -0.5 ;
  data[ 6 * outId + 1 ] = -0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  // 6
  data[ 6 * outId ] = 0.0 ;
  data[ 6 * outId + 1 ] = 0.0 ;
  data[ 6 * outId + 2 ] = 0.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  data[ 6 * outId ] = -0.5 ;
  data[ 6 * outId + 1 ] = 0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  // 7
  data[ 6 * outId ] = 0.0 ;
  data[ 6 * outId + 1 ] = 0.0 ;
  data[ 6 * outId + 2 ] = 0.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  data[ 6 * outId ] = 0.5 ;
  data[ 6 * outId + 1 ] = -0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  // 8
  data[ 6 * outId ] = 0.0 ;
  data[ 6 * outId + 1 ] = 0.0 ;
  data[ 6 * outId + 2 ] = 0.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;
  ++outId ;
  data[ 6 * outId ] = 0.5 ;
  data[ 6 * outId + 1 ] = 0.5 ;
  data[ 6 * outId + 2 ] = 1.0 ;
  data[ 6 * outId + 3 ] = 1.0 ;
  data[ 6 * outId + 4 ] = 1.0 ;
  data[ 6 * outId + 5 ] = 1.0 ;

  // set vertex data
  glFuncs->glGenVertexArrays( 1 , &m_vao ) ;
  glFuncs->glBindVertexArray( m_vao ) ;
  glFuncs->glGenBuffers( 1 , &m_vbo ) ;
  glFuncs->glBindBuffer( GL_ARRAY_BUFFER , m_vbo ) ;
  glFuncs->glBufferData( GL_ARRAY_BUFFER , nb_total * sizeof( GLfloat ) , data , GL_STATIC_DRAW ) ;

  GLint pos = m_shader->attribLocation( "inPos" ) ;
  GLint col = m_shader->attribLocation( "inCol" ) ;

  if( pos == -1 || col == -1 )
  {
    std::cerr << "Object wont be drawn correcly" << std::endl ;

    if( pos == -1 )
    {
      std::cerr << "shader does not have an active \"inPos\" attrib" << std::endl ;
    }
    if( col == -1 )
    {
      std::cerr << "shader does not have an active \"inCol\" attrib" << std::endl;
    }
  }
  if( pos != -1 )
  {
    glFuncs->glEnableVertexAttribArray( pos ) ;
    glFuncs->glVertexAttribPointer( pos , 3 , GL_FLOAT , GL_FALSE , 6 * sizeof( GLfloat ) , ( GLvoid * ) 0 ) ;
  }
  if( col != -1 )
  {
    glFuncs->glEnableVertexAttribArray( col ) ;
    glFuncs->glVertexAttribPointer( col , 3 , GL_FLOAT , GL_FALSE , 6 * sizeof( GLfloat ) , ( GLvoid* ) ( 3 * sizeof( GLfloat ) ) ) ;
  }

  glFuncs->glBindBuffer( GL_ARRAY_BUFFER , 0 ) ;
  glFuncs->glBindVertexArray( 0 );


  delete[] data ;
  m_prepared = true ;
}

/**
* @brief Draw code for the object
*/
void CameraGizmo::draw( void ) const
{
  QOpenGLExtraFunctions *glFuncs = QOpenGLContext::currentContext()->extraFunctions();

  glFuncs->glBindVertexArray( m_vao ) ;
  glFuncs->glDrawArrays( GL_LINES , 0 , m_nb_vert ) ;
  glFuncs->glBindVertexArray( 0 ) ;
}

/**
* @brief Get model matrix
* @return current model matrix (in left hand system)
*/
openMVG::Mat4 CameraGizmo::modelMat( void ) const
{
  // transform pose into a 4x4 matrix where origin
  // is the position of the center of the pose
  // and orientation corresponds to the rotation of the camera
  // It must ensure scale too
  openMVG::Mat4 scale = openMVG::Mat4::Identity() ;
  scale( 0 , 0 ) = m_size ;
  scale( 1 , 1 ) = m_size ;
  scale( 2 , 2 ) = m_size ;

  openMVG::Mat4 rot = openMVG::Mat4::Identity() ;
  rot.block( 0 , 0 , 3 , 3 ) = m_pose.rotation().transpose() ;

  openMVG::Mat4 tra = openMVG::Mat4::Identity() ;
  tra( 0 , 3 ) = m_pose.center()[0] ;
  tra( 1 , 3 ) = m_pose.center()[1] ;
  tra( 2 , 3 ) = m_pose.center()[2] ;

  return ( tra * rot * scale ).transpose() ;
}

} // namespace openMVG_gui