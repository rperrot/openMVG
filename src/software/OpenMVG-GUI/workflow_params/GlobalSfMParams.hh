#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_GLOBAL_SFM_PARAMS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_GLOBAL_SFM_PARAMS_HH_

#include "openMVG/sfm/pipelines/global/GlobalSfM_rotation_averaging.hpp"
#include "openMVG/sfm/pipelines/global/GlobalSfM_translation_averaging.hpp"

#include "IntrinsicRefinerOptions.hh"

namespace openMVG_gui
{

/**
* @brief Metric used in rotation averaging
*/
enum RotationAveragingMethod
{
  ROTATION_AVERAGING_L1 ,
  ROTATION_AVERAGING_L2
} ;

/**
* @brief Metric used in translation averaging
*/
enum TranslationAveragingMethod
{
  TRANSLATION_AVERAGING_L1 ,
  TRANSLATION_AVERAGING_L2 ,
  TRANSLATION_AVERAGING_SOFT_L1
} ;

/**
* @brief Convert from gui param to openMVG internal param
*/
openMVG::sfm::ERotationAveragingMethod convert( const RotationAveragingMethod & rot ) ;

/**
* @brief Convert from gui param to openMVG internal param
*/
openMVG::sfm::ETranslationAveragingMethod convert( const TranslationAveragingMethod & tra ) ;

/**
* @brief Class holding Global SfM options
*/
class GlobalSfMParams
{
  public:

    /**
    * @brief Ctr
    * @param ra Rotation averaging method
    * @param ta Translation averaging method
    * @param refiner Intrinsic refiner options used for BA
    */
    GlobalSfMParams( const RotationAveragingMethod & ra = ROTATION_AVERAGING_L2 ,
                     const TranslationAveragingMethod & ta = TRANSLATION_AVERAGING_SOFT_L1 ,
                     const IntrinsicRefinerOptions & refiner = IntrinsicRefinerOptions::INTRINSIC_REFINER_ALL ) ;

    /**
    * @brief Get rotation averaging method
    * @return Current rotation averaging method
    */
    RotationAveragingMethod rotationAveraging( void ) const ;

    /**
    * @brief Set rotation averaging method
    * @param ra New rotation averaging
    */
    void setRotationAveraging( const RotationAveragingMethod & ra ) ;

    /**
    * @brief Get translation averaging method
    * @return translation averaging method
    */
    TranslationAveragingMethod translationAveraging( void ) const ;

    /**
    * @brief Set translation averaging method
    * @param ta New translation averaging method
    */
    void setTranslationAveraging( const TranslationAveragingMethod & ta ) ;

    /**
    * @brief Get refiner options
    * @return refiner options
    */
    IntrinsicRefinerOptions refinerOptions( void ) const ;

    /**
    * @brief Set refiner options
    * @param opts refiner options
    */
    void setRefinerOptions( const IntrinsicRefinerOptions & opts ) ;

    template< class Archive >
    void load( Archive & ar ) ;

    template< class Archive >
    void save( Archive & ar ) const ;

  private:

    RotationAveragingMethod m_rotation_avg_method ;
    TranslationAveragingMethod m_translation_avg_method ;
    IntrinsicRefinerOptions m_refiner_option ;
} ;

template< class Archive >
void GlobalSfMParams::load( Archive & ar )
{
  ar( m_rotation_avg_method ) ;
  ar( m_translation_avg_method ) ;
  ar( m_refiner_option ) ;
}

template< class Archive >
void GlobalSfMParams::save( Archive & ar ) const
{
  ar( m_rotation_avg_method ) ;
  ar( m_translation_avg_method ) ;
  ar( m_refiner_option ) ;
}

} // namespace openMVG_gui

#endif