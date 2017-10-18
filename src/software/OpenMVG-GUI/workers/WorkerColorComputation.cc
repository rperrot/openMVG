// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "WorkerColorComputation.hh"

#include "OpenMVGImageInterface.hh"
#include "WorkerProgressInterface.hh"

#include "openMVG/sfm/sfm_data_io.hpp"
#include "openMVG/stl/stl.hpp"
#include "openMVG/types.hpp"
#include "software/SfM/SfMPlyHelper.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <QImage>

using namespace openMVG;
using namespace openMVG::image;
using namespace openMVG::sfm;

namespace openMVG_gui
{
/**
 * @brief Ctr Constructor
 */
WorkerColorComputation::WorkerColorComputation( std::shared_ptr<Project> proj )
  : m_project( proj )
{
}

/**
 * @brief get progress range
 */
void WorkerColorComputation::progressRange( int &min, int &max ) const
{
  min = 0;
  max = m_project->SfMData()->GetLandmarks().size() + 1;

  std::cout << "min : " << min << " - max : " << max << std::endl;
}

/// Find the color of the SfM_Data Landmarks/structure
bool WorkerColorComputation::ColorizeTracks( const std::shared_ptr<SfM_Data> &sfm_data, std::vector<Vec3> &vec_3dPoints, std::vector<Vec3> &vec_tracksColor )
{
  // Colorize each track
  //  Start with the most representative image
  //    and iterate to provide a color to each 3D point

  WorkerProgressInterface *progressInterface = new WorkerProgressInterface();
  {
    m_progress_value = 0;
    progressInterface->restart( sfm_data->GetLandmarks().size() );
    connect( progressInterface, SIGNAL( increment( int ) ), this, SLOT( hasIncremented( int ) ), Qt::DirectConnection );

    /*
    C_Progress_display my_progress_bar( sfm_data.GetLandmarks().size(),
                                        std::cout,
                                        "\nCompute scene structure color\n" );
                                        */
    vec_tracksColor.resize( sfm_data->GetLandmarks().size() );
    vec_3dPoints.resize( sfm_data->GetLandmarks().size() );

    // Build a list of contiguous index for the trackIds
    std::map<IndexT, IndexT> trackIds_to_contiguousIndexes;
    IndexT cpt = 0;
    for ( Landmarks::const_iterator it = sfm_data->GetLandmarks().begin(); it != sfm_data->GetLandmarks().end(); ++it, ++cpt )
    {
      trackIds_to_contiguousIndexes[ it->first ] = cpt;
      vec_3dPoints[ cpt ]                        = it->second.X;
    }

    // The track list that will be colored (point removed during the process)
    std::set<IndexT> remainingTrackToColor;
    std::transform( sfm_data->GetLandmarks().begin(), sfm_data->GetLandmarks().end(), std::inserter( remainingTrackToColor, remainingTrackToColor.begin() ), stl::RetrieveKey() );

    while ( !remainingTrackToColor.empty() )
    {
      // Find the most representative image (for the remaining 3D points)
      //  a. Count the number of observation per view for each 3Dpoint Index
      //  b. Sort to find the most representative view index

      std::map<IndexT, IndexT> map_IndexCardinal; // ViewId, Cardinal
      for ( std::set<IndexT>::const_iterator iterT = remainingTrackToColor.begin(); iterT != remainingTrackToColor.end(); ++iterT )
      {
        const size_t trackId    = *iterT;
        const Observations &obs = sfm_data->GetLandmarks().at( trackId ).obs;
        for ( Observations::const_iterator iterObs = obs.begin(); iterObs != obs.end(); ++iterObs )
        {
          const size_t viewId = iterObs->first;
          if ( map_IndexCardinal.find( viewId ) == map_IndexCardinal.end() )
          {
            map_IndexCardinal[ viewId ] = 1;
          }
          else
          {
            ++map_IndexCardinal[ viewId ];
          }
        }
      }

      // Find the View index that is the most represented
      std::vector<IndexT> vec_cardinal;
      std::transform( map_IndexCardinal.begin(), map_IndexCardinal.end(), std::back_inserter( vec_cardinal ), stl::RetrieveValue() );
      using namespace stl::indexed_sort;
      std::vector<sort_index_packet_descend<IndexT, IndexT>> packet_vec( vec_cardinal.size() );
      sort_index_helper( packet_vec, &vec_cardinal[ 0 ], 1 );

      // First image index with the most of occurence
      std::map<IndexT, IndexT>::const_iterator iterTT = map_IndexCardinal.begin();
      std::advance( iterTT, packet_vec[ 0 ].index );
      const size_t view_index          = iterTT->first;
      const View *view                 = sfm_data->GetViews().at( view_index ).get();
      const std::string sView_filename = stlplus::create_filespec( sfm_data->s_root_path, view->s_Img_path );
      QImage qt_img( sView_filename.c_str() );
      Image<RGBColor> image_rgb = QImageToOpenMVGImage( qt_img );
      const bool b_rgb_image    = true;

      // Iterate through the remaining track to color
      // - look if the current view is present to color the track
      std::set<IndexT> set_toRemove;
      for ( std::set<IndexT>::const_iterator iterT = remainingTrackToColor.begin(); iterT != remainingTrackToColor.end(); ++iterT )
      {
        const size_t trackId            = *iterT;
        const Observations &obs         = sfm_data->GetLandmarks().at( trackId ).obs;
        Observations::const_iterator it = obs.find( view_index );

        if ( it != obs.end() )
        {
          // Color the track
          const Vec2 &pt       = it->second.x;
          const RGBColor color = image_rgb( pt.y(), pt.x() ); // : RGBColor( image_gray( pt.y(), pt.x() ) );

          vec_tracksColor[ trackIds_to_contiguousIndexes[ trackId ] ] = Vec3( color.r(), color.g(), color.b() );
          set_toRemove.insert( trackId );
          ++( *progressInterface ); // my_progress_bar;
        }
      }
      // Remove colored track
      for ( std::set<IndexT>::const_iterator iter = set_toRemove.begin(); iter != set_toRemove.end(); ++iter )
      {
        remainingTrackToColor.erase( *iter );
      }
    }
  }

  delete progressInterface;

  return true;
}

/// Export camera poses positions as a Vec3 vector
void GetCameraPositions( const std::shared_ptr<SfM_Data> &sfm_data, std::vector<Vec3> &vec_camPosition )
{
  for ( const auto &view : sfm_data->GetViews() )
  {
    if ( sfm_data->IsPoseAndIntrinsicDefined( view.second.get() ) )
    {
      const geometry::Pose3 pose = sfm_data->GetPoseOrDie( view.second.get() );
      vec_camPosition.push_back( pose.center() );
    }
  }
}

/**
 * @brief Do the computation
 */
void WorkerColorComputation::process( void )
{
  std::shared_ptr<Project> project                 = m_project;
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData();
  const int last                                   = sfm_data->GetLandmarks().size();

  const std::string sfm_path = m_project->projectPaths().sfmReconstructionPath( m_project->sfMMethod() );
  const std::string sOutputPLY_Out = stlplus::create_filespec( sfm_path , "colorized.ply" );

  m_progress_value = 0;
  sendProgress();

  // Compute the scene structure color
  std::vector<Vec3> vec_3dPoints, vec_tracksColor, vec_camPosition;
  if ( ColorizeTracks( sfm_data, vec_3dPoints, vec_tracksColor ) )
  {
    GetCameraPositions( sfm_data, vec_camPosition );

    // Export the SfM_Data scene in the expected format
    if ( !plyHelper::exportToPly( vec_3dPoints, vec_camPosition, sOutputPLY_Out, &vec_tracksColor ) )
    {
      m_progress_value = last + 1;
      sendProgress();
      emit finished( nextAction() );
      return;
    }
  }

  m_progress_value = last + 1;
  sendProgress();
  emit finished( nextAction() );
}

void WorkerColorComputation::sendProgress( void )
{
  int progress_value = m_progress_value;
  emit progress( progress_value );
}

void WorkerColorComputation::hasIncremented( int nb )
{
  m_progress_value += nb;
  sendProgress();
}

} // namespace openMVG