#include "ProjectPaths.hh"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"


namespace openMVG_gui
{

ProjectPaths::ProjectPaths( const std::string & base_path )
  : m_base_path( base_path )
{

}

/**
 * @brief Base path for all gui related elements
 */
std::string ProjectPaths::guiPath( void ) const
{
  return stlplus::folder_append_separator( m_base_path ) + "gui" ;
}

/**
 * @brief Path of the thumbnails
 */
std::string ProjectPaths::thumbnailsPath( void ) const
{
  return stlplus::folder_append_separator( stlplus::folder_append_separator( m_base_path ) + "gui" ) + "thumbnails" ;
}

/**
 * @brief Path of the features depending on the given parameters
 */
std::string ProjectPaths::featuresPath( const FeatureParams & params ) const
{
  // Feature path :
  // folder_feature_detector / folder_descriptor / folder_preset

  std::string folder_detector ;
  std::string folder_descriptor ;
  std::string folder_preset ;

  switch( params.type() )
  {
    case FEATURE_TYPE_SIFT :
    {
      folder_detector = "SIFT" ;
      break ;
    }
    case FEATURE_TYPE_SIFT_ANATOMY :
    {
      folder_detector = "SIFT_ANATOMY" ;
      break ;
    }
    case FEATURE_TYPE_AKAZE_FLOAT :
    case FEATURE_TYPE_AKAZE_MLDB :
    {
      folder_detector = "AKAZE" ;
      break ;
    }
  }

  switch( params.type() )
  {
    case FEATURE_TYPE_SIFT :
    case FEATURE_TYPE_SIFT_ANATOMY :
    {
      folder_descriptor = "SIFT" ;
      break ;
    }
    case FEATURE_TYPE_AKAZE_FLOAT :
    {
      folder_descriptor = "MSURF" ;
      break ;
    }
    case FEATURE_TYPE_AKAZE_MLDB :
    {
      folder_descriptor = "MLDB" ;
      break ;
    }
  }

  switch( params.preset() )
  {
    case FEATURE_PRESET_NORMAL :
    {
      folder_preset = "NORMAL" ;
      break ;
    }
    case FEATURE_PRESET_HIGH :
    {
      folder_preset = "HIGH" ;
      break ;
    }
    case FEATURE_PRESET_ULTRA :
    {
      folder_preset = "ULTRA" ;
      break ;
    }
  }

  return
    stlplus::folder_append_separator(
      stlplus::folder_append_separator(
        stlplus::folder_append_separator( globalFeaturePath() ) + folder_detector ) +
      folder_descriptor ) + folder_preset ;
}

/**
 * @brief Get all combinations of paths for all features modes
 */
std::vector< std::string > ProjectPaths::featuresPaths( void ) const
{

  // All valid combinations
  // DETECTOR / DESCRIPTOR / PRESET
  const std::vector< std::vector< std::string > > combinations =
  {
    // SIFT
    { "SIFT" , "SIFT" , "NORMAL" } ,
    { "SIFT" , "SIFT" , "HIGH" } ,
    { "SIFT" , "SIFT" , "ULTRA" } ,
    // SIFT ANATOMY
    { "SIFT_ANATOMY" , "SIFT" , "NORMAL" } ,
    { "SIFT_ANATOMY" , "SIFT" , "HIGH" } ,
    { "SIFT_ANATOMY" , "SIFT" , "ULTRA" } ,
    // AKAZE
    { "AKAZE" , "MSURF" , "NORMAL" } ,
    { "AKAZE" , "MSURF" , "HIGH" } ,
    { "AKAZE" , "MSURF" , "ULTRA" } ,
    { "AKAZE" , "MLDB" , "NORMAL" } ,
    { "AKAZE" , "MLDB" , "HIGH" } ,
    { "AKAZE" , "MLDB" , "ULTRA" }
  } ;

  std::vector< std::string > res ;
  for( const auto & cur_combi : combinations )
  {
    std::string path = globalFeaturePath() ;
    for( const auto & param_combi : cur_combi )
    {
      path = stlplus::folder_append_separator( path ) + param_combi ;
    }

    if( stlplus::folder_exists( path ) )
    {
      res.emplace_back( path ) ;
    }
  }
  return res ;
}


std::string ProjectPaths::globalFeaturePath( void ) const
{
  return stlplus::folder_append_separator( stlplus::folder_append_separator( m_base_path ) + "sfm" ) + "features" ;
}


/**
 * @brief Path where matching is stored depending on the given parameters
 */
std::string ProjectPaths::matchesPath( ) const
{
  //  return featuresPath() ;
  return stlplus::folder_append_separator( stlplus::folder_append_separator( m_base_path ) + "sfm" ) + "matches" ;

}

/**
 *  @brief Path of the (un)colorized ply file
 */
std::string ProjectPaths::plyCloud( const SfMMethod & method ) const
{
  const std::string sfm_path = sfmReconstructionPath( method ) ;
  return stlplus::create_filespec( sfm_path , "cloud_and_poses.ply" ) ;
}

/**
 *  @brief Path of the colorized ply file
 */
std::string ProjectPaths::colorizedPlyCloud( const SfMMethod & method ) const
{
  const std::string sfm_path = sfmReconstructionPath( method ) ;
  return stlplus::create_filespec( sfm_path , "colorized.ply" ) ;
}

/**
 * @brief Base path of the sfm methods
 */
std::string ProjectPaths::sfmBasePath( void ) const
{
  return stlplus::folder_append_separator( m_base_path ) + "sfm" ;
}

/**
 *  @brief Path of the output of the sfm process
 */
std::string ProjectPaths::sfmReconstructionPath( const SfMMethod & method ) const
{
  if( method == SFM_METHOD_GLOBAL )
  {
    return stlplus::folder_append_separator( sfmBasePath( ) ) + "reconstruction_global" ;
  }
  else
  {
    return stlplus::folder_append_separator( sfmBasePath( ) ) + "reconstruction_sequential" ;
  }
}

/**
 * @brief Path where global reconstruction elements are stored
 */
std::string ProjectPaths::reconstructionGlobalPath( void ) const
{
  return stlplus::folder_append_separator( sfmBasePath( ) ) + "reconstruction_global" ;
}

/**
 * @brief Path where the incremental reconstruction elements are stored
 */
std::string ProjectPaths::reconstructionSequentialPath( void ) const
{
  return stlplus::folder_append_separator( sfmBasePath( ) ) + "reconstruction_sequential" ;
}

/**
 *  @brief Path of the hmtl file containing the report about sfm process
 */
std::string ProjectPaths::htmlReportPath( const SfMMethod & method ) const
{
  const std::string sfm_path = sfmReconstructionPath( method ) ;
  return stlplus::create_filespec( sfm_path , "Reconstruction_Report.html" ) ;
}

/**
 *  @brief Base path where exports to other projects are stored
 */
std::string ProjectPaths::exportPath( void ) const
{
  return stlplus::folder_append_separator( m_base_path ) + "export" ;

}

} // namespace openMVG_gui