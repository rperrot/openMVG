// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
CameraGizmo::CameraGizmo( std::shared_ptr<OpenGLContext> ctx ,
                          std::shared_ptr<ShaderProgram> shad ,
                          const openMVG::geometry::Pose3 & pose ,
                          const bool is_spherical ,
                          const double size )
  : RenderableObject( ctx , shad ) ,
    m_pose( pose ) ,
    m_size( size ) ,
    m_is_spherical( is_spherical ) ,
    m_nb_vert( 0 )
{

}


/**
* @brief Destructor
*/
CameraGizmo::~CameraGizmo( void )
{
  m_context->makeCurrent() ;
  destroyGLData() ;
  m_context->doneCurrent() ;
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

  if( ! m_is_spherical )
  {
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
      dataSelection[ id_vert * 6 + 3] = 60.0 / 255.0 ;
      dataSelection[ id_vert * 6 + 4] = 254.0 / 255.0 ;
      dataSelection[ id_vert * 6 + 5] = 39.0 / 255.0 ;

      ++id_vert ;
      dataSelection[ id_vert * 6 ] = B[0] ;
      dataSelection[ id_vert * 6 + 1] = B[1] ;
      dataSelection[ id_vert * 6 + 2] = B[2] ;
      dataSelection[ id_vert * 6 + 3] = 60.0 / 255.0 ;
      dataSelection[ id_vert * 6 + 4] = 254.0 / 255.0 ;
      dataSelection[ id_vert * 6 + 5] = 39.0 / 255.0 ;

      ++id_vert ;
      dataSelection[ id_vert * 6 ] = C[0] ;
      dataSelection[ id_vert * 6 + 1] = C[1] ;
      dataSelection[ id_vert * 6 + 2] = C[2] ;
      dataSelection[ id_vert * 6 + 3] = 60.0 / 255.0 ;
      dataSelection[ id_vert * 6 + 4] = 254.0 / 255.0 ;
      dataSelection[ id_vert * 6 + 5] = 39.0 / 255.0 ;
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
  }
  else
  {
    // Spherical camera
    const int nb_theta_segment = 24 ;
    const int nb_phi_segment = 24 ;
    const int nb_element_per_vert = 6 ;

    // Horizontal lines + nb_vertical segments
    const int nb_horizontal_line = ( nb_phi_segment - 1 ) * nb_theta_segment ;
    const int nb_vertical_line = nb_phi_segment * nb_theta_segment ;
    const int nb_line = nb_horizontal_line + nb_vertical_line ;
    m_nb_vert = 2 * nb_line ;
    const int nb_total = 2 * nb_line * nb_element_per_vert ;

    const double pi = 3.14159265358979 ;

    GLfloat * data = new GLfloat[ nb_total ] ;

    int cur_vert = 0 ;
    // Horizontal lines
    for( int id_phi = 1 ; id_phi < nb_phi_segment ; ++id_phi )
    {
      const double phi   = id_phi * pi / nb_phi_segment ; // [ 0 ; pi ]
      for( int id_theta = 0 ; id_theta < nb_theta_segment ; ++id_theta )
      {
        const double theta_current = id_theta * 2.0 * pi / nb_theta_segment ; // [ 0 ; 2pi ]
        const double theta_next    = ( ( id_theta + 1 ) % nb_theta_segment ) * 2.0 * pi / nb_theta_segment ;

        const double x1 = 0.5 * std::sin( phi ) * std::cos( theta_current ) ;
        const double y1 = 0.5 * std::sin( phi ) * std::sin( theta_current ) ;
        const double z1 = 0.5 * std::cos( phi ) ;

        const double x2 = 0.5 * std::sin( phi ) * std::cos( theta_next ) ;
        const double y2 = 0.5 * std::sin( phi ) * std::sin( theta_next ) ;
        const double z2 = 0.5 * std::cos( phi ) ;

        data[ 6 * cur_vert     ] = x1 ;
        data[ 6 * cur_vert + 1 ] = z1 ;
        data[ 6 * cur_vert + 2 ] = y1 ;
        data[ 6 * cur_vert + 3 ] = 1.0 ;
        data[ 6 * cur_vert + 4 ] = 1.0 ;
        data[ 6 * cur_vert + 5 ] = 1.0 ;
        ++cur_vert ;
        data[ 6 * cur_vert     ] = x2 ;
        data[ 6 * cur_vert + 1 ] = z2 ;
        data[ 6 * cur_vert + 2 ] = y2 ;
        data[ 6 * cur_vert + 3 ] = 1.0 ;
        data[ 6 * cur_vert + 4 ] = 1.0 ;
        data[ 6 * cur_vert + 5 ] = 1.0 ;
        ++cur_vert ;
      }
    }

    // Vertical lines
    for( int id_theta = 0 ; id_theta < nb_theta_segment ; ++id_theta )
    {
      const double theta = id_theta * 2.0 * pi / nb_theta_segment ; // [ 0 ; 2pi ]

      for( int id_phi = 0 ; id_phi < nb_phi_segment ; ++id_phi )
      {
        const double phi_current = id_phi * pi / nb_phi_segment ; // [ 0 ; pi ]
        const double phi_next = ( id_phi + 1 ) * pi / nb_phi_segment ; // [ 0 ; pi ]

        const double x1 = 0.5 * std::sin( phi_current ) * std::cos( theta ) ;
        const double y1 = 0.5 * std::sin( phi_current ) * std::sin( theta ) ;
        const double z1 = 0.5 * std::cos( phi_current ) ;

        const double x2 = 0.5 * std::sin( phi_next ) * std::cos( theta ) ;
        const double y2 = 0.5 * std::sin( phi_next ) * std::sin( theta ) ;
        const double z2 = 0.5 * std::cos( phi_next ) ;

        data[ 6 * cur_vert     ] = x1 ;
        data[ 6 * cur_vert + 1 ] = z1 ;
        data[ 6 * cur_vert + 2 ] = y1 ;
        data[ 6 * cur_vert + 3 ] = 1.0 ;
        data[ 6 * cur_vert + 4 ] = 1.0 ;
        data[ 6 * cur_vert + 5 ] = 1.0 ;
        ++cur_vert ;
        data[ 6 * cur_vert     ] = x2 ;
        data[ 6 * cur_vert + 1 ] = z2 ;
        data[ 6 * cur_vert + 2 ] = y2 ;
        data[ 6 * cur_vert + 3 ] = 1.0 ;
        data[ 6 * cur_vert + 4 ] = 1.0 ;
        data[ 6 * cur_vert + 5 ] = 1.0 ;
        ++cur_vert ;
      }
    }

    assert( cur_vert == 2 * nb_line ) ;

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
    int nb_quad_selection = nb_theta_segment * nb_phi_segment ;
    int nb_triangle_selection = 2 * nb_quad_selection ;
    int nb_vert_selection = 3 * nb_triangle_selection ;
    m_nb_vert_selection = nb_vert_selection ;
    int total_data_selection = nb_vert_selection * nb_element_per_vert ;

    GLfloat * dataSelection = new GLfloat[ total_data_selection ] ;

    cur_vert = 0 ;
    for( int id_phi = 0 ; id_phi < nb_phi_segment ; ++id_phi )
    {
      for( int id_theta = 0 ; id_theta < nb_theta_segment ; ++id_theta )
      {
        const double theta_cur  = id_theta * 2.0 * pi / nb_theta_segment + ( 0.1 * ( 2.0 * pi / nb_theta_segment ) ) ;
        const double theta_next = ( ( id_theta + 1 ) % nb_theta_segment ) * 2.0 * pi / nb_theta_segment - ( 0.1 * ( 2.0 * pi / nb_theta_segment ) ) ;

        const double phi_cur = id_phi * pi / nb_phi_segment + ( 0.1 * pi / nb_phi_segment ) ;
        const double phi_next = ( id_phi + 1 ) * pi / nb_phi_segment - ( 0.1 * pi / nb_phi_segment ) ;

        /*
          a--b
          |  |
          c--d

          A B C
          B D C
        */
        const openMVG::Vec3 A( 0.5 * std::sin( phi_cur ) * std::cos( theta_cur ) ,
                               0.5 * std::sin( phi_cur ) * std::sin( theta_cur ) ,
                               0.5 * std::cos( phi_cur ) ) ;
        const openMVG::Vec3 B( 0.5 * std::sin( phi_cur ) * std::cos( theta_next ) ,
                               0.5 * std::sin( phi_cur ) * std::sin( theta_next ) ,
                               0.5 * std::cos( phi_cur ) ) ;
        const openMVG::Vec3 C( 0.5 * std::sin( phi_next ) * std::cos( theta_cur ) ,
                               0.5 * std::sin( phi_next ) * std::sin( theta_cur ) ,
                               0.5 * std::cos( phi_next ) ) ;
        const openMVG::Vec3 D( 0.5 * std::sin( phi_next ) * std::cos( theta_next ) ,
                               0.5 * std::sin( phi_next ) * std::sin( theta_next ) ,
                               0.5 * std::cos( phi_next ) ) ;

        // A B C
        dataSelection[ 6 * cur_vert     ] = A.x() ;
        dataSelection[ 6 * cur_vert + 1 ] = A.z() ;
        dataSelection[ 6 * cur_vert + 2 ] = A.y() ;
        dataSelection[ 6 * cur_vert + 3 ] = 60.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 4 ] = 254.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 5 ] = 39.0 / 255.0 ;
        ++cur_vert ;
        dataSelection[ 6 * cur_vert     ] = B.x() ;
        dataSelection[ 6 * cur_vert + 1 ] = B.z() ;
        dataSelection[ 6 * cur_vert + 2 ] = B.y() ;
        dataSelection[ 6 * cur_vert + 3 ] = 60.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 4 ] = 254.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 5 ] = 39.0 / 255.0 ;
        ++cur_vert ;
        dataSelection[ 6 * cur_vert     ] = C.x() ;
        dataSelection[ 6 * cur_vert + 1 ] = C.z() ;
        dataSelection[ 6 * cur_vert + 2 ] = C.y() ;
        dataSelection[ 6 * cur_vert + 3 ] = 60.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 4 ] = 254.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 5 ] = 39.0 / 255.0 ;
        ++cur_vert ;

        // B D C
        dataSelection[ 6 * cur_vert     ] = B.x() ;
        dataSelection[ 6 * cur_vert + 1 ] = B.z() ;
        dataSelection[ 6 * cur_vert + 2 ] = B.y() ;
        dataSelection[ 6 * cur_vert + 3 ] = 60.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 4 ] = 254.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 5 ] = 39.0 / 255.0 ;
        ++cur_vert ;
        dataSelection[ 6 * cur_vert     ] = D.x() ;
        dataSelection[ 6 * cur_vert + 1 ] = D.z() ;
        dataSelection[ 6 * cur_vert + 2 ] = D.y() ;
        dataSelection[ 6 * cur_vert + 3 ] = 60.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 4 ] = 254.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 5 ] = 39.0 / 255.0 ;
        ++cur_vert ;
        dataSelection[ 6 * cur_vert     ] = C.x() ;
        dataSelection[ 6 * cur_vert + 1 ] = C.z() ;
        dataSelection[ 6 * cur_vert + 2 ] = C.y() ;
        dataSelection[ 6 * cur_vert + 3 ] = 60.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 4 ] = 254.0 / 255.0 ;
        dataSelection[ 6 * cur_vert + 5 ] = 39.0 / 255.0 ;
        ++cur_vert ;
      } // for
    } // for

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
  } // else

  m_prepared = true ;
}


