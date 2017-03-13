#include "frustum.hpp"

#include <fstream>
#include <iomanip>


namespace openMVG
{

/**
* @brief Namespace holding various classes and methods for geometry manipulation
*/
namespace geometry
{

  /**
  * @brief Default constructor
  */
  Frustum::Frustum()
    : z_near( -1. ),
      z_far ( -1. )
  {

  }


  /**
  * @brief Build a frustum from the image size, camera intrinsic and pose
  * @param w Width of image plane
  * @param h Height of image plane
  * @param K Intrinsic matrix
  * @param R Extrinsic rotation matrix
  * @param C Center of the camera (optical center)
  */
  Frustum::Frustum
  (
    const int w,
    const int h,
    const Mat3 & K,
    const Mat3 & R,
    const Vec3 & C
  )
    : z_near( -1. ),
      z_far ( -1. )
  {
    const Mat3 Kinv = K.inverse();
    const Mat3 Rt = R.transpose();

    // Definition of the frustum with the supporting points
    cones[0] = C;
    cones[1] = Rt * ( ( Kinv * Vec3( 0, 0, 1.0 ) ) ) + C;
    cones[2] = Rt * ( ( Kinv * Vec3( w, 0, 1.0 ) ) ) + C;
    cones[3] = Rt * ( ( Kinv * Vec3( w, h, 1.0 ) ) ) + C;
    cones[4] = Rt * ( ( Kinv * Vec3( 0, h, 1.0 ) ) ) + C;

    // Definition of the supporting planes
    planes.push_back( Half_plane_p( cones[0], cones[4], cones[1] ) );
    planes.push_back( Half_plane_p( cones[0], cones[1], cones[2] ) );
    planes.push_back( Half_plane_p( cones[0], cones[2], cones[3] ) );
    planes.push_back( Half_plane_p( cones[0], cones[3], cones[4] ) );

    // supporting point for drawing is a normalized cone, since infinity cannot be represented
    points = std::vector<Vec3>( &cones[0], &cones[0] + 5 );
  }

  /**
  * @brief Build a frustum from the image size, camera intrinsic and pose
  * @param w Width of image plane
  * @param h Height of image plane
  * @param K Intrinsic matrix
  * @param R Extrinsic rotation matrix
  * @param C Center of the camera (optical center)
  * @param Specify a far plane 
  */
  Frustum::Frustum
  (
    const int w,
    const int h,
    const Mat3 & K,
    const Mat3 & R,
    const Vec3 & C ,
    const double zFar 
  )
    : z_near( -1. ),
      z_far ( zFar )
  {
    const Mat3 Kinv = K.inverse();
    const Mat3 Rt = R.transpose();

    // Definition of the frustum with the supporting points
    cones[0] = C;
    cones[1] = Rt * ( z_far * ( Kinv * Vec3( 0, 0, 1.0 ) ) ) + C;
    cones[2] = Rt * ( z_far * ( Kinv * Vec3( w, 0, 1.0 ) ) ) + C;
    cones[3] = Rt * ( z_far * ( Kinv * Vec3( w, h, 1.0 ) ) ) + C;
    cones[4] = Rt * ( z_far * ( Kinv * Vec3( 0, h, 1.0 ) ) ) + C;

    // Definition of the supporting planes
    planes.push_back( Half_plane_p( cones[0], cones[4], cones[1] ) );
    planes.push_back( Half_plane_p( cones[0], cones[1], cones[2] ) );
    planes.push_back( Half_plane_p( cones[0], cones[2], cones[3] ) );
    planes.push_back( Half_plane_p( cones[0], cones[3], cones[4] ) );

    // supporting point for drawing is a normalized cone, since infinity cannot be represented
    points = std::vector<Vec3>( &cones[0], &cones[0] + 5 );
  }


