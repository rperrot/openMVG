// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_FEATURE_PARAMS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_FEATURE_PARAMS_HH_

#include <memory>
#include <vector>

namespace openMVG
{
namespace features
{
class Image_describer ;
}
}

namespace openMVG_gui
{

/**
* @brief The kind of features to use
*/
enum FeatureType
{
  /// Standard vlfeat sift
  FEATURE_TYPE_SIFT ,
  /// OpenMVG sift
  FEATURE_TYPE_SIFT_ANATOMY ,
  /// AKAZE with float description
  FEATURE_TYPE_AKAZE_FLOAT ,
  /// AKAZE with binary description
  FEATURE_TYPE_AKAZE_MLDB
} ;

/**
* @brief The different preset to use
*/
enum FeaturePreset
{
  /// Normal mode
  FEATURE_PRESET_NORMAL ,
  /// High
  FEATURE_PRESET_HIGH ,
  /// Ultra
  FEATURE_PRESET_ULTRA
} ;


/**
* @brief Class holding the parameters for the feature extraction
*/
class FeatureParams
{
  public:

    /**
    * @brief ctr
    * @param type The feature type to use
    * @param preset The feature preset to use
    * @param upright Indicate if feature are extracted using upright orientation
    * @param nb_parallel_job Number of parallel feature computation to do
    */
    FeatureParams( const FeatureType & type = FEATURE_TYPE_SIFT ,
                   const FeaturePreset & preset = FEATURE_PRESET_NORMAL ,
                   const bool upright = false ,
                   const int nb_parallel_job = 1 ) ;

    /**
    * @brief copy ctr
    * @param src Source
    */
    FeatureParams( const FeatureParams & src ) = default ;

    /**
    * @brief assignment operator
    * @param src Source
    * @return self after assignment
    */
    FeatureParams & operator=( const FeatureParams & src ) = default ;

    /**
    * @brief Get feature type
    */
    FeatureType type( void ) const ;

    /**
    * @brief Set feature type
    */
    void setType( const FeatureType & type ) ;


    /**
    * @brief Get preset mode
    */
    FeaturePreset preset( void ) const ;

    /**
    * @brief set preset mode
    */
    void setPreset( const FeaturePreset & preset ) ;

    /**
    * @brief indicate if the describer is in upright orientation
    */
    bool upright( void ) const ;

    /**
    * @brief set upright mode
    */
    void setUpright( const bool ur ) ;

    /**
     * @brief Get number of parallel feature computation job
     * @return nb parallel job
     */
    int nbParallelJob( void ) const ;

    /**
     * @brief Set number of parallel feature extraction to perform
     * @param nb_j New number of parallel job
     */
    void setNbParallelJob( const int nb_j ) ;

    /**
    * @brief Get describer
    * @return the describer corresponding to the current parameters
    */
    std::shared_ptr<openMVG::features::Image_describer> describer( void ) const;


    /**
    * @brief Serialize data
    * @param ar Achive
    */
    template <class Archive>
    void load( Archive & ar ) ;

    /**
    * @brief Serialize data
    * @param ar Achive
    */
    template <class Archive>
    void save( Archive & ar ) const ;

    /**
     * @brief Get all kinds of combinations (features_type/feature_presets)
     * @return All valid (usable) combinations
     */
    static std::vector< FeatureParams > allFeatures( void ) ;

  private:


    /**
    * @brief Create internal describer
    */
    void createDescriber( void ) ;

    FeatureType m_feat_type ;
    FeaturePreset m_feat_preset ;
    bool m_upright;
    int m_nb_parallel_job ;

    // The current describer
    std::shared_ptr<openMVG::features::Image_describer> m_describer ;
} ;

/**
* @brief Serialize data
* @param ar Achive
*/
template <class Archive>
void FeatureParams::load( Archive & ar )
{
  ar( m_feat_type ) ;
  ar( m_feat_preset ) ;
  ar( m_upright ) ;

  createDescriber() ;
}

/**
* @brief Serialize data
* @param ar Achive
*/
template <class Archive>
void FeatureParams::save( Archive & ar ) const
{
  ar( m_feat_type ) ;
  ar( m_feat_preset ) ;
  ar( m_upright ) ;
}


} // namespace openMVG_gui

#endif