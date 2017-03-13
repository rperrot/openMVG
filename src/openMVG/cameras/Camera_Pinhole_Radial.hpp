
// Copyright (c) 2015 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_CAMERAS_CAMERA_PINHOLE_RADIAL_HPP
#define OPENMVG_CAMERAS_CAMERA_PINHOLE_RADIAL_HPP

#include "openMVG/cameras/Camera_Common.hpp"
#include "openMVG/cameras/Camera_Pinhole.hpp"
#include "openMVG/numeric/numeric.h"

#include <vector>

namespace openMVG
{
namespace cameras
{

/**
* @brief Namespace handling radial distortion computation
*/
namespace radial_distortion
{


/**
* @brief Solve by bisection the p' radius such that Square(disto(radius(p'))) = r^2
* @param params Parameters of the distortion
* @param r2 Target radius
* @param functor Functor used to paramatrize the distortion
* @param epsilon Error driven threshold
* @return Best radius
*/
template <class Disto_Functor>
double bisection_Radius_Solve(
  const std::vector<double> & params, // radial distortion parameters
  double r2, // targeted radius
  Disto_Functor & functor,
  double epsilon = 1e-10 // criteria to stop the bisection
)
{
  // Guess plausible upper and lower bound
  double lowerbound = r2, upbound = r2;
  while ( functor( params, lowerbound ) > r2 )
  {
    lowerbound /= 1.05;
  }
  while ( functor( params, upbound ) < r2 )
  {
    upbound *= 1.05;
  }

  // Perform a bisection until epsilon accuracy is not reached
  while ( epsilon < upbound - lowerbound )
  {
    const double mid = .5 * ( lowerbound + upbound );
    if ( functor( params, mid ) > r2 )
    {
      upbound = mid;
    }
    else
    {
      lowerbound = mid;
    }
  }
  return .5 * ( lowerbound + upbound );
}

} // namespace radial_distortion

/**
 * @brief Implement a Pinhole camera with a 1 radial distortion coefficient.
 * \f$ x_d = x_u (1 + K_1 r^2 ) \f$
 */
class Pinhole_Intrinsic_Radial_K1 : public Pinhole_Intrinsic
{
  using class_type = Pinhole_Intrinsic_Radial_K1;

  protected:
    /// center of distortion is applied by the Intrinsics class
    std::vector<double> params_; // K1

  public:

    /**
    * @brief Constructor
    * @param w Width of the image
    * @param h Height of the image
    * @param focal Focal (in pixel) of the camera
    * @param ppx Principal point on X-Axis
    * @param ppy Principal point on Y-Axis
    * @param k1 Distortion coefficient
    */
    Pinhole_Intrinsic_Radial_K1(
      int w = 0, int h = 0,
      double focal = 0.0, double ppx = 0, double ppy = 0,
      double k1 = 0.0 );

    ~Pinhole_Intrinsic_Radial_K1() override = default;

    /**
    * @brief Tell from which type the embed camera is
    * @retval PINHOLE_CAMERA_RADIAL1
    */
    EINTRINSIC getType() const override ;

    /**
    * @brief Does the camera model handle a distortion field?
    * @retval true if intrinsic holds distortion
    * @retval false if intrinsic does not hold distortion
    */
    bool have_disto() const override ;

    /**
    * @brief Add the distortion field to a point (that is in normalized camera frame)
    * @param p Point before distortion computation (in normalized camera frame)
    * @return point with distortion
    */
    Vec2 add_disto( const Vec2 & p ) const override ;

    /**
    * @brief Remove the distortion to a camera point (that is in normalized camera frame)
    * @param p Point with distortion
    * @return Point without distortion
    */
    Vec2 remove_disto( const Vec2& p ) const override ;

    /**
    * @brief Data wrapper for non linear optimization (get data)
    * @return vector of parameter of this intrinsic
    */
    std::vector<double> getParams() const override ;

    /**
    * @brief Data wrapper for non linear optimization (update from data)
    * @param params List of params used to update this intrinsic
    * @retval true if update is correct
    * @retval false if there was an error during update
    */
    bool updateFromParams( const std::vector<double> & params ) override ;

    /**
    * @brief Return the list of parameter indexes that must be held constant
    * @param parametrization The given parametrization
    */
    std::vector<int> subsetParameterization
    (
      const Intrinsic_Parameter_Type & parametrization) const override ;

    /**
    * @brief Return the un-distorted pixel (with removed distortion)
    * @param p Input distorted pixel
    * @return Point without distortion
    */
    Vec2 get_ud_pixel( const Vec2& p ) const override ;

    /**
    * @brief Return the distorted pixel (with added distortion)
    * @param p Input pixel
    * @return Distorted pixel
    */
    Vec2 get_d_pixel( const Vec2& p ) const override ;

    /**
    * @brief Serialization out
    * @param ar Archive
    */
    template <class Archive>
    void save( Archive & ar ) const
    {
      Pinhole_Intrinsic::save( ar );
      ar( cereal::make_nvp( "disto_k1", params_ ) );
    }

