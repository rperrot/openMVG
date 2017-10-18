// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "WorkerProgressInterface.hh"

namespace openMVG_gui
{

WorkerProgressInterface::WorkerProgressInterface( void )
  : m_canceled( false )
{
}

/** @brief Indicator if the current operation should be aborted.
 * @return Return true if the process has been canceled by the user.
 **/
bool WorkerProgressInterface::hasBeenCanceled() const
{
  return m_canceled;
}

unsigned long WorkerProgressInterface::operator+=( unsigned long ulIncrement )
{
  std::lock_guard<std::mutex> lock( m_mutex );

  unsigned long res = C_Progress::operator+=( ulIncrement );
  emit increment( ulIncrement );
  return res;
}

unsigned long WorkerProgressInterface::operator++()
{
  return operator+=( 1 );
}

void WorkerProgressInterface::cancel( void )
{
  m_canceled = true;
}
}