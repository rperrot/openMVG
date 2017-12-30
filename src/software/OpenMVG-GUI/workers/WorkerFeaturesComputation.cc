// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "WorkerFeaturesComputation.hh"

#include "OpenMVGImageInterface.hh"
#include "utils/FeaturesStats.hh"

#include "openMVG/sfm/sfm.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/details/helpers.hpp>

#include "nonFree/sift/SIFT_describer_io.hpp"
#include "openMVG/features/image_describer_akaze_io.hpp"
#include "openMVG/features/sift/SIFT_Anatomy_Image_Describer_io.hpp"

#include <QImage>

#include <chrono>
#include <fstream>

namespace openMVG_gui
{

/**
 * @brief Ctr
 * @param pro The project to get inputs and parameters
 * @param overwrite_existing Indicate to overwrite existing computation
 * @param na Next action to transmit after computation finished
 */
WorkerFeaturesComputation::WorkerFeaturesComputation( std::shared_ptr<Project> &pro ,
    const bool overwrite_existing ,
    const WorkerNextAction &na  )
  : WorkerInterface( na ) ,
    m_overwrite_existing( overwrite_existing ) ,
    m_project( pro ) ,
    m_nb_processed( 0 )
{

}

/**
 * @brief get progress range
 */
void WorkerFeaturesComputation::progressRange( int &min, int &max ) const
{
  const int nb_image = m_project->SfMData()->GetViews().size();
  min                = 0;
  max                = nb_image;
}

/**
 * @brief Do the computation
 */
void WorkerFeaturesComputation::process( void )
{
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData();
  const int nb_image                               = sfm_data->GetViews().size();
  const std::string matches_dir                    = m_project->projectPaths().matchesPath();

  const FeatureParams f_params                                        = m_project->featureParams();
  std::shared_ptr<openMVG::features::Image_describer> image_describer = f_params.describer();
  const int nb_job = f_params.nbParallelJob() ;

  // Create output feature folder
  const std::string base_feature_path = m_project->projectPaths().globalFeaturePath();
  std::string base_detector_path;
  switch ( m_project->featureParams().type() )
  {
    case FEATURE_TYPE_SIFT:
    {
      base_detector_path = stlplus::folder_append_separator( base_feature_path ) + "SIFT";
      break;
    }
    case FEATURE_TYPE_SIFT_ANATOMY:
    {
      base_detector_path = stlplus::folder_append_separator( base_feature_path ) + "SIFT_ANATOMY";
      break;
    }
    case FEATURE_TYPE_AKAZE_FLOAT:
    case FEATURE_TYPE_AKAZE_MLDB:
    {
      base_detector_path = stlplus::folder_append_separator( base_feature_path ) + "AKAZE";
      break;
    }
  }
  if ( !stlplus::folder_exists( base_detector_path ) )
  {
    if ( !stlplus::folder_create( base_detector_path ) )
    {
      emit progress( nb_image );
      emit finished( NEXT_ACTION_ERROR );
      return;
    }
    if ( !stlplus::folder_exists( base_detector_path ) )
    {
      emit progress( nb_image );
      emit finished( NEXT_ACTION_ERROR );
      return;
    }
  }
  std::string base_descriptor_path;
  switch ( m_project->featureParams().type() )
  {
    case FEATURE_TYPE_SIFT:
    case FEATURE_TYPE_SIFT_ANATOMY:
    {
      base_descriptor_path = stlplus::folder_append_separator( base_detector_path ) + "SIFT";
      break;
    }
    case FEATURE_TYPE_AKAZE_FLOAT:
    {
      base_descriptor_path = stlplus::folder_append_separator( base_detector_path ) + "MSURF";
      break;
    }
    case FEATURE_TYPE_AKAZE_MLDB:
    {
      base_descriptor_path = stlplus::folder_append_separator( base_detector_path ) + "MLDB";
      break;
    }
  }
  if ( !stlplus::folder_exists( base_descriptor_path ) )
  {
    if ( !stlplus::folder_create( base_descriptor_path ) )
    {
      emit progress( nb_image );
      emit finished( NEXT_ACTION_ERROR );
      return;
    }
    if ( !stlplus::folder_exists( base_descriptor_path ) )
    {
      emit progress( nb_image );
      emit finished( NEXT_ACTION_ERROR );
      return;
    }
  }
  std::string feature_path;
  switch ( m_project->featureParams().preset() )
  {
    case FEATURE_PRESET_NORMAL:
    {
      feature_path = stlplus::folder_append_separator( base_descriptor_path ) + "NORMAL";
      break;
    }
    case FEATURE_PRESET_HIGH:
    {
      feature_path = stlplus::folder_append_separator( base_descriptor_path ) + "HIGH";
      break;
    }
    case FEATURE_PRESET_ULTRA:
    {
      feature_path = stlplus::folder_append_separator( base_descriptor_path ) + "ULTRA";
      break;
    }
  }
  if ( !stlplus::folder_exists( feature_path ) )
  {
    if ( !stlplus::folder_create( feature_path ) )
    {
      emit progress( nb_image );
      emit finished( NEXT_ACTION_ERROR );
      return;
    }
    if ( !stlplus::folder_exists( feature_path ) )
    {
      emit progress( nb_image );
      emit finished( NEXT_ACTION_ERROR );
      return;
    }
  }

  // Save the image_describer
  const std::string sImage_describer = stlplus::create_filespec( feature_path, "image_describer", "json" );
  {
    std::ofstream stream( sImage_describer.c_str() );
    if ( !stream.is_open() )
    {
      return;
    }
    cereal::JSONOutputArchive archive( stream );
    archive( cereal::make_nvp( "image_describer", image_describer ) );
    auto regionsType = image_describer->Allocate();
    archive( cereal::make_nvp( "regions_type", regionsType ) );
  }

  // Try to load a global mask
  openMVG::image::Image<unsigned char> globalMask;

  const std::string sGlobalMask_filename = stlplus::create_filespec( matches_dir, "mask.png" );
  if ( stlplus::file_exists( sGlobalMask_filename ) )
  {
    QImage img( sGlobalMask_filename.c_str() );
    if ( !img.isNull() ) // ReadImage( sGlobalMask_filename.c_str(), &globalMask ) )
    {
      globalMask = QImageToOpenMVGImageGrayscale( img );
      /*
      std::cout
          << "Feature extraction will use a GLOBAL MASK:\n"
          << sGlobalMask_filename << std::endl;
        */
    }
  }

  // Compute nb_thread and thread params
  const int nb_thread = nb_job ;
  const int nb_image_per_thread = nb_image / nb_thread ;

  std::vector< std::thread > threads ;
  bool * thread_ok = new bool[ nb_thread ] ; // because std::vector<bool> is a special container

  // Let's get party !
  emit progress( 0 );
  m_nb_processed = 0 ;
  int id_start = 0 ;
  for( int id_thread = 0 ; id_thread < nb_thread ; ++id_thread )
  {
    const int id_end = ( id_thread + 1 ) == nb_thread ? nb_image : id_start + nb_image_per_thread ;

    threads.emplace_back( &WorkerFeaturesComputation::processThread , this ,
                          std::ref( sfm_data ) , id_start , id_end , std::ref( feature_path ) , std::ref( globalMask ) , std::ref( image_describer ) , std::ref( thread_ok[ id_thread ] ) )  ;
    id_start += nb_image_per_thread ;
  }

  for( auto & t : threads )
  {
    t.join() ;
  }

  bool ok = true ;
  for( int id_thread = 0 ; id_thread < nb_thread ; ++id_thread )
  {
    ok &= thread_ok[ id_thread ] ;
  }

  if( ok )
  {
    emit progress( nb_image );
    emit finished( nextAction() );
  }
  else
  {
    emit progress( nb_image );
    emit finished( NEXT_ACTION_ERROR );
  }


}

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
void WorkerFeaturesComputation::processThread( std::shared_ptr<openMVG::sfm::SfM_Data> & sfm_data , const int id_start , const int id_end ,
    const std::string & feature_path ,
    const openMVG::image::Image<unsigned char> & globalMask ,
    std::shared_ptr<openMVG::features::Image_describer> & image_describer ,
    bool & ok )
{
  try
  {
    for ( int i = id_start ; i < id_end ; ++i )
    {
      openMVG::sfm::Views::const_iterator iterViews = sfm_data->views.begin();
      std::advance( iterViews, i );
      const openMVG::sfm::View *view   = iterViews->second.get();
      const std::string sView_filename = stlplus::create_filespec( sfm_data->s_root_path, view->s_Img_path );
      const std::string sFeat          = stlplus::create_filespec( feature_path, stlplus::basename_part( sView_filename ), "feat" );
      const std::string sDesc          = stlplus::create_filespec( feature_path, stlplus::basename_part( sView_filename ), "desc" );
      const std::string sStat          = stlplus::create_filespec( feature_path, stlplus::basename_part( sView_filename ), "stat" );

      // If features or descriptors file are missing, compute them
      if ( !stlplus::file_exists( sFeat ) || !stlplus::file_exists( sDesc ) || m_overwrite_existing )
      {
        QImage img( sView_filename.c_str() );
        if ( img.isNull() )
        {
          emit progress( m_nb_processed ) ;
          ++m_nb_processed ;
          continue;
        }
        // Convert to gray
        openMVG::image::Image<unsigned char> imageGray = QImageToOpenMVGImageGrayscale( img );

        openMVG::image::Image<unsigned char> *mask = nullptr; // The mask is null by default

        const std::string sImageMask_filename = stlplus::create_filespec( sfm_data->s_root_path, stlplus::basename_part( sView_filename ) + "_mask", "png" );

        openMVG::image::Image<unsigned char> imageMask;
        if ( m_project->maskEnabled( i ) && stlplus::file_exists( sImageMask_filename ) )
        {
          QImage maskimg( sImageMask_filename.c_str() );
          imageMask = QImageToOpenMVGImageGrayscale( maskimg );
        }

        // The mask point to the globalMask, if a valid one exists for the current image
        if ( globalMask.Width() == imageGray.Width() && globalMask.Height() == imageGray.Height() )
        {
          mask = const_cast<openMVG::image::Image<unsigned char> *>( &globalMask );
        }
        // The mask point to the imageMask (individual mask) if a valid one exists for the current image
        if ( imageMask.Width() == imageGray.Width() && imageMask.Height() == imageGray.Height() )
        {
          mask = &imageMask;
        }

        // Compute features and descriptors and export them to files
        std::unique_ptr<openMVG::features::Regions> regions;
        const auto start = std::chrono::high_resolution_clock::now();
        image_describer->Describe( imageGray, regions, mask );
        const auto end                                  = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double> elapsed_sec = std::chrono::duration_cast<std::chrono::duration<double>>( end - start );

        // Save description
        image_describer->Save( regions.get(), sFeat, sDesc );

        // Save statistics
        FeaturesStats statistics( regions->RegionCount(), elapsed_sec.count() );
        statistics.save( sStat );
      }
      emit progress( m_nb_processed );
      ++m_nb_processed ;
    }
    ok = true ;
  }
  catch( ... )
  {
    ok = false ;
  }
}

} // namespace openMVG_gui
