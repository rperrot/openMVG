#include "Similarity3.hpp"

namespace openMVG
{
namespace geometry
{


  /**
  * @brief Default constructor
  * @note This define identity transformation centered at origin of a cartesian frame
  */
  Similarity3::Similarity3()
    : pose_( Pose3() ),
      scale_( 1.0 )
  {

  }

  /**
  * @brief Constructor
  * @param pose a 3d pose
  * @param scale a scale factor
  */
  Similarity3::Similarity3( const Pose3 & pose, const double scale )
    : pose_( pose ),
      scale_( scale )
  {

  }

  /**
  * @brief Apply transformation to a point
  * @param point Input point
  * @return transformed point
  */
  Mat3X Similarity3::operator () ( const Mat3X & point ) const
  {
    return scale_ * pose_( point );
  }

  /**
  * @brief Concatenation of pose
  * @param pose Pose to be concatenated with the current one
  * @return Concatenation of poses
  */
  Pose3 Similarity3::operator () ( const Pose3 & pose ) const
  {
    return Pose3( pose.rotation() * pose_.rotation().transpose(), this->operator()( pose.center() ) );
  }

  /**
  * @brief Get inverse of the similarity
  * @return Inverse of the similarity
  */
  Similarity3 Similarity3::inverse() const
  {
    return Similarity3(pose_.inverse(), 1.0 / scale_);
  }

  } // namespace geometry
} // namespace openMVG
