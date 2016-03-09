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


  private:

    void GetCC( const GraphType & g , node_type * from_node , std::map<node_type*, bool> & used ) const ;

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


}
}

#endif