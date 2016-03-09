// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_GRAPH_GRAPH_TRAVERSAL_HPP_
#define _OPENMVG_GRAPH_GRAPH_TRAVERSAL_HPP_

#include <map>

namespace openMVG
{
namespace graph
{

template< typename Graph >
class GraphTraversalDFS
{
    typedef typename Graph::node_type node_type ;
    typedef typename Graph::edge_type edge_type ;

  public:

    /**
    * @brief Computes traversal
    */
    template< typename NodeFunctor>
    void NodeVisitorDFS( const Graph & g , const node_type * start_node , NodeFunctor & functor ) const ;

  private:
    template< typename NodeFunctor>
    void NodeVisitorDFS( const Graph & g , const node_type * start_node , NodeFunctor & functor , std::map< node_type * , bool > & visited ) const ;
} ;


template< typename Graph>
template< typename NodeFunctor>
void GraphTraversalDFS<Graph>::NodeVisitorDFS( const Graph & g , const GraphTraversalDFS<Graph>::node_type * start_node , NodeFunctor & functor ) const
{
  std::map< node_type * , bool > visited ;
  NodeVisitorDFS( g , start_node , functor , visited ) ;
}

template< typename Graph>
template< typename NodeFunctor>
void GraphTraversalDFS<Graph>::NodeVisitorDFS( const Graph & g ,
    const GraphTraversalDFS<Graph>::node_type * start_node ,
    NodeFunctor & functor ,
    std::map< node_type * , bool > & visited ) const
{
  visited[ start_node ] = true ;
  functor( start_node ) ;

  typedef typename node_type::adjacency_list_type adjacency_list_type ;

  adjacency_list_type & neigh = start_node->Neighbors() ;

  // For all edges in the adjacency list
  for( auto neigh_iter = neigh.begin() ; neigh_iter != neigh.end() ; ++neigh_iter )
  {
    // Get the corresponding node of this edge
    node_type * cur_node = neigh_iter->Opposite( start_node ) ;

    // If this not has not been visited, let's do it
    if( visited.find( cur_node ) == std::map< node_type * , bool>::end )
    {
      GraphTraversalDFS( g , cur_node , functor , visited ) ;
    }
  }
}


}
}

#endif