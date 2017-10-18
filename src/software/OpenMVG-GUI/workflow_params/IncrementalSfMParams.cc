// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "IncrementalSfMParams.hh"

namespace openMVG_gui
{

/**
* @brief Ctr
* @param initial_pair Initial pair (if pair is (-1,-1), use automatic selection)
* @param unknown_intrinsic Intrisic param used for view with unknown intrinsic
* @param refiner What intrinsics params will be refined by SfM optimizer
*/
IncrementalSfMParams::IncrementalSfMParams( const std::pair< int , int > & initial_pair  ,
    const IntrinsicParams & unknown_intrinsic  ,
    const IntrinsicRefinerOptions & refiner  )
  : m_initial_pair( initial_pair ),
    m_unknown_param( unknown_intrinsic ) ,
    m_refiner_option( refiner )
{

}

/**
* @brief Get initial pair
* @return initial pair
*/
std::pair<int, int> IncrementalSfMParams::initialPair( void ) const
{
  return m_initial_pair ;
}

/**
* @brief Set initial pair
* @param init Initial pair
*/
void IncrementalSfMParams::setInitialPair( const std::pair<int, int> & init )
{
  m_initial_pair = init ;
}

/**
* @brief Get intrinsic value for view with unknown intrinsics
* @return unknown intrinsic params
*/
IntrinsicParams IncrementalSfMParams::unknownIntrinsic( void ) const
{
  return m_unknown_param ;
}

/**
* @brief Set intrinsic value for view with unknown intrinsics
* @param params Intrinsic params to use
*/
void IncrementalSfMParams::unknownIntrinsic( const IntrinsicParams & params )
{
  m_unknown_param = params ;
}

/**
* @brief Get refiner options
* @return refiner options
*/
IntrinsicRefinerOptions IncrementalSfMParams::refinerOptions( void ) const
{
  return m_refiner_option ;
}

/**
* @brief Set refiner options
* @param opts refiner options
*/
void IncrementalSfMParams::setRefinerOptions( const IntrinsicRefinerOptions & opts )
{
  m_refiner_option = opts ;
}

} // namespace openMVG_gui