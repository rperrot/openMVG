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
    return std::bitset< 64 >( val ).count() ;
    /*
    static const unsigned long m1  = 0x5555555555555555; //binary: 0101...
    static const unsigned long m2  = 0x3333333333333333; //binary: 00110011..
    static const unsigned long m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...

    unsigned long x = val ;
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits
    x += x >>  8;  //put count of each 16 bits into their lowest 8 bits
    x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
    x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
    return x & 0x7f;
    */
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
    return val < min ? min :
           ( val > max ? max : val ) ;
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
}



#endif