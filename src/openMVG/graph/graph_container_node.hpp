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
    adjacency_list_type & Neighbors( void ) ;

    /*
    * @brief Add a neighbor from an edge
    * @param edge edge to be added
    */
    void AddNeighbor( edge_type * edge ) ;

    /**
    * @brief Remove a neighbor from an edge
    */
    void RemoveNeighbor( edge_type * edge ) ;

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
typename GraphNode<NodeData, EdgeData>::adjacency_list_type & GraphNode<NodeData, EdgeData>::Neighbors( void )
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



}
}

#endif