// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ResultViewWidget.hh"

#include "graphics/objects/Grid.hh"
#include "graphics/objects/SphericalGizmo.hh"
#include "utils/BellBall.hh"
#include "utils/Plane.hh"
#include "utils/Sphere.hh"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QCoreApplication>
#include <QOpenGLExtraFunctions>

namespace openMVG_gui
{
/**
 * @brief Ctr
 * @param parent Parent widget
 */
ResultViewWidget::ResultViewWidget( const openMVG::Vec4 & backgroundColor , QWidget *parent )
    : QOpenGLWidget( parent ),
    m_backgroundColor( backgroundColor ) ,
    m_scn( nullptr )
{
  /*
  QOpenGLContext * context = new QOpenGLContext ;
  context->setFormat( format ) ;
  context->create() ;
  setFormat( context->format() ) ;
  makeCurrent() ;
  */
}

ResultViewWidget::~ResultViewWidget( void )
{
  destroyGLData();
  m_scn = nullptr;
}

/**
 * @brief Initialize openGL context
 */
void ResultViewWidget::initializeGL( void )
{
  glClearColor( m_backgroundColor[0] , m_backgroundColor[1] , m_backgroundColor[2] , m_backgroundColor[3] );
  glEnable( GL_DEPTH_TEST );

  // Create point shader
  const std::string ressource_path =
      stlplus::folder_append_separator( QCoreApplication::applicationDirPath().toStdString() ) + "ressources";
  const std::string point_vert_path = stlplus::create_filespec( ressource_path, "point_shader.vert" );
  const std::string point_frag_path = stlplus::create_filespec( ressource_path, "point_shader.frag" );
  m_point_shader                    = std::make_shared<ShaderProgram>( point_vert_path, point_frag_path );

  // Create grid
  const openMVG::Vec3 origin( 0.0, 0.0, 0.0 );

  m_grid = std::shared_ptr<Grid>( new Grid( getContext(), m_point_shader, 201, 201 ) );
  m_grid->setVisible( false );
  m_sph_gizmo = std::shared_ptr<SphericalGizmo>( new SphericalGizmo( getContext(), m_point_shader, origin, 1.0 ) );
  m_sph_gizmo->setVisible( false );
}

/**
 * @brief main render loop
 */
void ResultViewWidget::paintGL( void )
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  const int w = width();
  const int h = height();

  if ( m_scn )
  {
    /*
    if( m_point_shader )
    {
      m_point_shader->enable() ;
      if( m_point_shader->hasUniform( "uUseUniformColor" ) )
      {
        m_point_shader->setUniform( "uUseUniformColor" , ( int ) 0 ) ;
        m_point_shader->setUniform( "uColor" , openMVG::Vec3( 1.0 , 1.0 , 1.0 ) ) ;
      }
    }
    */

    m_scn->render( (double)w, (double)h );
  }
}

/**
 * @brief When image is resized
 */
void ResultViewWidget::resizeGL( int w, int h )
{
  glViewport( 0, 0, w /* * devicePixelRatio() */, h /* * devicePixelRatio() */ );
}

/**
 * @brief Set current scene
 */
void ResultViewWidget::setScene( std::shared_ptr<SceneManager> mgr )
{
  m_scn = mgr;
  if ( m_scn )
  {
    std::shared_ptr<Camera> camera = m_scn->camera();
    if ( camera )
    {
      std::dynamic_pointer_cast<SphericalGizmo>( m_sph_gizmo )->setCenter( camera->destination() );
    }
  }
}

/**
 * @brief get standard point shader
 */
std::shared_ptr<ShaderProgram> ResultViewWidget::pointShader( void ) const
{
  return m_point_shader;
}

/**
 * @brief get standard grid
 */
std::shared_ptr<RenderableObject> ResultViewWidget::grid( void ) const
{
  return m_grid;
}

/**
 * @brief get standard spherical gizmo
 */
std::shared_ptr<RenderableObject> ResultViewWidget::sphericalGizmo( void ) const
{
  return m_sph_gizmo;
}

/**
 * @brief Prepare objects before rendering
 * This must be called each time objects have been added to the scene manager
 */
void ResultViewWidget::prepareObjects( void )
{
  makeCurrent();
  m_scn->prepare();
}

/**
 * @brief Update track size based on screen
 */
