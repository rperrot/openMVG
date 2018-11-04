// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2014 Romuald Perrot, Pierre Moulon.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_IMAGE_IMAGE_CONVOLUTION_BASE_HPP
#define OPENMVG_IMAGE_IMAGE_CONVOLUTION_BASE_HPP

#include "openMVG/system/cpu_instruction_set.hpp"

#ifdef __SSE2__
#include <xmmintrin.h>
#endif

#ifdef __AVX__
#include <immintrin.h>
#endif

#include <cstddef>
#include <iostream>

namespace openMVG
{
namespace image
{

#ifdef __SSE2__
/**
 * @brief Convolution buffer using SSE instructions 
 * 
 * @param buffer      Buffer to convolve 
 * @param kernel      Kernel to use 
 * @param buffer_size Buffer size 
 * @param kernel_size Kernel size 
 */
static inline void conv_buffer_sse( float* buffer, const float* kernel, const int buffer_size, const int kernel_size )
{
  __m128 kernel_sse[ kernel_size ];
  __m128 accumulator;

  for ( int i = 0; i < kernel_size; ++i )
  {
    kernel_sse[ i ] = _mm_set1_ps( kernel[ kernel_size ] );
  }

  int i = 0;
  for ( ; i < buffer_size; i += 4 )
  {
    accumulator = _mm_setzero_ps();

    for ( int j = 0; j < kernel_size; ++j )
    {
      accumulator = _mm_add_ps( accumulator, _mm_mul_ps( _mm_loadu_ps( buffer + i + j ), kernel_sse[ j ] ) );
    }

    _mm_storeu_ps( buffer + i, accumulator );
  }

  // Last data
  for ( ; i < buffer_size; ++i )
  {
    float sum = 0.f;
    for ( int j = 0; j < kernel_size; ++j )
    {
      sum += buffer[ i + j ] * kernel[ j ];
    }
    buffer[ i ] = sum;
  }
}
#endif

#ifdef __AVX__
static inline void conv_buffer_avx( float* buffer, const float* kernel, const int buffer_size, const int kernel_size )
{
  __m256 kernel_avx[ kernel_size ];
  __m256 accumulator;

  for ( int i = 0; i < kernel_size; ++i )
  {
    kernel_avx[ i ] = _mm256_set1_ps( kernel[ i ] );
  }

  int i = 0;
  for ( ; i < buffer_size; i += 8 )
  {
    accumulator = _mm256_setzero_ps();

    for ( int j = 0; j < kernel_size; ++j )
    {
      accumulator = _mm256_add_ps( accumulator, _mm256_mul_ps( _mm256_loadu_ps( buffer + i + j ), kernel_avx[ j ] ) );
    }

    _mm256_store_ps( buffer + i, accumulator );
  }

  // Last data
  for ( ; i < buffer_size; ++i )
  {
    float sum = 0.f;
    for ( int j = 0; j < kernel_size; ++j )
    {
      sum += buffer[ i + j ] * kernel[ j ];
    }
    buffer[ i ] = sum;
  }
}
#endif

inline void conv_buffer_( float* buffer, const float* kernel, const int buffer_size, const int kernel_size )
{
  static system::CpuInstructionSet set;

// DISPATCH on SSE/AVX if available at runtime
#ifdef __AVX__
  if ( set.supportAVX() )
  {
    conv_buffer_avx( buffer, kernel, buffer_size, kernel_size );
    return;
  }
#endif

#ifdef __SSE2__
  if ( set.supportSSE() )
  {
    conv_buffer_sse( buffer, kernel, buffer_size, kernel_size );
    return;
  }
#endif

  // No avx or SSE support
  for ( int i = 0; i < buffer_size; ++i )
  {
    float sum = 0.f;
    for ( int j = 0; j < kernel_size; ++j )
    {
      sum += buffer[ i + j ] * kernel[ j ];
    }
    buffer[ i ] = sum;
  }
}

/**
 ** Filter an extended row [halfKernelSize][row][halfKernelSize]
 ** @param buffer data to filter
 ** @param kernel kernel array
 ** @param rsize buffer length
 ** @param ksize kernel length
**/
template <class T1, class T2>
inline void conv_buffer_( T1* buffer, const T2* kernel, int rsize, int ksize )
{
  for ( size_t i = 0; i < rsize; ++i )
  {
    T2 sum( 0 );
    for ( size_t j = 0; j < ksize; ++j )
    {
      sum += buffer[ i + j ] * kernel[ j ];
    }
    buffer[ i ] = sum;
  }
}

} // namespace image
} // namespace openMVG

#endif // OPENMVG_IMAGE_IMAGE_CONVOLUTION_BASE_HPP
