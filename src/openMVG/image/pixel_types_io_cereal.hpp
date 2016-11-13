#ifndef _OPENMVG_IMAGE_PIXEL_TYPES_IO_CEREAL_HPP_
#define _OPENMVG_IMAGE_PIXEL_TYPES_IO_CEREAL_HPP_

#include "openMVG/image/pixel_types.hpp"

namespace cereal 
{
  template< typename Archive>
  void save( Archive & ar , const openMVG::image::Rgb<unsigned char> & col )
  {
    const unsigned char r = col.r() ;
    const unsigned char g = col.g() ;
    const unsigned char b = col.b() ; 

    ar( r ) ;
    ar( g ) ;
    ar( b ) ;
  }

  template< typename Archive >
  void load( Archive & ar , openMVG::image::Rgb<unsigned char> & col )
  {
    unsigned char r , g , b ; 
    ar( r ) ;
    ar( g ) ; 
    ar( b ) ; 
  
    col = openMVG::image::Rgb<unsigned char>( r , g , b ) ; 
  }
}

#endif