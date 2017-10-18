// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_INCREMENTAL_SFM_COMPUTATION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_INCREMENTAL_SFM_COMPUTATION_HH_

#include "Project.hh"
#include "WorkerInterface.hh"

#include <QObject>

namespace openMVG
{
namespace sfm
{
struct Features_Provider;
struct Matches_Provider;
} // namespace sfm
} // namespace openMVG

namespace openMVG_gui
{

/**
 * @brief Worker used to compute Incremental SfM Computation
 */
class WorkerIncrementalSfMComputation : public QObject, public WorkerInterface
{
  public:
    /**
     * @brief Ctr
     * @param proj Project
     * @param features_provider Features
     * @param matches_provider Matches
     */
    WorkerIncrementalSfMComputation( std::shared_ptr<Project> &proj, std::shared_ptr<openMVG::sfm::Features_Provider> &features_provider, std::shared_ptr<openMVG::sfm::Matches_Provider> &matches_provider, const bool reload_initial_intrinsics = true, const WorkerNextAction &na = NEXT_ACTION_NONE );

    /**
     * @brief get progress range
     */
    void progressRange( int &min, int &max ) const;

  public slots:

    /**
     * @brief Do the computation
     */
    void process( void );

  signals:

    // 0 -> nothing done
    // n -> all done (n : number of match pair)
    void progress( int );

    // After computation of all matches (ie: signal to clear memory)
    void finished( const WorkerNextAction &na );

  private:
    std::shared_ptr<Project> m_project;
    std::shared_ptr<openMVG::sfm::Features_Provider> m_feature_provider;
    std::shared_ptr<openMVG::sfm::Matches_Provider> m_match_provider;

    bool m_reload_initial_intrinsics;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif