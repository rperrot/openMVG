// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_PROJECT_CREATION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_PROJECT_CREATION_HH_

#include "Project.hh"
#include "WorkerInterface.hh"

#include <QObject>

#include <string>

namespace openMVG_gui
{

class SceneManager;

/**
 * @brief worker thread used to create a new project
 */
class WorkerProjectCreation : public QObject, public WorkerInterface
{
  public:
    /**
     * @brief Constructor
     * @param base_path Project output path
     * @param image_path Input image path
     * @param intrin Intrisic params
     * @param database_file Camera database file
     * @param s_mgr Scene Manager
     */
    WorkerProjectCreation( const std::string &base_path, const std::string &image_path, const IntrinsicParams &intrin,
                           const std::string &database_file, std::shared_ptr<SceneManager> s_mgr,
                           const WorkerNextAction &na = NEXT_ACTION_NONE );

    /**
     * @brief get progress range
     */
    void progressRange( int &min, int &max ) const;

    /**
     * @brief Function used to retrieve the project after creation
     * @return the newly created project (if success only )
     */
    std::shared_ptr<Project> project( void );

  public slots:
    /**
     * @brief create the project
     */
    void process( void );

    /**
     * @brief internal progress bar has been incremented, now signal it to the
     * external progress dialog
     */
    void hasIncremented( int );

    /**
     * @brief set progress value to the main thread
     */
    void sendProgress( void );

  signals:

    // 0 -> nothing done
    // 1 -> all done
    void progress( int );

    // After computation
    void finished( const WorkerNextAction &na );

  private:
    std::atomic<unsigned int> m_progress_value;

    // In
    std::string m_project_base_path;
    std::string m_input_image_path;
    IntrinsicParams m_intrin_params;
    std::string m_database_path;
    std::shared_ptr<SceneManager> m_scn_manager;
    // Out
    std::shared_ptr<Project> m_project;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif