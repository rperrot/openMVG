#include "Camera_Pinhole_Radial.hpp"

namespace openMVG
{
namespace cameras
{


    /**
    * @brief Constructor
    * @param w Width of the image
    * @param h Height of the image
    * @param focal Focal (in pixel) of the camera
    * @param ppx Principal point on X-Axis
    * @param ppy Principal point on Y-Axis
    * @param k1 Distortion coefficient
    */
    Pinhole_Intrinsic_Radial_K1::Pinhole_Intrinsic_Radial_K1(
      int w , int h ,
      double focal , double ppx , double ppy ,
      double k1 )
      : Pinhole_Intrinsic( w, h, focal, ppx, ppy )
    {
      params_ = {k1};
    }

    /**
    * @brief Tell from which type the embed camera is
    * @retval PINHOLE_CAMERA_RADIAL1
    */
    EINTRINSIC Pinhole_Intrinsic_Radial_K1::getType() const 
    {
      return PINHOLE_CAMERA_RADIAL1;
    }

    /**
    * @brief Does the camera model handle a distortion field?
    * @retval true if intrinsic holds distortion
    * @retval false if intrinsic does not hold distortion
    */
    bool Pinhole_Intrinsic_Radial_K1::have_disto() const 
    {
      return true;
    }

    /**
    * @brief Add the distortion field to a point (that is in normalized camera frame)
    * @param p Point before distortion computation (in normalized camera frame)
    * @return point with distortion
    */
    Vec2 Pinhole_Intrinsic_Radial_K1::add_disto( const Vec2 & p ) const 
    {

      const double k1 = params_[0];

      const double r2 = p( 0 ) * p( 0 ) + p( 1 ) * p( 1 );
      const double r_coeff = ( 1. + k1 * r2 );

      return ( p * r_coeff );
    }

    /**
    * @brief Remove the distortion to a camera point (that is in normalized camera frame)
    * @param p Point with distortion
    * @return Point without distortion
    */
    Vec2 Pinhole_Intrinsic_Radial_K1::remove_disto( const Vec2& p ) const 
    {
      // Compute the radius from which the point p comes from thanks to a bisection
      // Minimize disto(radius(p')^2) == actual Squared(radius(p))

      const double r2 = p( 0 ) * p( 0 ) + p( 1 ) * p( 1 );
      const double radius = ( r2 == 0 ) ?
                            1. :
                            ::sqrt( radial_distortion::bisection_Radius_Solve( params_, r2, distoFunctor ) / r2 );
      return radius * p;
    }

    /**
    * @brief Data wrapper for non linear optimization (get data)
    * @return vector of parameter of this intrinsic
    */
    std::vector<double> Pinhole_Intrinsic_Radial_K1::getParams() const 
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
    bool Pinhole_Intrinsic_Radial_K1::updateFromParams( const std::vector<double> & params ) 
    {
      if ( params.size() == 4 )
      {
        *this = Pinhole_Intrinsic_Radial_K1(
                  w_, h_,
                  params[0], params[1], params[2], // focal, ppx, ppy
                  params[3] ); //K1
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
    std::vector<int> Pinhole_Intrinsic_Radial_K1::subsetParameterization
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
      }
      return constant_index;
    }

    /**
    * @brief Return the un-distorted pixel (with removed distortion)
    * @param p Input distorted pixel
    * @return Point without distortion
    */
    Vec2 Pinhole_Intrinsic_Radial_K1::get_ud_pixel( const Vec2& p ) const 
    {
      return cam2ima( remove_disto( ima2cam( p ) ) );
    }

    /**
    * @brief Return the distorted pixel (with added distortion)
    * @param p Input pixel
    * @return Distorted pixel
    */
    Vec2 Pinhole_Intrinsic_Radial_K1::get_d_pixel( const Vec2& p ) const 
    {
      return cam2ima( add_disto( ima2cam( p ) ) );
    }


    /**
    * @brief Clone the object
    * @return A clone (copy of the stored object)
    */
    IntrinsicBase * Pinhole_Intrinsic_Radial_K1::clone( void ) const 
    {
      return new class_type( *this );
    }


    /**
    * @brief Functor to solve Square(disto(radius(p'))) = r^2
    * @param params List of parameters (only the first one is used)
    * @param r2 square distance (relative to center)
    * @return distance
    */
    double Pinhole_Intrinsic_Radial_K1::distoFunctor( const std::vector<double> & params, double r2 )
    {
      const double & k1 = params[0];
      return r2 * Square( 1. + r2 * k1 );
    }


    // Radial K3 

