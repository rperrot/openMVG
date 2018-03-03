// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "SphericalGizmo.hh"

#include <QOpenGLExtraFunctions>

namespace openMVG_gui
{

/**
* @brief Ctr
* @param pgm Programm shader
* @param center Center of the gizmo
* @param radius Radius of the gizmo
*/
SphericalGizmo::SphericalGizmo( std::shared_ptr<OpenGLContext> ctx ,
                                std::shared_ptr<ShaderProgram> pgm ,
                                const openMVG::Vec3 & center ,
                                const double radius )
  : RenderableObject( ctx , pgm ) ,
    m_center( center ) ,
    m_radius( radius ) ,
    m_nb_vert( 0 )
{
  buildModelMat() ;
}

SphericalGizmo::~SphericalGizmo( void )
{
  m_context->makeCurrent() ;
  destroyGLData() ;
  m_context->doneCurrent() ;
}


/**
* @brief Set center of the gizmo
* @param nCenter New center
*/
void SphericalGizmo::setCenter( const openMVG::Vec3 & nCenter )
{
  m_center = nCenter ;
  buildModelMat() ;
}

/**
* @brief Get center of the gizmo
* @return current center
*/
openMVG::Vec3 SphericalGizmo::center( void ) const
{
  return m_center ;
}

/**
* @brief Set radius of the gizmo
* @param nRad
*/
void SphericalGizmo::setRadius( const double nRad )
{
  m_radius = nRad ;
  buildModelMat() ;
}

/**
* @brief Get radius of the gizmo
* @return current radius
*/
double SphericalGizmo::radius( void ) const
{
  return m_radius ;
}

/**
* @brief Get model matrix
* @return current model matrix (in left hand system)
*/
openMVG::Mat4 SphericalGizmo::modelMat( void ) const
{
  return m_model ;
}

/**
* @brief Prepare object before rendering (ie: create buffers, prepare data)
* @note openGL context must be active when calling this function
*/
void SphericalGizmo::prepare( void )
{
  if( m_prepared )
  {
    return ;
  }

  QOpenGLExtraFunctions *glFuncs = QOpenGLContext::currentContext()->extraFunctions();

  const double pi = 3.14159265358979 ;

  const uint32_t nbPointPerCircle = 128 ;
  const uint32_t nbCircle = 3 ;
  const uint32_t nbVert = 2 * nbPointPerCircle * nbCircle ;
  const uint32_t nbComponentPerVert = 6 ;
  const uint32_t nbTotal = nbVert * nbComponentPerVert ;
  m_nb_vert = nbVert ;

  GLfloat * data = new GLfloat[ nbTotal ] ;
  // X axis
  uint32_t idOut = 0 ;
  for( uint32_t id_pt = 0 ; id_pt < nbPointPerCircle ; ++id_pt )
  {
    const uint32_t next_id = ( id_pt + 1 ) % nbPointPerCircle ;

    const double delta_angle_cur  = id_pt * 2.0 * pi / nbPointPerCircle ;
    const double delta_angle_next = next_id * 2.0 * pi / nbPointPerCircle ;

    // First vertex
    data[ idOut * 6 ] = 0.0 ;
    data[ idOut * 6 + 1 ] = std::cos( delta_angle_cur ) ;
    data[ idOut * 6 + 2 ] = std::sin( delta_angle_cur ) ;
    data[ idOut * 6 + 3 ] = 1.0 ;
    data[ idOut * 6 + 4 ] = 0.0 ;
    data[ idOut * 6 + 5 ] = 0.0 ;
    ++idOut ;

    // Second vertex
    data[ idOut * 6 ] = 0.0 ;
    data[ idOut * 6 + 1 ] = std::cos( delta_angle_next ) ;
    data[ idOut * 6 + 2 ] = std::sin( delta_angle_next ) ;
    data[ idOut * 6 + 3 ] = 1.0 ;
    data[ idOut * 6 + 4 ] = 0.0 ;
    data[ idOut * 6 + 5 ] = 0.0 ;
    ++idOut ;
  }


  // Y axis
  for( uint32_t id_pt = 0 ; id_pt < nbPointPerCircle ; ++id_pt )
  {
    const uint32_t next_id = ( id_pt + 1 ) % nbPointPerCircle ;

    const double delta_angle_cur  = id_pt * 2.0 * pi / nbPointPerCircle ;
    const double delta_angle_next = next_id * 2.0 * pi / nbPointPerCircle ;

    // First vertex
    data[ idOut * 6 ] = std::cos( delta_angle_cur ) ;
    data[ idOut * 6 + 1 ] = 0.0 ;
    data[ idOut * 6 + 2 ] = std::sin( delta_angle_cur ) ;
    data[ idOut * 6 + 3 ] = 0.0 ;
    data[ idOut * 6 + 4 ] = 1.0 ;
    data[ idOut * 6 + 5 ] = 0.0 ;
    ++idOut ;

    // Second vertex
    data[ idOut * 6 ] = std::cos( delta_angle_next ) ;
    data[ idOut * 6 + 1 ] = 0.0 ;
    data[ idOut * 6 + 2 ] = std::sin( delta_angle_next ) ;
    data[ idOut * 6 + 3 ] = 0.0 ;
    data[ idOut * 6 + 4 ] = 1.0 ;
    data[ idOut * 6 + 5 ] = 0.0 ;
    ++idOut ;
  }

  // Z axis
  for( uint32_t id_pt = 0 ; id_pt < nbPointPerCircle ; ++id_pt )
  {
    const uint32_t next_id = ( id_pt + 1 ) % nbPointPerCircle ;

    const double delta_angle_cur  = id_pt * 2.0 * pi / nbPointPerCircle ;
    const double delta_angle_next = next_id * 2.0 * pi / nbPointPerCircle ;

    // First vertex
    data[ idOut * 6 ] = std::cos( delta_angle_cur ) ;
    data[ idOut * 6 + 1 ] = std::sin( delta_angle_cur ) ;
    data[ idOut * 6 + 2 ] = 0.0 ;
    data[ idOut * 6 + 3 ] = 0.0 ;
    data[ idOut * 6 + 4 ] = 0.0 ;
    data[ idOut * 6 + 5 ] = 1.0 ;
    ++idOut ;

    // Second vertex
    data[ idOut * 6 ] = std::cos( delta_angle_next ) ;
    data[ idOut * 6 + 1 ] = std::sin( delta_angle_next ) ;
    data[ idOut * 6 + 2 ] = 0.0 ;
    data[ idOut * 6 + 3 ] = 0.0 ;
    data[ idOut * 6 + 4 ] = 0.0 ;
    data[ idOut * 6 + 5 ] = 1.0 ;
    ++idOut ;
  }
  assert( idOut == nbVert ) ;

  // set vertex data
  glFuncs->glGenVertexArrays( 1 , &m_vao ) ;
  glFuncs->glBindVertexArray( m_vao ) ;
  glFuncs->glGenBuffers( 1 , &m_vbo ) ;
  glFuncs->glBindBuffer( GL_ARRAY_BUFFER , m_vbo ) ;
  glFuncs->glBufferData( GL_ARRAY_BUFFER , nbTotal * sizeof( GLfloat ) , data , GL_STATIC_DRAW ) ;

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
void SphericalGizmo::draw( void ) const
{
  QOpenGLExtraFunctions *glFuncs = QOpenGLContext::currentContext()->extraFunctions();

  glFuncs->glBindVertexArray( m_vao ) ;
  glFuncs->glDrawArrays( GL_LINES , 0 , m_nb_vert ) ;
  glFuncs->glBindVertexArray( 0 ) ;
}

/**
* @brief Build model matrix
*/
void SphericalGizmo::buildModelMat( void )
{
  m_model( 0 , 0 ) = m_radius ;
  m_model( 0 , 1 ) = 0.0 ;
  m_model( 0 , 2 ) = 0.0 ;
  m_model( 0 , 3 ) = 0.0 ;

  m_model( 1 , 0 ) = 0.0 ;
  m_model( 1 , 1 ) = m_radius ;
  m_model( 1 , 2 ) = 0.0 ;
  m_model( 1 , 3 ) = 0.0 ;

  m_model( 2 , 0 ) = 0.0 ;
  m_model( 2 , 1 ) = 0.0 ;
  m_model( 2 , 2 ) = m_radius ;
  m_model( 2 , 3 ) = 0.0 ;

  m_model( 3 , 0 ) = m_center[0] ;
  m_model( 3 , 1 ) = m_center[1] ;
  m_model( 3 , 2 ) = m_center[2] ;
  m_model( 3 , 3 ) = 1.0 ;
}

/**
* @brief destroy all openGL data (if any present)
*/
void SphericalGizmo::destroyGLData( void )
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