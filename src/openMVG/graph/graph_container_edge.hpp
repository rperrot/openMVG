// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_GRAPH_GRAPH_CONTAINER_EDGE_HPP_
#define _OPENMVG_GRAPH_GRAPH_CONTAINER_EDGE_HPP_

#include "openMVG/graph/graph_utility.hpp"

namespace openMVG
{
namespace graph
{

template<typename NodeData, typename EdgeData>
class GraphNode ;

template< typename EdgeData = NullData , typename NodeData = NullData >
class GraphEdge
{

  public:

    typedef EdgeData edge_data_type ;
    typedef GraphNode<NodeData, EdgeData> node_type ;
    typedef GraphEdge<EdgeData, NodeData> type ;

    /**
    * @brief Ctr
    * @param start_node First node
    * @param end_node Second node
    * @param e_data Data in the node
    */
    GraphEdge( node_type * start_node , node_type * end_node , const EdgeData & e_data ) ;

    /**
    * @brief Get data stored in the edge
    */
    EdgeData Data( void ) const ;

    // Get opposite of a node on this edge
    node_type * Opposite( const node_type * node ) const ;

    /**
    * @brief Get source node
    */
    node_type * Source( void ) const ;

    /**
    * @brief Get destination node
    */
    node_type * Destination( void ) const ;

  private:

    // Current edge data
    EdgeData m_data ;

    node_type * m_start_node ;
    node_type * m_end_node ;
} ;


template< typename EdgeData , typename NodeData >
GraphEdge<EdgeData, NodeData>::GraphEdge( node_type * start_node , node_type * end_node , const EdgeData & e_data )
  : m_data( e_data ) ,
    m_start_node( start_node ) ,
    m_end_node( end_node )
{

}

template< typename EdgeData , typename NodeData >
EdgeData GraphEdge<EdgeData, NodeData>::Data( void ) const
{
  return m_data ;
}

// Get opposite of a node on this edge
template< typename EdgeData , typename NodeData >
typename GraphEdge<EdgeData, NodeData>::node_type * GraphEdge<EdgeData, NodeData>::Opposite( const node_type * node ) const
{
  return m_start_node == node ? m_end_node : m_start_node ;
}

template< typename EdgeData , typename NodeData >
typename GraphEdge<EdgeData, NodeData>::node_type * GraphEdge<EdgeData, NodeData>::Source( void ) const
{
  return m_start_node ;
}

template< typename EdgeData , typename NodeData >
typename GraphEdge<EdgeData, NodeData>::node_type * GraphEdge<EdgeData, NodeData>::Destination( void ) const
{
  return m_end_node ;
}

}
}

#endif