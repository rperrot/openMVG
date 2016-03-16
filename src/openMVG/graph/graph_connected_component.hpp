// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_GRAPH_GRAPH_CONNECTED_COMPONENT_HPP_
#define _OPENMVG_GRAPH_GRAPH_CONNECTED_COMPONENT_HPP_

// #include <map>
#include <unordered_map>

namespace openMVG
{
namespace graph
{

/**
* @brief Helper structure to extract and gets some informations on Connected Components of a graph
* Currently the helper can :
* - Extract the connected components
* - Get the number of nodes in each CC
* - Get the maximum number of nodes in a CC
* - Get a deep copy the CC
*/
template< typename GraphType >
struct GraphConnectedComponents
{
  public:

    typedef typename GraphType::node_type node_type ;
    typedef typename GraphType::edge_type edge_type ;

    /**
    * @brief Get a list of connected components
    * @param g Input graph
    * @return a list of Connected Components represented by a reference node
    * @note Reference node is choosen arbitrary
    */
    std::vector< node_type * > GetCC( const GraphType & g ) const ;

    /**
    * @brief Get number of node in each connected components
    * @param g Input graph
    * @return vector of size of nb nodes in each CC
    * @note Ordering result has no meaning
    */
    std::vector< size_t > GetCCNodeCount( const GraphType & g ) const ;

    /**
    * @brief Get largest CC
    * @param g Input graph
    * @return a representant of the largest CC
    * @note If two CC have the same number of node, result is arbitrary
    */
    node_type * GetLargestCC( const GraphType & g ) const ;

    /**
    * @brief Get a copy of each connected components
    * @param g Input graph
    * @return a copy of each CC
    * @note Result order is unrevelant
    * @note Each graph is obtain through a deep copy of nodes
    * @note If internal data (node or edge) is pointer-like, we only copy the pointer and data is shared on all graphs
    */
    std::vector< GraphType > GetCCCopy( const GraphType & g ) const ;

    /**
    * @brief Computes a list of cut points
    * @parma g Input graph
    * @note cut points are nodes that if removed makes a connected graph unconnected
    * @return list of cut points
    */
    std::vector< node_type * > GetCutPoints( const GraphType & g ) const ;


  private:

    /**
    * @brief Get CC helper
    * @param g Input graph
    * @param from_node Current node being processed
    * @param used a map to store already processed nodes
    * @note This do a DFS search to explore all nodes reachable from from_node
    * @note At the end of the process, all nodes belonging to the same CC as from_node are found
    */
    void GetCC( const GraphType & g , node_type * from_node , std::unordered_map<node_type*, bool> & used ) const ;

    /**
    * @brief Get CC Node count helper
    * @param g Input graph
    * @param from_node Current node beign processed
    * @param used a map to store already processed nodes
    * @return number of nodes that can be reached from from_node
    * @note This do a DFS search to count how many nodes can be reached from from_node
    * @note from_node is counted in the result
    */
    size_t GetCCNodeCount( const GraphType & g , node_type * from_node , std::unordered_map<node_type*, bool> & used ) const ;

    /**
    * @brief Get CC Copy helper
    * @param g Input graph
    * @param from_node Current node being processed
    * @param used a map to store already processed nodes in the input graph
    * @param map_node A map to get a 1 by 1 relation between the input graph nodes and the output graph nodes
    * @param map_edge A map to get a 1 by 1 relation between the input graph edges and the output graph edges
    * @param[out] cur_graph
    * @note This do a DFS search and for each nodes, build it's corresponding copy in cur_graph
    * @note At the end of the process, all nodes accessibles from from_node have a copy in cur_graph graph
    * @note We do a deep copy (nodes/edges of g are deeply copied in cur_graph)
    * @note Deep copy is only on the structure of the graph, if data of the nodes and/or the edges are pointers, we only to a shallow copy
    */
    void GetCCCopy( const GraphType & g ,
                    node_type * from_node ,
                    std::unordered_map<node_type*, bool> & used ,
                    std::unordered_map<node_type*, node_type*> & map_node ,
                    std::unordered_map<edge_type*, edge_type*> & map_edge ,
                    GraphType & cur_graph ) const ;

