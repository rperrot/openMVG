// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "PlyLoader.hh"

#include "ply_helper.hh"

#include <iostream>
#include <fstream>

namespace openMVG_gui
{

/**
  * @brief Load a ply file and gets is content points
  * @param path Path of the file to load
  * @param[out] pts The list of points in the given file
  * @param[out] nor The list of normals in the given file
  * @param[out] col The list of colors in the given file
  * @note If nor or col are equal to nullptr, nothing is retrived
  * @note If file does not contains nor or col, corresponding vectors will be empty
  * @retval true if load is successful
  * @retval false if load fails
  */
static inline bool PLYRead( const std::string &path,
                            std::vector<openMVG::Vec3> &pts,
                            std::vector<openMVG::Vec3> *nor,
                            std::vector<openMVG::Vec3uc> *col )
{
  std::ifstream file( path );
  if ( !file )
  {
    return false;
  }

  // Read header
  std::string line;
  std::getline( file, line );
  if ( trim( line ) != "ply" )
  {
    return false;
  }

  int nb_coord_pts = 0;
  int nb_coord_nor = 0;
  int nb_coord_col = 0;

  size_t property_pts_size = 0;
  size_t property_nor_size = 0;
  size_t property_col_size = 0;

  size_t nb_elt = 0;

  ply_endianness endianness = PLY_ASCII;

  while ( 1 )
  {
    std::getline( file, line );
    std::vector<std::string> tokens = tokenize( line );
    if ( tokens.size() == 0 )
    {
      continue;
    }
    if ( tokens[ 0 ] == "comment" )
    {
      continue;
    }
    else if ( tokens[ 0 ] == "format" )
    {
      if ( tokens.size() > 1 )
      {
        if ( tokens[ 1 ] == "ascii" )
        {
          endianness = PLY_ASCII;
        }
        else if ( tokens[ 1 ] == "binary_little_endian" )
        {
          endianness = PLY_LITTLE_ENDIAN;
        }
        else if ( tokens[ 1 ] == "binary_big_endian" )
        {
          endianness = PLY_BIG_ENDIAN;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else if ( tokens[ 0 ] == "element" )
    {
      if ( tokens.size() > 2 )
      {
        if ( tokens[ 1 ] == "vertex" )
        {
          Convert( tokens[ 2 ], nb_elt );
        }
      }
      else
      {
        return false;
      }
    }
    else if ( tokens[ 0 ] == "property" )
    {
      size_t current_size;
      if ( tokens.size() > 2 )
      {
        // read size
        const std::string type = tokens[ 1 ];
        if ( type == "uchar" || type == "char" )
        {
          current_size = 1;
        }
        else if ( type == "short" || type == "ushort" )
        {
          current_size = 2;
        }
        else if ( type == "int" || type == "uint" )
        {
          current_size = 4;
        }
        else if ( type == "float" )
        {
          current_size = 4;
        }
        else if ( type == "double" )
        {
          current_size = 8;
        }
        // read kind (x,y,z, nx,ny,nz, red, green, blue )
        const std::string item = tokens[ 2 ];
        if ( item == "x" || item == "y" || item == "z" )
        {
          property_pts_size = current_size;
          ++nb_coord_pts;
        }
        else if ( item == "nx" || item == "ny" || item == "nz" )
        {
          property_nor_size = current_size;
          ++nb_coord_nor;
        }
        else if ( item == "red" || item == "green" || item == "blue" )
        {
          property_col_size = current_size;
          ++nb_coord_col;
        }
      }
    }
    else if ( tokens[ 0 ] == "end_header" )
    {
      break;
    }
  }

  if ( nb_coord_pts != 3 )
  {
    std::cout << "nb_coord_pts != 3" << std::endl;
    return false;
  }
  if ( !( nb_coord_nor == 0 || nb_coord_nor == 3 ) )
  {
    std::cout << "!( nb_coord_nor == 0 || nb_coord_nor == 3 )" << __LINE__ << std::endl;
    return false;
  }
  if ( !( nb_coord_col == 0 || nb_coord_col == 3 ) )
  {
    std::cout << "!( nb_coord_col == 0 || nb_coord_col == 3 )" << __LINE__ << std::endl;
    return false;
  }
  if ( nb_elt == 0 )
  {
    std::cout << "nb_elt == 0" << __LINE__ << std::endl;
    return false;
  }
  if ( nb_coord_pts == 3 && property_pts_size == 0 )
  {
    std::cout << "nb_coord_pts == 3 && property_pts_size == 0" << __LINE__ << std::endl;
    return false;
  }
  if ( nb_coord_col == 3 && property_col_size == 0 )
  {
    std::cout << "nb_coord_col == 3 && property_col_size == 0" << __LINE__ << std::endl;
    return false;
  }
  if ( nb_coord_nor == 3 && property_nor_size == 0 )
  {
    std::cout << "nb_coord_nor == 3 && property_nor_size == 0" << __LINE__ << std::endl;
    return false;
  }

  // resize points
  pts.resize( nb_elt );
  if ( nor )
  {
    if ( nb_coord_nor == 3 )
    {
      nor->resize( nb_elt );
    }
    else
    {
      nor->clear();
    }
  }

  if ( col )
  {
    if ( nb_coord_col == 3 )
    {
      col->resize( nb_elt );
    }
    else
    {
      col->clear();
    }
  }

  EndianAgnosticReader<PLY_ASCII> ascii_reader;
  EndianAgnosticReader<PLY_LITTLE_ENDIAN> le_reader;
  EndianAgnosticReader<PLY_BIG_ENDIAN> be_reader;

  // now read the points
  for ( size_t id_point = 0; id_point < nb_elt; ++id_point )
  {
    if ( endianness == PLY_ASCII )
    {
      ascii_reader.Read( file, pts[ id_point ] );
      if ( nb_coord_nor > 0 )
      {
        if( nor )
        {
          ascii_reader.Read( file, ( *nor )[ id_point ] );
        }
        else
        {
          openMVG::Vec3 tmp ;
          ascii_reader.Read( file , tmp ) ;
        }
      }
      if ( nb_coord_col > 0 )
      {
        if( col )
        {
          ascii_reader.Read( file, ( *col )[ id_point ] );
        }
        else
        {
          openMVG::Vec3uc tmp ;
          ascii_reader.Read( file, tmp ) ;
        }
      }
      // Skip everything until
      std::string tmp;
      std::getline( file, tmp );
    }
    else if ( endianness == PLY_LITTLE_ENDIAN )
    {
      le_reader.Read( file, pts[ id_point ] );
      if ( nb_coord_nor > 0 )
      {
        if( nor )
        {
          le_reader.Read( file, ( *nor )[ id_point ] );
        }
        else
        {
          openMVG::Vec3 tmp ;
          le_reader.Read( file, tmp );
        }
      }
      if ( nb_coord_col > 0 )
      {
        if( col )
        {
          le_reader.Read( file, ( *col )[ id_point ] );
        }
        else
        {
          openMVG::Vec3uc tmp ;
          le_reader.Read( file, tmp );
        }
      }
    }
    else if ( endianness == PLY_BIG_ENDIAN )
    {
      be_reader.Read( file, pts[ id_point ] );
      if ( nb_coord_nor > 0 )
      {
        if( nor )
        {
          be_reader.Read( file, ( *nor )[ id_point ] );
        }
        else
        {
          openMVG::Vec3 tmp ;
          be_reader.Read( file, tmp );
        }
      }
      if ( nb_coord_col > 0 )
      {
        if( col )
        {
          be_reader.Read( file, ( *col )[ id_point ] );
        }
        else
        {
          openMVG::Vec3uc tmp ;
          be_reader.Read( file, tmp );
        }
      }
    }
  }

  return true;
}

/**
* @brief Load a ply file
* @param path Input path
* @param[out] pos Position
* @param[out] col Color
*/
void LoadPly( const std::string & path ,
              std::vector< openMVG::Vec3 > & pos ,
              std::vector< openMVG::Vec3 > & col )
{
  pos.clear() ;
  col.clear() ;

  std::vector< openMVG::Vec3uc > tmp_col ;
  PLYRead( path , pos , nullptr , &tmp_col ) ;

  for( auto & it : tmp_col )
  {
    openMVG::Vec3 tmp ;
    tmp[0] = it[0] / 255.0 ;
    tmp[1] = it[1] / 255.0 ;
    tmp[2] = it[2] / 255.0 ;
    col.emplace_back( tmp ) ;
  }
}


} // namespace openMVG_gui