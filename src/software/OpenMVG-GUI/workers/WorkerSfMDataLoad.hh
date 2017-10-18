// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_SFM_DATA_LOAD_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_SFM_DATA_LOAD_HH_

#include "WorkerInterface.hh"

#include "openMVG/sfm/sfm_data.hpp"

#include <QObject>

#include <memory>
#include <string>

namespace openMVG_gui
{
class WorkerSfMDataLoad : public QObject, public WorkerInterface
{
  public:

    /**
     * @brief Ctr
     * @param path The path to where the file is located
     * @param na Next action to execute after loading success
     */
    WorkerSfMDataLoad( const std::string &path , const WorkerNextAction & na = NEXT_ACTION_NONE );

    /**
     * @brief Get SfMData loaded
     */
    std::shared_ptr<openMVG::sfm::SfM_Data> SfMData() const;

    /**
     * @brief get progress range
     */
    void progressRange( int & min , int & max ) const ;

  public slots:

    /**
    * @brief Do the computation
    */
    void process( void ) ;

  signals:

    // 0 -> nothing done
    // n -> all done (n : number of images)
    void progress( int ) ;

    // After computation of all thumbnails (ie: signal to clear memory)
    void finished( const WorkerNextAction & na );

  private:
    std::string m_path;

    std::shared_ptr<openMVG::sfm::SfM_Data> m_sfm_data ;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif