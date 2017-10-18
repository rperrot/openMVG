// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_COLOR_COMPUTATION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_COLOR_COMPUTATION_HH_

#include "Project.hh"
#include "WorkerInterface.hh"

#include "openMVG/sfm/sfm_data.hpp"

#include <QObject>

#include <atomic>
#include <vector>

namespace openMVG_gui
{

/**
 * @brief worker class used to compute color on current sfm_data
 */
class WorkerColorComputation : public QObject, public WorkerInterface
{
  public:
    /**
     * @brief Ctr Constructor
     */
    WorkerColorComputation( std::shared_ptr<Project> proj );

    /**
     * @brief get progress range
     */
    void progressRange( int &min, int &max ) const;

  public slots:

    /**
     * @brief Do the computation
     */
    void process( void );

    /**
     * @brief internal progress bar has been incremented, now signal it to the external progress dialog
     */
    void hasIncremented( int );

    void sendProgress( void );

  signals:

    // 0 -> nothing done
    // n -> all done (n : number of points)
    void progress( int );

    // After computation of all features (ie: signal to clear memory)
    void finished( const WorkerNextAction &na );

  private:
    bool ColorizeTracks( const std::shared_ptr<openMVG::sfm::SfM_Data> &sfm_data, std::vector<openMVG::Vec3> &vec_3dPoints, std::vector<openMVG::Vec3> &vec_tracksColor );

    // Internal counter for the number of step already done
    std::atomic<unsigned int> m_progress_value;

    std::shared_ptr<Project> m_project;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif