// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_GRAPH_GRAPH_CONNECTED_COMPONENT_HPP_
#define _OPENMVG_GRAPH_GRAPH_CONNECTED_COMPONENT_HPP_

#include <map>

namespace openMVG
{
namespace graph
{

template< typename GraphType >
struct GraphConnectedComponents
{
  public:

    typedef typename GraphType::node_type node_type ;
    typedef typename GraphType::edge_type edge_type ;

    std::vector< node_type * > GetCC( const GraphType & g ) const ;

    /**
    * @brief Get number of node in each connected components
    */
    std::vector< size_t > GetCCNodeCount( const GraphType & g ) const ;

    /**
    * @brief Get largest CC
    * @return a representant of the largest CC
    */
    node_type * GetLargestCC( const GraphType & g ) const ;

    /**
    * @brief Get a copy of each connected components
    * @return a copy of each CC
    */
    std::vector< GraphType > GetCCCopy( const GraphType & g ) const ;


  private:

    void GetCC( const GraphType & g , node_type * from_node , std::map<node_type*, bool> & used ) const ;
    size_t GetCCNodeCount( const GraphType & g , node_type * from_node , std::map<node_type*, bool> & used ) const ;
} ;

template< typename GraphType >
std::vector< typename GraphConnectedComponents<GraphType>::node_type * > GraphConnectedComponents<GraphType>::GetCC( const GraphType & g ) const
{
  const std::vector< node_type * > & nodes = g.Nodes() ;
  std::vector< node_type * > res ;

  int nb_left = nodes.size() ;

  if( nodes.size() > 0 )
  {
    std::map<node_type *, bool> used ;

    while( nb_left > 0 )
    {
      // Look for an unused node and start from it
      int id = 0 ;
      while( used.count( nodes[id] ) != 0 )
      {
        ++id ;
      }

      // Computes CC from this node
      res.push_back( nodes[id] ) ;
      GetCC( g , nodes[id] , used ) ;

      nb_left = nodes.size() - used.size() ;
    }
  }

  return res ;
}

template< typename GraphType >
void GraphConnectedComponents<GraphType>::GetCC( const GraphType & g ,
    typename GraphConnectedComponents<GraphType>::node_type * from_node ,
    std::map<typename GraphConnectedComponents<GraphType>::node_type*, bool> & used ) const
{
  used[ from_node ] = true ;

  auto & neighs = from_node->Neighbors() ;

  // Traverse all neighbors if they were not marker before
  for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
  {
    const edge_type * cur_edge = *it_neigh ;
    node_type * opp = cur_edge->Opposite( from_node ) ;

    if( ! used.count( opp ) )
    {
      GetCC( g , opp , used ) ;
    }
  }
}

template< typename GraphType >
size_t GraphConnectedComponents<GraphType>::GetCCNodeCount( const GraphType & g , typename GraphConnectedComponents<GraphType>::node_type * from_node , std::map<typename GraphConnectedComponents<GraphType>::node_type*, bool> & used ) const
{
  used[ from_node ] = true ;

  size_t cur_size = 1 ;

  auto & neighs = from_node->Neighbors() ;

  for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
  {
    const edge_type * cur_edge = *it_neigh ;
    node_type * opp = cur_edge->Opposite( from_node ) ;

    if( ! used.count( opp ) )
    {
      cur_size += GetCCNodeCount( g , opp , used ) ;
    }
  }
  return cur_size ;
}


/**
* @brief Get number of node in each connected components
*/
template< typename GraphType >
std::vector< size_t > GraphConnectedComponents<GraphType>::GetCCNodeCount( const GraphType & g ) const
{
  const std::vector< node_type * > & nodes = g.Nodes() ;
  std::vector< size_t > res ;

  int nb_left = nodes.size() ;

  if( nodes.size() > 0 )
  {
    std::map<node_type *, bool> used ;

    // Loop until some nodes haven't been found
    while( nb_left > 0 )
    {
      // Look for an unused node and start from it
      int id = 0 ;
      while( used.count( nodes[id] ) != 0 )
      {
        ++id ;
      }

      // Computes CC from this node and returns it's size
      res.push_back( GetCCNodeCount( g , nodes[id] , used ) ) ;

      // Update nb unseen nodes
      nb_left = nodes.size() - used.size() ;
    }
  }

  return res ;
}

/**
* @brief Get largest CC
* @return a representant of the largest CC
*/
template< typename GraphType >
typename GraphConnectedComponents<GraphType>::node_type * GraphConnectedComponents<GraphType>::GetLargestCC( const GraphType & g ) const
{
  const std::vector< node_type * > & nodes = g.Nodes() ;
  node_type * res = nullptr ;
  size_t cur_max_cc_size = 0 ;


  int nb_left = nodes.size() ;

  if( nodes.size() > 0 )
  {
    std::map<node_type *, bool> used ;

    // Loop until some nodes haven't been found
    while( nb_left > 0 )
    {
      // Look for an unused node and start from it
      int id = 0 ;
      while( used.count( nodes[id] ) != 0 )
      {
        ++id ;
      }

      // Computes CC from this node and returns it's size
      const size_t cc_size = GetCCNodeCount( g , nodes[id] , used ) ;

      // Check if it's a bigger CC
      if( cc_size > cur_max_cc_size )
      {
        cur_max_cc_size = cc_size ;
        res = nodes[ id ] ;
      }

      // Update nb unseen nodes
      nb_left = nodes.size() - used.size() ;
    }
  }

  return res ;

}

/**
* @brief Get a copy of each connected components
* @return a copy of each CC
*/
template< typename GraphType >
std::vector< GraphType > GraphConnectedComponents<GraphType>::GetCCCopy( const GraphType & g ) const
{

}



}
}

#endif