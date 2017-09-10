#include "Grid.hh"


#include <QOpenGLExtraFunctions>

namespace openMVG_gui
{

/**
* @brief Ctr
* @param nb_element in x
* @param nb_element in y
*/
Grid::Grid( std::shared_ptr<ShaderProgram> pgm  ,
            const int nb_x ,
            const int nb_y ,
            const openMVG::Vec3 col_axis_x ,
            const openMVG::Vec3 col_axis_y ,
            const openMVG::Vec3 col_standard_lines ,
            const openMVG::Vec3 col_major_lines     )
  : RenderableObject( pgm ) ,
    m_nb_x( ( nb_x ) % 2 == 0 ? nb_x + 1 : nb_x  ) ,
    m_nb_y( ( nb_y ) % 2 == 0 ? nb_y + 1 : nb_y  ) ,
    m_color_x( col_axis_x ) ,
    m_color_y( col_axis_y ) ,
    m_color_standard( col_standard_lines ) ,
    m_color_major( col_major_lines ) ,
    m_nb_vert( 0 )
{

}

Grid::~Grid()
{
  destroyGLData() ;
}


/**
* @brief Prepare object before rendering (ie: create buffers, prepare data)
*/
void Grid::prepare( void )
{
  QOpenGLExtraFunctions *glFuncs = QOpenGLContext::currentContext()->extraFunctions();

  if( m_prepared )
  {
    return ;
  }

  const uint32_t nb_line = m_nb_x + m_nb_y ;
  const uint32_t nb_vert = 2 * nb_line ;
  m_nb_vert = nb_vert ;
  const uint32_t nb_component_per_vert = 6 ; // (XYZ,RGB)
  const uint32_t total_elt = nb_vert * nb_component_per_vert ;
  GLfloat * data = new GLfloat[ total_elt ] ;

  // Spacing between two lines of the grid
  const int interval_major = 5 ;
  GLfloat delta_x = 1.0 / interval_major ;
  GLfloat delta_y = 1.0 / interval_major ;

  // X lines
  uint32_t out_idx = 0 ;
  const int index_middle_x = m_nb_x / 2 ;
  const GLfloat min_x = - ( m_nb_y / 2 ) * delta_y ;
  const GLfloat max_x = ( m_nb_y / 2 ) * delta_y ;
  const GLfloat min_y = - ( m_nb_x / 2 ) * delta_x ;
  const GLfloat max_y = ( m_nb_x / 2 ) * delta_x ;

  for( int id_line = 0 ; id_line < m_nb_x ; ++id_line , ++out_idx )
  {
    const int v1_pos_idx_start = nb_component_per_vert * out_idx * 2 ;
    const int v1_col_idx_start = nb_component_per_vert * out_idx * 2 + 3 ;
    const int v2_pos_idx_start = nb_component_per_vert * out_idx * 2 + 6 ;
    const int v2_col_idx_start = nb_component_per_vert * out_idx * 2 + 9 ;

    // Vertex position
    // First vertex
    data[ v1_pos_idx_start ] = min_x ;
    data[ v1_pos_idx_start + 1 ] = min_y + id_line * delta_y ;
    data[ v1_pos_idx_start + 2 ] = 0.0f ;

    data[ v2_pos_idx_start ] = max_x ;
    data[ v2_pos_idx_start + 1 ] = min_y + id_line * delta_y ;
    data[ v2_pos_idx_start + 2 ] = 0.0f ;

    // Vertex color
    if( id_line == index_middle_x )
    {
      // origin axis axis
      // first vert
      data[ v1_col_idx_start  ] = m_color_x[0] ;
      data[ v1_col_idx_start + 1 ] = m_color_x[1] ;
      data[ v1_col_idx_start + 2 ] = m_color_x[2] ;
      // second vert
      data[ v2_col_idx_start  ] = m_color_x[0] ;
      data[ v2_col_idx_start + 1 ] = m_color_x[1] ;
      data[ v2_col_idx_start + 2 ] = m_color_x[2] ;
    }
    else if( ( index_middle_x - id_line % index_middle_x ) % interval_major == 0 )
    {
      // Major line
      // first vert
      data[ v1_col_idx_start  ] = m_color_major[0] ;
      data[ v1_col_idx_start + 1 ] = m_color_major[1] ;
      data[ v1_col_idx_start + 2 ] = m_color_major[2] ;
      // second vert
      data[ v2_col_idx_start  ] = m_color_major[0] ;
      data[ v2_col_idx_start + 1 ] = m_color_major[1] ;
      data[ v2_col_idx_start + 2 ] = m_color_major[2] ;
    }
    else
    {
      // Minor line
      data[ v1_col_idx_start  ] = m_color_standard[0] ;
      data[ v1_col_idx_start + 1 ] = m_color_standard[1] ;
      data[ v1_col_idx_start + 2 ] = m_color_standard[2] ;
      // second vert
      data[ v2_col_idx_start  ] = m_color_standard[0] ;
      data[ v2_col_idx_start + 1 ] = m_color_standard[1] ;
      data[ v2_col_idx_start + 2 ] = m_color_standard[2] ;
    }
  }

  // Y lines
  const int index_middle_y = m_nb_y / 2 ;
  for( int id_line = 0 ; id_line < m_nb_y ; ++id_line , ++out_idx )
  {
    const int v1_pos_idx_start = nb_component_per_vert * out_idx * 2 ;
    const int v1_col_idx_start = nb_component_per_vert * out_idx * 2 + 3 ;
    const int v2_pos_idx_start = nb_component_per_vert * out_idx * 2 + 6 ;
    const int v2_col_idx_start = nb_component_per_vert * out_idx * 2 + 9 ;

    // Vertex position
    // First vertex
    data[ v1_pos_idx_start ] = min_x + id_line * delta_x ;
    data[ v1_pos_idx_start + 1 ] = min_y ;
    data[ v1_pos_idx_start + 2 ] = 0.0f ;

    data[ v2_pos_idx_start ] = min_x + id_line * delta_x ;
    data[ v2_pos_idx_start + 1 ] = max_y ;
    data[ v2_pos_idx_start + 2 ] = 0.0f ;

    // Vertex color
    if( id_line == index_middle_y )
    {
      // origin axis axis
      // first vert
      data[ v1_col_idx_start  ] = m_color_y[0] ;
      data[ v1_col_idx_start + 1 ] = m_color_y[1] ;
      data[ v1_col_idx_start + 2 ] = m_color_y[2] ;
      // second vert
      data[ v2_col_idx_start  ] = m_color_x[0] ;
      data[ v2_col_idx_start + 1 ] = m_color_x[1] ;
      data[ v2_col_idx_start + 2 ] = m_color_x[2] ;
    }
    else if( ( index_middle_x - id_line % index_middle_x ) % interval_major == 0 )
    {
      // Major line
      // first vert
      data[ v1_col_idx_start  ] = m_color_major[0] ;
      data[ v1_col_idx_start + 1 ] = m_color_major[1] ;
      data[ v1_col_idx_start + 2 ] = m_color_major[2] ;
      // second vert
      data[ v2_col_idx_start  ] = m_color_major[0] ;
      data[ v2_col_idx_start + 1 ] = m_color_major[1] ;
      data[ v2_col_idx_start + 2 ] = m_color_major[2] ;
    }
    else
    {
      // Minor line
      data[ v1_col_idx_start  ] = m_color_standard[0] ;
      data[ v1_col_idx_start + 1 ] = m_color_standard[1] ;
      data[ v1_col_idx_start + 2 ] = m_color_standard[2] ;
      // second vert
      data[ v2_col_idx_start  ] = m_color_standard[0] ;
      data[ v2_col_idx_start + 1 ] = m_color_standard[1] ;
      data[ v2_col_idx_start + 2 ] = m_color_standard[2] ;
    }
  }

  // set vertex data
  glFuncs->glGenVertexArrays( 1 , &m_vao ) ;
  glFuncs->glBindVertexArray( m_vao ) ;
  glFuncs->glGenBuffers( 1 , &m_vbo ) ;
  glFuncs->glBindBuffer( GL_ARRAY_BUFFER , m_vbo ) ;
  glFuncs->glBufferData( GL_ARRAY_BUFFER , total_elt * sizeof( GLfloat ) , data , GL_STATIC_DRAW ) ;

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
void Grid::draw( void ) const
{
  QOpenGLExtraFunctions *glFuncs = QOpenGLContext::currentContext()->extraFunctions();

  glFuncs->glBindVertexArray( m_vao ) ;
  glFuncs->glDrawArrays( GL_LINES , 0 , m_nb_vert ) ;
  glFuncs->glBindVertexArray( 0 ) ;
}

/**
* @brief destroy all openGL data (if any present)
*/
void Grid::destroyGLData( void )
{
  if( m_nb_vert > 0 )
  {
    QOpenGLExtraFunctions *glFuncs = QOpenGLContext::currentContext()->extraFunctions();

    glFuncs->glDeleteVertexArrays( 1 , &m_vao ) ;
    glFuncs->glDeleteBuffers( 1 , &m_vbo ) ;

    m_nb_vert = 0 ;

    RenderableObject::destroyGLData() ;
  }
}

} // namespace openMVG_gui