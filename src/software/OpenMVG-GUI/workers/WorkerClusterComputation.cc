// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "WorkerClusterComputation.hh"

#include "openMVG/sfm/sfm.hpp"
#include "openMVG/system/timer.hpp"

#include "../SfM/clustering/domSetLibrary/domset.h"
#include "../SfM/clustering/domSetLibrary/types.h"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <iomanip>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef OPENMVG_USE_OPENMP
  #include <omp.h>
#endif

namespace openMVG_gui
{
WorkerClusterComputation::WorkerClusterComputation( std::shared_ptr<Project> &proj, const int cluster_lower_bound, const int cluster_upper_bound, const float voxelSize )
  : m_project( proj )
  , m_cluster_lower_bound( cluster_lower_bound )
  , m_cluster_upper_bound( cluster_upper_bound )
  , m_cluster_voxel_grid_size( voxelSize )
{
}

/**
 * @brief get progress range
 */
void WorkerClusterComputation::progressRange( int &min, int &max ) const
{
  // 0 -> start
  // 1 -> convert to domset
  // 2 -> clustering
  // 3 -> save
  min = 0;
  max = 3;
}

/**
 * @brief convert openMVG sfm_data to domset data
 * @param sfm_data openMVG dataset
 * @param[out] cameras list of intrinsics
 * @param[out] view list of views
 * @param[out] points list of points
 * @param[out] map_view map between views added and the global index
 *  We need this map to handle the fact that OpenMVG view Id can be non
 *   contiguous:
 *   - in case of missing pose or intrinsic for some view
 * @retval true if success
 * @retval false if failure
 */
static bool domsetImporter( const openMVG::sfm::SfM_Data &sfm_data, std::vector<nomoko::Camera> &cameras, std::vector<nomoko::View> &views, std::vector<nomoko::Point> &points, std::map<openMVG::IndexT, uint32_t> &map_view )
{

  // Convert OpenMVG data to domset library data
  openMVG::system::Timer loadDataTimer;

  // adding views
  for ( const auto &view : sfm_data.GetViews() )
  {
    if ( sfm_data.IsPoseAndIntrinsicDefined( view.second.get() ) )
    {
      map_view[ view.first ] = views.size(); // need to make global

      const openMVG::geometry::Pose3 poseMVG( sfm_data.GetPoseOrDie( view.second.get() ) );
      nomoko::View v;
      v.rot   = poseMVG.rotation().cast<float>();
      v.trans = poseMVG.center().transpose().cast<float>();
      views.push_back( v );
    }
  }

  // adding landmarks
  for ( const auto &it_landmark : sfm_data.GetLandmarks() )
  {
    const openMVG::sfm::Landmark &landmark = it_landmark.second;
    const openMVG::sfm::Observations &obs  = landmark.obs;
    // X, color, obsCount
    std::vector<size_t> vIds;
    for ( const auto &it_obs : obs )
    {
      vIds.push_back( map_view[ it_obs.first ] );
    }

    nomoko::Point p;
    p.pos      = landmark.X.transpose().cast<float>();
    p.viewList = vIds;
    points.push_back( p );
  }

  std::cout << std::endl << "Number of views  = " << views.size() << std::endl << "Number of points = " << points.size() << std::endl << "Loading data took (s): " << loadDataTimer.elapsed() << std::endl;
  return true;
}

/**
 * @brief Export a sfm_data file using a subset of the view of a given sfm_data
 * @param sfm_data The whole data set
 * @param outFilename Output file name
 * @param cluster List of view to consider
 * @retval true if success
 * @retval false if failure
 */
static bool exportData( const openMVG::sfm::SfM_Data &sfm_data, const std::string &outFilename, const std::set<size_t> &cluster )
{
  openMVG::sfm::SfM_Data cl_sfm_data;
  cl_sfm_data.s_root_path = sfm_data.s_root_path;

  // Copy the view (only the requested ones)
  for ( const auto view : sfm_data.GetViews() )
  {
    const bool inCluster = cluster.find( view.first ) != cluster.end();
    if ( inCluster && sfm_data.IsPoseAndIntrinsicDefined( view.second.get() ) )
    {
      cl_sfm_data.poses[ view.first ] = sfm_data.GetPoseOrDie( view.second.get() );
      cl_sfm_data.views[ view.first ] = view.second;

      const auto intrinsic = sfm_data.GetIntrinsics().at( view.second.get()->id_intrinsic );
      if ( cl_sfm_data.intrinsics.count( view.second.get()->id_intrinsic ) == 0 )
      {
        cl_sfm_data.intrinsics[ view.second.get()->id_intrinsic ] = intrinsic;
      }
    }
  }

  // Copy observations that have relation with the considered view
  for ( const auto &it_landmark : sfm_data.GetLandmarks() )
  {
    const openMVG::sfm::Landmark &landmark = it_landmark.second;
    openMVG::sfm::Observations obs;
    for ( const auto &observation : landmark.obs )
    {
      const auto &it = cl_sfm_data.views.find( observation.first );
      if ( it != cl_sfm_data.views.end() )
      {
        obs[ observation.first ] = observation.second;
      }
    }
    // Landmark observed in less than 3 view are ignored
    if ( obs.size() < 2 )
    {
      continue;
    }
    cl_sfm_data.structure[ it_landmark.first ].X   = landmark.X;
    cl_sfm_data.structure[ it_landmark.first ].obs = obs;
  }

  return Save( cl_sfm_data, outFilename, openMVG::sfm::ESfM_Data( openMVG::sfm::ALL ) );
}

/**
 * @brief create the project
 */
void WorkerClusterComputation::process( void )
{
  m_progress_value = 0;
  sendProgress();

  std::string sOutDir = stlplus::folder_append_separator( m_project->projectPaths().exportPath() ) + "clusters";
  // erase existing data (if it exists)
  if ( stlplus::folder_exists( sOutDir ) )
  {
    stlplus::folder_delete( sOutDir, true );
  }

  // Recreate from scratch
  if ( !stlplus::folder_exists( sOutDir ) )
  {
    if ( !stlplus::folder_create( sOutDir ) )
    {
      emit finished( NEXT_ACTION_ERROR );
      return;
    }
    if ( !stlplus::folder_exists( sOutDir ) )
    {
      emit finished( NEXT_ACTION_ERROR );
      return;
    }
  }

  // loading data
  std::vector<nomoko::Camera> cameras; // stores the various camera intrinsic parameters
  std::vector<nomoko::View> views;     // stores the poses for each view
  std::vector<nomoko::Point> points;   // 3d point positions

  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData();

  std::map<openMVG::IndexT, uint32_t> origViewMap; // need to keep track of original views ids
  if ( !domsetImporter( *sfm_data, cameras, views, points, origViewMap ) )
  {
    emit finished( NEXT_ACTION_ERROR );
    return;
  }
  hasIncremented( 1 );

  // Clustering
  //---------------------------------------
  // clustering views process
  //---------------------------------------
  openMVG::system::Timer clusteringTimer;

  nomoko::Domset domset( points, views, cameras, m_cluster_voxel_grid_size );
  domset.clusterViews( m_cluster_lower_bound, m_cluster_upper_bound );

  std::cout << "Clustering view took (s): " << clusteringTimer.elapsed() << std::endl;
  hasIncremented( 1 );

  // export to ply to visualize
  const std::string viewOut = stlplus::folder_append_separator( sOutDir ) + "views.ply";
  domset.exportToPLY( viewOut );

  // Retrieve the cluster and export them
  std::vector<std::set<size_t>> finalClusters;
  {
    const std::vector<std::vector<size_t>> clusters = domset.getClusters();

    // Remap the camera index from contiguous to original view Id
    std::map<openMVG::IndexT, uint32_t> origViewMap_reverse;
    for ( const auto &it : origViewMap )
    {
      origViewMap_reverse.insert( std::make_pair( it.second, it.first ) );
    }
    // For every cluster, remap the view Id
    for ( const auto &cl : clusters )
    {
      std::set<size_t> newCl;
      for ( const auto vId : cl )
      {
        newCl.insert( origViewMap_reverse[ vId ] );
      }
      finalClusters.emplace_back( newCl );
    }
  }

  const size_t numClusters = finalClusters.size();
  std::cout << "Number of clusters = " << numClusters << std::endl;

  // Save

#ifdef OPENMVG_USE_OPENMP
  #pragma omp parallel for
#endif
  for ( int i = 0; i < static_cast<int>( numClusters ); ++i )
  {
    std::stringstream filename;
    filename << "sfm_data";
    filename << std::setw( 4 ) << std::setfill( '0' ) << i;
    filename << ".bin";

#ifdef OPENMVG_USE_OPENMP
    #pragma omp critical
#endif
    {
      std::stringstream ss;
      ss << "Writing cluster to " << filename.str() << std::endl;
      std::cout << ss.str();
    }

    const std::string file_path = stlplus::folder_append_separator( sOutDir ) + filename.str();
    if ( !exportData( *sfm_data, file_path, finalClusters[ i ] ) )
    {
      std::stringstream str;
      str << "Could not write cluster : " << filename.str() << std::endl;
      std::cerr << str.str();
    }
  }

  hasIncremented( 1 );
  emit finished( nextAction() );
}

/**
 * @brief internal progress bar has been incremented, now signal it to the external progress dialog
 */
void WorkerClusterComputation::hasIncremented( int nb )
{
  m_progress_value += nb;
  sendProgress();
}

/**
 * @brief set progress value to the main thread
 */
void WorkerClusterComputation::sendProgress( void )
{
  int progress_value = m_progress_value;
  emit progress( progress_value );
}

} // namespace openMVG_gui