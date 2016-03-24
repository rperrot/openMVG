#ifndef _OPENMVG_GRAPH_GRAPH_PATH_ALGORITHM_HPP_
#define _OPENMVG_GRAPH_GRAPH_PATH_ALGORITHM_HPP_

#include <unordered_map>
#include <limits>
#include <deque>

#include "openMVG/graph/pairing_heap.hpp"
#include "openMVG/numeric/safe_arithmetic.hpp"

namespace openMVG
{
namespace graph
{

/**
* @brief class containing various methods for shortest path finding
*/
template< typename GraphType>
class GraphShortestPath
{
  public:

    typedef typename GraphType::node_type node_type ;
    typedef typename GraphType::edge_type edge_type ;

    /**
    * @brief Computes The shortest path between two nodes in the graph using Dijkstra algorithm
    * @param g Input graph
    * @param from_node Input node
    * @param to_node Output node
    * @return List of edges from from_node to edge_node
    * @note In this function, edge_data is used as path weight
    * @note As defined in the Dijkstra algorithm edge_weights must be positive
    */
    std::vector< edge_type * > ShortestPathDijkstra( const GraphType & g , const node_type * from_node , const node_type * to_node ) const ;


    /**
    * @brief Computes The shortest path between two nodes in the graph using Dijkstra algorithm
    * @param g Input graph
    * @param from_node Input node
    * @param to_node Output node
    * @param[in,out] has_negative_cyle if not null, tells if graph has negative cycle. In that case, no path is returned
    * @return List of edges from from_node to edge_node
    * @note In this function, edge_data is used as path weight
    * @note As opposed to the Dijkstra algorithm negative weights are allowed
    */
    std::vector< edge_type * > ShortestPathBellmanFord( const GraphType & g , const node_type * from_node , const node_type * to_node , bool * has_negative_cyle = nullptr ) const ;

