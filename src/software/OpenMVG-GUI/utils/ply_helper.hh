#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_PLY_HELPER_HH
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_PLY_HELPER_HH

#include "openMVG/numeric/numeric.h"

#include <fstream>
#include <sstream>


namespace openMVG_gui
{

// trim from start
static inline std::string &ltrim( std::string &s )
{
  s.erase( s.begin(), std::find_if( s.begin(), s.end(),
                                    std::not1( std::ptr_fun<int, int>( std::isspace ) ) ) );
  return s;
}

// trim from end
static inline std::string &rtrim( std::string &s )
{
  s.erase( std::find_if( s.rbegin(), s.rend(),
                         std::not1( std::ptr_fun<int, int>( std::isspace ) ) )
           .base(),
           s.end() );
  return s;
}

// trim from both ends
static inline std::string &trim( std::string &s )
{
  return ltrim( rtrim( s ) );
}

// from : http://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
static inline std::vector<std::string> tokenize( const std::string &str )
{
  // construct a stream from the string
  std::stringstream strstr( str );

  // use stream iterators to copy the stream to the vector as whitespace separated strings
  std::istream_iterator<std::string> it( strstr );
  std::istream_iterator<std::string> end;
  std::vector<std::string> results( it, end );

  return results;
}

/**
* @brief Store different kind of endianess that can be found on a PLY file
*/
enum ply_endianness
{
  PLY_ASCII,
  PLY_BIG_ENDIAN,
  PLY_LITTLE_ENDIAN
};

/**
* @brief Compute runtime endianess
* @return Current system endianess
*/
static inline ply_endianness GetSystemEndianness()
{
  short int tmp = 0x1;
  char *first   = ( char * )&tmp;
  return ( first[ 0 ] == 1 ) ? PLY_LITTLE_ENDIAN : PLY_BIG_ENDIAN;
}

/**
* @brief Fuction used to swap bytes (from LE to BE)
* @param val Value to inverse
*/
template <typename T>
T ByteSwap( T val )
{
  T retVal;
  char *pVal    = ( char * )&val;
  char *pRetVal = ( char * )&retVal;
  int size      = sizeof( T );
  for ( int i = 0; i < size; i++ )
  {
    pRetVal[ size - 1 - i ] = pVal[ i ];
  }

  return retVal;
}

/**
* @brief Class used to write a value using a specified PLY endianness
*/
template <int Endianness>
struct EndianAgnosticWriter
{
  public:
    /**
    * @brief Ctr
    */
    EndianAgnosticWriter();

    /**
    * @brief Write a 3d vector of double in a file
    * @param file File in which data will be written
    * @param vec vector to write
    */
    void Write( std::ofstream &file, const openMVG::Vec3 &vec );

    /**
    * @brief Write a 3d vector of unsigned char in a file
    * @param file File in which data will be written
    * @param vec vector to write
    */
    void Write( std::ofstream &file, const openMVG::Vec3uc &vec );

  private:
    /// Current system endianess
    ply_endianness m_system_endianness;
};

// Specialization for ascii
template <>
struct EndianAgnosticWriter<PLY_ASCII>
{
  public:
    /**
    * @brief Ctr
    */
    EndianAgnosticWriter()
      : m_system_endianness( GetSystemEndianness() )
    {
    }

    /**
    * @brief Write a 3d vector of double in a file
    * @param file File in which data will be written
    * @param vec vector to write
    */
    void Write( std::ofstream &file, const openMVG::Vec3 &vec )
    {
      file << vec[ 0 ] << " " << vec[ 1 ] << " " << vec[ 2 ] << " ";
    }

    /**
    * @brief Write a 3d vector of unsigned char in a file
    * @param file File in which data will be written
    * @param vec vector to write
    */
    void Write( std::ofstream &file, const openMVG::Vec3uc &vec )
    {
      file << ( int )vec[ 0 ] << " " << ( int )vec[ 1 ] << " " << ( int )vec[ 2 ] << " ";
    }

