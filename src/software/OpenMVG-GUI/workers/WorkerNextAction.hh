// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_NEXT_ACTION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_NEXT_ACTION_HH_

#include <QMetaType>

namespace openMVG_gui
{

/**
 * @brief Action to execute after a worker has finished
 */
enum WorkerNextAction : int
{
  // Special case
  NEXT_ACTION_NONE  = 0,
  NEXT_ACTION_ERROR = 1,

  // Load actions
  NEXT_ACTION_LOAD_FEATURES = 2,
  NEXT_ACTION_LOAD_MATCHES  = 4,
  NEXT_ACTION_LOAD_REGIONS  = 8,
  // Compute actions
  NEXT_ACTION_COMPUTE_FEATURES            = 16,
  NEXT_ACTION_COMPUTE_GEOMETRIC_FILTERING = 32,
  NEXT_ACTION_COMPUTE_MATCHES             = 64,
  NEXT_ACTION_COMPUTE_INCREMENTAL_SFM     = 128,
  NEXT_ACTION_COMPUTE_GLOBAL_SFM          = 256
};

/**
 * @brief Bitwise operation
 */
WorkerNextAction operator|( const WorkerNextAction &a, const WorkerNextAction &b );

/**
 * @brief Indicate if act contains a set of op
 * @param act Complex operation
 * @param op Single operation
 * @retval true if op is fully in act
 * @retval false else
 */
bool contains( const WorkerNextAction &act, const WorkerNextAction &op );

/**
 * @brief Remove an operation from a complex set of operation
 * @param act complex set of operation
 * @param op Operation
 * @return act without op
 */
WorkerNextAction remove( const WorkerNextAction &act, const WorkerNextAction &op );

} // namespace openMVG_gui

Q_DECLARE_METATYPE( openMVG_gui::WorkerNextAction );

// Q_DECLARE_METATYPE(openMVG_gui::WorkerNextAction) ;

#endif