/**
 * @brief Perform Linear interpolation in RGB 
 * @note this is not a correct color interpolation but enough for our purpose
 */
static inline openMVG::Vec3 ColorInterpolation( const openMVG::Vec3 & a , const openMVG::Vec3 & b , const double t )
{
  return a * (1.0 - t ) + b * t ;
}

/**
* @brief Draw code for the object
*/
void CameraGizmo::draw( void ) const
{
  QOpenGLExtraFunctions *glFuncs = QOpenGLContext::currentContext()->extraFunctions();

  if( selected() )
  {
    m_shader->setUniform( "uUseUniformColor" , 1 ) ;

    if( selectionWeight() > 1.0 )
    {
      // Camera selected
      const double r = 60.0 / 255.0 ;
      const double g = 254.0 / 255.0 ;
      const double b = 39.0 / 255.0 ;
      m_shader->setUniform( "uColor" , openMVG::Vec3( r , g , b ) ) ;
    }
    else
    {
      // camera selected indirectly (because linked to the selected one)
      const openMVG::Vec3 colLow( 21.0 / 255.0 , 70.0 / 255.0 , 0.0 ) ;
      const openMVG::Vec3 colHigh( 60.0 / 255.0 , 254.0 / 255.0 , 39.0 / 255.0 ) ;

      /*
      const double r = 21.0 / 255.0 * selectionWeight() ;
      const double g = 150 / 255.0 * selectionWeight() ;
      const double b = 0.0 / 255.0 ;
      */

      const openMVG::Vec3 col = ColorInterpolation( colLow , colHigh , selectionWeight() ) ;

      m_shader->setUniform( "uColor" , col ) ; //openMVG::Vec3( r , g , b ) ) ;
    }

    glFuncs->glBindVertexArray( m_vao_selection ) ;
    glFuncs->glDrawArrays( GL_TRIANGLES , 0 , m_nb_vert_selection ) ;
    glFuncs->glBindVertexArray( 0 ) ;
  }

  m_shader->setUniform( "uUseUniformColor" , 0 ) ;

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

/**
* @brief destroy all openGL data (if any present)
*/
void CameraGizmo::destroyGLData( void )
{
  if( m_nb_vert > 0 )
  {
    QOpenGLExtraFunctions *glFuncs = QOpenGLContext::currentContext()->extraFunctions();

    glFuncs->glDeleteVertexArrays( 1 , &m_vao ) ;
    glFuncs->glDeleteBuffers( 1 , &m_vbo ) ;

    glFuncs->glDeleteVertexArrays( 1 , &m_vao_selection ) ;
    glFuncs->glDeleteBuffers( 1 , &m_vbo_selection ) ;

    m_nb_vert = 0 ;

    RenderableObject::destroyGLData() ;
  }
}


} // namespace openMVG_gui