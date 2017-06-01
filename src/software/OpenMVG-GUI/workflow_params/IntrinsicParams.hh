#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_INTRINSIC_PARAMS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_INTRINSIC_PARAMS_HH_

#include "openMVG/cameras/Camera_Common.hpp"

#include <memory>

// fwrd decl
namespace openMVG
{
namespace cameras
{
class IntrinsicBase ;
}
}


namespace openMVG_gui
{

/**
* @brief Get intrinsic type used as init
*/
enum IntrinsicParamType
{
  /// Pinhole without distortion
  INTRINSIC_TYPE_PINHOLE ,
  /// Pinhole with 1 radial params
  INTRINSIC_TYPE_PINHOLE_RADIAL_1 ,
  /// Pinhole with 3 radial params
  INTRINSIC_TYPE_PINHOLE_RADIAL_3 ,
  /// Pinhole with 3 radial, 2 tangential params
  INTRINSIC_TYPE_PINHOLE_BROWN ,
  /// Pinhole with fisheye distortion
  INTRINSIC_TYPE_PINHOLE_FISHEYE
} ;

/**
* @brief convert from gui enum to openMVG enum
*/
openMVG::cameras::EINTRINSIC convert( const IntrinsicParamType & type ) ;


/**
* @brief class used to store intrinsic params used in SfM process
* @note This is only used now in the initialisation of the project
* Like in SfMInit_ImageListing
*/
class IntrinsicParams
{
  public:

    /**
    * @brief Ctr
    * @param intrinType Intrinsic type
    * @param groupModel Indicate if parameters could be shared between cameras
    */
    IntrinsicParams( const IntrinsicParamType & intrinType = INTRINSIC_TYPE_PINHOLE_RADIAL_3 ,
                     const bool groupModel = true ) ;


    /**
    * @brief get intrinsic type to use
    * @return intrinsic model to use
    */
    IntrinsicParamType type( void ) const ;

    /**
    * @brief set intrinsic type to use
    */
    void setType( const IntrinsicParamType & type ) ;

    /**
    * @brief tells if models can be shared between cameras
    * @retval true Models shared
    * @retval false Each camera have individual elements
    */
    bool groupedModels( void ) const ;

    /**
    * @brief set grouped model mode
    * @param gmodel true if model could be shared, false camera have their own models
    */
    void setGroupedModels( const bool gmodel ) ;

    /**
    * @brief Create instrinsic structure in openMVG format given parameters
    * @return intrinsic params
    */
    std::shared_ptr<openMVG::cameras::IntrinsicBase> getIntrinsic( const double width ,
        const double height ,
        const double focal ,
        const double ppx ,
        const double ppy ) const ;

    template <typename Archive >
    void load( Archive & ar ) ;

    template< typename Archive >
    void save( Archive & ar ) const ;

  private:

    /// Type of the intrinsic to use
    IntrinsicParamType m_intrin_type ;
    /// Indicate if some parameters are shared
    bool m_group_model ;

} ;

template <typename Archive >
void IntrinsicParams::load( Archive & ar )
{
  ar( m_intrin_type ) ;
  ar( m_group_model ) ;
}

template< typename Archive >
void IntrinsicParams::save( Archive & ar ) const
{
  ar( m_intrin_type ) ;
  ar( m_group_model ) ;
}

} // namespace openMVG_gui

#endif