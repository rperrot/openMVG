// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_FEATURES_STATS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_FEATURES_STATS_HH_

#include <cstdint>
#include <string>

namespace openMVG_gui
{

/**
* @brief Class holding statistics about features computed inside an image
*/
class FeaturesStats
{
  public:

    /**
    * @param nb_feat Number of features extracted
    * @param elapsed_time Total elapsed time used to extract the features (in second)
    */
    FeaturesStats( const uint32_t nb_feat = 0 , const double elapsed_time = 0.0 ) ;

    FeaturesStats( const FeaturesStats & src ) = default ;
    FeaturesStats( FeaturesStats && src ) = default ;

    FeaturesStats & operator=( const FeaturesStats & src ) = default ;
    FeaturesStats & operator=( FeaturesStats && src ) = default ;

    /**
    * @brief Get number of features
    * @return Number of features extracted
    */
    uint32_t nbFeature( void ) const ;

    /**
    * @brief Get elapsed time for feature extraction
    * @return elapsed time (in second)
    */
    double elapsedTime( void ) const ;

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
    static FeaturesStats load( const std::string & filename ) ;

    /**
    * @brief Serialization
    */
    template< class Archive >
    void serialize( Archive & ar )
    {
      ar( m_nb_features ) ;
      ar( m_elapsed_time ) ;
    }

  private:

    uint32_t m_nb_features ;
    double m_elapsed_time ;
} ;



}

#endif