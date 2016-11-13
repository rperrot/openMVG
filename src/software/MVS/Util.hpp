#ifndef _OPENMVG_MVS_UTIL_HPP_
#define _OPENMVG_MVS_UTIL_HPP_

#include "openMVG/numeric/numeric.h"

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