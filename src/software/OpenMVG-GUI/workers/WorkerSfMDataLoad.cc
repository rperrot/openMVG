// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "WorkerSfMDataLoad.hh"

#include "openMVG/sfm/sfm_data_io.hpp"

namespace openMVG_gui
{

/**
* @brief Ctr
* @param path The path to where the file is located
* @param na Next action to execute after loading success
*/
WorkerSfMDataLoad::WorkerSfMDataLoad( const std::string &path , const WorkerNextAction & na )
  : WorkerInterface( na ),
    m_path( path )
{

}

/**
 * @brief Get SfMData loaded
 */
std::shared_ptr<openMVG::sfm::SfM_Data> WorkerSfMDataLoad::SfMData() const
{
  return m_sfm_data ;
}

/**
 * @brief get progress range
 */
void WorkerSfMDataLoad::progressRange( int & min , int & max ) const
{
  min = 0 ;
  max = 1 ;
}

/**
* @brief Do the computation
*/
void WorkerSfMDataLoad::process( void )
{
  emit progress( 0 ) ;
  m_sfm_data = std::make_shared<openMVG::sfm::SfM_Data>() ;
  if( ! Load( *m_sfm_data , m_path , openMVG::sfm::ESfM_Data( openMVG::sfm::ESfM_Data::ALL ) ) )
  {
    emit progress( 1 ) ;
    emit finished( NEXT_ACTION_ERROR ) ;
    return ;
  }

  emit progress( 1 ) ;
  emit finished( nextAction() );
}

} // namespace openMVG_gui