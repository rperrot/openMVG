#ifndef _OPENMVG_MVS_UTIL_HPP_
#define _OPENMVG_MVS_UTIL_HPP_

#include "openMVG/numeric/numeric.h"

#include <bitset>
#include <cmath>
#include <string>

/*
#define MAX_COST_PM 10e6
#define MAX_COST_NCC 2.0

//#define DO_NCC
#ifdef DO_NCC
  #define MAXIMUM_COST MAX_COST_NCC
#else
  #define MAXIMUM_COST MAX_COST_PM
#endif
*/


namespace MVS
{
/**
* @brief Convert a string to a lowercase form
*/
std::string to_lower( const std::string & str ) ;

// Count the number of bit set to one
// @see wikipedia page hamming_weight
// @todo enable popcnt on CPU that supports it
static inline int popcount( const unsigned long long val )
{
  uint64_t x = val ;

  uint64_t m1 = 0x5555555555555555ll;
  uint64_t m2 = 0x3333333333333333ll;
  uint64_t m4 = 0x0F0F0F0F0F0F0F0Fll;
  uint64_t h01 = 0x0101010101010101ll;

  x -= ( x >> 1 ) & m1;
  x = ( x & m2 ) + ( ( x >> 2 ) & m2 );
  x = ( x + ( x >> 4 ) ) & m4;

  return ( x * h01 ) >> 56;
}

/**
* @brief Clamp value in a given range
* @param val Value to clamp
* @param min Minimum bound of the wanted range
* @param max Maximum bound of the wanted range
* @return value in range
*/
template< typename T >
T Clamp( const T & val , const T & min , const T & max )
{
  return ( val < min ) ? min :
         ( ( val > max ) ? max : val ) ;
}

/**
* @brief Compute l1 norm of a given value
* @param val Value
* @return l1 norm of the given value
*/
static inline double l1_norm( const double val )
{
  return std::fabs( val ) ;
}

/**
* @brief Given a gamma value, computes exp( - i / gamma ) for i in range [0;255]
* @param gamma, gamma value
* @return exponential table
*/
std::vector< double > GetExpTable( const double gamma );


/**
* @brief Get content of a file into a string
* @param path Input path
* @return content of the file specified by path
*/
std::string GetFileContent( const std::string &path ) ;

/**
* @brief Compute angle in radian between two normalized vectors
* @param v1 first vector
* @param v2 second vector
* @return angle between v1 and v2
*/
double AngleBetween( const openMVG::Vec3 & v1 , const openMVG::Vec3 & v2 ) ;

/**
* @brief Sample a unit direction
* @param u1 random value (between 0-1)
* @param u2 random value (between 0-1)
* @param alpha_max Maximum angle of the cone to sample (in radian)
* @return direction with solid angle centered on Z
*/
openMVG::Vec3 UniformSampleWRTSolidAngle( const double u1 , const double u2 , const double alpha_max ) ;


/**
* @brief Sample a unit direction
* @param u1 random value (between 0-1)
* @param u2 random value (between 0-1)
* @param alpha_max Maximum angle of the cone to sample (in radian)
* @param n Sampling direction
* @return direction with solid angle centered on Z
*/
openMVG::Vec3 UniformSampleWRTSolidAngle( const double u1 , const double u2 , const double alpha_max , const openMVG::Vec3 & n );


/**
* @brief Given a direction generate an ortho-normalized coordinate frame
* @param n Main direction (will be the z of the new coordinate frame)
* @param[out] x New x direction
* @param[out] y New y direction
*/
void GenerateNormalizedFrame( const openMVG::Vec3 & n , openMVG::Vec3 & x , openMVG::Vec3 & y ) ;

/**
* @brief Computes Barycentric coordinates of P in triangle (A,B,C)
* @param A First point of the triangle
* @param B Second point of the triangle
* @param C Third point of the triangle
* @param p point to compute
* @return (alpha,beta,gamma) The barycentric coordinates of P in (A,B,C)
* @note p is : alpha * A + beta * B + gamma * C
*/
openMVG::Vec3 BarycentricCoordinates( const openMVG::Vec3 & A , const openMVG::Vec3 & B , const openMVG::Vec3 & C ,
                                      const openMVG::Vec3 & p ) ;
} // namespace MVS



#endif