    /**
    * @brief  Serialization in
    * @param ar Archive
    */
    template <class Archive>
    void load( Archive & ar )
    {
      Pinhole_Intrinsic::load(ar);
      ar( cereal::make_nvp( "disto_k1", params_ ) );
    }

    /**
    * @brief Clone the object
    * @return A clone (copy of the stored object)
    */
    IntrinsicBase * clone( void ) const override ;

  private:


    /**
    * @brief Functor to solve Square(disto(radius(p'))) = r^2
    * @param params List of parameters (only the first one is used)
    * @param r2 square distance (relative to center)
    * @return distance
    */
    static double distoFunctor( const std::vector<double> & params, double r2 ) ;
};

/**
* @brief Implement a Pinhole camera with a 3 radial distortion coefficients.
* \f$ x_d = x_u (1 + K_1 r^2 + K_2 r^4 + K_3 r^6) \f$
*/
class Pinhole_Intrinsic_Radial_K3 : public Pinhole_Intrinsic
{
  using class_type = Pinhole_Intrinsic_Radial_K3;

  protected:
    // center of distortion is applied by the Intrinsics class
    /// K1, K2, K3
    std::vector<double> params_;

  public:

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
    Pinhole_Intrinsic_Radial_K3(
      int w = 0, int h = 0,
      double focal = 0.0, double ppx = 0, double ppy = 0,
      double k1 = 0.0, double k2 = 0.0, double k3 = 0.0 );

    ~Pinhole_Intrinsic_Radial_K3() override = default;

    /**
    * @brief Tell from which type the embed camera is
    * @retval PINHOLE_CAMERA_RADIAL3
    */
    EINTRINSIC getType() const override ;

    /**
    * @brief Does the camera model handle a distortion field?
    * @retval true
    */
    bool have_disto() const override;

    /**
    * @brief Add the distortion field to a point (that is in normalized camera frame)
    * @param p Point before distortion computation (in normalized camera frame)
    * @return point with distortion
    */
    Vec2 add_disto( const Vec2 & p ) const override ;

    /**
    * @brief Remove the distortion to a camera point (that is in normalized camera frame)
    * @param p Point with distortion
    * @return Point without distortion
    */
    Vec2 remove_disto( const Vec2& p ) const override ;

    /**
    * @brief Data wrapper for non linear optimization (get data)
    * @return vector of parameter of this intrinsic
    */
    std::vector<double> getParams() const override ;

    /**
    * @brief Data wrapper for non linear optimization (update from data)
    * @param params List of params used to update this intrinsic
    * @retval true if update is correct
    * @retval false if there was an error during update
    */
    bool updateFromParams( const std::vector<double> & params ) override ;

    /**
    * @brief Return the list of parameter indexes that must be held constant
    * @param parametrization The given parametrization
    */
    std::vector<int> subsetParameterization
    ( const Intrinsic_Parameter_Type & parametrization) const override ;

    /**
    * @brief Return the un-distorted pixel (with removed distortion)
    * @param p Input distorted pixel
    * @return Point without distortion
    */
    Vec2 get_ud_pixel( const Vec2& p ) const override ;

    /**
    * @brief Return the distorted pixel (with added distortion)
    * @param p Input pixel
    * @return Distorted pixel
    */
    Vec2 get_d_pixel( const Vec2& p ) const override ;

    /**
    * @brief Serialization out
    * @param ar Archive
    */
    template <class Archive>
    void save( Archive & ar ) const
    {
      Pinhole_Intrinsic::save(ar);
      ar( cereal::make_nvp( "disto_k3", params_ ) );
    }

    /**
    * @brief  Serialization in
    * @param ar Archive
    */
    template <class Archive>
    void load( Archive & ar )
    {
      Pinhole_Intrinsic::load(ar);
      ar( cereal::make_nvp( "disto_k3", params_ ) );
    }

    /**
    * @brief Clone the object
    * @return A clone (copy of the stored object)
    */
    IntrinsicBase * clone( void ) const override ;
  private:


    /**
    * @brief Functor to solve Square(disto(radius(p'))) = r^2
    * @param params List of parameters (only the first one is used)
    * @param r2 square distance (relative to center)
    * @return distance
    */
    static double distoFunctor( const std::vector<double> & params, double r2 ) ;
};

} // namespace cameras
} // namespace openMVG

#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>

CEREAL_REGISTER_TYPE_WITH_NAME(openMVG::cameras::Pinhole_Intrinsic_Radial_K1, "pinhole_radial_k1");
CEREAL_REGISTER_POLYMORPHIC_RELATION(openMVG::cameras::IntrinsicBase, openMVG::cameras::Pinhole_Intrinsic_Radial_K1);
CEREAL_REGISTER_TYPE_WITH_NAME(openMVG::cameras::Pinhole_Intrinsic_Radial_K3, "pinhole_radial_k3");
CEREAL_REGISTER_POLYMORPHIC_RELATION(openMVG::cameras::IntrinsicBase, openMVG::cameras::Pinhole_Intrinsic_Radial_K3);

#endif // #ifndef OPENMVG_CAMERAS_CAMERA_PINHOLE_RADIAL_K_HPP
