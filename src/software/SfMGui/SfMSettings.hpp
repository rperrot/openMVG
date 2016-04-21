#ifndef _OPENMVG_SFMGUI_SFM_SETTINGS_HPP_
#define _OPENMVG_SFMGUI_SFM_SETTINGS_HPP_

#include <string>
#include <cstdint>

namespace openMVG
{
namespace SfMGui
{

/**
* @brief Settings for kind of features
*/
enum class FeatureType : std::int8_t
{
  FEATURE_TYPE_SIFT = 0 ,
  FEATURE_TYPE_AKAZE ,

  FEATURE_TYPE_UNKNOWN
} ;

/**
* @brief Settings for feature quality
*/
enum class FeatureMode : std::int8_t
{
  FEATURE_MODE_NORMAL = 0 ,
  FEATURE_MODE_HIGH ,
  FEATURE_MODE_ULTRA ,

  FEATURE_MODE_UNKNOWN
} ;

/**
* @brief Settings for pipeline type
*/
enum class PipelineType : std::int8_t
{
  PIPELINE_TYPE_INCREMENTAL = 0 ,
  PIPELINE_TYPE_GLOBAL ,

  PIPELINE_TYPE_UNKNOWN
} ;

/**
* @brief Get string corresponding to a feature type
* @return string relative to this feature type
*/
std::string ToString( const FeatureType & feat_type ) ;

/**
* @brief Get string corresponding to a feature mode
* @return string relative to this feature mode
*/
std::string ToString( const FeatureMode & feat_mode ) ;

/**
* @brief Get string corresponding to a pipeline type
* @return string relative to this pipeline type
*/
std::string ToString( const PipelineType & pipeline_type ) ;

/**
* @brief Get Feature type from a string
* @param sFeat_type string containing the feature type
* @param[out] feat_type Feature type
*/
void FromString( const std::string & sFeat_type , FeatureType & feat_type  ) ;

/**
* @brief Get Feature mode from a string
* @param sFeat_mode string containing the feature mode
* @param[out] feat_mode Feature mode
*/
void FromString( const std::string & sFeat_mode , FeatureMode & feat_mode ) ;

/**
* @brief Get PipelineType type from a string
* @param sPipeline_type string containing the pipeline type
* @param[out] pipeline_type Pipeline type
*/
void FromString( const std::string & sPipeline_type , PipelineType & pipeline_type ) ;


/**
* @brief Structure managing all settings for whole SfM process
*/
struct SfMSettings
{
  /**
  * @brief Ctr
  * @note Make default settings
  */
  SfMSettings( void ) ;

  /**
  * @brief Copy ctr
  * @param src Source
  */
  SfMSettings( const SfMSettings & src ) = default ;

  /**
  * @brief Move ctr
  * @param src Source
  */
  SfMSettings( SfMSettings && src ) = default ;

  /**
  * @brief Copy assignment operator
  * @param src Source
  * @return self after assignment
  */
  SfMSettings & operator=( const SfMSettings & src ) = default ;

  /**
  * @brief Move assignment openMVG
  * @param src Source
  * @return self after assignment
  */
  SfMSettings & operator=( SfMSettings && src ) = default ;


  /// Kind of feature to use
  FeatureType m_feat_type ;

  /// Quality of features
  FeatureMode m_feat_mode ;

  /// Pipeline to use
  PipelineType m_pipeline_type ;

} ;


/**
* @brief Helper function used to iterate over settings enumerations
*/
template < typename EnumType >
struct SettingIterator
{
  public:

    /**
    * @brief Get first value of enumeration
    */
    static EnumType begin()  ;

    /**
    * @brief Get next value after last value of enumeration
    */
    static EnumType end() ;

    /**
    * @brief Given an enumeration value, returns next value
    */
    static EnumType next( const EnumType & feat )
    {
      int8_t val = static_cast<int8_t>( feat ) ;
      return static_cast<EnumType>( val + 1 ) ;
    }
} ;

/**
* @brief Specialization for FeatureMode
*/
template<>
struct SettingIterator<FeatureMode>
{
  public:
    static FeatureMode begin()
    {
      return FeatureMode::FEATURE_MODE_NORMAL ;
    }

    static FeatureMode end()
    {
      return FeatureMode::FEATURE_MODE_UNKNOWN ;
    }

    static FeatureMode next( const FeatureMode & feat )
    {
      int8_t val = static_cast<int8_t>( feat ) ;
      return static_cast<FeatureMode>( val + 1 ) ;
    }
} ;

/**
* @brief Specialization for FeatureType
*/
template<>
struct SettingIterator<FeatureType>
{
  public:

    static FeatureType begin()
    {
      return FeatureType::FEATURE_TYPE_SIFT ;
    }

    static FeatureType end()
    {
      return FeatureType::FEATURE_TYPE_UNKNOWN ;
    }

    static FeatureType next( const FeatureType & feat )
    {
      int8_t val = static_cast<int8_t>( feat ) ;
      return static_cast<FeatureType>( val + 1 ) ;
    }

};

/**
* @brief Specialization for PipelineType
*/
template<>
struct SettingIterator<PipelineType>
{
  public:

    static PipelineType begin()
    {
      return PipelineType::PIPELINE_TYPE_INCREMENTAL;
    }

    static PipelineType end()
    {
      return PipelineType::PIPELINE_TYPE_UNKNOWN;
    }

    static PipelineType next( const PipelineType & feat )
    {
      int8_t val = static_cast<int8_t>( feat ) ;
      return static_cast<PipelineType>( val + 1 ) ;
    }
} ;

} // namespace SfMGui
} // namespace openMVG

#endif