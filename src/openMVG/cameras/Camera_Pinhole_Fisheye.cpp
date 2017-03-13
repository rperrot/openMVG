#include "Camera_Pinhole_Fisheye.hpp"

namespace openMVG
{
namespace cameras
{


    /**
    * @brief Constructor
    * @param w Width of image plane
    * @param h Height of image plane
    * @param focal Focal distance in pixel
    * @param ppx Principal point on X-axis
    * @param ppy Principal point on Y-axis
    * @param k1 Distortion coefficient
    * @param k2 Distortion coefficient
    * @param k3 Distortion coefficient
    * @param k4 Distortion coefficient
    */
    Pinhole_Intrinsic_Fisheye::Pinhole_Intrinsic_Fisheye(
      int w , int h ,
      double focal , double ppx , double ppy ,
      double k1 , double k2 , double k3 , double k4 )
      : Pinhole_Intrinsic( w, h, focal, ppx, ppy )
    {
      params_ = {k1, k2, k3, k4};
    }

    /**
    * @brief Tell from which type the embed camera is
    * @retval PINHOLE_CAMERA_FISHEYE
    */
    EINTRINSIC Pinhole_Intrinsic_Fisheye::getType() const 
    {
      return PINHOLE_CAMERA_FISHEYE;
    }

    /**
    * @brief Does the camera model handle a distortion field?
    * @retval true
    */
    bool Pinhole_Intrinsic_Fisheye::have_disto() const 
    {
      return true;
    }

    /**
    * @brief Add the distortion field to a point (that is in normalized camera frame)
    * @param p Point before distortion computation (in normalized camera frame)
    * @return point with distortion
    */
    Vec2 Pinhole_Intrinsic_Fisheye::add_disto( const Vec2 & p ) const 
    {
      const double eps = 1e-8;
      const double k1 = params_[0], k2 = params_[1], k3 = params_[2], k4 = params_[3];
      const double r = std::sqrt( p( 0 ) * p( 0 ) + p( 1 ) * p( 1 ) );
      const double theta = std::atan( r );
      const double
      theta2 = theta * theta,
      theta3 = theta2 * theta,
      theta4 = theta2 * theta2,
      theta5 = theta4 * theta,
      theta6 = theta3 * theta3,
      theta7 = theta6 * theta,
      theta8 = theta4 * theta4,
      theta9 = theta8 * theta;
      const double theta_dist = theta + k1 * theta3 + k2 * theta5 + k3 * theta7 + k4 * theta9;
      const double inv_r = r > eps ? 1.0 / r : 1.0;
      const double cdist = r > eps ? theta_dist * inv_r : 1.0;
      return  p * cdist;
    }

    /**
    * @brief Remove the distortion to a camera point (that is in normalized camera frame)
    * @param p Point with distortion
    * @return Point without distortion
    */
    Vec2 Pinhole_Intrinsic_Fisheye::remove_disto( const Vec2 & p ) const 
    {
      const double eps = 1e-8;
      double scale = 1.0;
      const double theta_dist = std::sqrt( p[0] * p[0] + p[1] * p[1] );
      if ( theta_dist > eps )
      {
        double theta = theta_dist;
        for ( int j = 0; j < 10; ++j )
        {
          const double
            theta2 = theta * theta,
            theta4 = theta2 * theta2,
            theta6 = theta4 * theta2,
            theta8 = theta6 * theta2;
            theta = theta_dist /
                    ( 1 + params_[0] * theta2
                      + params_[1] * theta4
                      + params_[2] * theta6
                      + params_[3] * theta8 );
        }
        scale = std::tan( theta ) / theta_dist;
      }
      return p * scale;
    }

    /**
    * @brief Data wrapper for non linear optimization (get data)
    * @return vector of parameter of this intrinsic
    */
    std::vector<double> Pinhole_Intrinsic_Fisheye::getParams() const 
    {
      std::vector<double> params = Pinhole_Intrinsic::getParams();
      params.insert(params.end(), std::begin(params_), std::end(params_));
      return params;
    }

    /**
    * @brief Data wrapper for non linear optimization (update from data)
    * @param params List of params used to update this intrinsic
    * @retval true if update is correct
    * @retval false if there was an error during update
    */
    bool Pinhole_Intrinsic_Fisheye::updateFromParams( const std::vector<double> & params ) 
    {
      if ( params.size() == 7 )
      {
        *this = Pinhole_Intrinsic_Fisheye(
                  w_, h_,
                  params[0], params[1], params[2], // focal, ppx, ppy
                  params[3], params[4], params[5], params[6] ); // k1, k2, k3, k4
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
    std::vector<int> Pinhole_Intrinsic_Fisheye::subsetParameterization
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
      if ( !(param & (int)Intrinsic_Parameter_Type::ADJUST_DISTORTION)
          || param & (int)Intrinsic_Parameter_Type::NONE )
      {
        constant_index.push_back(3);
        constant_index.push_back(4);
        constant_index.push_back(5);
        constant_index.push_back(6);
      }
      return constant_index;
    }

    /**
    * @brief Return the un-distorted pixel (with removed distortion)
    * @param p Input distorted pixel
    * @return Point without distortion
    */
    Vec2 Pinhole_Intrinsic_Fisheye::get_ud_pixel( const Vec2& p ) const 
    {
      return cam2ima( remove_disto( ima2cam( p ) ) );
    }

    /**
    * @brief Return the distorted pixel (with added distortion)
    * @param p Input pixel
    * @return Distorted pixel
    */
    Vec2 Pinhole_Intrinsic_Fisheye::get_d_pixel( const Vec2& p ) const 
    {
      return cam2ima( add_disto( ima2cam( p ) ) );
    }


    /**
    * @brief Clone the object
    * @return A clone (copy of the stored object)
    */
    IntrinsicBase * Pinhole_Intrinsic_Fisheye::clone( void ) const 
    {
      return new class_type( *this );
    }

    } // namespace cameras
} // namespace openMVG
