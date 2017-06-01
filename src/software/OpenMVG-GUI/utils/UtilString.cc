#include "UtilString.hh"

#include <fstream>

namespace openMVG_gui
{

/**
* @brief get full content of a file into a string
* @param path of the file
* @return string content of the file
* @note if file could not be read, return an empty string
*/
std::string fileContent( const std::string & path )
{
  // TODO : maybe a c interface fseek/fread should give better perf
  std::ifstream file( path );
  if( ! file )
  {
    return "" ;
  }

  std::string content( ( std::istreambuf_iterator<char>( file ) ),
                       ( std::istreambuf_iterator<char>()    ) );

  return content ;
}

} // namespace openMVG_gui