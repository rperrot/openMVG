// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "WorkerInterface.hh"

namespace openMVG_gui
{

/**
 * @brief next action to be executed after this worker
 */
WorkerInterface::WorkerInterface( const WorkerNextAction &na )
  : m_next_action( na )
{
}

/**
 * @brief remaining actions to be executed after this worker thread
 */
WorkerNextAction WorkerInterface::nextAction( void ) const
{
  return m_next_action;
}

} // namespace openMVG_gui