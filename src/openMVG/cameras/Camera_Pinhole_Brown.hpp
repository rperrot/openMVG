
// Copyright (c) 2015 Sida Li.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_CAMERAS_CAMERA_PINHOLE_BROWN_HPP
#define OPENMVG_CAMERAS_CAMERA_PINHOLE_BROWN_HPP

#include "openMVG/cameras/Camera_Common.hpp"
#include "openMVG/cameras/Camera_Pinhole.hpp"
#include "openMVG/numeric/numeric.h"

#include <vector>

namespace openMVG
{
namespace cameras
{

/**
* @brief Implement a Pinhole camera with a 3 radial distortion coefficients and 2 tangential distortion coefficients.
* \f$ x_d = x_u (1 + K_1 r^2 + K_2 r^4 + K_3 r^6) + (T_2 (r^2 + 2 x_u^2) + 2 T_1 x_u y_u) \f$
* \f$ y_d = y_u (1 + K_1 r^2 + K_2 r^4 + K_3 r^6) + (T_1 (r^2 + 2 y_u^2) + 2 T_2 x_u y_u) \f$
*/
class Pinhole_Intrinsic_Brown_T2 : public Pinhole_Intrinsic
{
  using class_type = Pinhole_Intrinsic_Brown_T2;

  protected:

    /// center of distortion is applied by the Intrinsics class
    std::vector<double> params_; // K1, K2, K3, T1, T2

  public:

    /**
    * @brief Constructor
    * @param w Width of image
    * @param h Height of image
    * @param focal Focal distance (in pixel)
    * @param ppx Principal point on X-axis
    * @param ppy Principal point on Y-axis
    * @param k1 First radial distortion coefficient
    * @param k2 Second radial distortion coefficient
    * @param k3 Third radial distortion coefficient
    * @param t1 First tangential distortion coefficient
    * @param t2 Second tangential distortion coefficient
    */
    Pinhole_Intrinsic_Brown_T2(
      int w = 0, int h = 0,
      double focal = 0.0, double ppx = 0, double ppy = 0,
      double k1 = 0.0, double k2 = 0.0, double k3 = 0.0,
      double t1 = 0.0, double t2 = 0.0 ) ;

    /**
    * @brief Get type of the intrinsic
    * @retval PINHOLE_CAMERA_BROWN
    */
    EINTRINSIC getType() const override ;

    /**
    * @brief Does the camera model handle a distortion field?
    * @retval true
    */
    virtual bool have_disto() const override ;

    /**
    * @brief Add the distortion field to a point (that is in normalized camera frame)
    * @param p Point before distortion computation (in normalized camera frame)
    * @return point with distortion
    */
    virtual Vec2 add_disto( const Vec2 & p ) const override ;

    /**
    * @brief Remove the distortion to a camera point (that is in normalized camera frame)
    * @param p Point with distortion
    * @return Point without distortion
    * @note numerical approximation based on
    * Heikkila J (2000) Geometric Camera Calibration Using Circular Control Points.
    * IEEE Trans. Pattern Anal. Mach. Intell., 22:1066-1077
    */
    virtual Vec2 remove_disto( const Vec2 & p ) const override ;

    /**
    * @brief Data wrapper for non linear optimization (get data)
    * @return vector of parameter of this intrinsic
    */
    virtual std::vector<double> getParams() const override ;

    /**
    * @brief Data wrapper for non linear optimization (update from data)
    * @param params List of params used to update this intrinsic
    * @retval true if update is correct
    * @retval false if there was an error during update
    */
    virtual bool updateFromParams( const std::vector<double> & params ) override ;

    /**
    * @brief Return the list of parameter indexes that must be held constant
    * @param parametrization The given parametrization
    */
    virtual std::vector<int> subsetParameterization
    (const Intrinsic_Parameter_Type & parametrization) const override ;

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
      ar(cereal::base_class<Pinhole_Intrinsic>(this));
      ar( cereal::make_nvp( "disto_t2", params_ ) );
    }

    /**
    * @brief  Serialization in
    * @param ar Archive
    */
    template <class Archive>
    void load( Archive & ar )
    {
      ar(cereal::base_class<Pinhole_Intrinsic>(this));
      ar( cereal::make_nvp( "disto_t2", params_ ) );
    }

    /**
    * @brief Clone the object
    * @return A clone (copy of the stored object)
    */
    virtual IntrinsicBase * clone( void ) const override ;

  private:


    /**
    * @brief Functor to calculate distortion offset accounting for both radial and tangential distortion
    * @param params List of parameters to define a Brown camera
    * @param p Input point
    * @return Transformed point
    */
    static Vec2 distoFunction( const std::vector<double> & params, const Vec2 & p ) ;
};


} // namespace cameras
} // namespace openMVG

#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>

CEREAL_REGISTER_TYPE_WITH_NAME( openMVG::cameras::Pinhole_Intrinsic_Brown_T2, "pinhole_brown_t2" );
CEREAL_REGISTER_POLYMORPHIC_RELATION(openMVG::cameras::IntrinsicBase, openMVG::cameras::Pinhole_Intrinsic_Brown_T2)

#endif // #ifndef OPENMVG_CAMERAS_CAMERA_PINHOLE_BROWN_HPP