    /**
    * @brief Compute list of cut points (helper)
    * @param from_node Start of the DFS search
    * @param parent Current parent of the from_node
    * @param[in,out] used map to know if a node is already used in DFS
    * @param[in,out] discovery_time time when a node is firstly seen during DFS
    * @param[in,out] lowest_time lowest time to see a node during DFS
    * @param[out] indicate if a node is a cut point
    */
    void GetCutPoints( node_type * from_node ,
                       size_t & current_time ,
                       std::unordered_map<node_type*, bool> & used ,
                       std::unordered_map<node_type*, size_t> & discovery_time ,
                       std::unordered_map<node_type*, size_t> & lowest_time ,
                       std::unordered_map<node_type*, node_type*> & parent ,
                       std::unordered_map<node_type*, bool> & is_cp ) const ;
} ;

/**
* @brief Get a list of connected components
* @param g Input graph
* @return a list of Connected Components represented by a reference node
* @note Reference node is choosen arbitrary
*/
template< typename GraphType >
std::vector< typename GraphConnectedComponents<GraphType>::node_type * > GraphConnectedComponents<GraphType>::GetCC( const GraphType & g ) const
{
  const std::vector< node_type * > & nodes = g.Nodes() ;
  std::vector< node_type * > res ;

  int nb_left = nodes.size() ;

  if( nodes.size() > 0 )
  {
    std::unordered_map<node_type *, bool> used ;

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

/**
* @brief Get CC helper
* @param g Input graph
* @param from_node Current node being processed
* @param used a map to store already processed nodes
* @note This do a DFS search to explore all nodes reachable from from_node
* @note At the end of the process, all nodes belonging to the same CC as from_node are found
*/
template< typename GraphType >
void GraphConnectedComponents<GraphType>::GetCC( const GraphType & g ,
    typename GraphConnectedComponents<GraphType>::node_type * from_node ,
    std::unordered_map<typename GraphConnectedComponents<GraphType>::node_type*, bool> & used ) const
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


/**
* @brief Get CC Node count helper
* @param g Input graph
* @param from_node Current node beign processed
* @param used a map to store already processed nodes
* @return number of nodes that can be reached from from_node
* @note This do a DFS search to count how many nodes can be reached from from_node
* @note from_node is counted in the result
*/
template< typename GraphType >
size_t GraphConnectedComponents<GraphType>::GetCCNodeCount( const GraphType & g , typename GraphConnectedComponents<GraphType>::node_type * from_node , std::unordered_map<typename GraphConnectedComponents<GraphType>::node_type*, bool> & used ) const
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
* @param g Input graph
* @return vector of size of nb nodes in each CC
* @note Ordering result has no meaning
*/
template< typename GraphType >
std::vector< size_t > GraphConnectedComponents<GraphType>::GetCCNodeCount( const GraphType & g ) const
{
  const std::vector< node_type * > & nodes = g.Nodes() ;
  std::vector< size_t > res ;

  int nb_left = nodes.size() ;

  if( nodes.size() > 0 )
  {
    std::unordered_map<node_type *, bool> used ;

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
* @param g Input graph
* @return a representant of the largest CC
* @note If two CC have the same number of node, result is arbitrary
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
    std::unordered_map<node_type *, bool> used ;

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
* @brief Get CC Copy helper
* @param g Input graph
* @param from_node Current node being processed
* @param used a map to store already processed nodes in the input graph
* @param map_node A map to get a 1 by 1 relation between the input graph nodes and the output graph nodes
* @param map_edge A map to get a 1 by 1 relation between the input graph edges and the output graph edges
* @param[out] cur_graph
* @note This do a DFS search and for each nodes, build it's corresponding copy in cur_graph
* @note At the end of the process, all nodes accessibles from from_node have a copy in cur_graph graph
* @note We do a deep copy (nodes/edges of g are deeply copied in cur_graph)
* @note Deep copy is only on the structure of the graph, if data of the nodes and/or the edges are pointers, we only to a shallow copy
*/
template< typename GraphType >
void GraphConnectedComponents<GraphType>::GetCCCopy( const GraphType & g ,
    typename GraphConnectedComponents<GraphType>::node_type * from_node ,
    std::unordered_map<typename GraphConnectedComponents<GraphType>::node_type*, bool> & used ,
    std::unordered_map<typename GraphConnectedComponents<GraphType>::node_type*, typename GraphConnectedComponents<GraphType>::node_type*> & map_node ,
    std::unordered_map<typename GraphConnectedComponents<GraphType>::edge_type*, typename GraphConnectedComponents<GraphType>::edge_type*> & map_edge ,
    GraphType & cur_graph ) const
{
  used[ from_node ] = true ;

  // Add this node to the resulting graph and make correspondance between the old node and the new one
  map_node[ from_node ] = cur_graph.AddNode( from_node->Data() ) ;

  // Build all edges based on from_node edges
  auto & neighs = from_node->Neighbors() ;
  for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
  {
    edge_type * cur_edge = *it_neigh ;
    node_type * opp = cur_edge->Opposite( from_node ) ;

    // The node hasn't been processed, do it then we'll add the edge
    if( ! used.count( opp ) )
    {
      GetCCCopy( g , opp , used , map_node , map_edge , cur_graph ) ;
    }

    // We are sure here that map_node[from_node] and map_node[opp] are present, so add the edge to the new graph
    // Check if edge has already been created ?
    if( ! map_edge.count( cur_edge ) )
    {
      cur_graph.m_nb_edge += 1 ;
      map_edge[cur_edge] = new edge_type( map_node[from_node] , map_node[opp] , cur_edge->Data() ) ;
    }

    map_node[from_node]->AddNeighbor( map_edge[cur_edge] ) ;
  }
}


/**
* @brief Get a copy of each connected components
* @return a copy of each CC
*/
template< typename GraphType >
std::vector< GraphType > GraphConnectedComponents<GraphType>::GetCCCopy( const GraphType & g ) const
{
  std::vector< GraphType > res ;

  const std::vector< node_type * > & nodes = g.Nodes() ;

  int nb_left = nodes.size() ;

  if( nodes.size() > 0 )
  {
    std::unordered_map<node_type *, bool> used ;
    std::unordered_map<node_type * , node_type * > map_node ;
    std::unordered_map<edge_type * , edge_type * > map_edge ;

    // Loop until some nodes haven't been found
    while( nb_left > 0 )
    {
      // Look for an unused node and start from it
      int id = 0 ;
      while( used.count( nodes[id] ) != 0 )
      {
        ++id ;
      }

      // Computes CC from this node and it's corresponding graph
      GraphType tmp ;
      res.push_back( tmp ) ;
      GetCCCopy( g , nodes[id] , used , map_node , map_edge , res[ res.size() - 1 ] ) ;

      // Update nb unseen nodes
      nb_left = nodes.size() - used.size() ;
    }
  }

  return res ;
}



/**
* @brief Computes a list of cut points
* @parma g Input graph
* @note cut points are nodes that if removed makes a connected graph unconnected
* @return list of cut points
*/
template< typename GraphType >
std::vector< typename GraphConnectedComponents<GraphType>::node_type * > GraphConnectedComponents<GraphType>::GetCutPoints( const GraphType & g ) const
{
  if( g.NbNode() == 0 )
  {
    std::vector< node_type * > res ;
    return res ;
  }

  const std::vector< node_type * > & nodes = g.Nodes() ;

  std::unordered_map< node_type * , bool > used ;
  std::unordered_map< node_type * , size_t > discovery_time ; // Time of discovery in DFS search of all nodes
  std::unordered_map< node_type * , size_t > lowest_time ; // lowest_time of any vertex reachable from a given vertex
  std::unordered_map< node_type * , node_type * > parent ; // Given a node get it's parent in the DFS tree
  std::unordered_map< node_type * , bool > is_cp ; // Indicate if a node is a cut point

  // Start from first node -> it's the root of the tree so no parent
  parent[ nodes[0] ] = nullptr ;

  size_t cur_time = 0 ;
  GetCutPoints( nodes[0] , cur_time , used , discovery_time , lowest_time , parent , is_cp ) ;

  std::vector< node_type * > res ;
  for( auto it = is_cp.cbegin() ; it != is_cp.end() ; ++it )
  {
    res.push_back( it->first ) ;
  }

  return res ;
}

/**
* @brief Compute list of cut points (helper)
* @param from_node Start of the DFS search
* @param parent Current parent of the from_node
* @param used map to know if a node is already used in DFS
* @param discovery_time time when a node is firstly seen during DFS
* @param lowest_time lowest time to see a node during DFS
* @param[out] result List of resulting cut nodes
*/
template< typename GraphType >
void GraphConnectedComponents<GraphType>::GetCutPoints( typename GraphConnectedComponents<GraphType>::node_type * from_node ,
    size_t & current_time ,
    std::unordered_map<typename GraphConnectedComponents<GraphType>::node_type *, bool> & used ,
    std::unordered_map<typename GraphConnectedComponents<GraphType>::node_type *, size_t> & discovery_time ,
    std::unordered_map<typename GraphConnectedComponents<GraphType>::node_type *, size_t> & lowest_time ,
    std::unordered_map<typename GraphConnectedComponents<GraphType>::node_type *, typename GraphConnectedComponents<GraphType>::node_type *> & parent ,
    std::unordered_map<typename GraphConnectedComponents<GraphType>::node_type *, bool> & is_cp ) const
{
  used[ from_node ] = true ;
  discovery_time[ from_node ] = current_time ;
  lowest_time[ from_node ] = current_time ;
  ++current_time ;

  auto & neighs = from_node->Neighbors() ;

  // Number of children in the tree with from_node as root
  // note: this is not the number of neighbors
  int nb_child = 0 ;

  for( auto it_neigh = neighs.begin() ; it_neigh != neighs.end() ; ++it_neigh )
  {
    node_type * opp = ( *it_neigh )->Opposite( from_node ) ;

    if( used.count( opp ) )
    {
      // Already found, just update the time if from_node is not direct child of opp
      if( parent[ from_node ] != opp )
      {
        lowest_time[ from_node ] = std::min( lowest_time[ from_node ] , discovery_time[ opp ] ) ;
      }
    }
    else // Node not discovered yet
    {
      parent[ opp ] = from_node ;
      ++nb_child ;

      // Build DFS tree
      GetCutPoints( opp , current_time , used , discovery_time , lowest_time , parent , is_cp ) ;

      // Now the DFS is built for all children starting from opp, see if from_node is a cut point
      // -> Update the lowest value of from_node based on the children of opp
      lowest_time[ from_node ] = std::min( lowest_time[ from_node ] , lowest_time[ opp ] ) ;

      node_type * cur_parent = parent[ from_node ] ;
      // if we are the root of the tree and we connect more than one node
      if( cur_parent == nullptr && nb_child > 1 )
      {
        is_cp[ from_node ] = true ;
      }

      // if we're not the root and some child is discovered later than from_node (ie: it can not be discovered passing by another node)
      if( cur_parent != nullptr && lowest_time[ opp ] >= discovery_time[ from_node ] )
      {
        is_cp[ from_node ] = true ;
      }

    }
  }


}




}
}

#endif