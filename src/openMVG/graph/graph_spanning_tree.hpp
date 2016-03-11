#ifndef _OPENMVG_GRAPH_GRAPH_SPANNING_TREE_HPP_
#define _OPENMVG_GRAPH_GRAPH_SPANNING_TREE_HPP_

#include <map>
#include <queue>
#include <utility>

#include "openMVG/tracks/union_find.hpp"


namespace openMVG
{
namespace graph
{
template< typename GraphType>
struct GraphSpanningTree
{
    typedef typename GraphType::node_type node_type ;
    typedef typename GraphType::edge_type edge_type ;
    typedef typename edge_type::edge_data_type edge_data_type ;

  public:

    /**
    * @brief Get a graph representing the minimum spanning tree of the input tree
    * @param g Input tree
    * @return spanning tree (deep copy) of g
    * @note We use the edge_data as spanning weight
    * @note Input graph must be a connected graph
    */
    GraphType MST( const GraphType & g ) ;

    /**
    * @brief Get a graph representing the minimum spanning tree of the input tree using some weights
    * @param g Input tree
    * @param edge_w Weights to be used for the edges
    * @return Spanning tree (deep copy) of the graph g
    * @note If no weight is found for a specified edge, we ignore the edge
    * @note Input graph must be a connected graph
    */
    template< typename WeightType>
    GraphType MST( const GraphType & g , const std::map<edge_type*, WeightType> & edge_w ) const ;

  private:
} ;


/**
* @brief Functor used to sort edges given their internal data
*/
template<typename edge_type>
struct edge_comparator_greater
{
  /**
  * @retval true if p1 > p2
  * @retval false if p1 <= p2
  */
  bool operator()( const edge_type & p1 , const edge_type & p2 ) const
  {
    return p1->Data() > p2->Data() ;
  }
} ;

template<typename edge_type, typename WeightType>
struct edge_map_comparator_greater
{
    edge_map_comparator_greater( const std::map<edge_type, WeightType> & w )
      : m_map_w( w )
    {

    }

    bool operator()( const edge_type & p1 , const edge_type & p2 ) const
    {
      return m_map_w.at( const_cast<edge_type&>( p1 ) ) > m_map_w.at( p2 ) ;
    }

  private:

