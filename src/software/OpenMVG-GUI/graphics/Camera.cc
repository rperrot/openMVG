#include "Camera.hh"

#include "DualQuaternion.hh"

namespace openMVG_gui
{

/**
* @brief Ctr
* @param pos Position of the camera
* @param dest Destination point of the camera
* @param up Up Vector
* @param fov Field Of View (in radian)
* @param near_plane Near plane
* @param far_plane Far plane
*/
Camera::Camera( const openMVG::Vec3 & pos ,
                const openMVG::Vec3 & dest ,
                const openMVG::Vec3 & up ,
                const double fov ,
                const double near_plane ,
                const double far_plane ,
                const bool is_ortho )
  : m_pos( pos ) ,
    m_dest( dest ) ,
    m_up( up ) ,
    m_fov( fov ) ,
    m_near( near_plane ) ,
    m_far( far_plane ) ,
    m_is_ortho( false )
{

}

/**
* @brief Get camera position
* @return current position of the camera
*/
openMVG::Vec3 Camera::position( void ) const
{
  return m_pos;
}

/**
* @brief Set camera position
* @param p New position
*/
void Camera::setPosition( const openMVG::Vec3 & p )
{
  m_pos = p ;
  orthonormalize() ;
}

/**
* @brief Set camera destination
* @return current camera destination point
*/
openMVG::Vec3 Camera::destination( void ) const
{
  return m_dest ;
}

/**
* @brief Set destination point
* @param d New destination point
*/
void Camera::setDestination( const openMVG::Vec3 & d )
{
  m_dest = d ;
  orthonormalize() ;
}

/**
* @brief Get up vector
* @return Current up vector
*/
openMVG::Vec3 Camera::up( void ) const
{
  return m_up ;
}

/**
* @brief Set up vector
* @param u New up vector
*/
void Camera::setUp( const openMVG::Vec3 & u )
{
  m_up = u ;
  orthonormalize() ;
}

/**
* @brief get Near plane
* @return Current near plane
*/
double Camera::nearPlane( void ) const
{
  return m_near ;
}

/**
* @brief Set near plane
* @param near New near plane
*/
void Camera::setNearPlane( const double near )
{
  m_near = near ;
}

/**
* @brief Get Far plane
* @return Current far plane
*/
double Camera::farPlane( void ) const
{
  return m_far ;
}

/**
* @brief Set Far plane
* @param far New far plane
*/
void Camera::setFarPlane( const double far )
{
  m_far = far ;
}

/**
* @brief Get Field Of View
* @return FOV
* @TODO: precise if it's horizontal or vertical FOV
* @note FOV is in radian
*/
double Camera::fov( void ) const
{
  return m_fov ;
}

/**
* @brief Set Field of View
* @param fov New FOV
* @note fov must be defined in radian
*/
void Camera::setFov( const double fov )
{
  m_fov = fov ;
}

/**
* @brief Get view matrix for the current parameters
* @return View matrix
* @note It should be directly used in openGL (no transpose needed)
*/
openMVG::Mat4 Camera::viewMatrix( void ) const
{
  const openMVG::Vec3 z = ( m_pos - m_dest ).normalized() ;
  const openMVG::Vec3 x = ( m_up.cross( z ) ).normalized() ;
  const openMVG::Vec3 y = z.cross( x ) ;

  openMVG::Mat4 res ;

  res( 0 , 0 ) = x[0] ;
  res( 0 , 1 ) = y[0] ;
  res( 0 , 2 ) = z[0] ;
  res( 0 , 3 ) = 0.0 ;

  res( 1 , 0 ) = x[1] ;
  res( 1 , 1 ) = y[1] ;
  res( 1 , 2 ) = z[1] ;
  res( 1 , 3 ) = 0.0 ;

  res( 2 , 0 ) = x[2] ;
  res( 2 , 1 ) = y[2] ;
  res( 2 , 2 ) = z[2] ;
  res( 2 , 3 ) = 0.0 ;

  res( 3 , 0 ) = - x.dot( m_pos ) ;
  res( 3 , 1 ) = - y.dot( m_pos ) ;
  res( 3 , 2 ) = - z.dot( m_pos ) ;
  res( 3 , 3 ) = 1.0 ;

  return res ;
}

/**
* @brief Get projection matrix for the current parameters
* @param aspect Aspect ratio (w/h)
* @return Projection matrix
* @note It should be directly used in openGL (no transpose needed)
*/
openMVG::Mat4 Camera::projMatrix( const double width , const double height ) const
{
  if( ! m_is_ortho )
  {
    const double aspect = width / height ;
    const double range = std::tan( m_fov / 2.0 ) * m_near ;

    const double left = - range * aspect ;
    const double right = range * aspect ;
    const double bottom = -range ;
    const double top = range ;

    const double tmp = 2.0 * m_near ;

    openMVG::Mat4 res ;

    res( 0 , 0 ) = tmp / ( right - left ) ;
    res( 0 , 1 ) = 0.0 ;
    res( 0 , 2 ) = 0.0 ;
    res( 0 , 3 ) = 0.0 ;

    res( 1 , 0 ) = 0.0 ;
    res( 1 , 1 ) = tmp / ( top - bottom ) ;
    res( 1 , 2 ) = 0.0 ;
    res( 1 , 3 ) = 0.0 ;

    res( 2 , 0 ) = ( right + left ) / ( right - left ) ;
    res( 2 , 1 ) = ( top + bottom ) / ( top - bottom ) ;
    res( 2 , 2 ) = ( - m_far - m_near ) / ( m_far - m_near ) ;
    res( 2 , 3 ) = -1.0 ;

    res( 3 , 0 ) = 0.0 ;
    res( 3 , 1 ) = 0.0 ;
    res( 3 , 2 ) = ( - tmp * m_far ) / ( m_far - m_near ) ;
    res( 3 , 3 ) = 0.0 ;

    return res ;
  }
  else
  {
    const double left = -width / height ;
    const double right = width / height ;
    const double bottom = -1.0 ;
    const double top = 1.0 ;

    const double sum1 = left + right ;
    const double sum2 = top + bottom ;
    const double sum3 = m_far + m_near ;

    const double diff1 = right - left ;
    const double diff2 = top - bottom ;
    const double diff3 = m_far - m_near ;

    const double tx = - sum1 / diff1 ;
    const double ty = - sum2 / diff2 ;
    const double tz = - sum3 / diff3 ;

    openMVG::Mat4 res ;

    res( 0 , 0 ) = 2.0 / diff1 ;
    res( 0 , 1 ) = 0.0 ;
    res( 0 , 2 ) = 0.0 ;
    res( 0 , 3 ) = tx ;

    res( 1 , 0 ) = 0.0 ;
    res( 1 , 1 ) = 2.0 / diff2 ;
    res( 1 , 2 ) = 0.0 ;
    res( 1 , 3 ) = ty ;

    res( 2 , 0 ) = 0.0 ;
    res( 2 , 1 ) = 0.0 ;
    res( 2 , 2 ) = - 2.0 / diff3 ;
    res( 2 , 3 ) = tz ;

    res( 3 , 0 ) = 0.0 ;
    res( 3 , 1 ) = 0.0 ;
    res( 3 , 2 ) = 0.0 ;
    res( 3 , 3 ) = 1.0 ;

    return res.transpose() ;
  }
}

/**
* @brief center camera to a specified position while keeping orientation frame
* @param nCenter new center
* @note this move position and destination
* @note at final, dest is at new center and position is move to keep direction vector the same
*/
void Camera::centerTo( const openMVG::Vec3 & nCenter )
{
  const openMVG::Vec3 oldDir = m_dest - m_pos ;
  m_dest = nCenter ;
  m_pos = m_dest + oldDir ;
}

/**
* @brief Set camera to fit a specific sphere
* @param center Center of the sphere
* @param rad Radius of the sphere
*/
void Camera::fitBoundingSphere( const openMVG::Vec3 & center , const double rad )
{
  // Compute distance that the camera should be to the center of the scene to fit the bounding box
  // this is directly related to the fov
  const double distance = rad / std::tan( m_fov / 2.0 ) ;
  const openMVG::Vec3 oldDirNz = ( m_dest - m_pos ).normalized() ;
  // New center is the center of the BS
  m_dest = center ;
  // Keep same orientation, but move such as to see all the sphere
  m_pos = m_dest - oldDirNz * distance ;
}

/**
* @brief Zoom in (or out )
* @param factor zooming factor
*/
void Camera::zoom( const double factor )
{
  const openMVG::Vec3 dir = m_dest - m_pos ;
  const double n = dir.norm() ;
  const openMVG::Vec3 nDir = dir / n ;

  // Compute new position
  openMVG::Vec3 nPos = m_pos + nDir * factor * n / 10.0 ;
  const openMVG::Vec3 newDir = ( m_dest - nPos ).normalized() ;
  // If if change orientation, do nothing
  if( newDir.dot( dir ) < 0.0 )
  {
    return ;
  }
  m_pos = nPos ;
}

/**
* @brief Move dest and dir from this vector
*/
void Camera::pan( const openMVG::Vec3 & delta )
{
  m_dest += delta ;
  m_pos += delta ;
  orthonormalize() ; 
}

/**
* @brief Move position and update up vector accordingly
*/
void Camera::rotateAroundDestination( const openMVG::Vec3 & axis , const double angle )
{
  const DualQuaternion dqinv = DualQuaternion::translation( - m_dest ) ;
  const DualQuaternion dqrot = DualQuaternion::rotation( axis , angle ) ;
  const DualQuaternion dq    = DualQuaternion::translation( m_dest ) ;

  const DualQuaternion transform = dq * dqrot * dqinv ;

  m_pos = transform.applyPoint( m_pos ) ;
  m_up = transform.applyVector( m_up ) ;

  orthonormalize() ;
}


/**
* @brief Given a screen position, gets it's corresponding 3d point
* @param pt Point to transform (screen_x,screen_y,depth)
* @param viewport [start_x,start_y,width,height]
* @note this is similar to gluUnproject
*/
openMVG::Vec3 Camera::unProject( const openMVG::Vec3 & pt ,
                                 const double viewport[4] ) const
{
  const openMVG::Mat4 view = viewMatrix() ;
  const openMVG::Mat4 proj = projMatrix( viewport[2] , viewport[3] ) ;

  // Note because matrices are left handed, proj * view -> view * proj * transpose
  const openMVG::Mat4 inv = ( view * proj ).transpose().inverse();

  openMVG::Vec4 tmp ;

  // Rescale to [-1;1]
  tmp[0] = 2.0 * ( pt[0] - viewport[0] ) / viewport[2] - 1.0 ;
  tmp[1] = 2.0 * ( pt[1] - viewport[1] ) / viewport[3] - 1.0 ;
  tmp[2] = 2.0 * pt[2] - 1.0 ;
  tmp[3] = 1.0 ;

  openMVG::Vec4 res = inv * tmp ;
  res /= res[3] ;

  return openMVG::Vec3( res[0] , res[1] , res[2] ) ;
}

/**
* @brief Indicate if it's an orthographic camera
* @retval true if it's ortho
* @retval false if not
*/
bool Camera::isOrtho( void ) const
{
  return m_is_ortho ;
}

/**
* @brief Set orthographic mode
* @param iso orthographic activation mode
*/
void Camera::setOrtho( const bool iso )
{
  m_is_ortho = iso ;
}

/**
* @brief Orthonormalize frame
* ie: reset up vector to make an orthonormal frame
*/
void Camera::orthonormalize( void )
{
  const openMVG::Vec3 dir = ( m_dest - m_pos ).normalized() ;
  openMVG::Vec3 right = ( dir.cross( m_up ) ).normalized() ;
  m_up = right.cross( dir ) ;
}


} // namespace openMVG_gui