#include "IntrinsicRefinerOptions.hh"


namespace openMVG_gui
{

/**
* @brief perform bitwise OR operation on two options
* @param a first option
* @param b second option
* @return bitwise operation on two options
*/
constexpr IntrinsicRefinerOptions operator|( IntrinsicRefinerOptions a ,
    IntrinsicRefinerOptions b )
{
  return static_cast<IntrinsicRefinerOptions>( static_cast<int>( a ) | static_cast<int>( b ) ) ;
}

/**
* @brief Indicate if a complex option contains another option
* @param option Source option
* @param query Query
* @retval true if Query is fully contained in Source
* @retval if intersection between Query and Source is partial
*/
bool contains( const IntrinsicRefinerOptions& option ,
               const IntrinsicRefinerOptions & query )
{
  return ( static_cast<int>( option ) & static_cast<int>( query ) ) != 0 ;
}

/**
* @brief Convert from gui param to openMVG params
*/
openMVG::cameras::Intrinsic_Parameter_Type convert( const IntrinsicRefinerOptions & opt )
{
  using namespace openMVG::cameras;

  if( contains( opt , IntrinsicRefinerOptions::INTRINSIC_REFINER_ALL ) )
  {
    return Intrinsic_Parameter_Type::ADJUST_ALL ;
  }
  if( contains( opt , IntrinsicRefinerOptions::INTRINSIC_REFINER_NONE ) )
  {
    return Intrinsic_Parameter_Type::NONE ;
  }

  const IntrinsicRefinerOptions ALL_OPTS[] =
  {
    IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH  ,
    IntrinsicRefinerOptions::INTRINSIC_REFINER_PRINCIPAL_POINT  ,
    IntrinsicRefinerOptions::INTRINSIC_REFINER_DISTORTION
  } ;

  int res = 0 ;
  for( auto & cur_param : ALL_OPTS )
  {
    if( ! contains( opt , cur_param ) )
    {
      continue ;
    }

    switch( cur_param )
    {
      case IntrinsicRefinerOptions::INTRINSIC_REFINER_FOCAL_LENGTH :
      {
        res |= static_cast<int>( Intrinsic_Parameter_Type::ADJUST_FOCAL_LENGTH ) ;
        break ;
      }
      case IntrinsicRefinerOptions::INTRINSIC_REFINER_PRINCIPAL_POINT :
      {
        res |= static_cast<int>( Intrinsic_Parameter_Type::ADJUST_PRINCIPAL_POINT ) ;
        break ;
      }
      case IntrinsicRefinerOptions::INTRINSIC_REFINER_DISTORTION :
      {
        res |= static_cast<int>( Intrinsic_Parameter_Type::ADJUST_DISTORTION ) ;
        break ;
      }
    }
  }

  return static_cast<openMVG::cameras::Intrinsic_Parameter_Type>( res ) ;
}



} // namespace OpenMVG_gui