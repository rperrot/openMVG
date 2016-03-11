// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_GRAPH_GRAPH_CONTAINER_H_
#define _OPENMVG_GRAPH_GRAPH_CONTAINER_H_

#include "openMVG/graph/graph_container_node.hpp"
#include "openMVG/graph/graph_container_edge.hpp"
#include "openMVG/graph/graph_utility.hpp"
#include "openMVG/graph/graph_traversal.hpp"

#include <cstdint>
#include <string>
#include <sstream>

namespace openMVG
{
namespace graph
{

/**
* @brief Generic class for Undirected graph
*/
template< typename NodeData = NullData , typename EdgeData = NullData >
class UndirectedGraph
{
  public:
    typedef GraphNode<NodeData, EdgeData> node_type ;
    typedef GraphEdge<EdgeData, NodeData> edge_type ;


    /**
    * @brief Build an empty graph
    */
    UndirectedGraph( void ) ;

    /**
    * @brief dtor
    */
    ~UndirectedGraph( void ) ;

    /**
    * @brief Copy a graph
    * @param graph Input graph
    * @note This makes a deep copy
    */
    explicit UndirectedGraph( const UndirectedGraph & graph ) ;

    /**
    * @brief Assignement of a graph
    * @param g Input source graph
    * @return Graph after assignment
    * @note This makes a deep copy
    */
    UndirectedGraph & operator=( const UndirectedGraph & g ) ;

    /**
    * @brief Add a node to the graph
    * @param data Data to be stored in this node
    * @return A pointer to the newly created node
    */
    node_type * AddNode( const NodeData & data = NodeData() ) ;

    /**
    * @brief Remove a node from the graph
    * @param node_id Pointer to the node to remove
    * @note This removes the node and all it's adjacent edges
    */
    void RemoveNode( node_type * node_id ) ;

    /**
    * @brief Add an edge to the graph
    * @param source_id Pointer to the source of the edge
    * @param dest_id Pointer to the destination of the edge
    * @param e_data Data to be stored in the edge
    * @return A pointer to the created edge
    */
    edge_type * AddEdge( node_type * source_id ,
                         node_type * dest_id ,
                         const EdgeData & e_data = EdgeData() ) ;

    /**
    * @brief Remove and edge from the graph
    * @param edge Pointer to the edge to remove
    */
    void RemoveEdge( edge_type * edge ) ;

    /**
    * @brief Remove all elements (node and edges) of the graph
    */
    void clear( void ) ;

    /**
    * @brief Get number of nodes in the graph
    * @return Number of nodes
    */
    size_t NbNode( void ) const ;

    /**
    * @brief Get number of edges in the graph
    * @return Number of edges of the graph
    */
    size_t NbEdge( void ) const ;


    /**
    * @brief Tests if there is a path (of any length) that can go from source to dest
    * @param source_id Pointer to the source node
    * @param dest_id Pointer to the destination node
    * @retval true if there exists a path
    * @retval false if no path exists
    */
    bool ExistPathBetween( const node_type * source_id , const node_type * dest_id ) const ;

    /**
    * @brief Tests if there is a path (of length 1) that can go from source to dest
    * @param source_id Pointer to the source node
    * @param dest_id Pointer to the destination node
    * @retval true if there exists an edge between the two nodes
    */
    bool ExistEdgeBetween( const node_type * source_id , const node_type * dest_id ) const ;

    /**
    * @brief Get a list of nodes that are directly accessible from a node
    * @param node Pointer to the node to interrogate
    * @return All nodes directly accessible from queried node
    * @note This gets all nodes at distance 0 or 1
    */
    std::vector< node_type * > GetNeighboring( const node_type * node ) ;

    /**
    * @brief Get all nodes of the graph
    */
    std::vector< node_type * > Nodes( void ) const ;

    /**
    * @brief Get a valid dot string that can be processed by graphviz
    * @param map_node An associative array used to display the labels of the nodes
    * @param map_edge An associative array used to display the labels of the edges
    * @return the graphviz string
    */
    std::string GetGraphVizString( const std::map< node_type * , std::string > & map_node ,
                                   const std::map< edge_type * , std::string > & map_edge ) const ;

    // Test graph isomorphism
    // /!\ It's a hard pb with exponential worst case complexity
    template< typename NodeData1 , typename EdgeData1 >
    bool SameStructure( const UndirectedGraph<NodeData1, EdgeData1> & g ) const ;


    /**
    * @brief Get the maximum node degree
    * @return degree of the node with maximum degree
    */
    size_t MaxDegree( void ) const ;

