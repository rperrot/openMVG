// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_PROJECT_PATHS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_PROJECT_PATHS_HH_

#include "workflow_params/FeatureParams.hh"
#include "workflow_params/SfMMethod.hh"

#include <string>

namespace openMVG_gui
{
/**
 * @brief Helper function used to get all project paths
 */
class ProjectPaths
{
  public:

    /**
     * @brief ctr
     * @param base_path Base path of the SfM project
     */
    ProjectPaths( const std::string & base_path ) ;

    ProjectPaths( const ProjectPaths & src ) = default ;
    ProjectPaths( ProjectPaths && src ) = default ;

    ProjectPaths & operator=( const ProjectPaths & src ) = default ;
    ProjectPaths & operator=( ProjectPaths && src ) = default ;

    /**
     * @brief Base path for all gui related elements
     */
    std::string guiPath( void ) const ;

    /**
     * @brief Path of the thumbnails
     */
    std::string thumbnailsPath( void ) const ;

    /**
     * @brief base path of the features
     */
    std::string globalFeaturePath( void ) const ;

    /**
     * @brief Path of the features depending on the given parameters
     */
    std::string featuresPath( const FeatureParams & params ) const ;

    /**
     * @brief Get all combinations of paths for all features modes
     */
    std::vector< std::string > featuresPaths( void ) const ;


    /**
     * @brief Path where sfm_data.json is located
     * @TODO: name is not good here since matches are no longer in this folder
     */
    std::string matchesPath( ) const ;

    /**
     *  @brief Path of the (un)colorized ply file
     */
    std::string plyCloud( const SfMMethod & method ) const ;

    /**
     *  @brief Path of the colorized ply file
     */
    std::string colorizedPlyCloud( const SfMMethod & method ) const ;

    /**
     * @brief Base path of the sfm methods
     */
    std::string sfmBasePath( void ) const ;

    /**
     *  @brief Path of the output of the sfm process
     */
    std::string sfmReconstructionPath( const SfMMethod & method ) const ;

    /**
     * @brief Path where global reconstruction elements are stored
     */
    std::string reconstructionGlobalPath( void ) const ;

    /**
     * @brief Path where the incremental reconstruction elements are stored
     */
    std::string reconstructionSequentialPath( void ) const ;

    /**
     *  @brief Path of the hmtl file containing the report about sfm process
     */
    std::string htmlReportPath( const SfMMethod & method ) const ;

    /**
     *  @brief Base path where exports to other projects are stored
     */
    std::string exportPath( void ) const ;

  private:

    std::string m_base_path ;

} ;

} // namespace openMVG_gui

#endif