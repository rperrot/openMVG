// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "WorkerThumbnailGeneration.hh"

#include "openMVG/sfm/sfm_data.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QImage>

namespace openMVG_gui
{

/**
* @brief Ctr
* @param pro The project for which thumbnails are required
*/
WorkerThumbnailGeneration::WorkerThumbnailGeneration( std::shared_ptr<Project> & pro ,
    const WorkerNextAction & na )
  : WorkerInterface( na ),
    m_project( pro )
{

}

/**
* @brief get progress range
*/
void WorkerThumbnailGeneration::progressRange( int & min , int & max ) const
{
  const int nb_image = m_project->SfMData()->GetViews().size() ;
  min = 0 ;
  max = nb_image ;
}

/**
* @brief Do the computation
*/
void WorkerThumbnailGeneration::process( void )
{
  const int nb_image = m_project->SfMData()->GetViews().size() ;

  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData() ;

  const std::string thumbnail_path = m_project->projectPaths().thumbnailsPath() ;

  emit progress( 0 ) ;
  for( int i = 0; i < nb_image ; ++i )
  {
    openMVG::sfm::Views::const_iterator iterViews = sfm_data->views.begin();
    std::advance( iterViews, i );
    const openMVG::sfm::View * view = iterViews->second.get();
    const std::string sView_filename = stlplus::create_filespec( sfm_data->s_root_path, view->s_Img_path ) ;
    const std::string sView_thumbname = stlplus::create_filespec( thumbnail_path , view->s_Img_path ) ;

    if( ! stlplus::file_exists( sView_thumbname ) )
    {
      // Open the filename
      QImage inImg( sView_filename.c_str() ) ;
      const int inWidth = inImg.width() ;
      const int inHeight = inImg.height() ;
      QImage scaled ;
      if( inWidth > inHeight )
      {
        scaled = inImg.scaledToWidth( 256 , Qt::SmoothTransformation ) ;
      }
      else
      {
        scaled = inImg.scaledToHeight( 256 , Qt::SmoothTransformation ) ;
      }

      // Save the image
      scaled.save( sView_thumbname.c_str() ) ;
    }

    emit progress( i ) ;
  }
  emit progress( nb_image ) ;
  emit finished( nextAction() ) ;
}

} // namespace openMVG_gui