    /**
    * @brief Test if node has loop
    * @retval true if at least a node has a loop
    * @retval false if no node has a loop
    */
    bool HasLoopNode( void ) const ;

    /**
    * @brief Remove all existing loops on nodes
    */
    void RemoveNodeLoops( void ) ;

    /**
    * @brief Indicate there exists two nodes with more
    */
    bool HasMultipleEdgeBetweenNodes( void ) ;

    /**
    * @brief Test if graph is simple
    * -> No loop
    * -> No more than one edge between two nodes
    */
    bool IsSimple( void ) ;



  private:

    std::vector< node_type * > m_nodes ;
    size_t m_nb_edge ;
} ;

// Build an empty graph
template< typename NodeData, typename EdgeData >
UndirectedGraph<NodeData, EdgeData>::UndirectedGraph( void )
  : m_nb_edge( 0 )
{

}

/**
* @brief dtor
*/
template< typename NodeData, typename EdgeData >
UndirectedGraph<NodeData, EdgeData>::~UndirectedGraph( void )
{
  clear() ;
}


template< typename NodeData, typename EdgeData >
UndirectedGraph<NodeData, EdgeData>::UndirectedGraph( const UndirectedGraph<NodeData, EdgeData> & graph )
  : m_nb_edge( graph.m_nb_edge )
{
  // Copy all nodes and get a map from source to destination
  std::map< node_type * , node_type * > map_nodes ;

  for( auto it = graph.m_nodes.begin() ; it != graph.m_nodes.end() ; ++it )
  {
    map_nodes[ *it ] = AddNode( ( *it )->Data() ) ;
  }

  // Build neighbor list of all nodes
  std::map< edge_type * , edge_type * > map_edges ;
  for( auto it = graph.m_nodes.begin() ; it != graph.m_nodes.end() ; ++it )
  {
    const node_type * cur_node = *it ;

    auto & neighs = cur_node->Neighbors() ;

    for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
    {
      edge_type * cur_edge = *it_neigh ;
      // Find if this edge has already been built
      if( map_edges.count( cur_edge ) == 0 )
      {
        // Build the edge
        edge_type * dst_edge = new edge_type( map_nodes[ cur_edge->Source() ] , map_nodes[ cur_edge->Destination() ] , cur_edge->Data() ) ;
        map_edges[ cur_edge ] = dst_edge ;
      }

      map_nodes[ cur_node ].AddNeighbor( map_edges[ cur_edge ] ) ;
    }
  }

}

template< typename NodeData, typename EdgeData >
UndirectedGraph<NodeData, EdgeData> & UndirectedGraph<NodeData, EdgeData>::operator=( const UndirectedGraph<NodeData, EdgeData> & graph )
{
  if( this != &graph )
  {
    clear() ;
    // Copy all nodes and get a map from source to destination
    std::map< node_type * , node_type * > map_nodes ;

    for( auto it = graph.m_nodes.begin() ; it != graph.m_nodes.end() ; ++it )
    {
      map_nodes[ *it ] = AddNode( ( *it )->Data() ) ;
    }

    // Build neighbor list of all nodes
    std::map< edge_type * , edge_type * > map_edges ;
    for( auto it = graph.m_nodes.begin() ; it != graph.m_nodes.end() ; ++it )
    {
      const node_type * cur_node = *it ;

      auto & neighs = cur_node->Neighbors() ;

      for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
      {
        edge_type * cur_edge = *it_neigh ;
        // Find if this edge has already been built
        if( map_edges.count( cur_edge ) == 0 )
        {
          // Build the edge
          edge_type * dst_edge = new edge_type( map_nodes[ cur_edge->Source() ] , map_nodes[ cur_edge->Destination() ] , cur_edge->Data() ) ;
          map_edges[ cur_edge ] = dst_edge ;
        }

        map_nodes[ cur_node ].AddNeighbor( map_edges[ cur_edge ] ) ;
      }
    }

  }
  return *this ;
}

template< typename NodeData, typename EdgeData >
typename UndirectedGraph<NodeData, EdgeData>::node_type * UndirectedGraph<NodeData, EdgeData>::AddNode( const NodeData & data )
{
  node_type * cur_node = new node_type( data ) ;
  m_nodes.push_back( cur_node ) ;

  return cur_node ;
}

template< typename NodeData, typename EdgeData >
void UndirectedGraph<NodeData, EdgeData>::RemoveNode( node_type * node_id )
{
  // Get iterator of this element
  auto it = std::find( m_nodes.begin() , m_nodes.end() , node_id ) ;

  // 1-st Remove all reference to this node (using adjacency_list)
  if( it != m_nodes.end() )
  {
    node_type * cur_node = *it ;
    auto & neighs = cur_node->Neighbors() ;

    for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
    {
      // Get opposite node of node_id
      node_type * opp_node = ( *it_neigh )->Opposite( node_id ) ;

      // Remove node_id in node
      if( opp_node != cur_node )
      {
        opp_node->RemoveNeighbor( *it_neigh ) ;
      }

      // Delete the edge (because it's not referenced elsewhere)
      delete *it_neigh ;
      --m_nb_edge ;
    }

    // Delete the node
    delete cur_node ;

    // Now remove the node from node array
    m_nodes.erase( it ) ;
  }
}

template< typename NodeData, typename EdgeData >
typename UndirectedGraph<NodeData, EdgeData>::edge_type * UndirectedGraph<NodeData, EdgeData>::AddEdge( node_type * source_id ,
    node_type * dest_id ,
    const EdgeData & e_data )
{
  edge_type * edge = new edge_type( source_id , dest_id , e_data ) ;

  source_id->AddNeighbor( edge ) ;
  if( source_id != dest_id )
  {
    dest_id->AddNeighbor( edge ) ;
  }

  ++m_nb_edge ;

  return edge ;
}

template< typename NodeData, typename EdgeData >
void UndirectedGraph<NodeData, EdgeData>::RemoveEdge( edge_type * edge )
{
  node_type * source = edge->Source() ;
  node_type * dest = edge->Destination() ;

  source->RemoveNeighbor( edge ) ;
  if( source != dest )
  {
    dest->RemoveNeighbor( edge ) ;
  }

  delete edge ;
  --m_nb_edge ;
}

// Clear the graph
template< typename NodeData, typename EdgeData >
void UndirectedGraph<NodeData, EdgeData>::clear( void )
{
  for( auto it = m_nodes.begin() ; it != m_nodes.end() ; ++it )
  {
    node_type * cur_node = *it ;

    auto & neighs = cur_node->Neighbors() ;

    // Remove all reference to this node on all nodes
    for( auto neigh_it = neighs.begin() ; neigh_it != neighs.end() ; ++neigh_it )
    {
      node_type * opp_node = ( *neigh_it )->Opposite( cur_node ) ;

      if( opp_node != cur_node )
      {
        opp_node->RemoveNeighbor( *neigh_it ) ;
      }

      // Delete the edge
      delete *neigh_it ;
      --m_nb_edge ;
    }

    // Delete the node
    delete cur_node ;
  }
  m_nodes.clear() ;
}


template< typename NodeData, typename EdgeData >
size_t UndirectedGraph<NodeData, EdgeData>::NbNode( void ) const
{
  return m_nodes.size() ;
}

template< typename NodeData, typename EdgeData >
size_t UndirectedGraph<NodeData, EdgeData>::NbEdge( void ) const
{
  return m_nb_edge ;
}

// Indicate if there exist a path between two nodes
template< typename NodeData, typename EdgeData >
bool UndirectedGraph<NodeData, EdgeData>::ExistPathBetween( const node_type * source_id , const node_type * dest_id ) const
{
  GraphTraversalDFS< UndirectedGraph<NodeData, EdgeData> > dfs_functor ;

  return dfs_functor.NodeSearch( *this , source_id , dest_id ) ;
}

// Indicate if there exist an edge between two nodes
template< typename NodeData, typename EdgeData >
bool UndirectedGraph<NodeData, EdgeData>::ExistEdgeBetween( const node_type * source_id , const node_type * dest_id ) const
{
  auto & neighs = source_id->Neighbors() ;
  for( auto it = neighs.begin() ; it != neighs.end() ; ++it )
  {
    node_type * opp = ( *it )->Opposite( source_id ) ;

    if( opp == dest_id )
    {
      return true ;
    }
  }
  return false ;
}

// Get list of node neighboring a specific node
template< typename NodeData, typename EdgeData >
std::vector< typename UndirectedGraph<NodeData, EdgeData>::node_type * > UndirectedGraph<NodeData, EdgeData>::GetNeighboring( const node_type * node )
{
  std::vector< node_type * > res ;

  auto & neighs = node->Neighbors() ;
  for( auto it = neighs.begin() ; it != neighs.end() ; ++it )
  {
    res.push_back( *it ) ;
  }

  return res ;
}

// Get a string in (graphViz) dot format
template< typename NodeData, typename EdgeData >
std::string UndirectedGraph<NodeData, EdgeData>::GetGraphVizString( const std::map< typename UndirectedGraph<NodeData, EdgeData>::node_type * , std::string > & map_node ,
    const std::map< typename UndirectedGraph<NodeData, EdgeData>::edge_type * , std::string > & map_edge ) const
{
  std::stringstream res ;

  res << "graph G" << std::endl ;
  res << "{" << std::endl ;

  // Graph nodes ;
  uint64_t node_ID = 1 ;
  std::map< node_type * , uint64_t > map_node_ID ;

  for( auto it_node = m_nodes.begin() ; it_node != m_nodes.end() ; ++it_node , ++node_ID )
  {
    node_type * cur_node = *it_node ;

    if( map_node.count( cur_node ) > 0 )
    {
      res << "  node" << node_ID << "[label=\"" << map_node.at( cur_node ) << "\"] ;" << std::endl;
    }
    else
    {
      res << "  node" << node_ID << ";" << std::endl ;
    }

    // Save node ID
    map_node_ID[ cur_node ] = node_ID ;
  }

  res << std::endl ;

  // Graph edges ;
  std::map< edge_type * , bool > map_edge_exist ;
  for( auto it_node = m_nodes.begin() ; it_node != m_nodes.end() ; ++it_node )
  {
    node_type * cur_node = *it_node ;

    auto & neighs = cur_node->Neighbors() ;

    // Get all neighbors of this node
    for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
    {
      edge_type * cur_edge = *it_neigh ;
      // Add this edge only if it was not drawn yet
      if( map_edge_exist.count( cur_edge ) == 0 )
      {
        node_type * opp = cur_edge->Opposite( cur_node ) ;

        const uint64_t source_ID = map_node_ID[ cur_node ] ;
        const uint64_t dest_ID = map_node_ID[ opp ] ;

        if( map_edge.count( cur_edge ) > 0 )
        {
          res << "  node" << source_ID << " -- " << "node" << dest_ID << " [label=\"" << map_edge.at( *it_neigh ) << "\"] ;" << std::endl ;
        }
        else
        {
          res << "  node" << source_ID << " -- " << "node" << dest_ID << ";" << std::endl ;
        }
      }

      // Mark this edge as treated
      map_edge_exist[ cur_edge ] = true ;
    }
  }



  res << "}" << std::endl ;


  return res.str() ;
}

/**
* @brief Get all nodes of the graph
*/
template< typename NodeData, typename EdgeData >
std::vector< typename UndirectedGraph<NodeData, EdgeData>::node_type * > UndirectedGraph<NodeData, EdgeData>::Nodes( void ) const
{
  return m_nodes ;
}

/**
* @brief Get the maximum node degree
* @return degree of the node with maximum degree
*/
template< typename NodeData, typename EdgeData >
size_t UndirectedGraph<NodeData, EdgeData>::MaxDegree( void ) const
{
  size_t max_degree = 0 ;
  for( auto it_node = m_nodes.begin() ; it_node != m_nodes.end() ; ++it_node )
  {
    max_degree = std::max( max_degree , ( *it_node )->Degree() ) ;
  }
  return max_degree ;
}

/**
* @brief Test if node has loop
* @retval true if at least a node has a loop
* @retval false if no node has a loop
*/
template< typename NodeData, typename EdgeData >
bool UndirectedGraph<NodeData, EdgeData>::HasLoopNode( void ) const
{
  for( auto it_node = m_nodes.begin() ; it_node != m_nodes.end() ; ++it_node )
  {
    if( ( *it_node )->HasLoop() )
    {
      return true ;
    }
  }
  return false ;
}

/**
* @brief Remove all existing loops on nodes
*/
template< typename NodeData, typename EdgeData >
void UndirectedGraph<NodeData, EdgeData>::RemoveNodeLoops( void )
{
  for( auto it_node = m_nodes.begin() ; it_node != m_nodes.end() ; ++it_node )
  {
    m_nb_edge -= ( *it_node )->RemoveLoops() ;
  }

}

/**
* @brief Indicate there exists two nodes with more
*/
template< typename NodeData, typename EdgeData >
bool UndirectedGraph<NodeData, EdgeData>::HasMultipleEdgeBetweenNodes( void )
{
  for( auto it_node = m_nodes.begin() ; it_node != m_nodes.end() ; ++it_node )
  {
    if( ( *it_node )->HasMultipleLinks() )
    {
      return true ;
    }
  }
  return false ;
}

/**
* @brief Test if graph is simple
* -> No loop
* -> No more than one edge between two nodes
*/
template< typename NodeData, typename EdgeData >
bool UndirectedGraph<NodeData, EdgeData>::IsSimple( void )
{
  return ! HasMultipleEdgeBetweenNodes() && ! HasLoopNode() ;
}


}
}

#endif