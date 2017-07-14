#include "FeaturesStats.hh"

#include <cereal/archives/xml.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/utility.hpp>

#include <fstream>

namespace openMVG_gui
{

/**
* @param nb_feat Number of features extracted
* @param elapsed_time Total elapsed time used to extract the features (in second)
*/
FeaturesStats::FeaturesStats( const uint32_t nb_feat , const double elapsed_time )
  : m_nb_features( nb_feat ) ,
    m_elapsed_time( elapsed_time )
{

}


/**
* @brief Get number of features
* @return Number of features extracted
*/
uint32_t FeaturesStats::nbFeature( void ) const
{
  return m_nb_features ;
}

/**
* @brief Get elapsed time for feature extraction
* @return elapsed time (in second)
*/
double FeaturesStats::elapsedTime( void ) const
{
  return m_elapsed_time ;
}

/**
* @brief Save to disk
* @param filename Path of the file where data is saved
*/
void FeaturesStats::save( const std::string & filename )
{
  std::ofstream file( filename ) ;
  if( ! file )
  {
    std::cerr << "Could not save " << filename << std::endl ;
    return ;
    // TODO : throw something ?
  }

  cereal::XMLOutputArchive archive( file );

  int major_version = 0 ;
  int minor_version = 1 ;
  int revision_version = 0 ;

  archive( cereal::make_nvp( "major_version" , major_version ) ) ;
  archive( cereal::make_nvp( "minor_version" , minor_version ) ) ;
  archive( cereal::make_nvp( "revision_version" , revision_version ) ) ;

  archive( cereal::make_nvp( "nb_feature" , m_nb_features ) ) ;
  archive( cereal::make_nvp( "elapsed_time" , m_elapsed_time ) ) ;
}

/**
* @brief Load from disk
* @param filename Path of the file where data is loaded
* @return The loaded data
*/
FeaturesStats FeaturesStats::load( const std::string & filename )
{
  std::ifstream file( filename ) ;

  if( ! file )
  {
    return FeaturesStats( -1 , -1.0 ) ;
  }


  FeaturesStats res;

  // Save global project state
  cereal::XMLInputArchive archive( file );

  int major_version ;
  int minor_version ;
  int revision_version ;
  archive( cereal::make_nvp( "major_version" , major_version ) ) ;
  archive( cereal::make_nvp( "minor_version" , minor_version ) ) ;
  archive( cereal::make_nvp( "revision_version" , revision_version ) ) ;

  archive( cereal::make_nvp( "nb_feature" , res.m_nb_features ) ) ;
  archive( cereal::make_nvp( "elapsed_time" , res.m_elapsed_time ) ) ;

  return res ; 
}

}// namespace openMVG_gui