  /**
  * @brief Build a frustum from image size, camera intrinsics, pose and clip planes
  * @param w Width of image plane
  * @param h Height of image plane
  * @param K Intrinsic matrix
  * @param R Extrinsic rotation matrix
  * @param C Center of the camera (optical center)
  * @param zNear Near clip plane distance
  * @param zFar Far clip plane distance
  */
  Frustum::Frustum
  (
    const int w,
    const int h,
    const Mat3 & K,
    const Mat3 & R,
    const Vec3 & C,
    const double zNear,
    const double zFar
  )
  {
    *this = Frustum( w, h, K, R, C );

    // update near & far planes & clear set points
    z_near = zNear;
    z_far = zFar;
    points.clear();
    assert( zFar > zNear );

    // Add Znear and ZFar half plane using the cam looking direction
    const Vec3 camLookDirection_n = R.row( 2 ).normalized();
    const double d_near = - zNear - camLookDirection_n.dot( C );
    planes.push_back( Half_plane( camLookDirection_n, d_near ) );

    const double d_Far = zFar + camLookDirection_n.dot( C );
    planes.push_back( Half_plane( -camLookDirection_n, d_Far ) );

    // supporting point are the points defined by the truncated cone
    const Mat3 Kinv = K.inverse();
    const Mat3 Rt = R.transpose();
    points.push_back( Rt * ( z_near * ( Kinv * Vec3( 0, 0, 1.0 ) ) ) + C );
    points.push_back( Rt * ( z_near * ( Kinv * Vec3( w, 0, 1.0 ) ) ) + C );
    points.push_back( Rt * ( z_near * ( Kinv * Vec3( w, h, 1.0 ) ) ) + C );
    points.push_back( Rt * ( z_near * ( Kinv * Vec3( 0, h, 1.0 ) ) ) + C );

    points.push_back( Rt * ( z_far * ( Kinv * Vec3( 0, 0, 1.0 ) ) ) + C );
    points.push_back( Rt * ( z_far * ( Kinv * Vec3( w, 0, 1.0 ) ) ) + C );
    points.push_back( Rt * ( z_far * ( Kinv * Vec3( w, h, 1.0 ) ) ) + C );
    points.push_back( Rt * ( z_far * ( Kinv * Vec3( 0, h, 1.0 ) ) ) + C );
  }



    /**
  * @brief Tells if the Frustum is an infinite one
  * @retval true If frustum is infinite
  * @retval false If frustum is not infinite
  */
  bool Frustum::isInfinite() const
  {
    return planes.size() == 4;
  }

  /**
  * @brief Tells if the Frustum is truncated
  * @retval true If frustum is truncated
  * @retval false If frustum is not truncated
  */
  bool Frustum::isTruncated() const
  {
    return planes.size() == 6;
  }

  /**
  * @brief Return the supporting frustum points
  * @return Supporting frustum points
  * @note 5 points for infinite frustum
  * @note 8 points for truncated frustum
  */
  const std::vector<Vec3> & Frustum::frustum_points() const
  {
    return points;
  }

  /**
  * @brief Export the Frustum as a PLY file (infinite frustum as exported as a normalized cone)
  * @return true if the file can be saved on disk
  */
  bool Frustum::export_Ply
  (
    const Frustum & frustum,
    const std::string & filename
  )
  {
    std::ofstream of(filename.c_str());
    if (!of.is_open())
      return false;

    // Vertex count evaluation
    const size_t vertex_count = frustum.frustum_points().size();
    // Faces count evaluation
    const size_t face_count = frustum.isInfinite() ? 4 : 6;

    of << std::fixed << std::setprecision (std::numeric_limits<double>::digits10 + 1);
    of << "ply" << '\n'
      << "format ascii 1.0" << '\n'
      << "element vertex " << vertex_count << '\n'
      << "property double x" << '\n'
      << "property double y" << '\n'
      << "property double z" << '\n'
      << "element face " << face_count << '\n'
      << "property list uchar int vertex_index" << '\n'
      << "end_header" << '\n';

    // Export frustums points
    {
      const std::vector<Vec3> & points = frustum.frustum_points();
      for (size_t i = 0; i < points.size(); ++i)
        of << points[i].transpose() << '\n';
    }

    // Export frustums faces
    {
      if (frustum.isInfinite())
      {
        // infinite frustum: drawn normalized cone: 4 faces
        of
          << "3 0 4 1\n"
          << "3 0 1 2\n"
          << "3 0 2 3\n"
          << "3 0 3 4\n";
      }
      else
      {
        of
          << "4 0 1 2 3\n"
          << "4 0 1 5 4\n"
          << "4 1 5 6 2\n"
          << "4 3 7 6 2\n"
          << "4 0 4 7 3\n"
          << "4 4 5 6 7\n";
      }
    }
    of.flush();
    const bool bOk = of.good();
    of.close();
    return bOk;
  }

} // namespace geometry
} // namespace openMVG
