#ifndef _NUMERIC_IO_CEREAL_HPP_
#define _NUMERIC_IO_CEREAL_HPP_

#include "openMVG/numeric/numeric.h"

namespace cereal
{
  // Save Vec3 
  template< class Archive >
  void save( Archive & ar , const openMVG::Vec3 & vec )
  {
    ar( vec[0] ) ;
    ar( vec[1] ) ;
    ar( vec[2] ) ; 
  }

  // Load vec3 
  template< class Archive >
  void load( Archive & ar , openMVG::Vec3 & vec ) 
  {
    ar( vec[0] ) ;
    ar( vec[1] ) ;
    ar( vec[2] ) ; 
  }

  // Save vec4 
  template< class Archive >
  void save( Archive & ar , const openMVG::Vec4 & vec )
  {
    ar( vec[0] ) ;
    ar( vec[1] ) ;
    ar( vec[2] ) ; 
    ar( vec[3] ) ; 
  }

  // Load vec4 
  template< class Archive >
  void load( Archive & ar , openMVG::Vec4 & vec ) 
  {
    ar( vec[0] ) ;
    ar( vec[1] ) ;
    ar( vec[2] ) ; 
    ar( vec[3] ) ; 
  }

  // Load save Vec2 
  template< class Archive >
  void save( Archive & ar , const openMVG::Vec2 & vec )
  {
    ar( vec[0] ) ;
    ar( vec[1] ) ;
  }

  template< class Archive >
  void load( Archive & ar , openMVG::Vec2 & vec )
  {
    ar( vec[0] ) ;
    ar( vec[1] ) ; 
  }

  // Load save Mat34 
  template< class Archive >
  void save( Archive & ar , const openMVG::Mat34 & mat )
  {
    ar( mat(0,0) ) ;
    ar( mat(0,1) ) ;
    ar( mat(0,2) ) ;
    ar( mat(0,3) ) ;

    ar( mat(1,0) ) ;
    ar( mat(1,1) ) ;
    ar( mat(1,2) ) ;
    ar( mat(1,3) ) ;

    ar( mat(2,0) ) ;
    ar( mat(2,1) ) ;
    ar( mat(2,2) ) ;
    ar( mat(2,3) ) ;
  }

  template< class Archive >
  void load( Archive & ar , openMVG::Mat34 & mat )
  {
    ar( mat(0,0) ) ;
    ar( mat(0,1) ) ;
    ar( mat(0,2) ) ;
    ar( mat(0,3) ) ;

    ar( mat(1,0) ) ;
    ar( mat(1,1) ) ;
    ar( mat(1,2) ) ;
    ar( mat(1,3) ) ;

    ar( mat(2,0) ) ;
    ar( mat(2,1) ) ;
    ar( mat(2,2) ) ;
    ar( mat(2,3) ) ;    
  }
  
  // Load/Save Mat3 
  template< class Archive >
  void save( Archive & ar , const openMVG::Mat3 & mat )
  {
    ar( mat(0,0) ) ;
    ar( mat(0,1) ) ;
    ar( mat(0,2) ) ;

    ar( mat(1,0) ) ;
    ar( mat(1,1) ) ;
    ar( mat(1,2) ) ;

    ar( mat(2,0) ) ;
    ar( mat(2,1) ) ;
    ar( mat(2,2) ) ;
  }

  template< class Archive >
  void load( Archive & ar , openMVG::Mat3 & mat )
  {
    ar( mat(0,0) ) ;
    ar( mat(0,1) ) ;
    ar( mat(0,2) ) ;

    ar( mat(1,0) ) ;
    ar( mat(1,1) ) ;
    ar( mat(1,2) ) ;

    ar( mat(2,0) ) ;
    ar( mat(2,1) ) ;
    ar( mat(2,2) ) ;    
  }

}

#endif