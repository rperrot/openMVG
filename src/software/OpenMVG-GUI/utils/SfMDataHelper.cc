#include "SfMDataHelper.hh"

#include "openMVG/sfm/sfm_data.hpp"

#include <algorithm>
#include <utility>

bool pairCompareFirst( const std::pair<int, double> & a , const std::pair<int, double> & b )
{
  return a.first < b.first ;
}

bool pairEqualFirst( const std::pair<int, double> & a , const std::pair<int, double> & b )
{
  return a.first == b.first ;
}

bool pairIntIntSorted( const std::pair<int, int> & a , const std::pair<int, int> & b )
{
  return a.first < b.first ||
         ( ( a.first == b.first ) && ( a.second < b.second ) ) ;
}



namespace openMVG_gui
{
/**
* @brief Ctr
* @param sfm_data the structure on which information will be extracted
*/
SfMDataHelper::SfMDataHelper( std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data )
  : m_sfm_data( sfm_data )
{

}

/**
 * @brief Get all views id that are linked to a given one
 * @return vector of all linked views
 */
std::vector< int > SfMDataHelper::linkedViews( const int id ) const
{
  if( ! m_sfm_data )
  {
    return {} ;
  }

  std::vector<openMVG::IndexT> connected_views ;
  std::vector<openMVG::IndexT> views_for_this_track ;
  for ( const auto & it_landmark : m_sfm_data->GetLandmarks() )
  {
    const openMVG::sfm::Observations & obs = it_landmark.second.obs;
    bool track_interesting = false ;
    views_for_this_track.clear() ;
    for ( const auto & it_obs : obs )
    {
      views_for_this_track.emplace_back( it_obs.first ) ;
      if( id == it_obs.first )
      {
        track_interesting = true ;
      }
    }
    if( track_interesting )
    {
      for( auto it_view : views_for_this_track )
      {
        if( it_view != id )
        {
          connected_views.emplace_back( it_view );
        }
      }
    }
  }
  std::sort( connected_views.begin() , connected_views.end() ) ;
  const auto end = std::unique( connected_views.begin() , connected_views.end() ) ;

  return std::vector<int>( connected_views.begin() , end ) ;
}

/**
 * @brief Get all views id that are linked to a given one with their corresponding strength
 * @return vector of all linked views with their corresponding strength
 * @note Strength is normalized between 0 and 1
 * @note Strength = 1 is the maximum (ie: best link with the given view wrt the input id)
 */
std::vector< std::pair<int, double> > SfMDataHelper::linkedViewsWithStrength( const int id ) const
{
  if( ! m_sfm_data )
  {
    return {} ;
  }

  std::vector<openMVG::IndexT> connected_views ;
  std::vector<openMVG::IndexT> views_for_this_track ;

  // For each view, get the number of pair occurence wrt to the input id
  // We use postal sort, so we suppose the highest Id of the scene wont be too large
  //
  std::vector<uint32_t> nbOccurrence;
  {
    auto views = m_sfm_data->GetViews() ;
    openMVG::IndexT max_view_id = 0 ;
    for( auto it_view : views )
    {
      max_view_id = std::max( max_view_id , it_view.first ) ;
    }
    nbOccurrence.resize( max_view_id , static_cast<uint32_t>( 0 ) ) ;
  }

  for ( const auto & it_landmark : m_sfm_data->GetLandmarks() )
  {
    const openMVG::sfm::Observations & obs = it_landmark.second.obs;
    bool track_interesting = false ;
    views_for_this_track.clear() ;
    for ( const auto & it_obs : obs )
    {
      views_for_this_track.emplace_back( it_obs.first ) ;
      if( id == it_obs.first )
      {
        track_interesting = true ;
      }
    }
    if( track_interesting )
    {
      for( auto it_view : views_for_this_track )
      {
        if( it_view != id )
        {
          ++nbOccurrence[ it_view ] ;
          connected_views.emplace_back( it_view );
        }
      }
    }
  }
  std::sort( connected_views.begin() , connected_views.end() ) ;
  const auto end = std::unique( connected_views.begin() , connected_views.end() ) ;

  const uint32_t max_occurrences = *std::max_element( nbOccurrence.begin() , nbOccurrence.end() ) ;

  std::vector< std::pair<int, double> > result( std::distance( connected_views.begin() , end ) ) ;

  size_t res_id = 0 ;
  for( auto it_view = connected_views.begin() ; it_view != end ; ++it_view , ++res_id )
  {
    result[ res_id ].first  = *it_view ;
    result[ res_id ].second = static_cast<double>( nbOccurrence[ *it_view ] ) / max_occurrences ;
  }

  return result ;
}

/**
 * @brief Get all view pairs in the scene
 * @return vector of all view pairs
 */
std::vector< std::pair<int, int> > SfMDataHelper::allViewPairs( void ) const
{
  if( ! m_sfm_data )
  {
    return {} ;
  }

  std::vector<std::pair<int, int>> all_pairs ;
  std::vector<int> cur_track_views ;
  for ( const auto & it_landmark : m_sfm_data->GetLandmarks() )
  {
    const openMVG::sfm::Observations & obs = it_landmark.second.obs;
    bool track_interesting = false ;
    cur_track_views.clear();

    for ( const auto & it_obs : obs )
    {
      cur_track_views.emplace_back( static_cast<int>( it_obs.first ) ) ;
    }

    for( int i = 0 ; i < cur_track_views.size() ; ++i )
    {
      for( int j = i + 1 ; j < cur_track_views.size() ; ++j )
      {
        all_pairs.emplace_back( std::make_pair( cur_track_views[i] , cur_track_views[j] ) ) ;
      }
    }
  }
  std::sort( all_pairs.begin() , all_pairs.end() , pairIntIntSorted ) ;
  const auto end = std::unique( all_pairs.begin() , all_pairs.end() ) ;

  return std::vector<std::pair<int, int>>( all_pairs.begin() , end ) ;
}

/**
 * @brief Get all view pairs in the scene with their corresponding strength
 * @return vector of all view pairs with their corresponding strength
 * @note Strength is normalized between 0 and 1
 * @note Strength = 1 is the maximum (ie: best link among all pairs)
 */
std::vector< std::tuple< int , int , double > > SfMDataHelper::allViewPairsWithStrength( void ) const
{
  if( ! m_sfm_data )
  {
    return {} ;
  }

}

} // namespace openMVG_gui