    /**
    * @brief Constructor
    * @param w Width of image
    * @param h Height of image
    * @param focal Focal (in pixel) of the camera
    * @param ppx Principal point on X-Axis
    * @param ppy Principal point on Y-Axis
    * @param k1 First radial distortion coefficient
    * @param k2 Second radial distortion coefficient
    * @param k3 Third radial distortion coefficient
    */
    Pinhole_Intrinsic_Radial_K3::Pinhole_Intrinsic_Radial_K3(
      int w , int h ,
      double focal , double ppx , double ppy ,
      double k1 , double k2 , double k3  )
      : Pinhole_Intrinsic( w, h, focal, ppx, ppy )
    {
      params_ = {k1, k2, k3};
    }


    /**
    * @brief Tell from which type the embed camera is
    * @retval PINHOLE_CAMERA_RADIAL3
    */
    EINTRINSIC Pinhole_Intrinsic_Radial_K3::getType() const 
    {
      return PINHOLE_CAMERA_RADIAL3;
    }

    /**
    * @brief Does the camera model handle a distortion field?
    * @retval true
    */
    bool Pinhole_Intrinsic_Radial_K3::have_disto() const 
    {
      return true;
    }

    /**
    * @brief Add the distortion field to a point (that is in normalized camera frame)
    * @param p Point before distortion computation (in normalized camera frame)
    * @return point with distortion
    */
    Vec2 Pinhole_Intrinsic_Radial_K3::add_disto( const Vec2 & p ) const 
    {
      const double & k1 = params_[0], & k2 = params_[1], & k3 = params_[2];

      const double r2 = p( 0 ) * p( 0 ) + p( 1 ) * p( 1 );
      const double r4 = r2 * r2;
      const double r6 = r4 * r2;
      const double r_coeff = ( 1. + k1 * r2 + k2 * r4 + k3 * r6 );

      return ( p * r_coeff );
    }

    /**
    * @brief Remove the distortion to a camera point (that is in normalized camera frame)
    * @param p Point with distortion
    * @return Point without distortion
    */
    Vec2 Pinhole_Intrinsic_Radial_K3::remove_disto( const Vec2& p ) const 
    {
      // Compute the radius from which the point p comes from thanks to a bisection
      // Minimize disto(radius(p')^2) == actual Squared(radius(p))

      const double r2 = p( 0 ) * p( 0 ) + p( 1 ) * p( 1 );
      const double radius = ( r2 == 0 ) ? //1. : ::sqrt(bisectionSolve(_params, r2) / r2);
                            1. :
                            ::sqrt( radial_distortion::bisection_Radius_Solve( params_, r2, distoFunctor ) / r2 );
      return radius * p;
    }

    /**
    * @brief Data wrapper for non linear optimization (get data)
    * @return vector of parameter of this intrinsic
    */
    std::vector<double> Pinhole_Intrinsic_Radial_K3::getParams() const 
    {
      std::vector<double> params = Pinhole_Intrinsic::getParams();
      params.insert( params.end(), std::begin(params_), std::end(params_));
      return params;
    }

    /**
    * @brief Data wrapper for non linear optimization (update from data)
    * @param params List of params used to update this intrinsic
    * @retval true if update is correct
    * @retval false if there was an error during update
    */
    bool Pinhole_Intrinsic_Radial_K3::updateFromParams( const std::vector<double> & params ) 
    {
      if ( params.size() == 6 )
      {
        *this = Pinhole_Intrinsic_Radial_K3(
                  w_, h_,
                  params[0], params[1], params[2], // focal, ppx, ppy
                  params[3], params[4], params[5] ); // K1, K2, K3
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
    std::vector<int> Pinhole_Intrinsic_Radial_K3::subsetParameterization
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
      }
      return constant_index;
    }

    /**
    * @brief Return the un-distorted pixel (with removed distortion)
    * @param p Input distorted pixel
    * @return Point without distortion
    */
    Vec2 Pinhole_Intrinsic_Radial_K3::get_ud_pixel( const Vec2& p ) const 
    {
      return cam2ima( remove_disto( ima2cam( p ) ) );
    }

    /**
    * @brief Return the distorted pixel (with added distortion)
    * @param p Input pixel
    * @return Distorted pixel
    */
    Vec2 Pinhole_Intrinsic_Radial_K3::get_d_pixel( const Vec2& p ) const 
    {
      return cam2ima( add_disto( ima2cam( p ) ) );
    }

    /**
    * @brief Clone the object
    * @return A clone (copy of the stored object)
    */
    IntrinsicBase * Pinhole_Intrinsic_Radial_K3::clone( void ) const 
    {
      return new class_type( *this );
    }



    /**
    * @brief Functor to solve Square(disto(radius(p'))) = r^2
    * @param params List of parameters (only the first one is used)
    * @param r2 square distance (relative to center)
    * @return distance
    */
    double Pinhole_Intrinsic_Radial_K3::distoFunctor( const std::vector<double> & params, double r2 )
    {
      const double & k1 = params[0], & k2 = params[1], & k3 = params[2];
      return r2 * Square( 1. + r2 * ( k1 + r2 * ( k2 + r2 * k3 ) ) );
    }



} // namespace cameras
} // namespace openMVG
