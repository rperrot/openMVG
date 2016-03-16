// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "testing/testing.h"

#include "openMVG/graph/graph.hpp"

using namespace openMVG::graph ;

TEST( graphMST , EdgeMST )
{
  typedef UndirectedGraph<std::string, int> graph_type ;
  graph_type g ;

  graph_type::node_type * n1 = g.AddNode( "a" ) ;
  graph_type::node_type * n2 = g.AddNode( "b" ) ;
  graph_type::node_type * n3 = g.AddNode( "c" ) ;
  graph_type::node_type * n4 = g.AddNode( "d" ) ;
  graph_type::node_type * n5 = g.AddNode( "e" ) ;

  graph_type::edge_type * e1 = g.AddEdge( n1 , n2 , 3 ) ;
  graph_type::edge_type * e2 = g.AddEdge( n2 , n3 , 5 ) ;
  graph_type::edge_type * e3 = g.AddEdge( n2 , n5 , 4 ) ;
  graph_type::edge_type * e4 = g.AddEdge( n3 , n4 , 2 ) ;
  graph_type::edge_type * e5 = g.AddEdge( n3 , n5 , 6 ) ;
  graph_type::edge_type * e6 = g.AddEdge( n4 , n5 , 7 ) ;
  graph_type::edge_type * e7 = g.AddEdge( n1 , n5 , 1 ) ;

  GraphSpanningTree<graph_type> MST_solver ;

  graph_type mst = MST_solver.MST( g ) ;

  EXPECT_EQ( mst.NbNode() , 5 ) ;
  EXPECT_EQ( mst.NbEdge() , 4 ) ;

  const std::string g_viz = g.GetGraphVizString( ) ;

  std::ofstream file( "input.dot" );
  file << g_viz << std::endl ;
  file.close() ;

  const std::string g_viz_mst = mst.GetGraphVizString( ) ;

  std::ofstream file2( "mst.dot" );
  file2 << g_viz_mst << std::endl ;
  file2.close() ;
}

TEST( graphMST , EdgeMST_SpecialWeights )
{
  typedef UndirectedGraph<std::string, std::string> graph_type ;
  graph_type g ;

  graph_type::node_type * n1 = g.AddNode( "a" ) ;
  graph_type::node_type * n2 = g.AddNode( "b" ) ;
  graph_type::node_type * n3 = g.AddNode( "c" ) ;
  graph_type::node_type * n4 = g.AddNode( "d" ) ;
  graph_type::node_type * n5 = g.AddNode( "e" ) ;

  graph_type::edge_type * e1 = g.AddEdge( n1 , n2 , "3 -> 3" ) ;
  graph_type::edge_type * e2 = g.AddEdge( n2 , n3 , "5 -> 5" ) ;
  graph_type::edge_type * e3 = g.AddEdge( n2 , n5 , "4 -> 4" ) ;
  graph_type::edge_type * e4 = g.AddEdge( n3 , n4 , "2 -> 2" ) ;
  graph_type::edge_type * e5 = g.AddEdge( n3 , n5 , "6 -> 6" ) ;
  graph_type::edge_type * e6 = g.AddEdge( n4 , n5 , "7 -> 0" ) ;
  graph_type::edge_type * e7 = g.AddEdge( n1 , n5 , "1 -> 1" ) ;

  std::map<graph_type::edge_type*, float> edge_w ;
  edge_w[ e1 ] = 3.f ;
  edge_w[ e2 ] = 5.f ;
  edge_w[ e3 ] = 4.f ;
  edge_w[ e4 ] = 2.f ;
  edge_w[ e5 ] = 6.f ;
  edge_w[ e6 ] = 0.f ; // /!\ modification with special weights to have another tree
  edge_w[ e7 ] = 1.f ;

  GraphSpanningTree<graph_type> MST_solver ;

  graph_type mst = MST_solver.MST( g , edge_w ) ;

  EXPECT_EQ( mst.NbNode() , 5 ) ;
  EXPECT_EQ( mst.NbEdge() , 4 ) ;

  const std::string g_viz = g.GetGraphVizString( ) ;

  std::ofstream file( "input_w.dot" );
  file << g_viz << std::endl ;
  file.close() ;

  const std::string g_viz_mst = mst.GetGraphVizString( ) ;

  std::ofstream file2( "mst_w.dot" );
  file2 << g_viz_mst << std::endl ;
  file2.close() ;

}

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
