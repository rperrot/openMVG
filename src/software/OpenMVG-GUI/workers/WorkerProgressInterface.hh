// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_PROGRESS_INTERFACE_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_PROGRESS_INTERFACE_HH_

#include "third_party/progress/progress.hpp"

#include <QObject>

#include <mutex>

namespace openMVG_gui
{
// This is an helper class used to make an interface with standard openMVG
// it emit a signal updated(int) after all ++ to get interaction with long computation
class WorkerProgressInterface : public QObject, public C_Progress
{
  public:
    WorkerProgressInterface( void );

    /** @brief Indicator if the current operation should be aborted.
     * @return Return true if the process has been canceled by the user.
     **/
    bool hasBeenCanceled() const override;

    /**
     * @brief Increment of multiple steps
     */
    unsigned long operator+=( unsigned long ulIncrement ) override;

    /**
     * @brief Increment of one step
     */
    unsigned long operator++() override;

  public slots:

    /**
     * @brief indicate that it would stop
     */
    void cancel( void );

  signals:

    /**
     * @brief signal we have incremented the progress
     */
    void increment( int );

  private:
    bool m_canceled;
    std::mutex m_mutex;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif