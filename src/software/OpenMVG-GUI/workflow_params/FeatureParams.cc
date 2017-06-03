#include "FeatureParams.hh"

#include "openMVG/features/image_describer_akaze.hpp"
#include "openMVG/features/sift/SIFT_Anatomy_Image_Describer.hpp"
#include "nonFree/sift/SIFT_describer.hpp"

using namespace openMVG ;
using namespace openMVG::features ;


namespace openMVG_gui
{

/**
* @brief ctr
* @param type The feature type to use
* @param preset The feature preset to use
* @param upright Indicate if feature are extracted using upright orientation
*/
FeatureParams::FeatureParams( const FeatureType & type ,
                              const FeaturePreset & preset ,
                              const bool upright  )
  : m_feat_type( type ) ,
    m_feat_preset( preset ) ,
    m_upright( upright ) ,
    m_describer( nullptr )
{
  createDescriber() ;
}

/**
* @brief Get feature type
*/
FeatureType FeatureParams::type( void ) const
{
  return m_feat_type ;
}

/**
* @brief Set feature type
*/
void FeatureParams::setType( const FeatureType & type )
{
  if( type != m_feat_type )
  {
    createDescriber() ;
  }
}


/**
* @brief Get preset mode
*/
FeaturePreset FeatureParams::preset( void ) const
{
  return m_feat_preset ;
}

/**
* @brief set preset mode
*/
void FeatureParams::setPreset( const FeaturePreset & preset )
{
  if( preset != m_feat_preset )
  {
    m_feat_preset = preset ;
    createDescriber() ;
  }
}


/**
* @brief indicate if the describer is in upright orientation
*/
bool FeatureParams::upright( void ) const
{
  return m_upright ;
}

/**
* @brief set upright mode
*/
void FeatureParams::setUpright( const bool ur )
{
  if( ur != m_upright )
  {
    m_upright = ur ;
    createDescriber() ;
  }
}

/**
* @brief Get describer
* @return the describer corresponding to the current parameters
*/
std::shared_ptr<openMVG::features::Image_describer> FeatureParams::describer( void ) const
{
  return m_describer ;
}

/**
* @brief Create internal describer
*/
void FeatureParams::createDescriber( void )
{
  m_describer = nullptr ;
  // Create the desired Image_describer method.
  switch( m_feat_type )
  {
    case FEATURE_TYPE_SIFT :
    {
      m_describer = std::make_shared< SIFT_Image_describer >( SIFT_Image_describer::Params() , ! m_upright ) ;
      break ;
    }
    case FEATURE_TYPE_SIFT_ANATOMY:
    {
      m_describer = std::make_shared< SIFT_Anatomy_Image_describer >( SIFT_Anatomy_Image_describer::Params() ) ;
      break ;
    }
    case FEATURE_TYPE_AKAZE_FLOAT :
    {
      m_describer = std::make_shared<AKAZE_Image_describer>( AKAZE_Image_describer::Params( AKAZE::Params(), AKAZE_MSURF ), ! m_upright ) ;
      break ;
    }
    case FEATURE_TYPE_AKAZE_MLDB :
    {
      m_describer = std::make_shared<AKAZE_Image_describer>( AKAZE_Image_describer::Params( AKAZE::Params(), AKAZE_MLDB ), !m_upright ) ;
      break ;
    }
  }
  if( m_describer )
  {
    switch( m_feat_preset )
    {
      case FEATURE_PRESET_NORMAL:
      {
        m_describer->Set_configuration_preset( NORMAL_PRESET ) ;
        break ;
      }
      case FEATURE_PRESET_HIGH :
      {
        m_describer->Set_configuration_preset( HIGH_PRESET ) ;
        break ;
      }
      case FEATURE_PRESET_ULTRA :
      {
        m_describer->Set_configuration_preset( ULTRA_PRESET ) ;
        break ;
      }
    }
  }
}

} // namespace openMVG_gui