void ResultViewWidget::updateTrackballSize( void )
{
  if ( m_scn )
  {
    std::shared_ptr<Camera> camera = m_scn->camera();
    if ( camera )
    {

      openMVG::Vec3 pos;
      if ( width() * 0.8 < height() * 0.8 )
      {
        pos = openMVG::Vec3( width() * 0.8, height() / 2.0, 0.0 );
      }
      else
      {
        pos = openMVG::Vec3( width() / 2.0, height() * 0.8, 0.0 );
      }

      const double viewport[] = {0, 0, static_cast<double>( width() ), static_cast<double>( height() )};

      const openMVG::Vec3 realPos = camera->unProject( pos, viewport );

      // Camera direction
      const openMVG::Vec3 cam_plane_n = camera->destination() - camera->position();
      const Plane         cam_plane( cam_plane_n, camera->destination() );

      const openMVG::Vec3 interPos = cam_plane.intersectionPosition( camera->position(), realPos );
      const double        nRad     = ( interPos - camera->destination() ).norm();

      std::dynamic_pointer_cast<SphericalGizmo>( m_sph_gizmo )->setRadius( nRad );
    }
  }
}

/**
 * @brief Update scene by zooming in or out
 */
void ResultViewWidget::wheelEvent( QWheelEvent *event )
{
  const double numDegrees = (double)( event->angleDelta().y() ) / 8;
  const double numSteps   = (double)numDegrees / 15;

  enum MouseAction
  {
    ZOOM,
    CHANGE_FOV,  // shift
    CHANGE_NEAR, // ctrl
    CHANGE_FAR   // alt
  };

  MouseAction act = ZOOM;

  if ( event->modifiers() & Qt::ShiftModifier )
  {
    act = CHANGE_FOV;
  }
  /*
  else if( event->modifiers() & Qt::ControlModifier )
  {
    act = CHANGE_NEAR ;
  }
  else if( event->modifiers() & Qt::AltModifier )
  {
    act = CHANGE_FAR ;
  }
  */

  if ( m_scn )
  {
    std::shared_ptr<Camera> camera = m_scn->camera();
    if ( camera )
    {
      switch ( act )
      {
      case ZOOM:
      {
        camera->zoom( numSteps );

        break;
      }
      case CHANGE_NEAR:
      {
        const double cur_near = camera->nearPlane();
        const double cur_far  = camera->farPlane();

        const double ratio = 0.1;

        const double new_np = std::min( std::max( 0.000001, cur_near + ratio * numSteps ), cur_far );

        camera->setNearPlane( new_np );

        break;
      }
      case CHANGE_FAR:
      {
        const double cur_near = camera->nearPlane();
        const double cur_far  = camera->farPlane();

        const double ratio = ( cur_far - cur_near ) / 100.0;

        const double new_np = std::max( cur_far + ratio * numSteps, cur_near );

        camera->setFarPlane( new_np );
        break;
      }
      case CHANGE_FOV:
      {
        const double cur_fov = openMVG::R2D( camera->fov() );

        const double new_fov = std::min( 90.0, std::max( 5.0, cur_fov + 1.2 * numSteps ) );

        camera->setFov( openMVG::D2R( new_fov ) );

        break;
      }
      }

      updateTrackballSize();
    }
  }
  //  event->accept();
  update();
}

/**
 * @brief Mouse event management
 * Used to pan/rotate camera
 */
void ResultViewWidget::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton )
  {
    m_last_mouse_x = event->x();
    m_last_mouse_y = event->y();

    m_sph_gizmo->setVisible( true );
  }

  update();
}

/**
 * @brief Mouse event management
 * Used to pan/rotate camera for ex
 */
