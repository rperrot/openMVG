#include "UtilFile.hh"

#include <fstream>

namespace openMVG_gui
{
/**
  * @brief Merge two files 
  * 
  * @param path1  Path of the first file 
  * @param path2  Path of the second file 
  * @return true  If success
  * @return false If failure
  */
bool mergeFiles( const std::string &path1, const std::string &path2, const std::string &outPath )
{
  std::ifstream file1( path1, std::ios_base::binary );
  std::ifstream file2( path2, std::ios_base::binary );
  std::ofstream fout( outPath, std::ios_base::binary );

  if ( !file1 || !file2 || !fout )
  {
    return false;
  }

  fout << file1.rdbuf() << file2.rdbuf();

  return true;
}

} // namespace openMVG_gui