  private:
};


/**
* @brief Computes The shortest path between two nodes in the graph using Dijkstra algorithm
* @param g Input graph
* @param from_node Input node
* @param to_node Output node
* @note In this function, edge_data is used as path weight
* @note As defined in the Dijkstra algorithm edge_weights must be positive
*/
template< typename GraphType>
std::vector< typename GraphShortestPath<GraphType>::edge_type * > GraphShortestPath<GraphType>::ShortestPathDijkstra( const GraphType & g ,
    const typename GraphShortestPath<GraphType>::node_type * from_node ,
    const typename GraphShortestPath<GraphType>::node_type * to_node ) const
{
  // Store the best edge to get to a specified node
  std::unordered_map< const node_type * , edge_type * > best_from ;
  // List nodes allready processed
  std::unordered_map< const node_type * , bool > proceed ;

  const std::vector< node_type * > & nodes = g.Nodes() ;

  // Store minimum distance to a specified node
  typedef typename edge_type::edge_data_type w_type ;
  PairingHeap< typename edge_type::edge_data_type , const node_type * > min_dist( nodes.size() ) ;
  typedef typename PairingHeap< typename edge_type::edge_data_type , const node_type * >::node_type q_node_type ;
  // Get relation between a graph node and it's node in the priority queue
  std::unordered_map< const node_type * , q_node_type * > node_map ;

  // Append the first node
  q_node_type * corresp = min_dist.Insert( w_type( 0 ) , from_node ) ;
  node_map[ nodes[ 0 ] ] = corresp ;
  best_from[ nodes[ 0 ] ] = nullptr ;

  // Append all the other nodes
  for( size_t i = 0 ; i < nodes.size() ; ++i )
  {
    if( nodes[i] != from_node )
    {
      // Add element to the priority queue
      q_node_type * corresp = min_dist.Insert( std::numeric_limits<w_type>::max() , nodes[ i ] ) ;
      // Keep relation from this graph node to it's node in the priority queue
      node_map[ nodes[i] ] = corresp ;
    }
  }

  // Find minimum path
  while( ! min_dist.Empty() )
  {
    q_node_type * cur_min_node = min_dist.FindMin( ) ;
    min_dist.DeleteMin() ;

    const w_type cur_min_dist = min_dist.GetKey( cur_min_node ) ;
    const node_type * cur_node = min_dist.GetData( cur_min_node ) ;

    auto & neigh = cur_node->Neighbors() ;

    proceed[ cur_node ] = true ;

    for( auto it = neigh.begin() ; it != neigh.end() ; ++it )
    {
      const node_type * opp = ( *it )->Opposite( cur_node ) ;

      // Process only nodes that are still in the queue
      if( ! proceed.count( opp ) )
      {
        // Compute distance from from_node to opp using this edge
        const w_type dist_opp_with_it = cur_min_dist + ( *it )->Data() ;

        q_node_type * opp_node_in_pqueue = node_map[ opp ] ;

        // Does this path lower the distance ?
        if( dist_opp_with_it < min_dist.GetKey( opp_node_in_pqueue ) )
        {
          // If so, update key value and preceding node
          min_dist.DecreaseKey( opp_node_in_pqueue , dist_opp_with_it ) ;

          best_from[ opp ] = ( *it ) ;
        }
      }
    }
  }

  // At this stage we have all shortest distance from from_node to all the accessible nodes
  if( best_from.count( to_node ) == 0 )
  {
    // No path found from from_node to to_node
    std::vector< edge_type * > res ;
    return res ;
  }
  else
  {
    // Get edges list by following the reverse path
    std::deque< edge_type * > tmp ;
    node_type * cur_node = const_cast<node_type*>( to_node ) ;
    while( cur_node != from_node )
    {
      // Add the edge
      edge_type * edge = best_from[ cur_node ] ;
      tmp.push_front( edge ) ;

      // Find the preceding vertex
      cur_node = edge->Opposite( cur_node ) ;
    }

    std::vector< edge_type * > res( tmp.begin() , tmp.end() ) ;

    return res ;
  }
}

/**
* @brief Computes The shortest path between two nodes in the graph using Dijkstra algorithm
* @param g Input graph
* @param from_node Input node
* @param to_node Output node
* @param[in,out] has_negative_cyle if not null, tells if graph has negative cycle. In that case, no path is returned
* @return List of edges from from_node to edge_node
* @note In this function, edge_data is used as path weight
* @note As opposed to the Dijkstra algorithm negative weights are allowed
*/
template< typename GraphType>
std::vector< typename GraphShortestPath<GraphType>::edge_type * > GraphShortestPath<GraphType>::ShortestPathBellmanFord( const GraphType & g ,
    const typename GraphShortestPath<GraphType>::node_type * from_node ,
    const typename GraphShortestPath<GraphType>::node_type * to_node ,
    bool * has_negative_cycle ) const
{
  typedef typename edge_type::edge_data_type w_type ;

  // Minimum distance to get from from_node to this node
  std::unordered_map< const node_type * , w_type > min_distance ;
  // Best predecessor to go from from_node to this node
  std::unordered_map< const node_type * , edge_type * > best_from ;

  const std::vector< node_type * > nodes = g.Nodes() ;

  // Initialize the algorithm
  for( size_t i = 0 ; i < nodes.size() ; ++i )
  {
    // infinite distance
    min_distance[ nodes[ i ] ] = std::numeric_limits<w_type>::max() ;
    // No way to get to this node
    best_from[ nodes[ i ] ] = nullptr ;
  }
  // Just set from node
  min_distance[ from_node ] = w_type( 0 ) ;


  if( nodes.size() < 2 )
  {
    std::vector< edge_type * > res ;
    return res ;
  }

  // Relax edges
  for( size_t i = 0 ; i < nodes.size() - 1 ; ++i )
  {
    // For all edges
    for( size_t id_node = 0 ; id_node < nodes.size() ; ++id_node )
    {
      const node_type * cur_node = nodes[ id_node ] ;
      auto & neighs = cur_node->Neighbors() ;

      const w_type cur_min_dist = min_distance[ cur_node ] ;

      for( auto it = neighs.begin( ) ; it != neighs.end() ; ++it )
      {
        const node_type * opp = ( *it )->Opposite( cur_node ) ;
        const w_type new_dist = SafeAdd( min_distance[ cur_node ] , ( *it )->Data() ) ;
        if( new_dist < min_distance[ opp ] )
        {
          min_distance[ opp ] = new_dist ;
          best_from[ opp ] = ( *it ) ;
        }
      }
    }
  }


  // Test negative cycle
  bool cycle = false ;
  if( has_negative_cycle )
  {
    *has_negative_cycle = false ;
  }
  for( size_t id_node = 0 ; id_node < nodes.size() && ! cycle ; ++id_node )
  {
    const node_type * cur_node = nodes[ id_node ] ;
    auto & neighs = cur_node->Neighbors() ;
    for( auto it = neighs.begin( ) ; it != neighs.end() ; ++it )
    {
      const node_type * opp = ( *it )->Opposite( cur_node ) ;
      const w_type new_dist = SafeAdd( min_distance[ cur_node ], ( *it )->Data() ) ;
      if( new_dist < min_distance[ opp ] )
      {
        // A new best path is possible -> only possible if there's a (cyclic) negative path
        cycle = true ;
        if( has_negative_cycle )
        {
          *has_negative_cycle = true ;
        }
        break ;
      }
    }
  }

  /* Get path */
  if( best_from[ to_node ] == nullptr || cycle )
  {
    // No way to get to to_node
    std::vector< edge_type * > res ;
    return res ;
  }
  else
  {
    // Get edges list by following the reverse path
    std::deque< edge_type * > tmp ;
    node_type * cur_node = const_cast<node_type*>( to_node ) ;
    while( cur_node != from_node )
    {
      // Add the edge
      edge_type * edge = best_from[ cur_node ] ;
      tmp.push_front( edge ) ;

      // Find the preceding vertex
      cur_node = edge->Opposite( cur_node ) ;
    }

    std::vector< edge_type * > res( tmp.begin() , tmp.end() ) ;

    return res ;
  }

}


}
}

#endif