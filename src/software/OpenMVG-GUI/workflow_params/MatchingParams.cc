#include "MatchingParams.hh"

#include "openMVG/matching_image_collection/Matcher_Regions.hpp"
#include "openMVG/matching_image_collection/Cascade_Hashing_Matcher_Regions.hpp"
#include "openMVG/matching_image_collection/GeometricFilter.hpp"

namespace openMVG_gui
{

/**
* @brief Ctr
* @note default parameters for matching sift and preparing incremental SfM
*/
MatchingParams::MatchingParams( const MatchingMethod & method ,
                                const float distRatio ,
                                const MatchingGeometryType & geom ,
                                const int max_iter_geom_filtering )
  : m_geometry( geom ) ,
    m_max_iteration_filtering( max_iter_geom_filtering ) ,
    m_method( method ) ,
    m_ratio( distRatio )
{
  buildMatcher() ;
}

/**
* @brief Move ctr
* @param src Source
*/
MatchingParams::MatchingParams( MatchingParams && src )
  : m_geometry( src.m_geometry ) ,
    m_max_iteration_filtering( src.m_max_iteration_filtering ) ,
    m_method( src.m_method ) ,
    m_ratio( src.m_ratio )
{
  buildMatcher() ;
}

/**
* @brief Copy ctr
* @param src Source
*/
MatchingParams::MatchingParams( const MatchingParams & src )
  : m_geometry( src.m_geometry ) ,
    m_max_iteration_filtering( src.m_max_iteration_filtering ) ,
    m_method( src.m_method ) ,
    m_ratio( src.m_ratio )
{
  buildMatcher() ;
}

/**
* @brief Assignment operator
* @param src Source
* @return self after assignment
*/
MatchingParams & MatchingParams::operator=( const MatchingParams & src )
{
  if( this != &src )
  {
    m_geometry = src.m_geometry ;
    m_max_iteration_filtering = src.m_max_iteration_filtering ;
    m_method = src.m_method ;
    m_ratio = src.m_ratio ;

    buildMatcher() ;
  }
  return ( *this ) ;
}

/**
* @brief Move Assignment operator
* @param src Source
* @return self after assignment
*/
MatchingParams & MatchingParams::operator=( MatchingParams && src )
{
  m_geometry = src.m_geometry ;
  m_max_iteration_filtering = src.m_max_iteration_filtering ;
  m_method = src.m_method ;
  m_ratio = src.m_ratio ;

  buildMatcher() ;

  return ( *this ) ;
}

/**
* @brief Get geometric model used for matching
* @return geometric model
*/
MatchingGeometryType MatchingParams::geometricModel() const
{
  return m_geometry ;
}

/**
* @brief Set geometric model for matching
* @param geom the geometric model to use
*/
void MatchingParams::setGeometricModel( const MatchingGeometryType & geom )
{
  if( m_geometry != geom )
  {
    //    buildMatcher() ;
    m_geometry = geom ;
  }
}

/**
* @brief Get the number of iteration for geometric filtering
* @return max iteration
*/
int MatchingParams::maxIterationFiltering( void ) const
{
  return m_max_iteration_filtering ;
}

/**
* @brief Set the number of iteration for geometric filtering
* @return nb_iter New number of iteration
*/
void MatchingParams::setMaxIterationFiltering( const int nb_iter )
{
  m_max_iteration_filtering = nb_iter ;
}


/**
* @brief Get matching method
* @return matching method
*/
MatchingMethod MatchingParams::method( void ) const
{
  return m_method ;
}


/**
* @brief Set matching method
* @param mtd the matching method to use
*/
void MatchingParams::setMethod( const MatchingMethod & mtd )
{
  if( m_method != mtd )
  {
    m_method = mtd ;
    buildMatcher() ;
  }
}

/**
* @brief Distance ratio used to select good features
* @return distance ratio
*/
float MatchingParams::distanceRatio( void ) const
{
  return m_ratio ;
}

/**
* @brief Compute distance ratio
* @param dist distance ratio to compute
*/
void MatchingParams::setDistanceRatio( const float dist )
{
  if( m_ratio != dist )
  {
    m_ratio = dist ;
    buildMatcher() ;
  }
}

void MatchingParams::buildMatcher( void )
{
  m_matcher = nullptr ;
  switch( m_method )
  {
    case MATCHING_METHOD_BRUTEFORCE_L2 :
    {
      m_matcher = std::make_shared<openMVG::matching_image_collection::Matcher_Regions>( m_ratio , openMVG::matching::BRUTE_FORCE_L2 ) ;
      break ;
    }
    case MATCHING_METHOD_ANN_L2 :
    {
      m_matcher = std::make_shared<openMVG::matching_image_collection::Matcher_Regions>( m_ratio , openMVG::matching::ANN_L2 ) ;
      break ;
    }
    case MATCHING_METHOD_CASCADE_HASHING_L2 :
    {
      m_matcher = std::make_shared<openMVG::matching_image_collection::Matcher_Regions>( m_ratio , openMVG::matching::CASCADE_HASHING_L2 ) ;
      break ;
    }
    case MATCHING_METHOD_FAST_CASCADE_HASHING_L2 :
    {
      m_matcher = std::make_shared<openMVG::matching_image_collection::Cascade_Hashing_Matcher_Regions>( m_ratio ) ;
      break ;
    }
    case MATCHING_METHOD_BRUTE_FORCE_HAMMING :
    {
      m_matcher = std::make_shared<openMVG::matching_image_collection::Matcher_Regions>( m_ratio , openMVG::matching::BRUTE_FORCE_HAMMING ) ;
      break ;
    }
  }
}


/**
* @brief Get matcher with the current parameters
* @return current matcher
*/
std::shared_ptr<openMVG::matching_image_collection::Matcher> MatchingParams::matcher( void ) const
{
  return m_matcher ;
}


} // namespace openMVG_gui