void ResultViewWidget::mouseMoveEvent( QMouseEvent *event )
{
  std::shared_ptr<Camera> camera = nullptr;
  if ( m_scn )
  {
    camera = m_scn->camera();
    if ( !camera )
    {
      return;
    }
  }
  else
  {
    return;
  }

  enum MouseAction
  {
    PAN,
    ZOOM,
    ROTATE,

    NONE
  };

  MouseAction cur_act = NONE;

  // 1st : select action to do

  if ( event->buttons() == Qt::MiddleButton )
  {
    cur_act = PAN;
  }
  else if ( event->buttons() == Qt::LeftButton )
  {
    if ( event->modifiers() & Qt::ControlModifier )
    {
      cur_act = PAN;
    }
    else
    {
      cur_act = ROTATE;
    }
  }

  // 2nd Apply action
  switch ( cur_act )
  {
  case PAN:
  {
    openMVG::Vec3 oldPos( m_last_mouse_x, static_cast<double>( height() ) - m_last_mouse_y, 0.0 );
    openMVG::Vec3 newPos( event->x(), static_cast<double>( height() ) - event->y(), 0.0 );

    const double viewport[] = {0, 0, static_cast<double>( width() ), static_cast<double>( height() )};

    const openMVG::Vec3 realOld = camera->unProject( oldPos, viewport );
    const openMVG::Vec3 realNew = camera->unProject( newPos, viewport );

    // Camera direction
    const openMVG::Vec3 cam_plane_n = camera->destination() - camera->position();
    const Plane         cam_plane( cam_plane_n, camera->destination() );

    const openMVG::Vec3 interOld = cam_plane.intersectionPosition( camera->position(), realOld );
    const openMVG::Vec3 interNew = cam_plane.intersectionPosition( camera->position(), realNew );

    const openMVG::Vec3 panVector = interOld - interNew;

    camera->pan( panVector );
    std::dynamic_pointer_cast<SphericalGizmo>( m_sph_gizmo )->setCenter( camera->destination() );

    break;
  }
  case ROTATE:
  {
    // Compute parameters of the rotation
    const BellBall ball( std::min( 0.8 * width() / 2.0, 0.8 * height() / 2.0 ) );

    const openMVG::Vec3 realOld = ball.get( m_last_mouse_x - width() / 2.0,
                                            ( static_cast<double>( height() ) - m_last_mouse_y ) - height() / 2.0 );
    const openMVG::Vec3 realNew =
        ball.get( event->x() - width() / 2.0, ( static_cast<double>( height() ) - event->y() ) - height() / 2.0 );

    const openMVG::Vec3 axis  = realOld.cross( realNew );
    const double        angle = std::atan( axis.norm() / realOld.dot( realNew ) );

    // Change frame for local frame to global frame ( for rotation axis )
    const openMVG::Vec3 y     = -camera->up().normalized();
    const openMVG::Vec3 z     = ( camera->destination() - camera->position() ).normalized();
    const openMVG::Vec3 x     = z.cross( y );
    const openMVG::Vec3 naxis = openMVG::Vec3( axis.dot( openMVG::Vec3( x[ 0 ], y[ 0 ], z[ 0 ] ) ),
                                               axis.dot( openMVG::Vec3( x[ 1 ], y[ 1 ], z[ 1 ] ) ),
                                               axis.dot( openMVG::Vec3( x[ 2 ], y[ 2 ], z[ 2 ] ) ) );

    // Rotate the camera around center of projection
    camera->rotateAroundDestination( naxis, angle );
    break;
  }
  case ZOOM:
  {
    break;
  }
  default:
  {
    break;
  }
  }

  m_last_mouse_x = event->x();
  m_last_mouse_y = event->y();
  update();
}

/**
 * @brief Mouse event management
 * Use to pan/rotate camera for ex
 */
void ResultViewWidget::mouseReleaseEvent( QMouseEvent *event )
{
  m_sph_gizmo->setVisible( false );
  update();
}

std::shared_ptr<OpenGLContext> ResultViewWidget::getContext( void )
{
  static std::shared_ptr<OpenGLContext> ctx = std::make_shared<OpenGLContext>( this );
  return ctx;
}

/**
 * @brief Change background color
 * @param color The new background color
 */
void ResultViewWidget::setBackgroundColor( const openMVG::Vec4 &color )
{
  m_backgroundColor = color ; 
  makeCurrent();
  glClearColor( color[ 0 ], color[ 1 ], color[ 2 ], color[ 3 ] );
}

/**
 * @brief clean openGL data before quitting
 */
void ResultViewWidget::destroyGLData( void )
{
  qInfo( "destroyGLData" );
  makeCurrent();

  m_point_shader = nullptr;
  m_grid         = nullptr;
  m_sph_gizmo    = nullptr;

  if ( m_scn )
  {
    m_scn->destroyGLData();
  }
  doneCurrent();
}

void ResultViewWidget::makeConnections( void )
{
  // see the Qt doc : we must be sure that we have a direct connection to use makeCurrent in slot
  connect( context(), SIGNAL( aboutToBeDestroyed() ), this, SLOT( destroyGLData() ), Qt::DirectConnection );
}

} // namespace openMVG_gui