#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_FEATURE_PARAMS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_FEATURE_PARAMS_HH_

#include <memory>

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
    */
    FeatureParams( const FeatureType & type = FEATURE_TYPE_SIFT ,
                   const FeaturePreset & preset = FEATURE_PRESET_NORMAL ,
                   const bool upright = false ) ;

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


  private:


    /**
    * @brief Create internal describer
    */
    void createDescriber( void ) ;

    FeatureType m_feat_type ;
    FeaturePreset m_feat_preset ;
    bool m_upright;

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