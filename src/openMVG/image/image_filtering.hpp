// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2014 Romuald Perrot.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_IMAGE_FILTERING_HPP
#define OPENMVG_IMAGE_IMAGE_FILTERING_HPP

/**
 ** @file
 ** standard image filtering functions :
 ** - X,Y derivatives using : Central difference, Sobel filter, Scharr filter
 ** - Gaussian blur
 **/

//------------------
//-- Bibliography --
//------------------
//- [1] "A Scheme for Coherence-Enhancing Diffusion Filtering with Optimized
//Rotation Invariance."
//- Authors: Joachim Weickert and Hanno Scharr.
//- Date: September 2002.
//- Journal : Journal of Visual Communication and Image Representation.

#include "openMVG/image/image_convolution.hpp"

#include <cmath>

// DEBUG only
#include <iostream>

namespace openMVG
{
namespace image
{

/**
 ** Compute X-derivative using central difference
 ** @param img Input image
 ** @param out Output image
 ** @param normalize true if kernel must be scaled by 1/2
 **/
template <typename Image>
void ImageXDerivative( const Image& img,
                       Image&       out,
                       const bool   normalize = true )
{
  Vec3 kernel( -1.0, 0.0, 1.0 );

  if ( normalize )
  {
    kernel *= 0.5;
  }

  ImageHorizontalConvolution( img, kernel, out );
}

/**
 ** Compute Y-derivative using central difference
 ** @param img Input image
 ** @param out Output image
 ** @param normalize true if kernel must be normalized
 **/
template <typename Image>
void ImageYDerivative( const Image& img,
                       Image&       out,
                       const bool   normalize = true )
{
  Vec3 kernel( -1.0, 0.0, 1.0 );

  if ( normalize )
  {
    kernel *= 0.5;
  }

  ImageVerticalConvolution( img, kernel, out );
}

/**
 ** Compute X-derivative using 3x3 Sobel kernel
 ** @param img Input image
 ** @param out Output image
 ** @param normalize true if kernel must be scaled by 1/8
 **/
template <typename Image>
void ImageSobelXDerivative( const Image& img,
                            Image&       out,
                            const bool   normalize = true )
{
  Vec3 kernel_horiz( -1.0, 0.0, 1.0 );

  if ( normalize )
  {
    kernel_horiz *= 0.5;
  }

  Vec3 kernel_vert( 1.0, 2.0, 1.0 );

  if ( normalize )
  {
    kernel_vert *= 0.25;
  }

  ImageSeparableConvolution( img, kernel_horiz, kernel_vert, out );
}

/**
 ** Compute Y-derivative using 3x3 Sobel kernel
 ** @param img Input image
 ** @param out Output image
 ** @param normalize true if kernel must be scaled by 1/8
 **/
template <typename Image>
void ImageSobelYDerivative( const Image& img,
                            Image&       out,
                            const bool   normalize = true )
{
  Vec3 kernel_horiz( 1.0, 2.0, 1.0 );

  if ( normalize )
  {
    kernel_horiz *= 0.25;
  }

  Vec3 kernel_vert( -1.0, 0.0, 1.0 );

  if ( normalize )
  {
    kernel_vert *= 0.5;
  }

  ImageSeparableConvolution( img, kernel_horiz, kernel_vert, out );
}

/**
 ** Compute X-derivative using 3x3 Scharr kernel
 ** @param img Input image
 ** @param out Output image
 ** @param normalize true if kernel must be scaled by 1/32
 **/
template <typename Image>
void ImageScharrXDerivative( const Image& img,
                             Image&       out,
                             const bool   normalize = true )
{
  Vec3 kernel_horiz( -1.0, 0.0, 1.0 );

  if ( normalize )
  {
    kernel_horiz *= 0.5;
  }

  Vec3 kernel_vert( 3.0, 10.0, 3.0 );

  if ( normalize )
  {
    kernel_vert *= 1.0 / 16.0;
  }

  ImageSeparableConvolution( img, kernel_horiz, kernel_vert, out );
}

/**
 ** Compute Y-derivative using 3x3 Scharr filter
 ** @param img Input image
 ** @param out Output image
 ** @param normalize true if kernel must be scaled by 1/32
 **/
template <typename Image>
void ImageScharrYDerivative( const Image& img,
                             Image&       out,
                             const bool   normalize = true )
{
  Vec3 kernel_horiz( 3.0, 10.0, 3.0 );

  if ( normalize )
  {
    kernel_horiz *= 1.0 / 16.0;
  }

  Vec3 kernel_vert( -1.0, 0.0, 1.0 );

  if ( normalize )
  {
    kernel_vert *= 0.5;
  }

  ImageSeparableConvolution( img, kernel_horiz, kernel_vert, out );
}

/**
 ** Compute X-derivative using scaled Scharr filter
 ** @param img Input image
 ** @param out Output image
 ** @param scale scale of filter (1 -> 3x3 filter; 2 -> 5x5, ...)
 ** @param bNormalize true if kernel must be normalized
 **/
template <typename Image>
void ImageScaledScharrXDerivative( const Image& img,
                                   Image&       out,
                                   const int    scale,
                                   const bool   bNormalize = true )
{
  const int kernel_size = 3 + 2 * ( scale - 1 );

  Vec kernel_vert( kernel_size );
  Vec kernel_horiz( kernel_size );

  /*
  General X-derivative function
                              | -1   0   1 |
  D = 1 / ( 2 h * ( w + 2 ) ) | -w   0   w |
                              | -1   0   1 |
  */

  kernel_horiz.fill( 0.0 );
  kernel_horiz( 0 ) = -1.0;
  // kernel_horiz( kernel_size / 2 ) = 0.0;
  kernel_horiz( kernel_size - 1 ) = 1.0;

  // Scharr parameter for derivative
  const double w = 10.0 / 3.0;

  kernel_vert.fill( 0.0 );
  kernel_vert( 0 )               = 1.0;
  kernel_vert( kernel_size / 2 ) = w;
  kernel_vert( kernel_size - 1 ) = 1.0;

  if ( bNormalize )
  {
    kernel_vert *= 1.0 / ( 2.0 * scale * ( w + 2.0 ) );
  }

  ImageSeparableConvolution( img, kernel_horiz, kernel_vert, out );
}

/**
 ** Compute Y-derivative using scaled Scharr filter
 ** @param img Input image
 ** @param out Output image
 ** @param scale scale of filter (1 -> 3x3 filter; 2 -> 5x5, ...)
 ** @param bNormalize true if kernel must be normalized
 **/
template <typename Image>
void ImageScaledScharrYDerivative( const Image& img,
                                   Image&       out,
                                   const int    scale,
                                   const bool   bNormalize = true )
{
  /*
  General Y-derivative function
                              | -1  -w  -1 |
  D = 1 / ( 2 h * ( w + 2 ) ) |  0   0   0 |
                              |  1   w   1 |

  */
  const int kernel_size = 3 + 2 * ( scale - 1 );

  Vec kernel_vert( kernel_size );
  Vec kernel_horiz( kernel_size );

  // Scharr parameter for derivative
  const double w = 10.0 / 3.0;

  kernel_horiz.fill( 0.0 );
  kernel_horiz( 0 )               = 1.0;
  kernel_horiz( kernel_size / 2 ) = w;
  kernel_horiz( kernel_size - 1 ) = 1.0;

  if ( bNormalize )
  {
    kernel_horiz *= 1.0 / ( 2.0 * scale * ( w + 2.0 ) );
  }

  kernel_vert.fill( 0.0 );
  kernel_vert( 0 ) = -1.0;
  // kernel_vert( kernel_size / 2 ) = 0.0;
  kernel_vert( kernel_size - 1 ) = 1.0;

  ImageSeparableConvolution( img, kernel_horiz, kernel_vert, out );
}

/**
 ** Compute (isotropic) gaussian filtering of an image using filter width of k *
 *sigma
 ** @param img Input image
 ** @param sigma standard deviation of kernel
 ** @param out Output image
 ** @param k confidence interval param - kernel is width k * sigma * 2 + 1 --
 *using k = 3 gives 99% of gaussian curve
 ** @param border_mgmt either BORDER_COPY or BORDER_CROP to tell what to do with
 *borders
 **/
template <typename Image>
void ImageGaussianFilter( const Image& img,
                          const double sigma,
                          Image&       out,
                          const int    k          = 3,
                          const bool   normalized = true )
{
  // Compute Gaussian filter
  const int k_size      = (int)2 * k * sigma + 1;
  const int half_k_size = k_size / 2;

  double       scale     = ( normalized ) ? 1.0 : 1.0 / ( std::sqrt( 2.0 * M_PI ) * sigma );
  const double exp_scale = 1.0 / ( 2.0 * sigma * sigma );

  // Compute 1D Gaussian filter
  Vec kernel_horiz( k_size );

  double sum = 0;
  for ( int i = 0; i < k_size; ++i )
  {
    const double dx   = ( i - half_k_size );
    kernel_horiz( i ) = scale * exp( -dx * dx * exp_scale );
    sum += kernel_horiz( i );
  }

  // Normalize kernel (to have \sum_i kernel_horiz( i ) = 1 and avoid energy loss)
  if ( normalized )
  {
    const double inv = 1.0 / sum;
    for ( int i = 0; i < k_size; ++i )
    {
      kernel_horiz( i ) *= inv;
    }
  }

  // Vertical kernel is the same as the horizontal one
  const Vec& kernel_vert = kernel_horiz;

  ImageSeparableConvolution( img, kernel_horiz, kernel_vert, out );
}

/**
 ** @brief Compute 1D gaussian kernel of specified width
 ** @param size Size of kernel (0 for automatic window)
 ** @param sigma Gaussian scale
 ** @return Kernel using specified parameters
 **/
inline Vec ComputeGaussianKernel( const size_t size, const double sigma, const bool normalized = true )
{
  // If kernel size is 0 computes its size using uber formula
  size_t k_size = ( size == 0 ) ? ceil( 2.0 * ( 1.0 + ( sigma - 0.8 ) / ( 0.3 ) ) ) : size;

  // Make kernel odd width
  k_size                   = ( k_size % 2 == 0 ) ? k_size + 1 : k_size;
  const size_t half_k_size = ( k_size - 1 ) / 2;

  Vec res( k_size );

  double       scale     = ( normalized ) ? 1.0 : 1.0 / ( std::sqrt( 2.0 * M_PI ) * sigma );
  const double exp_scale = 1.0 / ( 2.0 * sigma * sigma );

  // Compute unnormalized kernel
  double sum = 0.0;
  for ( size_t i = 0; i < k_size; ++i )
  {
    const double dx = ( static_cast<double>( i ) - static_cast<double>( half_k_size ) );
    res( i )        = scale * exp( -dx * dx * exp_scale );
    sum += res( i );
  }

  // Normalize kernel
  if ( normalized )
  {

    const double inv = 1.0 / sum;
    for ( size_t i = 0; i < k_size; ++i )
    {
      res( i ) *= inv;
    }
  }

  return res;
}

/**
  * @brief Compute 1D gaussian central difference kernel of specified width
  * @param size Size of kernel (0 for automatic window)
  * @param sigma Gaussian scale
  * @return Kernel using specified parameters
  */
inline Vec ComputeGaussianCentralDifferenceKernel( const size_t size, const double sigma )
{
  // If kernel size is 0 computes its size using uber formula
  size_t k_size = ( size == 0 ) ? ceil( 2.0 * ( 1.0 + ( sigma - 0.8 ) / ( 0.3 ) ) ) : size;

  // Make kernel odd width
  k_size                   = ( k_size % 2 == 0 ) ? k_size + 1 : k_size;
  const size_t half_k_size = ( k_size - 1 ) / 2;

  Vec res( k_size );

  double       scale     = 1.0 / ( std::sqrt( 2.0 * M_PI ) * sigma );
  const double exp_scale = 1.0 / ( 2.0 * sigma * sigma );

  // Compute unnormalized kernel
  double sum = 0.0;
  for ( size_t i = 0; i < k_size; ++i )
  {
    const double dx  = ( static_cast<double>( i ) - static_cast<double>( half_k_size ) );
    const double dxp = dx + 1.0;
    const double dxm = dx - 1.0;
    res( i )         = scale * 0.5 * ( std::exp( -dxp * dxp * exp_scale ) - std::exp( -dxm * dxm * exp_scale ) );
    sum += res( i );
  }

  // Normalize kernel
  /*
  const double inv = 1.0 / sum;
  for ( size_t i = 0; i < k_size; ++i )
  {
    res( i ) *= inv;
  }
  */

  return res;
}

/**
 * @brief Compute 1d gaussian filter times x :
 * x * g(x)
 * 
 * @param size    width of the kernel  
 * @param sigma   sigma of the gaussian kernel 
 * @return the kernel
 */
inline Vec ComputeGaussianTimesXKernel( const size_t size, const double sigma )
{
  // If kernel size is 0 computes its size using uber formula
  size_t k_size = ( size == 0 ) ? ceil( 2.0 * ( 1.0 + ( sigma - 0.8 ) / ( 0.3 ) ) ) : size;

  // Make kernel odd width
  k_size                   = ( k_size % 2 == 0 ) ? k_size + 1 : k_size;
  const size_t half_k_size = ( k_size - 1 ) / 2;

  Vec res( k_size );

  const double scale     = 1.f / ( std::sqrt( 2.f * M_PI ) * sigma );
  const double exp_scale = 1.0 / ( 2.0 * sigma * sigma );

  // Compute unnormalized kernel
  double sum = 0.0;
  for ( size_t i = 0; i < k_size; ++i )
  {
    const double dx = ( static_cast<double>( i ) - static_cast<double>( half_k_size ) );
    res( i )        = scale * dx * exp( -dx * dx * exp_scale );
    sum += res( i );
  }

  return res;
}

/**
 * @brief Compute 1d gaussian filter times x^2 :
 * x^2 * g(x)
 * 
 * @param size    width of the kernel  
 * @param sigma   sigma of the gaussian kernel 
 * @return the kernel
 */
inline Vec ComputeGaussianTimesXSquaredKernel( const size_t size, const double sigma )
{
  // If kernel size is 0 computes its size using uber formula
  size_t k_size = ( size == 0 ) ? ceil( 2.0 * ( 1.0 + ( sigma - 0.8 ) / ( 0.3 ) ) ) : size;

  // Make kernel odd width
  k_size                   = ( k_size % 2 == 0 ) ? k_size + 1 : k_size;
  const size_t half_k_size = ( k_size - 1 ) / 2;

  Vec res( k_size );

  const double scale     = 1.f / ( std::sqrt( 2.f * M_PI ) * sigma );
  const double exp_scale = 1.0 / ( 2.0 * sigma * sigma );

  // Compute unnormalized kernel
  double sum = 0.0;
  for ( size_t i = 0; i < k_size; ++i )
  {
    const double dx  = ( static_cast<double>( i ) - static_cast<double>( half_k_size ) );
    const double dx2 = dx * dx;
    res( i )         = scale * dx2 * exp( -dx2 * exp_scale );
    sum += res( i );
  }

  return res;
}

/**
 ** Compute gaussian filtering of an image using user defined filter widths
 ** @param img Input image
 ** @param sigma standard deviation of kernel
 ** @param out Output image
 ** @param kernel_size_x Size of horizontal kernel (must be an odd number or 0
 *for automatic computation)
 ** @param kernel_size_y Size of vertical kernel (must be an add number or 0 for
 *automatic computation)
 **/
template <typename Image>
void ImageGaussianFilter( const Image& img,
                          const double sigma,
                          Image&       out,
                          const size_t kernel_size_x,
                          const size_t kernel_size_y )
{
  assert( kernel_size_x % 2 == 1 || kernel_size_x == 0 );
  assert( kernel_size_y % 2 == 1 || kernel_size_y == 0 );

  const Vec kernel_horiz = ComputeGaussianKernel( kernel_size_x, sigma );
  const Vec kernel_vert  = ComputeGaussianKernel( kernel_size_y, sigma );

  ImageSeparableConvolution( img, kernel_horiz, kernel_vert, out );
}

/**
 * @brief Compute X-derivative of a gaussian of an image 
 * 
 * @param img             Input image 
 * @param sigma           gaussian parameter 
 * @param out             Output image 
 * @param kernel_size_x   Kernel size on x 
 * @param kernel_size_y   Kernel size on y 
 */
template <typename Image>
void ImageGaussianXDerivativeFilter( const Image& img,
                                     const double sigma,
                                     Image&       out,
                                     const size_t kernel_size_x = 0,
                                     const size_t kernel_size_y = 0 )
{
  const Vec kernel_horiz = ComputeGaussianCentralDifferenceKernel( kernel_size_x, sigma );
  const Vec kernel_vert  = ComputeGaussianKernel( kernel_size_y, sigma , false );

  ImageSeparableConvolution( img, kernel_horiz, kernel_vert, out );
}

/**
 * @brief Compute Y-derivative of a gaussian of an image 
 * 
 * @param img             Input image 
 * @param sigma           gaussian parameter 
 * @param out             Output image 
 * @param kernel_size_x   Kernel size on x 
 * @param kernel_size_y   Kernel size on y 
 */
template <typename Image>
void ImageGaussianYDerivativeFilter( const Image& img,
                                     const double sigma,
                                     Image&       out,
                                     const size_t kernel_size_x = 0,
                                     const size_t kernel_size_y = 0 )
{
  const Vec kernel_horiz = ComputeGaussianKernel( kernel_size_y, sigma , false );
  const Vec kernel_vert  = ComputeGaussianCentralDifferenceKernel( kernel_size_x, sigma );

  ImageSeparableConvolution( img, kernel_horiz, kernel_vert, out );
}

/**
 * @brief Compute Rotation of given gradient 
 * 
 * @tparam Image 
 * @param gx        Initial gradient on X 
 * @param gy        Initial gradient on Y 
 * @param a_rad     Rotation angle (in radian)
 * @param out_gx    Output gradient on X after rotation 
 * @param out_gy    Output gradient on Y after rotation 
 */
template <typename Image>
void ImageRotatedGradient( const Image& gx,
                           const Image& gy,
                           const double a_rad,
                           Image&       out_gx,
                           Image&       out_gy )
{
  const double ca = std::cos( a_rad );
  const double sa = std::sin( a_rad );

  out_gx = ca * gx + sa * gy;
  out_gy = ca * gy - sa * gx;
}

/**
 * @brief Normalize image such as it has values in range [r_min;r_max]
 * 
 * @tparam Image 
 * @param img 
 * @param out 
 * 
 * Note: only works for mono channel image 
 */
template <typename Image>
void ImageNormalize( const Image&                  img,
                     Image&                        out,
                     const typename Image::Tpixel& r_min = typename Image::Tpixel( 0 ),
                     const typename Image::Tpixel& r_max = typename Image::Tpixel( 1 ) )
{
  const typename Image::Tpixel currentMin   = img.minCoeff();
  const typename Image::Tpixel currentMax   = img.maxCoeff();
  const typename Image::Tpixel currentRange = currentMax - currentMin;

  const typename Image::Tpixel outRange = r_max - r_min;

  out = ( ( ( img.array() - currentMin ) / currentRange ) * outRange + r_min ).matrix();
}

} // namespace image
} // namespace openMVG

#endif // OPENMVG_IMAGE_IMAGE_FILTERING_HPP
