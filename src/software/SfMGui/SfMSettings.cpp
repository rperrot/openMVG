#include "software/SfMGui/SfMSettings.hpp"

#include <iostream>

namespace openMVG
{
namespace SfMGui
{


/**
* @brief Get string corresponding to a feature type
* @return string relative to this feature type
*/
std::string ToString( const FeatureType & feat_type )
{
  switch( feat_type )
  {
    case FeatureType::FEATURE_TYPE_SIFT :
    {
      return "SIFT" ;
      break;
    }
    case FeatureType::FEATURE_TYPE_AKAZE :
    {
      return "AKAZE" ;
      break ;
    }
    default:
      return "" ;
  }
}

/**
* @brief Get string corresponding to a feature mode
* @return string relative to this feature mode
*/
std::string ToString( const FeatureMode & feat_mode )
{
  switch( feat_mode )
  {
    case FeatureMode::FEATURE_MODE_NORMAL :
    {
      return "NORMAL" ;
      break ;
    }
    case FeatureMode::FEATURE_MODE_HIGH :
    {
      return "HIGH" ;
      break ;
    }
    case FeatureMode::FEATURE_MODE_ULTRA :
    {
      return "ULTRA" ;
      break ;
    }
    default :
      return "";
  }
}

/**
* @brief Get string corresponding to a pipeline type
* @return string relative to this pipeline type
*/
std::string ToString( const PipelineType & pipeline_type )
{
  switch( pipeline_type )
  {
    case PipelineType::PIPELINE_TYPE_INCREMENTAL :
    {
      return "Incremental" ;
      break ;
    }
    case PipelineType::PIPELINE_TYPE_GLOBAL :
    {
      return "Global" ;
      break ;
    }
    default:
      return "";
  }
}

/**
* @brief Get Feature type from a string
* @param sFeat_type string containing the feature type
* @param[out] feat_type Feature type
*/
void FromString( const std::string & sFeat_type , FeatureType & feat_type  )
{
  if( sFeat_type == "SIFT" )
  {
    feat_type = FeatureType::FEATURE_TYPE_SIFT ;
  }
  else if( sFeat_type == "AKAZE" )
  {
    feat_type = FeatureType::FEATURE_TYPE_AKAZE ;
  }
  else
  {
    std::cerr << "Error - Unknown feature type : " << sFeat_type << std::endl ;
  }
}

/**
* @brief Get Feature mode from a string
* @param sFeat_mode string containing the feature mode
* @param[out] feat_mode Feature mode
*/
void FromString( const std::string & sFeat_mode , FeatureMode & feat_mode )
{
  if( sFeat_mode == "NORMAL" )
  {
    feat_mode = FeatureMode::FEATURE_MODE_NORMAL ;
  }
  else if( sFeat_mode == "HIGH" )
  {
    feat_mode = FeatureMode::FEATURE_MODE_HIGH ;
  }
  else if( sFeat_mode == "ULTRA" )
  {
    feat_mode = FeatureMode::FEATURE_MODE_ULTRA ;
  }
  else
  {
    std::cerr << "Error - Unknown feature mode : " << sFeat_mode << std::endl ;
  }
}

/**
* @brief Get PipelineType type from a string
* @param sPipeline_type string containing the pipeline type
* @param[out] pipeline_type Pipeline type
*/
void FromString( const std::string & sPipeline_type , PipelineType & pipeline_type )
{
  if( sPipeline_type == "Incremental" )
  {
    pipeline_type = PipelineType::PIPELINE_TYPE_INCREMENTAL ;
  }
  else if( sPipeline_type == "Global" )
  {
    pipeline_type = PipelineType::PIPELINE_TYPE_GLOBAL ;
  }
  else
  {
    std::cerr << "Error - Unknown pipeline type : " << sPipeline_type << std::endl ;
  }
}

/**
* @brief Ctr
* @note Make default settings
*/
SfMSettings::SfMSettings( void )
  : m_feat_type( FeatureType::FEATURE_TYPE_SIFT ) ,
    m_feat_mode( FeatureMode::FEATURE_MODE_NORMAL ) ,
    m_pipeline_type( PipelineType::PIPELINE_TYPE_GLOBAL )
{

}

} // namespace SfMGui
} // namespace openMVG