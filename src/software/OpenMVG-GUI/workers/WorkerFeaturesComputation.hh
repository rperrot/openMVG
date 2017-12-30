// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_FEATURES_COMPUTATION_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKERS_WORKER_FEATURES_COMPUTATION_HH_

#include "Project.hh"
#include "WorkerInterface.hh"

#include "openMVG/image/image_container.hpp"

#include <QObject>

#include <atomic>

namespace openMVG_gui
{

/**
 * @brief Worker that compute features on images
 * @TODO : need a way to compute only certain features (pass a vector of ID ?)
 */
class WorkerFeaturesComputation : public QObject, public WorkerInterface
{
  public:

    /**
     * @brief Ctr
     * @param pro The project to get inputs and parameters
     * @param overwrite_existing Indicate to overwrite existing computation
     * @param na Next action to transmit after computation finished
     */
    WorkerFeaturesComputation( std::shared_ptr<Project> &pro,
                               const bool overwrite_existing,
                               const WorkerNextAction &na = NEXT_ACTION_NONE );

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
    // n -> all done (n : number of images)
    void progress( int );

    // After computation of all features (ie: signal to clear memory)
    void finished( const WorkerNextAction &na );

  private:

    /**
     * @brief Thread function
     * @param sfm_data Sfm Data
     * @param id_start Start id to process
     * @param id_end End id to process (not included)
     * @param nb_processed Number of images processed to far (in order to emit correct values to the interface)
     * @param feature_path Path where features are exported
     * @param globalMask global mask image
     * @param image_describer functor used to compute description of the images
    * @param[out] ok Handle if computation was a success (true) or failure (false)
     */
    void processThread( std::shared_ptr<openMVG::sfm::SfM_Data> & sfm_data , const int id_start , const int id_end ,
                        const std::string & feature_path ,
                        const openMVG::image::Image<unsigned char> & globalMask ,
                        std::shared_ptr<openMVG::features::Image_describer> & image_describer ,
                        bool & ok ) ;

    bool m_overwrite_existing ;
    std::shared_ptr<Project> m_project ;

    std::atomic<int> m_nb_processed ;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif