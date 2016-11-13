#include "Util.hpp"

#include <cmath>
#include <fstream>
#include <iostream>

namespace MVS
{
  std::string GetFileContent( const std::string &path )
  {
    std::ifstream file( path ) ;
    if( ! file )
    {
      std::cerr << "Could not file : " << path << std::endl ;
      return std::string() ;
    }
    std::string res( ( std::istreambuf_iterator<char>( file ) ),
                     ( std::istreambuf_iterator<char>()    ) );
    return res ;
  }

  double AngleBetween( const openMVG::Vec3 & v1 , const openMVG::Vec3 & v2 )
  {
    const double c = v1.dot( v2 ) ;
    return std::acos( MVS::Clamp( c , -1.0 , 1.0 ) ) ;
  }

  std::vector< double > GetExpTable( const double gamma )
  {
    std::vector< double > res ;

    for( int i = 0 ; i < 255 ; ++i )
    {
      res.push_back( std::exp( -static_cast<double>( i ) / gamma ) );
    }
    return res ;
  }


  /**
  * @brief Sample a unit direction
  * @param u1 random value (between 0-1)
  * @param u2 random value (between 0-1)
  * @param alpha_max Maximum angle of the cone to sample (in radian)
  * @return direction with solid angle centered on Z
  */
  openMVG::Vec3 UniformSampleWRTSolidAngle( const double u1 , const double u2 , const double alpha_max )
  {
    // @see Total Compendium Dutre, p19
    static const double pi = 3.141592653589793238462643383279 ;

    const double ct  = ( 1.0 - u1 ) + u1 * std::cos( alpha_max ) ;
    const double st  = std::sqrt( MVS::Clamp( 1.0 - ct * ct , 0.0 , 1.0 ) ) ;
    const double phi = u2 * 2.0 * pi ;

    return openMVG::Vec3( std::cos( phi ) * st , std::sin( phi ) * st , ct ) ;
  }


  /**
  * @brief Sample a unit direction
  * @param u1 random value (between 0-1)
  * @param u2 random value (between 0-1)
  * @param alpha_max Maximum angle of the cone to sample (in radian)
  * @param n Sampling direction
  * @return direction with solid angle centered on Z
  */
  openMVG::Vec3 UniformSampleWRTSolidAngle( const double u1 , const double u2 , const double alpha_max , const openMVG::Vec3 & n )
  {
    openMVG::Vec3 x , y ;
    GenerateNormalizedFrame( n.normalized() , x , y ) ;

    // Project on new frame
    const openMVG::Vec3 dir = UniformSampleWRTSolidAngle( u1 , u2 , alpha_max ) ;

    return openMVG::Vec3( x[0] * dir[0] + y[0] * dir[1] + n[0] * dir[2] ,
                          x[1] * dir[0] + y[1] * dir[1] + n[1] * dir[2] ,
                          x[2] * dir[0] + y[2] * dir[1] + n[2] * dir[2] ) ;
  }

  /**
  * @brief Given a direction generate an ortho-normalized coordinate frame
  * @param n Main direction (will be the z of the new coordinate frame)
  * @param[out] x New x direction
  * @param[out] y New y direction
  */
  void GenerateNormalizedFrame( const openMVG::Vec3 & n , openMVG::Vec3 & x , openMVG::Vec3 & y )
  {
    if( std::fabs( n[0] ) > std::fabs( n[1] ) )
    {
      // build x using nx and nz
      x = openMVG::Vec3( -n[2] , 0.0 , n[0] ) / std::sqrt( n[0] * n[0] + n[2] * n[2] ) ;
    }
    else
    {
      // build x using ny and nz
      x = openMVG::Vec3( 0.0 , n[2] , -n[1] ) / std::sqrt( n[1] * n[1] + n[2] * n[2] ) ;
    }
    y = n.cross( x ) ;
  }


}


