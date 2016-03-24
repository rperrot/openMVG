// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_GRAPH_GRAPH_SPANNING_TREE_HPP_
#define _OPENMVG_GRAPH_GRAPH_SPANNING_TREE_HPP_

#include <map>
#include <utility>

#include "openMVG/tracks/union_find.hpp"
#include "openMVG/graph/pairing_heap.hpp"


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
  std::map< const node_type * , int > representant_id ; // Map from nodes to integers
  {
    int node_id = 0 ;
    for( auto it = nodes.begin() ; it != nodes.end() ; ++it, ++node_id )
    {
      representant_id[ *it ] = node_id ;
    }
  }

  size_t nb_tree = nodes.size() ;

  // Build set of edges, classified by increasing weigth
  PairingHeap<edge_data_type, edge_type*> e_queue( g.NbEdge() ) ;
  typedef typename PairingHeap<edge_data_type, edge_type*>::node_type queue_node_type ;

  std::map< edge_type *, bool> already_added_edge ; // Ensure edge is added only once

  for( auto it = nodes.begin() ; it < nodes.end() ; ++it )
  {
    auto & neighs = ( *it )->Neighbors() ;
    for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
    {
      if( ! already_added_edge.count( *it_neigh ) )
      {
        e_queue.Insert( ( *it_neigh )->Data() , *it_neigh ) ;
        already_added_edge[ *it_neigh ] = true ;
      }
    }
  }

  std::vector< const edge_type * > output_edge ;
  // Now compute minimum spanning tree
  while( nb_tree > 1 && ! e_queue.Empty() )
  {
    queue_node_type * min_edge_node = e_queue.FindMin() ;
    e_queue.DeleteMin() ;
    const edge_type * cur_edge = e_queue.GetData( min_edge_node ) ;

    const node_type * start_node = cur_edge->Source() ;
    const node_type * end_node = cur_edge->Destination() ;

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
  std::map< const node_type * , node_type *> map_node ;
  for( auto it = nodes.begin() ; it != nodes.end() ; ++it )
  {
    map_node[ *it ] = res.AddNode( ( *it )->Data() ) ;
  }

  // Build the edge
  for( auto it = output_edge.begin() ; it != output_edge.end() ; ++it )
  {
    const edge_type * cur_edge = *it;

    const node_type * start_node = cur_edge->Source() ;
    const node_type * end_node = cur_edge->Destination() ;

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
  std::map< const node_type * , int > representant_id ; // Map from nodes to integers
  {
    int node_id = 0 ;
    for( auto it = nodes.begin() ; it != nodes.end() ; ++it, ++node_id )
    {
      representant_id[ *it ] = node_id ;
    }
  }

  size_t nb_tree = nodes.size() ;

  // Build set of edges, classified by increasing weigth
  PairingHeap<WeightType, edge_type*> e_queue( edge_w.size() ) ;
  typedef typename PairingHeap<WeightType, edge_type*>::node_type queue_node_type ;

  std::map< const edge_type *, bool> already_added_edge ; // Ensure edge is added only once


  for( auto it = nodes.begin() ; it < nodes.end() ; ++it )
  {
    auto & neighs = ( *it )->Neighbors() ;
    for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
    {
      // Dont duplicate edges
      if( ! already_added_edge.count( *it_neigh ) )
      {
        auto it_w = edge_w.find( *it_neigh ) ;

        // Only add edge if it has a weight associated with
        if( it_w != edge_w.end() )
        {
          e_queue.Insert( it_w->second , *it_neigh ) ;
          already_added_edge[ *it_neigh ] = true ;
        }
      }
    }
  }

  std::vector< const edge_type * > output_edge ;
  // Now compute minimum spanning tree
  while( nb_tree > 1 && ! e_queue.Empty() )
  {
    queue_node_type * min_edge_node = e_queue.FindMin() ;
    e_queue.DeleteMin() ;
    const edge_type * cur_edge = e_queue.GetData( min_edge_node ) ;

    const node_type * start_node = cur_edge->Source() ;
    const node_type * end_node = cur_edge->Destination() ;

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
  std::map< const node_type * , node_type *> map_node ;
  for( auto it = nodes.begin() ; it != nodes.end() ; ++it )
  {
    map_node[ *it ] = res.AddNode( ( *it )->Data() ) ;
  }

  // Build the edge
  for( auto it = output_edge.begin() ; it != output_edge.end() ; ++it )
  {
    const edge_type * cur_edge = *it;

    const node_type * start_node = cur_edge->Source() ;
    const node_type * end_node = cur_edge->Destination() ;

    res.AddEdge( map_node[ start_node ] , map_node[ end_node ] , cur_edge->Data() ) ;
  }

  return res ;
}
}

}


#endif