  private:
    /// Current system endianness
    ply_endianness m_system_endianness;
};

template <>
struct EndianAgnosticWriter<PLY_LITTLE_ENDIAN>
{
  public:
    /**
    * @brief Ctr
    */
    EndianAgnosticWriter()
      : m_system_endianness( GetSystemEndianness() )
    {
    }

    /**
    * @brief Write a 3d vector of double in a file
    * @param file File in which data will be written
    * @param vec vector to write
    */
    void Write( std::ofstream &file, const openMVG::Vec3 &vec )
    {
      if ( m_system_endianness == PLY_LITTLE_ENDIAN )
      {
        // No conversion
        file.write( reinterpret_cast<const char *>( vec.data() ), sizeof( openMVG::Vec3 ) );
      }
      else
      {
        // Conversion
        const openMVG::Vec3 r( ByteSwap( vec[ 0 ] ), ByteSwap( vec[ 1 ] ), ByteSwap( vec[ 2 ] ) );
        file.write( reinterpret_cast<const char *>( r.data() ), sizeof( openMVG::Vec3 ) );
      }
    }

    /**
    * @brief Write a 3d vector of unsigned char in a file
    * @param file File in which data will be written
    * @param vec vector to write
    */
    void Write( std::ofstream &file, const openMVG::Vec3uc &vec )
    {
      // No reverse because sizeof char == 1
      file.write( reinterpret_cast<const char *>( vec.data() ), sizeof( openMVG::Vec3uc ) );
    }

  private:
    /// Current system endianness
    ply_endianness m_system_endianness;
};

template <>
struct EndianAgnosticWriter<PLY_BIG_ENDIAN>
{
  public:
    /**
    * @brief Ctr
    */
    EndianAgnosticWriter()
      : m_system_endianness( GetSystemEndianness() )
    {
    }

    /**
    * @brief Write a 3d vector of double in a file
    * @param file File in which data will be written
    * @param vec vector to write
    */
    void Write( std::ofstream &file, const openMVG::Vec3 &vec )
    {
      if ( m_system_endianness == PLY_BIG_ENDIAN )
      {
        // No conversion
        file.write( reinterpret_cast<const char *>( vec.data() ), sizeof( openMVG::Vec3 ) );
      }
      else
      {
        // Conversion
        const openMVG::Vec3 r( ByteSwap( vec[ 0 ] ), ByteSwap( vec[ 1 ] ), ByteSwap( vec[ 2 ] ) );
        file.write( reinterpret_cast<const char *>( r.data() ), sizeof( openMVG::Vec3 ) );
      }
    }

    /**
    * @brief Write a 3d vector of unsigned char in a file
    * @param file File in which data will be written
    * @param vec vector to write
    */
    void Write( std::ofstream &file, const openMVG::Vec3uc &vec )
    {
      // No reverse because sizeof char == 1
      file.write( reinterpret_cast<const char *>( vec.data() ), sizeof( openMVG::Vec3uc ) );
    }

  private:
    /// Current system endianness
    ply_endianness m_system_endianness;
};

template <typename T>
void Convert( const std::string &str, T &val )
{
  std::stringstream sstr( str );
  sstr >> val;
}

/**
* @brief Class used to read Vector data in agnostic way
*/
template <int Endianness>
class EndianAgnosticReader
{
  public:
    /**
      * @brief ctr
      */
    EndianAgnosticReader();

    /**
      * @brief Read vector data in double format
      * @param file Stream in which data is read
      * @param[out] vec output vector
      */
    void Read( std::ifstream &file, openMVG::Vec3 &vec );

    /**
      * @brief Read vector data in unsigned char format
      * @param file Stream in which data is read
      * @param[out] vec output vector
      */
    void Read( std::ifstream &file, openMVG::Vec3uc &vec );

  private:
    /// Current system
    ply_endianness m_system_endianness;
};

/**
* @brief Class used to read Vector data in agnostic way
* -> Specialization for ascii data
*/
template <>
class EndianAgnosticReader<PLY_ASCII>
{
  public:
    /**
      * @brief ctr
      */
    EndianAgnosticReader()
      : m_system_endianness( GetSystemEndianness() )
    {
    }

