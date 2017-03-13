
// Copyright (c) 2012, 2013 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_CAMERAS_CAMERA_PINHOLECAMERA_HPP
#define OPENMVG_CAMERAS_CAMERA_PINHOLECAMERA_HPP

#include "openMVG/numeric/numeric.h"

namespace openMVG
{
namespace cameras
{

  /// Pinhole camera P = K[R|t], t = -RC
  struct PinholeCamera
  {
    PinholeCamera(
        const Mat3 &K = Mat3::Identity(),
        const Mat3 &R = Mat3::Identity(),
        const Vec3 &t = Vec3::Zero() );

    PinholeCamera( const Mat34 &P );

    /// Projection of a 3D point into the camera plane
    static Vec2 Project( const Mat34 &P, const Vec3 &pt3D );

    /// Projection of a 3D point into the camera plane (member function)
    Vec2 Project( const Vec3 &pt3D ) const;

    /// Return the residual value to the given 2d point
    static double Residual(
        const Mat34 &P,
        const Vec3 &pt3D,
        const Vec2 &ref );

    /// Return the residual value to the given 2d point
    double Residual( const Vec3 &pt3D, const Vec2 &ref ) const;

    double ResidualSquared( const Vec3 &pt3D, const Vec2 &ref ) const;

    // Compute the depth of the X point. R*X[2]+t[2].
    double Depth( const Vec3 &X ) const;

    /// Return the angle (degree) between two pinhole point rays
    static double AngleBetweenRay(
        const PinholeCamera &cam1,
        const PinholeCamera &cam2,
        const Vec2 &x1, const Vec2 &x2 );

    /// Projection matrix P = K[R|t]
    Mat34 _P;

    /// Intrinsic parameter (Focal, principal point)
    Mat3 _K;

    /// Extrinsic Rotation
    Mat3 _R;

    /// Extrinsic translation
    Vec3 _t;

    /// Camera center
    Vec3 _C;
  };

} // namespace cameras
} // namespace openMVG

#endif // #ifndef OPENMVG_CAMERAS_CAMERA_PINHOLECAMERA_HPP
