// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_MATCHES_PROVIDER_LOAD_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_MATCHES_PROVIDER_LOAD_HH_

#include "Project.hh"
#include "WorkerInterface.hh"

#include <QObject>

// Fwrd decl
namespace openMVG
{
namespace sfm
{
struct Matches_Provider;
}
}

namespace openMVG_gui
{

/**
 * @brief Worker class used to load matches
 */
class WorkerMatchesProviderLoad : public QObject, public WorkerInterface
{
  public:
    WorkerMatchesProviderLoad( std::shared_ptr<Project> project, const std::string &filename, const WorkerNextAction &na = NEXT_ACTION_NONE );

    /**
     * @brief get progress range
     */
    void progressRange( int &min, int &max ) const;

    /**
     * @brief get the matches provider after loading
     * @note result is only valid after success of the process function
     */
    std::shared_ptr<openMVG::sfm::Matches_Provider> matchesProvider( void ) const;

  public slots:

    /**
     * @brief Do the computation
     */
    void process( void );

  signals:

    // 0 -> nothing done
    // 1 -> all done
    void progress( int );

    // After computation of all matches (ie: signal to clear memory)
    void finished( const WorkerNextAction &na );

  private:
    std::shared_ptr<Project> m_project;
    std::shared_ptr<openMVG::sfm::Matches_Provider> m_matches_provider;
    std::string m_matches_filename;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif