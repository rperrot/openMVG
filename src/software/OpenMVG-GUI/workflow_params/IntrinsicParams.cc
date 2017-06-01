#include "IntrinsicParams.hh"

#include "openMVG/cameras/Camera_Pinhole.hpp"
#include "openMVG/cameras/Camera_Pinhole_Brown.hpp"
#include "openMVG/cameras/Camera_Pinhole_Fisheye.hpp"
#include "openMVG/cameras/Camera_Pinhole_Radial.hpp"

using namespace openMVG::cameras ;

namespace openMVG_gui
{

/**
* @brief convert from gui enum to openMVG enum
*/
openMVG::cameras::EINTRINSIC convert( const IntrinsicParamType & type )
{
  using namespace openMVG::cameras ;

  switch( type )
  {
    case INTRINSIC_TYPE_PINHOLE :
    {
      return EINTRINSIC::PINHOLE_CAMERA ;
    }
    case INTRINSIC_TYPE_PINHOLE_RADIAL_1 :
    {
      return EINTRINSIC::PINHOLE_CAMERA_RADIAL1 ;
    }
    case INTRINSIC_TYPE_PINHOLE_RADIAL_3 :
    {
      return EINTRINSIC::PINHOLE_CAMERA_RADIAL3 ;
    }
    case INTRINSIC_TYPE_PINHOLE_BROWN :
    {
      return EINTRINSIC::PINHOLE_CAMERA_BROWN ;
    }
    case INTRINSIC_TYPE_PINHOLE_FISHEYE :
    {
      return EINTRINSIC::PINHOLE_CAMERA_FISHEYE ;
    }
    default :
    {
      return EINTRINSIC::PINHOLE_CAMERA_RADIAL3 ;
    }
  }
}


/**
* @brief Ctr
* @param intrinType Intrinsic type
* @param groupModel Indicate if parameters could be shared between cameras
*/
IntrinsicParams::IntrinsicParams( const IntrinsicParamType & intrinType ,
                                  const bool groupModel )
  : m_intrin_type( intrinType ) ,
    m_group_model( groupModel )
{

}


/**
* @brief get intrinsic type to use
* @return intrinsic model to use
*/
IntrinsicParamType IntrinsicParams::type( void ) const
{
  return m_intrin_type ;
}

/**
* @brief set intrinsic type to use
*/
void IntrinsicParams::setType( const IntrinsicParamType & type )
{
  m_intrin_type = type ;
}

/**
* @brief tells if models can be shared between cameras
* @retval true Models shared
* @retval false Each camera have individual elements
*/
bool IntrinsicParams::groupedModels( void ) const
{
  return m_group_model ;
}

/**
* @brief set grouped model mode
* @param gmodel true if model could be shared, false camera have their own models
*/
void IntrinsicParams::setGroupedModels( const bool gmodel )
{
  m_group_model = gmodel ;
}

/**
* @brief Create instrinsic structure in openMVG format given parameters
* @return intrinsic params
*/
std::shared_ptr<openMVG::cameras::IntrinsicBase> IntrinsicParams::getIntrinsic( const double width ,
    const double height ,
    const double focal ,
    const double ppx ,
    const double ppy ) const
{
  switch( m_intrin_type )
  {
    case INTRINSIC_TYPE_PINHOLE :
    {
      return std::make_shared<Pinhole_Intrinsic>( width, height, focal, ppx, ppy );
    }
    case INTRINSIC_TYPE_PINHOLE_RADIAL_1 :
    {
      // setup no distortion as initial guess
      return std::make_shared<Pinhole_Intrinsic_Radial_K1>( width, height, focal, ppx, ppy, 0.0 );
    }
    case INTRINSIC_TYPE_PINHOLE_RADIAL_3 :
    {
      // setup no distortion as initial guess
      return std::make_shared<Pinhole_Intrinsic_Radial_K3>( width, height, focal, ppx, ppy, 0.0, 0.0, 0.0 );
    }
    case INTRINSIC_TYPE_PINHOLE_BROWN :
    {
      // setup no distortion as initial guess
      return std::make_shared<Pinhole_Intrinsic_Brown_T2>( width, height, focal, ppx, ppy, 0.0, 0.0, 0.0, 0.0, 0.0 );
    }
    case INTRINSIC_TYPE_PINHOLE_FISHEYE :
    {
      // setup no distortion as initial guess
      return std::make_shared<Pinhole_Intrinsic_Fisheye>( width, height, focal, ppx, ppy, 0.0, 0.0, 0.0, 0.0 );
    }
  }
}

} // namespace openMVG_gui