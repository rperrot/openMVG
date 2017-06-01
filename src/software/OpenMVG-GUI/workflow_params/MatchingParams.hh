#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_MATCHING_PARAMS_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_MATCHING_PARAMS_HH_

#include <memory>

namespace openMVG
{
namespace matching_image_collection
{
class Matcher ;
}
}

namespace openMVG_gui
{

/**
* @brief Indicate the type of geometry to compute
*/
enum MatchingGeometryType
{
  MATCHING_GEOMETRY_TYPE_FUNDAMENTAL ,
  MATCHING_GEOMETRY_TYPE_ESSENTIAL ,
  MATCHING_GEOMETRY_TYPE_HOMOGRAPHY
} ;

/**
* @brief Get matching method
*/
enum MatchingMethod
{
  MATCHING_METHOD_BRUTEFORCE_L2 ,
  MATCHING_METHOD_ANN_L2 ,
  MATCHING_METHOD_CASCADE_HASHING_L2 ,
  MATCHING_METHOD_FAST_CASCADE_HASHING_L2 ,
  MATCHING_METHOD_BRUTE_FORCE_HAMMING
} ;

/**
* @brief class holding the matching params
*/
class MatchingParams
{
  public:

    /**
    * @brief Ctr
    * @note default parameters for matching sift and preparing incremental SfM
    */
    MatchingParams( const MatchingMethod & method = MATCHING_METHOD_FAST_CASCADE_HASHING_L2 ,
                    const float distRatio = 0.8 ,
                    const MatchingGeometryType & geom = MATCHING_GEOMETRY_TYPE_FUNDAMENTAL ,
                    const int max_iter_geom_filtering = 2048 ) ;
    
    /**
    * @brief Copy ctr 
    * @param src Source 
    */
    MatchingParams( const MatchingParams & src ) ;

    /**
    * @brief Move ctr 
    * @param src Source 
    */
    MatchingParams( MatchingParams && src) = default ; 

    /**
    * @brief Assignment operator 
    * @param src Source 
    * @return self after assignment 
    */
    MatchingParams & operator=( const MatchingParams & src ) ; 

    /**
    * @brief Move Assignment operator 
    * @param src Source 
    * @return self after assignment 
    */
    MatchingParams & operator=( MatchingParams && src ) ; 


    /**
    * @brief Get geometric model used for matching
    * @return geometric model
    */
    MatchingGeometryType geometricModel() const ;

    /**
    * @brief Set geometric model for matching
    * @param geom the geometric model to use
    */
    void setGeometricModel( const MatchingGeometryType & geom ) ;

    /**
    * @brief Get the number of iteration for geometric filtering
    * @return max iteration
    */
    int maxIterationFiltering( void ) const ;

    /**
    * @brief Set the number of iteration for geometric filtering
    * @return nb_iter New number of iteration
    */
    void setMaxIterationFiltering( const int nb_iter ) ;

    /**
    * @brief Get matching method
    * @return matching method
    */
    MatchingMethod method( void ) const ;


    /**
    * @brief Set matching method
    * @param mtd the matching method to use
    */
    void setMethod( const MatchingMethod & mtd ) ;

    /**
    * @brief Distance ratio used to select good features
    * @return distance ratio
    */
    float distanceRatio( void ) const ;

    /**
    * @brief Compute distance ratio
    * @param dist distance ratio to compute
    */
    void setDistanceRatio( const float dist ) ;

    /**
    * @brief Get matcher with the current parameters
    * @return current matcher
    */
    std::shared_ptr<openMVG::matching_image_collection::Matcher> matcher( void ) const ;

    template< class Archive >
    void load( Archive & ar ) ;

    template <class Archive >
    void save( Archive & ar ) const ;

  private:

    void buildMatcher( void ) ;

    // Geometric filtering
    MatchingGeometryType m_geometry ;
    int m_max_iteration_filtering ;

    // Blind matching
    MatchingMethod m_method ;
    float m_ratio ;

    std::shared_ptr<openMVG::matching_image_collection::Matcher> m_matcher ;
} ;

template< class Archive >
void MatchingParams::load( Archive & ar )
{
  ar( m_geometry ) ;
  ar( m_max_iteration_filtering ) ;
  ar( m_method ) ;
  ar( m_ratio ) ;

  buildMatcher() ;
}

template <class Archive >
void MatchingParams::save( Archive & ar ) const
{
  ar( m_geometry ) ;
  ar( m_max_iteration_filtering ) ;
  ar( m_method ) ;
  ar( m_ratio ) ;
}

} // openMVG_gui

#endif