    const std::map<edge_type, WeightType> m_map_w ;
} ;

/**
* @brief Get a graph representing the minimum spanning tree of the input tree
* @param g Input tree
* @return spanning tree (deep copy) of g
* @note We use the edge_data as spanning weight
*/
template< typename GraphType>
GraphType GraphSpanningTree<GraphType>::MST( const GraphType & g )
{
  const std::vector< node_type * > & nodes = g.Nodes() ;

  // Build initial forest (each nodes is it's own representant)
  UnionFind forest_representant;
  forest_representant.InitSets( nodes.size() ) ;
  std::map< node_type * , int > representant_id ; // Map from nodes to integers
  {
    int node_id = 0 ;
    for( auto it = nodes.begin() ; it != nodes.end() ; ++it, ++node_id )
    {
      representant_id[ *it ] = node_id ;
    }
  }

  size_t nb_tree = nodes.size() ;

  // Build set of edges, classified by increasing weigth
  // TODO: use a specialized queue (pair heap)
  std::priority_queue<edge_type*, std::vector<edge_type*>, edge_comparator_greater<edge_type*>> e_queue ;
  std::map< edge_type *, bool> already_added_edge ; // Ensure edge is added only once

  for( auto it = nodes.begin() ; it < nodes.end() ; ++it )
  {
    auto & neighs = ( *it )->Neighbors() ;
    for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
    {
      if( ! already_added_edge.count( *it_neigh ) )
      {
        e_queue.push( *it_neigh ) ;
        already_added_edge[ *it_neigh ] = true ;
      }
    }
  }

  std::vector< edge_type * > output_edge ;
  // Now compute minimum spanning tree
  while( nb_tree > 1 && ! e_queue.empty() )
  {
    edge_type * cur_edge = e_queue.top() ;
    e_queue.pop() ;

    node_type * start_node = cur_edge->Source() ;
    node_type * end_node = cur_edge->Destination() ;

    const int id_start = representant_id[ start_node ] ;
    const int id_end = representant_id[ end_node ] ;

    // Check if edge does not rely two nodes in the same tree
    const int rep_start_id = forest_representant.Find( id_start ) ;
    const int rep_end_id = forest_representant.Find( id_end ) ;

    if( rep_start_id != rep_end_id )
    {
      // Make the tree in the same forest
      forest_representant.Union( id_start , id_end ) ;

      // Update the set of edges of the tree
      output_edge.push_back( cur_edge ) ;

      nb_tree-- ;
    }
  }

  // Here, every representant should point to the same node, if not, that's because we have not a connected graph
  GraphType res ;
  // Build the node
  std::map< node_type * , node_type *> map_node ;
  for( auto it = nodes.begin() ; it != nodes.end() ; ++it )
  {
    map_node[ *it ] = res.AddNode( ( *it )->Data() ) ;
  }

  // Build the edge
  for( auto it = output_edge.begin() ; it != output_edge.end() ; ++it )
  {
    edge_type * cur_edge = *it;

    node_type * start_node = cur_edge->Source() ;
    node_type * end_node = cur_edge->Destination() ;

    res.AddEdge( map_node[ start_node ] , map_node[ end_node ] , cur_edge->Data() ) ;
  }

  return res ;
}

/**
* @brief Get a graph representing the minimum spanning tree of the input tree using some weights
* @param g Input tree
* @param edge_w Weights to be used for the edges
* @return Spanning tree (deep copy) of the graph g
* @note If no weight is found for a specified edge, we ignore the edge
*/
template< typename GraphType>
template< typename WeightType>
GraphType GraphSpanningTree<GraphType>::MST( const GraphType & g ,
    const std::map<typename GraphSpanningTree<GraphType>::edge_type*, WeightType> & edge_w ) const
{
  const std::vector< node_type * > & nodes = g.Nodes() ;

  // Build initial forest (each nodes is it's own representant)
  UnionFind forest_representant;
  forest_representant.InitSets( nodes.size() ) ;
  std::map< node_type * , int > representant_id ; // Map from nodes to integers
  {
    int node_id = 0 ;
    for( auto it = nodes.begin() ; it != nodes.end() ; ++it, ++node_id )
    {
      representant_id[ *it ] = node_id ;
    }
  }

  size_t nb_tree = nodes.size() ;

  // Build set of edges, classified by increasing weigth
  // TODO: use a specialized queue (pair heap)
  edge_map_comparator_greater<edge_type*, WeightType> comp( edge_w ) ;
  std::priority_queue<edge_type*, std::vector<edge_type*>, edge_map_comparator_greater<edge_type*, WeightType>> e_queue( comp ) ;
  std::map< edge_type *, bool> already_added_edge ; // Ensure edge is added only once

  for( auto it = nodes.begin() ; it < nodes.end() ; ++it )
  {
    auto & neighs = ( *it )->Neighbors() ;
    for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
    {
      // Dont duplicate edges
      if( ! already_added_edge.count( *it_neigh ) )
      {
        // Only add edge if it has a weight associated with
        if( edge_w.count( *it_neigh ) )
        {
          e_queue.push( *it_neigh ) ;
          already_added_edge[ *it_neigh ] = true ;
        }
      }
    }
  }

  std::vector< edge_type * > output_edge ;
  // Now compute minimum spanning tree
  while( nb_tree > 1 && ! e_queue.empty() )
  {
    edge_type * cur_edge = e_queue.top() ;
    e_queue.pop() ;

    node_type * start_node = cur_edge->Source() ;
    node_type * end_node = cur_edge->Destination() ;

    const int id_start = representant_id[ start_node ] ;
    const int id_end = representant_id[ end_node ] ;

    // Check if edge does not rely two nodes in the same tree
    const int rep_start_id = forest_representant.Find( id_start ) ;
    const int rep_end_id = forest_representant.Find( id_end ) ;

    if( rep_start_id != rep_end_id )
    {
      // Make the tree in the same forest
      forest_representant.Union( id_start , id_end ) ;

      // Update the set of edges of the tree
      output_edge.push_back( cur_edge ) ;

      nb_tree-- ;
    }
  }

  // Here, every representant should point to the same node, if not, that's because we have not a connected graph
  GraphType res ;
  // Build the node
  std::map< node_type * , node_type *> map_node ;
  for( auto it = nodes.begin() ; it != nodes.end() ; ++it )
  {
    map_node[ *it ] = res.AddNode( ( *it )->Data() ) ;
  }

  // Build the edge
  for( auto it = output_edge.begin() ; it != output_edge.end() ; ++it )
  {
    edge_type * cur_edge = *it;

    node_type * start_node = cur_edge->Source() ;
    node_type * end_node = cur_edge->Destination() ;

    res.AddEdge( map_node[ start_node ] , map_node[ end_node ] , cur_edge->Data() ) ;
  }

  return res ;
}
}

}


#endif