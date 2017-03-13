#include "Camera_Intrinsics.hpp"

namespace openMVG
{
namespace cameras
{
    /**
  * @brief Constructor
  * @param w Width of the image
  * @param h Height of the image
  */
  IntrinsicBase::IntrinsicBase( unsigned int w , unsigned int h )
    : w_( w ),
      h_( h )
  {

  }

  /**
  * @brief Get width of the image
  * @return width of the image
  */
  unsigned int IntrinsicBase::w() const
  {
    return w_;
  }

  /**
  * @brief Get height of the image
  * @return height of the image
  */
  unsigned int IntrinsicBase::h() const
  {
    return h_;
  }

  /**
  * @brief Compute projection of a 3D point into the image plane
  * (Apply pose, disto (if any) and Intrinsics)
  * @param pose Pose used to compute projection
  * @param pt3D 3D-point to project on image plane
  * @return Projected (2D) point on image plane
  */
  Vec2 IntrinsicBase::project(
    const geometry::Pose3 & pose,
    const Vec3 & pt3D ) const
  {
    const Vec3 X = pose( pt3D ); // apply pose
    if ( this->have_disto() ) // apply disto & intrinsics
    {
      return this->cam2ima( this->add_disto( X.hnormalized() ) );
    }
    else // apply intrinsics
    {
      return this->cam2ima( X.hnormalized() );
    }
  }

  /**
  * @brief Compute the residual between the 3D projected point and an image observation
  * @param pose Pose used to project point on camera plane
  * @param X 3d point to project on camera plane
  * @param x image observation
  * @brief Relative 2d distance between projected and observed points
  */
  Vec2 IntrinsicBase::residual(
    const geometry::Pose3 & pose,
    const Vec3 & X,
    const Vec2 & x ) const
  {
    const Vec2 proj = this->project( pose, X );
    return x - proj;
  }

  /**
  * @brief Does the camera model handle a distortion field?
  * @retval true if intrinsic holds distortion
  * @retval false if intrinsic does not hold distortion
  */
  bool IntrinsicBase::have_disto() const
  {
    return false;
  }


  /**
  * @brief Generate a unique Hash from the camera parameters (used for grouping)
  * @return Hash value
  */
  std::size_t IntrinsicBase::hashValue() const
  {
    size_t seed = 0;
    stl::hash_combine( seed, static_cast<int>( this->getType() ) );
    stl::hash_combine( seed, w_ );
    stl::hash_combine( seed, h_ );
    const std::vector<double> params = this->getParams();
    for ( const auto & param : params )
      stl::hash_combine( seed , param );
    return seed;
  }


/**
* @brief Compute angle between two bearing rays
* Bearing rays are computed from position on image plane in each cameras
*
* @param pose1 Pose of the first camera
* @param intrinsic1 Intrinsic of the first camera
* @param pose2 Pose of the second camera
* @param intrinsic2 Intrinsic of the second camera
* @param x1 Image coordinate of a point in first camera
* @param x2 Image coordinate of a point in the second camera
*
* @return Angle (in degree) between the two rays
*/
double AngleBetweenRay(
  const geometry::Pose3 & pose1,
  const IntrinsicBase * intrinsic1,
  const geometry::Pose3 & pose2,
  const IntrinsicBase * intrinsic2,
  const Vec2 & x1, const Vec2 & x2 )
{
  // x = (u, v, 1.0)  // image coordinates
  // X = R.t() * K.inv() * x + C // Camera world point
  // getting the ray:
  // ray = X - C = R.t() * K.inv() * x
  const Vec3 ray1 = ( pose1.rotation().transpose() * intrinsic1->operator()( x1 ) ).normalized();
  const Vec3 ray2 = ( pose2.rotation().transpose() * intrinsic2->operator()( x2 ) ).normalized();
  const double mag = ray1.norm() * ray2.norm();
  const double dotAngle = ray1.dot( ray2 );
  return R2D( acos( clamp( dotAngle / mag, -1.0 + 1.e-8, 1.0 - 1.e-8 ) ) );
}


} // namespace cameras
} // namespace openMVG
