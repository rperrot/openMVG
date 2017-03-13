#include "Camera_Pinhole.hpp"

#include "openMVG/multiview/projection.hpp"

namespace openMVG
{
namespace cameras
{


    /**
    * @brief Constructor
    * @param w Width of the image plane
    * @param h Height of the image plane
    * @param focal_length_pix Focal length (in pixel) of the camera
    * @param ppx Principal point on x-axis
    * @param ppy Principal point on y-axis
    */
    Pinhole_Intrinsic::Pinhole_Intrinsic(
      unsigned int w , unsigned int h ,
      double focal_length_pix ,
      double ppx , double ppy )
      : IntrinsicBase( w, h )
    {
      K_ << focal_length_pix, 0., ppx, 0., focal_length_pix, ppy, 0., 0., 1.;
      Kinv_ = K_.inverse();
    }

    /**
    * @brief Constructor
    * @param w Width of the image plane
    * @param h Height of the image plane
    * @param K Intrinsic Matrix (3x3) {f,0,ppx; 0,f,ppy; 0,0,1}
    */
    Pinhole_Intrinsic::Pinhole_Intrinsic(
      unsigned int w,
      unsigned int h,
      const Mat3& K)
      : IntrinsicBase( w, h ), K_(K)
    {
      K_(0,0) = K_(1,1) = (K(0,0) + K(1,1)) / 2.0;
      Kinv_ = K_.inverse();
    }

    /**
    * @brief Get type of the intrinsic
    * @retval PINHOLE_CAMERA
    */
    EINTRINSIC Pinhole_Intrinsic::getType() const 
    {
      return PINHOLE_CAMERA;
    }

    /**
    * @brief Get the intrinsic matrix
    * @return 3x3 intrinsic matrix
    */
    const Mat3& Pinhole_Intrinsic::K() const
    {
      return K_;
    }

    /**
    * @brief Get the inverse of the intrinsic matrix
    * @return Inverse of intrinsic matrix
    */
    const Mat3& Pinhole_Intrinsic::Kinv() const
    {
      return Kinv_;
    }


    /**
    * @brief Return the value of the focal in pixels
    * @return Focal of the camera (in pixel)
    */
    double Pinhole_Intrinsic::focal() const
    {
      return K_( 0, 0 );
    }

    /**
    * @brief Get principal point of the camera
    * @return Principal point of the camera
    */
    Vec2 Pinhole_Intrinsic::principal_point() const
    {
      return Vec2( K_( 0, 2 ), K_( 1, 2 ) );
    }


    /**
    * @brief Get bearing vector of a point given an image coordinate
    * @return bearing vector
    */
    Vec3 Pinhole_Intrinsic::operator () ( const Vec2& p ) const 
    {
      Vec3 p3( p( 0 ), p( 1 ), 1.0 );
      return ( Kinv_ * p3 ).normalized();
    }

    /**
    * @brief Transform a point from the camera plane to the image plane
    * @param p Camera plane point
    * @return Point on image plane
    */
    Vec2 Pinhole_Intrinsic::cam2ima( const Vec2& p ) const 
    {
      return focal() * p + principal_point();
    }

    /**
    * @brief Transform a point from the image plane to the camera plane
    * @param p Image plane point
    * @return camera plane point
    */
    Vec2 Pinhole_Intrinsic::ima2cam( const Vec2& p ) const 
    {
      return ( p -  principal_point() ) / focal();
    }

    /**
    * @brief Does the camera model handle a distortion field?
    * @retval false if intrinsic does not hold distortion
    */
    bool Pinhole_Intrinsic::have_disto() const 
    {
      return false;
    }

    /**
    * @brief Add the distortion field to a point (that is in normalized camera frame)
    * @param p Point before distortion computation (in normalized camera frame)
    * @return point with distortion
    */
    Vec2 Pinhole_Intrinsic::add_disto( const Vec2& p ) const 
    {
      return p;
    }

    /**
    * @brief Remove the distortion to a camera point (that is in normalized camera frame)
    * @param p Point with distortion
    * @return Point without distortion
    */
    Vec2 Pinhole_Intrinsic::remove_disto( const Vec2& p ) const 
    {
      return p;
    }

    /**
    * @brief Normalize a given unit pixel error to the camera plane
    * @param value Error in image plane
    * @return error of passing from the image plane to the camera plane
    */
    double Pinhole_Intrinsic::imagePlane_toCameraPlaneError( double value ) const 
    {
      return value / focal();
    }

    /**
    * @brief Return the projection matrix (interior & exterior) as a simplified projective projection
    * @param pose Extrinsic matrix
    * @return Concatenation of intrinsic matrix and extrinsic matrix
    */
    Mat34 Pinhole_Intrinsic::get_projective_equivalent( const geometry::Pose3 & pose ) const 
    {
      Mat34 P;
      P_From_KRt( K(), pose.rotation(), pose.translation(), &P );
      return P;
    }


    /**
    * @brief Data wrapper for non linear optimization (get data)
    * @return vector of parameter of this intrinsic
    */
    std::vector<double> Pinhole_Intrinsic::getParams() const 
    {
      return  { K_(0, 0), K_(0, 2), K_(1, 2) };
    }


    /**
    * @brief Data wrapper for non linear optimization (update from data)
    * @param params List of params used to update this intrinsic
    * @retval true if update is correct
    * @retval false if there was an error during update
    */
    bool Pinhole_Intrinsic::updateFromParams(const std::vector<double> & params) 
    {
      if ( params.size() == 3 )
      {
        *this = Pinhole_Intrinsic( w_, h_, params[0], params[1], params[2] );
        return true;
      }
      else
      {
        return false;
      }
    }

    /**
    * @brief Return the list of parameter indexes that must be held constant
    * @param parametrization The given parametrization
    */
    std::vector<int> Pinhole_Intrinsic::subsetParameterization
    (
      const Intrinsic_Parameter_Type & parametrization) const 
    {
      std::vector<int> constant_index;
      const int param = static_cast<int>(parametrization);
      if ( !(param & (int)Intrinsic_Parameter_Type::ADJUST_FOCAL_LENGTH)
           || param & (int)Intrinsic_Parameter_Type::NONE )
      {
        constant_index.push_back(0);
      }
      if ( !(param & (int)Intrinsic_Parameter_Type::ADJUST_PRINCIPAL_POINT)
          || param & (int)Intrinsic_Parameter_Type::NONE )
      {
        constant_index.push_back(1);
        constant_index.push_back(2);
      }
      return constant_index;
    }

    /**
    * @brief Return the un-distorted pixel (with removed distortion)
    * @param p Input distorted pixel
    * @return Point without distortion
    */
    Vec2 Pinhole_Intrinsic::get_ud_pixel( const Vec2& p ) const 
    {
      return p;
    }

    /**
    * @brief Return the distorted pixel (with added distortion)
    * @param p Input pixel
    * @return Distorted pixel
    */
    Vec2 Pinhole_Intrinsic::get_d_pixel( const Vec2& p ) const 
    {
      return p;
    }


    /**
    * @brief Clone the object
    * @return A clone (copy of the stored object)
    */
    IntrinsicBase * Pinhole_Intrinsic::clone( void ) const 
    {
      return new class_type( *this );
    }

  } // namespace cameras
} // namespace openMVG
