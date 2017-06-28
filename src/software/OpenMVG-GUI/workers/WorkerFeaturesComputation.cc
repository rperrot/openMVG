#include "WorkerFeaturesComputation.hh"

#include "OpenMVGImageInterface.hh"

#include "openMVG/sfm/sfm.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/details/helpers.hpp>

#include <QImage>

#include <fstream>


namespace openMVG_gui
{

WorkerFeaturesComputation::WorkerFeaturesComputation( std::shared_ptr<Project> & pro ,
    const bool overwrite_existing ,
    const WorkerNextAction & na )
  :
  WorkerInterface( na ) ,
  m_overwrite_existing( overwrite_existing ) ,
  m_project( pro )
{

}

/**
* @brief get progress range
*/
void WorkerFeaturesComputation::progressRange( int & min , int & max ) const
{
  const int nb_image = m_project->SfMData()->GetViews().size() ;
  min = 0 ;
  max = nb_image ;
}

/**
* @brief Do the computation
*/
void WorkerFeaturesComputation::process( void )
{
  std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = m_project->SfMData() ;
  const int nb_image = sfm_data->GetViews().size() ;
  const std::string matches_dir = m_project->matchesPath() ;

  const FeatureParams f_params = m_project->featureParams() ;
  std::shared_ptr<openMVG::features::Image_describer> image_describer = f_params.describer() ;


  // Create output feature folder
  const std::string base_feature_path = m_project->globalFeaturePath() ;
  std::string base_detector_path ;
  switch( m_project->featureParams().type() )
  {
    case FEATURE_TYPE_SIFT :
    {
      base_detector_path = stlplus::folder_append_separator( base_feature_path ) + "SIFT" ;
      break ;
    }
    case FEATURE_TYPE_SIFT_ANATOMY :
    {
      base_detector_path = stlplus::folder_append_separator( base_feature_path ) + "SIFT_ANATOMY" ;
      break ;
    }
    case FEATURE_TYPE_AKAZE_FLOAT :
    case FEATURE_TYPE_AKAZE_MLDB :
    {
      base_detector_path = stlplus::folder_append_separator( base_feature_path ) + "AKAZE" ;
      break ;
    }
  }
  if( ! stlplus::folder_exists( base_detector_path ) )
  {
    if( ! stlplus::folder_create( base_detector_path ) )
    {
      emit progress( nb_image ) ;
      emit finished( NEXT_ACTION_ERROR ) ;
      return ;
    }
    if( ! stlplus::folder_exists( base_detector_path ) )
    {
      emit progress( nb_image ) ;
      emit finished( NEXT_ACTION_ERROR ) ;
      return ;
    }
  }
  std::string base_descriptor_path ;
  switch( m_project->featureParams().type() )
  {
    case FEATURE_TYPE_SIFT :
    case FEATURE_TYPE_SIFT_ANATOMY :
    {
      base_descriptor_path = stlplus::folder_append_separator( base_detector_path ) + "SIFT" ;
      break ;
    }
    case FEATURE_TYPE_AKAZE_FLOAT :
    {
      base_descriptor_path = stlplus::folder_append_separator( base_detector_path ) + "MSURF" ;
      break ;
    }
    case FEATURE_TYPE_AKAZE_MLDB :
    {
      base_descriptor_path = stlplus::folder_append_separator( base_detector_path ) + "MLDB" ;
      break ;
    }
  }
  if( ! stlplus::folder_exists( base_descriptor_path ) )
  {
    if( ! stlplus::folder_create( base_descriptor_path ) )
    {
      emit progress( nb_image ) ;
      emit finished( NEXT_ACTION_ERROR ) ;
      return ;
    }
    if( ! stlplus::folder_exists( base_descriptor_path ) )
    {
      emit progress( nb_image ) ;
      emit finished( NEXT_ACTION_ERROR ) ;
      return ;
    }
  }
  std::string feature_path ;
  switch( m_project->featureParams().preset() )
  {
    case FEATURE_PRESET_NORMAL :
    {
      feature_path = stlplus::folder_append_separator( base_descriptor_path ) + "NORMAL" ;
      break ;
    }
    case FEATURE_PRESET_HIGH :
    {
      feature_path = stlplus::folder_append_separator( base_descriptor_path ) + "HIGH" ;
      break ;
    }
    case FEATURE_PRESET_ULTRA :
    {
      feature_path = stlplus::folder_append_separator( base_descriptor_path ) + "ULTRA" ;
      break ;
    }
  }
  if( ! stlplus::folder_exists( feature_path ) )
  {
    if( ! stlplus::folder_create( feature_path ) )
    {
      emit progress( nb_image ) ;
      emit finished( NEXT_ACTION_ERROR ) ;
      return ;
    }
    if( ! stlplus::folder_exists( feature_path ) )
    {
      emit progress( nb_image ) ;
      emit finished( NEXT_ACTION_ERROR ) ;
      return ;
    }
  }

  // Save the image_describer
  const std::string sImage_describer = stlplus::create_filespec( feature_path , "image_describer", "json" );
  {
    std::ofstream stream( sImage_describer.c_str() );
    if ( !stream.is_open() )
    {
      return ;
    }
    cereal::JSONOutputArchive archive( stream );
    archive( cereal::make_nvp( "image_describer", image_describer ) );
    std::unique_ptr<openMVG::features::Regions> regionsType;
    image_describer->Allocate( regionsType );
    archive( cereal::make_nvp( "regions_type", regionsType ) );
  }

  // Try to load a global mask
  openMVG::image::Image<unsigned char> globalMask;

  const std::string sGlobalMask_filename = stlplus::create_filespec( matches_dir , "mask.png" );
  if ( stlplus::file_exists( sGlobalMask_filename ) )
  {
    QImage img( sGlobalMask_filename.c_str() ) ;
    if ( !img.isNull() ) // ReadImage( sGlobalMask_filename.c_str(), &globalMask ) )
    {
      globalMask = QImageToOpenMVGImageGrayscale( img ) ;
      /*
      std::cout
          << "Feature extraction will use a GLOBAL MASK:\n"
          << sGlobalMask_filename << std::endl;
        */
    }
  }

  emit progress( 0 ) ;

  // Now process all the images
  for( int i = 0; i < static_cast<int>( sfm_data->views.size() ); ++i )
  {
    openMVG::sfm::Views::const_iterator iterViews = sfm_data->views.begin();
    std::advance( iterViews, i );
    const openMVG::sfm::View * view = iterViews->second.get();
    const std::string
    sView_filename = stlplus::create_filespec( sfm_data->s_root_path, view->s_Img_path ),
    sFeat = stlplus::create_filespec( feature_path, stlplus::basename_part( sView_filename ), "feat" ),
    sDesc = stlplus::create_filespec( feature_path, stlplus::basename_part( sView_filename ), "desc" );

    //If features or descriptors file are missing, compute them
    if ( !stlplus::file_exists( sFeat ) || !stlplus::file_exists( sDesc ) || m_overwrite_existing )
    {
      QImage img( sView_filename.c_str() ) ;
      if( img.isNull() )
      {
        emit progress( i ) ;
        continue;
      }
      // Convert to gray
      openMVG::image::Image<unsigned char> imageGray = QImageToOpenMVGImageGrayscale( img ) ;

      openMVG::image::Image<unsigned char> * mask = nullptr; // The mask is null by default


      const std::string sImageMask_filename =
        stlplus::create_filespec( sfm_data->s_root_path,
                                  stlplus::basename_part( sView_filename ) + "_mask", "png" );

      openMVG::image::Image<unsigned char> imageMask;
      if ( m_project->maskEnabled( i ) && stlplus::file_exists( sImageMask_filename ) )
      {
        QImage maskimg( sImageMask_filename.c_str() ) ;
        imageMask = QImageToOpenMVGImageGrayscale( maskimg ) ;
        //        ReadImage( sImageMask_filename.c_str(), &imageMask );
      }

      // The mask point to the globalMask, if a valid one exists for the current image
      if ( globalMask.Width() == imageGray.Width() && globalMask.Height() == imageGray.Height() )
      {
        mask = &globalMask;
      }
      // The mask point to the imageMask (individual mask) if a valid one exists for the current image
      if ( imageMask.Width() == imageGray.Width() && imageMask.Height() == imageGray.Height() )
      {
        mask = &imageMask;
      }

      // Compute features and descriptors and export them to files
      std::unique_ptr<openMVG::features::Regions> regions;
      image_describer->Describe( imageGray, regions, mask );
      image_describer->Save( regions.get(), sFeat, sDesc );
    }
    emit progress( i ) ;
  }

  emit progress( nb_image ) ;
  emit finished( nextAction() ) ;
}

} // namespace openMVG_gui
