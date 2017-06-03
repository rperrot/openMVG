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

  // Selection
  int nb_triangle_selection = 4 ;
  int nb_vert_selection = 3 * nb_triangle_selection ;
  m_nb_vert_selection = nb_vert_selection ; 
  int total_data_selection = nb_vert_selection * nb_component_per_vert ;
  GLfloat * dataSelection = new GLfloat[ total_data_selection ] ;

  openMVG::Vec3 orig( 0 , 0 , 0 ) ;
  openMVG::Vec3 v1( -0.5 , - 0.5 , 1.0 ) ;
  openMVG::Vec3 v2( -0.5 , 0.5 , 1.0 ) ;
  openMVG::Vec3 v3( 0.5 , 0.5 , 1.0 ) ;
  openMVG::Vec3 v4( 0.5 , -0.5 , 1.0 ) ;
  openMVG::Vec3 v[] = { v1 , v2 , v3 , v4 } ;
  for( int id_tri = 0 ; id_tri < 4 ; ++id_tri )
  {
    const openMVG::Vec3 c = ( orig + v[id_tri] + v[( id_tri + 1 ) % 4] ) / 3.0 ;
    const openMVG::Vec3 dv1 = c - orig ;
    const openMVG::Vec3 dv2 = c - v[id_tri] ;
    const openMVG::Vec3 dv3 = c - v[( id_tri + 1 ) % 4];

    const openMVG::Vec3 A = orig + 0.2 * dv1 ;
    const openMVG::Vec3 B = v[id_tri] + 0.2 * dv2 ;
    const openMVG::Vec3 C = v[( id_tri + 1 ) % 4] + 0.2 * dv3 ;

    int id_vert = 3 * id_tri ;
    dataSelection[ id_vert * 6 ] = A[0] ;
    dataSelection[ id_vert * 6 + 1] = A[1] ;
    dataSelection[ id_vert * 6 + 2] = A[2] ;
    dataSelection[ id_vert * 6 + 3] = 1.0 ;
    dataSelection[ id_vert * 6 + 4] = 0.0 ;
    dataSelection[ id_vert * 6 + 5] = 0.0 ;

    ++id_vert ;
    dataSelection[ id_vert * 6 ] = B[0] ;
    dataSelection[ id_vert * 6 + 1] = B[1] ;
    dataSelection[ id_vert * 6 + 2] = B[2] ;
    dataSelection[ id_vert * 6 + 3] = 1.0 ;
    dataSelection[ id_vert * 6 + 4] = 0.0 ;
    dataSelection[ id_vert * 6 + 5] = 0.0 ;

    ++id_vert ;
    dataSelection[ id_vert * 6 ] = C[0] ;
    dataSelection[ id_vert * 6 + 1] = C[1] ;
    dataSelection[ id_vert * 6 + 2] = C[2] ;
    dataSelection[ id_vert * 6 + 3] = 1.0 ;
    dataSelection[ id_vert * 6 + 4] = 0.0 ;
    dataSelection[ id_vert * 6 + 5] = 0.0 ;
  }


  // set vertex data
  {
    glFuncs->glGenVertexArrays( 1 , &m_vao_selection ) ;
    glFuncs->glBindVertexArray( m_vao_selection ) ;
    glFuncs->glGenBuffers( 1 , &m_vbo_selection ) ;
    glFuncs->glBindBuffer( GL_ARRAY_BUFFER , m_vbo_selection ) ;
    glFuncs->glBufferData( GL_ARRAY_BUFFER , total_data_selection * sizeof( GLfloat ) , dataSelection , GL_STATIC_DRAW ) ;

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
  }

  delete[] data ;
  delete[] dataSelection ; 
  m_prepared = true ;
}

/**
* @brief Draw code for the object
*/
void CameraGizmo::draw( void ) const
{
  QOpenGLExtraFunctions *glFuncs = QOpenGLContext::currentContext()->extraFunctions();

  if( selected() )
  {
    glFuncs->glBindVertexArray( m_vao_selection ) ;
    glFuncs->glDrawArrays( GL_TRIANGLES , 0 , m_nb_vert_selection ) ;
    glFuncs->glBindVertexArray( 0 ) ;
  }


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