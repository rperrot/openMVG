#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_DUAL_QUATERNION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_DUAL_QUATERNION_HH_

#include "openMVG/numeric/numeric.h"

namespace openMVG_gui
{

/**
* @brief class holding a dual quaternion
*/
class DualQuaternion
{
  public:

    /**
    * @brief Identity dual quaternion
    * no rotation - no translation
    */
    DualQuaternion( void ) ;

    /**
    * @brief Addition of two dual quaternions
    * @param dq1 First dual quaternion
    * @param dq2 Second dual quaternion
    * @return sum of dq1 and dq2
    */
    friend DualQuaternion operator+( const DualQuaternion & dq1 , const DualQuaternion & dq2 ) ;

    /**
    * @brief Subtraction of two dual quaternions
    * @param dq1 First dual quaternion
    * @param dq2 Second dual quaternion
    * @return subtraction of dq1 by dq2
    */
    friend DualQuaternion operator-( const DualQuaternion & dq1 , const DualQuaternion & dq2 ) ;

    /**
    * @brief Scalar multiplication
    * @param dq Dual quaternion
    * @param s Scalar factor
    * @return scaling of dq by s
    */
    friend DualQuaternion operator*( const DualQuaternion & dq , const double s ) ;

    /**
    * @brief Scalar multiplication
    * @param dq Dual quaternion
    * @param s Scalar factor
    * @return scaling of dq by s
    */
    friend DualQuaternion operator*( const double s , const DualQuaternion & dq ) ;

    /**
    * @brief Dual quaternion product
    * @param dq Dual quaternion
    * @param dq2 Dual quaternion
    * @return product of the two dual quaternions
    */
    friend DualQuaternion operator*( const DualQuaternion & dq , const DualQuaternion & dq2 ) ;

    /**
    * @brief Get conjugate of the dual quaternion
    * @return conjugate
    */
    DualQuaternion conjugate( void ) const ;

    /**
    * @brief Get normalization of the dual quaternion
    * @return self normalized
    */
    DualQuaternion normalize( void ) const ;

    /**
    * @brief Get rotational part of the dual quaternion
    * @return Rotational part as a quaternion
    */
    Eigen::Quaterniond rotationalPart( void ) const ;

    /**
    * @brief Get translational part of the dual quaternion
    * @return Translational part as a 3d vector
    */
    openMVG::Vec3 translationalPart( void ) const ;

    /**
    * @brief Convert the dual quaternion to a 4x4 transformation matrix
    * @return 4x4 matrix of the current transformation
    */
    openMVG::Mat4 toMatrix( void ) const ;

    /**
    * @brief Build a pure rotation around an axis of a given angle
    * @param axis Axis of rotation
    * @param aRad angle of rotation
    * @return dual quaternion corresponding to this transformation
    * @note angle is in radian
    */
    static DualQuaternion rotation( const openMVG::Vec3 & axis , const double aRad ) ;

    /**
    * @brief Build a pure translation of a given vector
    * @param d Translation vector
    * @return dual quaternion corresponding to this transformation
    */
    static DualQuaternion translation( const openMVG::Vec3 & d ) ;

    /**
    * @brief Apply transformation to a point
    * @param pt Input point
    * @return transformed point
    */
    openMVG::Vec3 applyPoint( const openMVG::Vec3 & pt ) const ;

    /**
    * @brief Apply transformation to a vector
    * @param vec Input vector
    * @return transformed vector
    */
    openMVG::Vec3 applyVector( const openMVG::Vec3 & pt ) const ;

  private:

    /**
    * @brief Full constructor
    * @param qr Rotational part
    * @param qe Translational part
    */
    DualQuaternion( const Eigen::Quaterniond & qr , const Eigen::Quaterniond & qe ) ;

    Eigen::Quaterniond m_qr ; // real part (ie: rotation)
    Eigen::Quaterniond m_qd ; // dual part (ie: translation)
} ;

} // namespace openMVG_gui

#endif