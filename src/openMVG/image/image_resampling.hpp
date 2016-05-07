// Copyright (c) 2014, 2015 Romuald Perrot.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_IMAGE_RESAMPLING_HPP_
#define OPENMVG_IMAGE_IMAGE_RESAMPLING_HPP_


namespace openMVG
{
namespace image
{

/**
 ** Half sample an image (ie reduce it's size by a factor 2) using bilinear interpolation
 ** @param src input image
 ** @param out output image
 **/
template < typename Image >
void ImageHalfSample( const Image & src , Image & out )
{
  const int new_width  = src.Width() / 2 ;
  const int new_height = src.Height() / 2 ;

  out.resize( new_width , new_height ) ;

  const Sampler2d<SamplerLinear> sampler;

  for( int i = 0 ; i < new_height ; ++i )
  {
    for( int j = 0 ; j < new_width ; ++j )
    {
      // Use .5f offset to ensure mid pixel and correct bilinear sampling
      out( i , j ) =  sampler( src, 2.f * ( i + .5f ), 2.f * ( j + .5f ) );
    }
  }
}

/**
 ** @brief Ressample an image using given sampling positions
 ** @param src Input image
 ** @param sampling_pos A list of coordinates where the image needs to be ressampled (samples are (Y,X) )
 ** @param output_width Width of the output image.
 ** @param output_height Height of the output image
 ** @param sampling_func Ressampling functor used to sample the Input image
 ** @param[out] Output image
 ** @note sampling_pos.size() must be equal to output_width * output_height
 **/
template <typename Image , typename RessamplingFunctor>
void GenericRessample( const Image & src ,
                       const std::vector< std::pair< float , float > > & sampling_pos ,
                       const int output_width ,
                       const int output_height ,
                       const RessamplingFunctor & sampling_func ,
                       Image & out )
{
  assert( sampling_pos.size() == output_width * output_height );

  out.resize( output_width , output_height );

  std::vector< std::pair< float , float > >::const_iterator it_pos = sampling_pos.begin();

  for( int i = 0 ; i < output_height ; ++i )
  {
    for( int j = 0 ; j < output_width ; ++j , ++it_pos )
    {
      const float input_x = it_pos->second ;
      const float input_y = it_pos->first ;

      out( i , j ) = sampling_func( src , input_y , input_x ) ;
    }
  }
}

template< typename Image, typename RessamplingFunctor>
void GenericResize( const Image & src , 
                    const int output_width , 
                    const int output_height , 
                    const bool preserve_ratio , 
                    const RessamplingFunctor & sampling_func ,
                    Image & out )
{
  const int input_width  = src.Width() ; 
  const int input_height = src.Height() ; 
  
  int real_width = output_width ;
  int real_height = output_height ;
  
  if( preserve_ratio )
  {
    if( input_width > input_height )
    {
      // input_width maps to real_width, so adjust height in accordance 
      const float ratio = static_cast<float>( input_height ) / static_cast<float>( input_width ) ; 
      real_height = static_cast<int>( ratio * static_cast<float>( output_height ) ) ; 
    } 
    else 
    {
      // input_height maps to real_height, so adjust with in accordance 
      const float ratio = static_cast<float>( input_width ) / static_cast<float>( input_height ) ; 
      real_width = static_cast<int>( ratio * static_cast<float>( output_width ) ) ; 
    }
  }    
  // Compute sampling position 
  std::vector< std::pair< float , float > > sampling_pos ; 
  
  for( int id_row = 0 ; id_row < real_height ; ++id_row )
  {
    for( int id_col = 0 ; id_col < real_width ; ++id_col )
    {
      const float y = static_cast<float>(id_row) / static_cast<float>(real_height) * static_cast<float>(input_height) ;
      const float x = static_cast<float>(id_col) / static_cast<float>(real_width) * static_cast<float>(input_width) ;      
      sampling_pos.push_back( std::make_pair( y , x ) ) ; 
    }
  }        
  
  // Now ressample 
  GenericRessample( src , sampling_pos , real_width , real_height , sampling_func , out ) ; 
}

} // namespace image
} // namespace openMVG

#endif
