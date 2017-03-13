#include "PinholeCamera.hpp"

#include "openMVG/multiview/projection.hpp"

namespace openMVG
{
namespace cameras
{

  PinholeCamera::PinholeCamera(
      const Mat3 &K,
      const Mat3 &R,
      const Vec3 &t )
      : _K( K ), _R( R ), _t( t )
  {
    _C = -R.transpose() * t;
    P_From_KRt( _K, _R, _t, &_P );
  }

  PinholeCamera::PinholeCamera( const Mat34 &P )
  {
    _P = P;
    KRt_From_P( _P, &_K, &_R, &_t );
    _C = -_R.transpose() * _t;
  }

  /// Projection of a 3D point into the camera plane
  Vec2 PinholeCamera::Project( const Mat34 &P, const Vec3 &pt3D )
  {
    return openMVG::Project( P, pt3D );
  }

  /// Projection of a 3D point into the camera plane (member function)
  Vec2 PinholeCamera::Project( const Vec3 &pt3D ) const
  {
    return openMVG::Project( _P, pt3D );
  }

  /// Return the residual value to the given 2d point
  double PinholeCamera::Residual(
      const Mat34 &P,
      const Vec3 &pt3D,
      const Vec2 &ref )
  {
    return ( ref - openMVG::Project( P, pt3D ) ).norm();
  }

  /// Return the residual value to the given 2d point
  double PinholeCamera::Residual( const Vec3 &pt3D, const Vec2 &ref ) const
  {
    return ( ref - openMVG::Project( _P, pt3D ) ).norm();
  }

  double PinholeCamera::ResidualSquared( const Vec3 &pt3D, const Vec2 &ref ) const
  {
    return ( ref - openMVG::Project( _P, pt3D ) ).squaredNorm();
  }

  // Compute the depth of the X point. R*X[2]+t[2].
  double PinholeCamera::Depth( const Vec3 &X ) const
  {
    return openMVG::Depth( _R, _t, X );
  }

  /// Return the angle (degree) between two pinhole point rays
  double PinholeCamera::AngleBetweenRay(
      const PinholeCamera &cam1,
      const PinholeCamera &cam2,
      const Vec2 &x1, const Vec2 &x2 )
  {
    // x = (u, v, 1.0)  // image coordinates
    // X = R.t() * K.inv() * x + C // Camera world point
    // getting the ray:
    // ray = X - C = R.t() * K.inv() * x
    Vec3 ray1 = ( cam1._R.transpose() *
                  ( cam1._K.inverse() * Vec3( x1( 0 ), x1( 1 ), 1. ) ) )
                    .normalized();
    Vec3 ray2 = ( cam2._R.transpose() *
                  ( cam2._K.inverse() * Vec3( x2( 0 ), x2( 1 ), 1. ) ) )
                    .normalized();
    double mag      = ray1.norm() * ray2.norm();
    double dotAngle = ray1.dot( ray2 );
    return R2D( acos( clamp( dotAngle / mag, -1.0 + 1.e-8, 1.0 - 1.e-8 ) ) );
  }

} // namespace cameras
} // namespace openMVG