    /**
      * @brief Read vector data in double format
      * @param file Stream in which data is read
      * @param[out] vec output vector
      */
    void Read( std::ifstream &file, openMVG::Vec3 &vec )
    {
      double x, y, z;
      file >> x >> y >> z;
      vec = openMVG::Vec3( x, y, z );
    }

    /**
      * @brief Read vector data in unsigned char format
      * @param file Stream in which data is read
      * @param[out] vec output vector
      */
    void Read( std::ifstream &file, openMVG::Vec3uc &vec )
    {
      int a, b, c;
      file >> a >> b >> c;
      vec = openMVG::Vec3uc( a, b, c );
    }

  private:
    /// Current system
    ply_endianness m_system_endianness;
};

/**
* @brief Class used to read Vector data in agnostic way
* -> Specialization for ascii data
*/
template <>
class EndianAgnosticReader<PLY_LITTLE_ENDIAN>
{
  public:
    /**
      * @brief ctr
      */
    EndianAgnosticReader()
      : m_system_endianness( GetSystemEndianness() )
    {
    }

    /**
      * @brief Read vector data in double format
      * @param file Stream in which data is read
      * @param[out] vec output vector
      */
    void Read( std::ifstream &file, openMVG::Vec3 &vec )
    {
      double data[ 3 ];
      file.read( reinterpret_cast<char *>( data ), 3 * sizeof( double ) );

      if ( m_system_endianness != PLY_LITTLE_ENDIAN )
      {
        data[ 0 ] = ByteSwap( data[ 0 ] );
        data[ 1 ] = ByteSwap( data[ 1 ] );
        data[ 2 ] = ByteSwap( data[ 2 ] );
      }
      vec = openMVG::Vec3( data[ 0 ], data[ 1 ], data[ 2 ] );
    }

    /**
      * @brief Read vector data in unsigned char format
      * @param file Stream in which data is read
      * @param[out] vec output vector
      */
    void Read( std::ifstream &file, openMVG::Vec3uc &vec )
    {
      unsigned char data[ 3 ];
      file.read( reinterpret_cast<char *>( data ), 3 * sizeof( unsigned char ) );
      vec = openMVG::Vec3uc( data[ 0 ], data[ 1 ], data[ 2 ] );
    }

  private:
    /// Current system
    ply_endianness m_system_endianness;
};

/**
* @brief Class used to read Vector data in agnostic way
* -> Specialization for ascii data
*/
template <>
class EndianAgnosticReader<PLY_BIG_ENDIAN>
{
  public:
    /**
      * @brief ctr
      */
    EndianAgnosticReader()
      : m_system_endianness( GetSystemEndianness() )
    {
    }

    /**
      * @brief Read vector data in double format
      * @param file Stream in which data is read
      * @param[out] vec output vector
      */
    void Read( std::ifstream &file, openMVG::Vec3 &vec )
    {
      double data[ 3 ];
      file.read( reinterpret_cast<char *>( data ), 3 * sizeof( double ) );

      if ( m_system_endianness != PLY_BIG_ENDIAN )
      {
        data[ 0 ] = ByteSwap( data[ 0 ] );
        data[ 1 ] = ByteSwap( data[ 1 ] );
        data[ 2 ] = ByteSwap( data[ 2 ] );
      }
      vec = openMVG::Vec3( data[ 0 ], data[ 1 ], data[ 2 ] );
    }

    /**
      * @brief Read vector data in unsigned char format
      * @param file Stream in which data is read
      * @param[out] vec output vector
      */
    void Read( std::ifstream &file, openMVG::Vec3uc &vec )
    {
      unsigned char data[ 3 ];
      file.read( reinterpret_cast<char *>( data ), 3 * sizeof( unsigned char ) );
      vec = openMVG::Vec3uc( data[ 0 ], data[ 1 ], data[ 2 ] );
    }

  private:
    /// Current system
    ply_endianness m_system_endianness;
};

} // namespace openMVG_gui

#endif