// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_GRAPH_GRAPH_CONTAINER_NODE_HPP_
#define _OPENMVG_GRAPH_GRAPH_CONTAINER_NODE_HPP_

#include "openMVG/graph/graph_utility.hpp"
#include "openMVG/graph/graph_container_edge.hpp"

#include <deque>
#include <algorithm>
#include <unordered_map>

namespace openMVG
{
namespace graph
{

/**
 * @brief Class containing Graph Node
 */
template< typename NodeData = NullData , typename EdgeData = NullData  >
class GraphNode
{
  public:

    typedef GraphNode<NodeData, EdgeData> type ;
    typedef GraphEdge<EdgeData, NodeData> edge_type ;
    typedef std::deque< edge_type * > adjacency_list_type ;

    /**
    * @brief Constructor
    */
    GraphNode( const NodeData & data = NodeData() ) ;

    /**
    * @brief Copy ctr
    */
    explicit GraphNode( const GraphNode<NodeData, EdgeData> & src ) ;

    /**
    * @brief Move ctr
    */
    GraphNode( GraphNode<NodeData, EdgeData> && src ) ;

    /**
    * @brief Assignment operator
    */
    GraphNode<NodeData, EdgeData> & operator=( const GraphNode<NodeData, EdgeData> & ) ;

    /**
    * @brief Get data stored in the graph
    */
    NodeData Data( void ) const ;

    /**
    * @brief Get list of direct neighors
    * @return List of edge that links this nodes to it's neighbors
    */
    const adjacency_list_type & Neighbors( void ) const ;

    /*
    * @brief Add a neighbor from an edge
    * @param edge edge to be added
    */
    void AddNeighbor( edge_type * edge ) ;

    /**
    * @brief Remove a neighbor from an edge
    */
    void RemoveNeighbor( edge_type * edge ) ;

    /**
    * @brief Get the degree of the node
    */
    size_t Degree( void ) const ;

    /**
    * @brief Has loop
    */
    bool HasLoop( void ) const ;

    /**
    * @brief Remove edges that links this node to it-self
    * @return Number of removed node
    */
    size_t RemoveLoops( void ) ;

    /**
    * @brief Indicate if the node has multiple edges linking to the same node
    */
    bool HasMultipleLinks( void ) const ;

  private:

    // Label of the node
    NodeData m_data ;

    // List of neighboring nodes
    adjacency_list_type m_adjacency_list ;

} ;

template< typename NodeData , typename EdgeData>
GraphNode<NodeData, EdgeData>::GraphNode( const NodeData & data )
  : m_data( data )
{

}


template< typename NodeData , typename EdgeData>
GraphNode<NodeData, EdgeData>::GraphNode( const GraphNode<NodeData, EdgeData> & src )
  : m_data( src.m_data ) ,
    m_adjacency_list( src.m_adjacency_list )
{

}

template< typename NodeData , typename EdgeData>
GraphNode<NodeData, EdgeData>::GraphNode( GraphNode<NodeData, EdgeData> && src )
  : m_data( std::move( src.m_data ) ) ,
    m_adjacency_list( std::move( src.m_adjacency_list ) )
{

}

template< typename NodeData , typename EdgeData>
GraphNode<NodeData, EdgeData> & GraphNode<NodeData, EdgeData>::operator=( const GraphNode<NodeData, EdgeData> & src )
{
  if( this != &src )
  {
    m_data = src.m_data ;
    m_adjacency_list = src.m_adjacency_list ;
  }
  return ( *this );
}

// Get data of the node
template< typename NodeData , typename EdgeData>
NodeData GraphNode<NodeData, EdgeData>::Data( void ) const
{
  return m_data ;
}

template< typename NodeData , typename EdgeData>
const typename GraphNode<NodeData, EdgeData>::adjacency_list_type & GraphNode<NodeData, EdgeData>::Neighbors( void ) const
{
  return m_adjacency_list ;
}

/*
 * @brief Add a neighbor from an edge
 */
template< typename NodeData , typename EdgeData>
void GraphNode<NodeData, EdgeData>::AddNeighbor( edge_type * edge )
{
  m_adjacency_list.push_back( edge ) ;
}

/**
 * @brief Remove a neighbor from an edge
 */
template< typename NodeData , typename EdgeData>
void GraphNode<NodeData, EdgeData>::RemoveNeighbor( edge_type * edge )
{
  auto it = find( m_adjacency_list.begin() , m_adjacency_list.end() , edge ) ;
  if( it != m_adjacency_list.end() )
  {
    m_adjacency_list.erase( it ) ;
  }
}

/**
* @brief Get the degree of the node
*/
template< typename NodeData , typename EdgeData>
size_t GraphNode<NodeData, EdgeData>::Degree( void ) const
{
  return m_adjacency_list.size() ;
}

/**
* @brief Has loop
*/
template< typename NodeData , typename EdgeData>
bool GraphNode<NodeData, EdgeData>::HasLoop( void ) const
{
  for( auto it = m_adjacency_list.begin() ; it != m_adjacency_list.end() ; ++it )
  {
    const edge_type * cur_edge = *it ;
    const type * opp = cur_edge->Opposite( this ) ;

    if( opp == this )
    {
      return true ;
    }
  }
  return false;
}

/**
* @brief Remove edges that links this node to it-self
*/
template< typename NodeData , typename EdgeData>
size_t GraphNode<NodeData, EdgeData>::RemoveLoops( void )
{
  size_t res = 0 ;
  auto it = m_adjacency_list.begin() ;

  while( it != m_adjacency_list.end() )
  {
    const edge_type * cur_edge = *it ;
    const type * opp = cur_edge->Opposite( this ) ;

    if( opp == this )
    {
      it = m_adjacency_list.erase( it ) ;
      ++res ;
    }
    else
    {
      ++it ;
    }
  }
  return res ;
}

/**
* @brief Indicate if the node has multiple edges linking to the same node
*/
template< typename NodeData , typename EdgeData>
bool GraphNode<NodeData, EdgeData>::HasMultipleLinks( void ) const
{
  std::unordered_map< const type * , bool > found ;

  for( auto it = m_adjacency_list.begin() ; it != m_adjacency_list.end() ; ++it )
  {
    const edge_type * const cur_edge = *it ;
    const type * const opp = cur_edge->Opposite( this ) ;

    if( found.find( opp ) != found.end() )
    {
      return true ;
    }
    else
    {
      found[ opp ] = true ;
    }
  }

  return false ;
}



}
}

#endif