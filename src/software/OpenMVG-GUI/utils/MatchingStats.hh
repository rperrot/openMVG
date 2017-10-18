// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_MATCHING_STATS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_MATCHING_STATS_HH_

#include <string>

namespace openMVG_gui
{

/**
* @brief Class bringing statistics over matching process
*/
class MatchingStats
{
  public:

    /**
    * @brief Constructor
    * @param putative_elapsed Putative matching elapsed time (in seconds)
    * @param filtering_elapsed Geometric filtering elapsed time (in seconds)
    */
    MatchingStats( const double putative_elapsed = 0.0 ,
                   const double filtering_elapsed = -1.0 ) ;

    MatchingStats( const MatchingStats & src ) = default ;
    MatchingStats( MatchingStats && src ) = default ;

    MatchingStats& operator=( const MatchingStats & src ) = default ;
    MatchingStats& operator=( MatchingStats && src ) = default ;

    /**
    * @brief Get elapsed time for putative matching
    * @return elapsed time (in second)
    */
    double putativeElapsedTime( void ) const ;

    /**
    * @brief Get elapsed time for geometric filtering
    * @return elapsed time (in second)
    */
    double filteringElapsedTime( void ) const ;

    /**
    * @brief Save to disk
    * @param filename Path of the file where data is saved
    */
    void save( const std::string & filename ) ;

    /**
    * @brief Load from disk
    * @param filename Path of the file where data is loaded
    * @return The loaded data
    */
    static MatchingStats load( const std::string & filename ) ;

    /**
    * @brief Serialization
    */
    template< class Archive >
    void serialize( Archive & ar )
    {
      ar( m_putative_elapsed_time ) ;
      ar( m_filtering_elapsed_time ) ;
    }

  private:

    /// Elapsed time (putative)
    double m_putative_elapsed_time ;
    /// Elapsed time (filtering)
    double m_filtering_elapsed_time ;
} ;

} // namespace openMVG_gui

#endif