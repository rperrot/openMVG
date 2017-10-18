// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GlobalSfMParams.hh"

namespace openMVG_gui
{

/**
* @brief Convert from gui param to openMVG internal param
*/
openMVG::sfm::ERotationAveragingMethod convert( const RotationAveragingMethod & rot )
{
  switch( rot )
  {
    case ROTATION_AVERAGING_L1 :
      return openMVG::sfm::ERotationAveragingMethod::ROTATION_AVERAGING_L1 ;
    case ROTATION_AVERAGING_L2 :
      return openMVG::sfm::ERotationAveragingMethod::ROTATION_AVERAGING_L2 ;
  }
}

/**
* @brief Convert from gui param to openMVG internal param
*/
openMVG::sfm::ETranslationAveragingMethod convert( const TranslationAveragingMethod & tra )
{
  switch( tra )
  {
    case TRANSLATION_AVERAGING_L1 :
      return openMVG::sfm::ETranslationAveragingMethod::TRANSLATION_AVERAGING_L1 ;
    case TRANSLATION_AVERAGING_L2 :
      return openMVG::sfm::ETranslationAveragingMethod::TRANSLATION_AVERAGING_L2_DISTANCE_CHORDAL ;
    case TRANSLATION_AVERAGING_SOFT_L1 :
      return openMVG::sfm::ETranslationAveragingMethod::TRANSLATION_AVERAGING_SOFTL1 ;
  }
}


/**
* @brief Ctr
* @param ra Rotation averaging method
* @param ta Translation averaging method
* @param refiner Intrinsic refiner options used for BA
*/
GlobalSfMParams::GlobalSfMParams( const RotationAveragingMethod & ra ,
                                  const TranslationAveragingMethod & ta ,
                                  const IntrinsicRefinerOptions & refiner )
  : m_rotation_avg_method( ra ) ,
    m_translation_avg_method( ta ) ,
    m_refiner_option( refiner )
{

}

/**
* @brief Get rotation averaging method
* @return Current rotation averaging method
*/
RotationAveragingMethod GlobalSfMParams::rotationAveraging( void ) const
{
  return m_rotation_avg_method ;
}

/**
* @brief Set rotation averaging method
* @param ra New rotation averaging
*/
void GlobalSfMParams::setRotationAveraging( const RotationAveragingMethod & ra )
{
  m_rotation_avg_method = ra ;
}

/**
* @brief Get translation averaging method
* @return translation averaging method
*/
TranslationAveragingMethod GlobalSfMParams::translationAveraging( void ) const
{
  return m_translation_avg_method ;
}

/**
* @brief Set translation averaging method
* @param ta New translation averaging method
*/
void GlobalSfMParams::setTranslationAveraging( const TranslationAveragingMethod & ta )
{
  m_translation_avg_method = ta ;
}

/**
* @brief Get refiner options
* @return refiner options
*/
IntrinsicRefinerOptions GlobalSfMParams::refinerOptions( void ) const
{
  return m_refiner_option ;
}

/**
* @brief Set refiner options
* @param opts refiner options
*/
void GlobalSfMParams::setRefinerOptions( const IntrinsicRefinerOptions & opts )
{
  m_refiner_option = opts ;
}

} // namespace openMVG_gui