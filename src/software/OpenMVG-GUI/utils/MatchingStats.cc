// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "MatchingStats.hh"

#include "Version.hh"

#include <cereal/archives/xml.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/utility.hpp>

#include <fstream>

namespace openMVG_gui
{

/**
* @brief Constructor
* @param putative_elapsed Putative matching elapsed time (in seconds)
* @param filtering_elapsed Geometric filtering elapsed time (in seconds)
*/
MatchingStats::MatchingStats( const double putative_elapsed ,
                              const double filtering_elapsed )
  : m_putative_elapsed_time( putative_elapsed ) ,
    m_filtering_elapsed_time( filtering_elapsed )
{

}

/**
* @brief Get elapsed time for putative matching
* @return elapsed time (in second)
*/
double MatchingStats::putativeElapsedTime( void ) const
{
  return m_putative_elapsed_time ;
}
/**
* @brief Get elapsed time for geometric filtering
* @return elapsed time (in second)
*/
double MatchingStats::filteringElapsedTime( void ) const
{
  return m_filtering_elapsed_time ;
}


/**
* @brief Save to disk
* @param filename Path of the file where data is saved
*/
void MatchingStats::save( const std::string & filename )
{
  std::ofstream file( filename ) ;
  if( ! file )
  {
    std::cerr << "Could not save " << filename << std::endl ;
    return ;
    // TODO : throw something ?
  }

  cereal::XMLOutputArchive archive( file );

  archive( cereal::make_nvp( "major_version" , GUIVersionMajorNumber() ) ) ;
  archive( cereal::make_nvp( "minor_version" , GUIVersionMinorNumber() ) ) ;
  archive( cereal::make_nvp( "revision_version" , GUIVersionRevisionNumber() ) ) ;

  archive( cereal::make_nvp( "matching_elapsed_time" , m_putative_elapsed_time ) ) ;
  archive( cereal::make_nvp( "filtering_elapsed_time" , m_filtering_elapsed_time ) ) ;
}

/**
* @brief Load from disk
* @param filename Path of the file where data is loaded
* @return The loaded data
*/
MatchingStats MatchingStats::load( const std::string & filename )
{
  std::ifstream file( filename ) ;

  if( ! file )
  {
    // TODO throw something ?
    return MatchingStats( -1.0 , -1.0 ) ;
  }


  MatchingStats res;

  // Save global project state
  cereal::XMLInputArchive archive( file );

  int major_version ;
  int minor_version ;
  int revision_version ;
  archive( cereal::make_nvp( "major_version" , major_version ) ) ;
  archive( cereal::make_nvp( "minor_version" , minor_version ) ) ;
  archive( cereal::make_nvp( "revision_version" , revision_version ) ) ;

  archive( cereal::make_nvp( "matching_elapsed_time" , res.m_putative_elapsed_time ) ) ;
  archive( cereal::make_nvp( "filtering_elapsed_time" , res.m_filtering_elapsed_time ) ) ;

  return res ;
}

} // namespace openMVG_gui