#include "pose3.hpp"

#include "openMVG/multiview/projection.hpp"


namespace openMVG
{
namespace geometry
{

/**
    * @brief Default constructor
    * @note This defines a Null transform (aligned with cartesian frame, centered at origin)
    */
Pose3::Pose3()
    : rotation_( Mat3::Identity() ),
      center_( Vec3::Zero() )
{
}

/**
    * @brief Constructor
    * @param r Rotation
    * @param c Center
    */
Pose3::Pose3( const Mat3 &r, const Vec3 &c )
    : rotation_( r ),
      center_( c )
{
}


    /**
    * @brief Get Rotation matrix
    * @return Rotation matrix
    */
    const Mat3& Pose3::rotation() const
    {
      return rotation_;
    }

    /**
    * @brief Get Rotation matrix
    * @return Rotation matrix
    */
    Mat3& Pose3::rotation()
    {
      return rotation_;
    }

    /**
    * @brief Get center of rotation
    * @return center of rotation
    */
    const Vec3& Pose3::center() const
    {
      return center_;
    }

    /**
    * @brief Get center of rotation
    * @return Center of rotation
    */
    Vec3& Pose3::center()
    {
      return center_;
    }

    /**
    * @brief Get translation vector
    * @return translation vector
    * @note t = -RC
    */
    Vec3 Pose3::translation() const
    {
      return -( rotation_ * center_ );
    }


    /**
    * @brief Apply pose
    * @param p Point
    * @return transformed point
    */
    Mat3X Pose3::operator () ( const Mat3X& p ) const
    {
      return rotation_ * ( p.colwise() - center_ );
    }


    /**
    * @brief Composition of poses
    * @param P a Pose
    * @return Composition of current pose and parameter pose
    */
    Pose3 Pose3::operator * ( const Pose3& P ) const
    {
      return Pose3( rotation_ * P.rotation_, P.center_ + P.rotation_.transpose() * center_ );
    }


    /**
    * @brief Get inverse of the pose
    * @return Inverse of the pose
    */
    Pose3 Pose3::inverse() const
    {
      return Pose3( rotation_.transpose(),  -( rotation_ * center_ ) );
    }


    /**
    * @brief Return the depth (distance) of a point respect to the camera center
    * @param X Input point
    * @return Distance to center
    */
    double Pose3::depth( const Vec3 &X ) const
    {
      return ( rotation_ * ( X - center_ ) )[2];
    }

} // namespace geometry
} // namespace openMVG
