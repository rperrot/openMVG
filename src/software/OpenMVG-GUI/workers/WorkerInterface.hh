// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_INTERFACE_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_INTERFACE_HH_

#include "WorkerNextAction.hh"

namespace openMVG_gui
{

/**
 * @brief common interface for workers
 */
class WorkerInterface
{
  public:
    /**
     * @brief next action to be executed after this worker
     */
    WorkerInterface( const WorkerNextAction &na = NEXT_ACTION_NONE );

    /**
     * @brief remaining actions to be executed after this worker thread
     */
    WorkerNextAction nextAction( void ) const;

  protected:
    WorkerNextAction m_next_action;
};

} // namespace openMVG

#endif