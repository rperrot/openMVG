// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "testing/testing.h"

#include "openMVG/graph/graph.hpp"

using namespace openMVG::graph ;

TEST( GraphIO , saveAndLoad )
{
  typedef UndirectedGraph< int , int > graph_type ;
  typedef graph_type::node_type node_type ;
  typedef graph_type::edge_type edge_type ;

  graph_type g ;
  node_type * n1 = g.AddNode( 10 ) ;
  node_type * n2 = g.AddNode( 20 ) ;
  node_type * n3 = g.AddNode( 30 ) ;
  node_type * n4 = g.AddNode( 40 ) ;
  node_type * n5 = g.AddNode( 50 ) ;
  node_type * n6 = g.AddNode( 60 ) ;
  node_type * n7 = g.AddNode( 70 ) ;

  edge_type * e1 = g.AddEdge( n1 , n2 , 1020 ) ;
  edge_type * e2 = g.AddEdge( n2 , n3 , 2030 ) ;
  edge_type * e3 = g.AddEdge( n3 , n4 , 3040 ) ;
  edge_type * e4 = g.AddEdge( n4 , n5 , 4050 ) ;
  edge_type * e5 = g.AddEdge( n5 , n6 , 5060 ) ;
  edge_type * e6 = g.AddEdge( n6 , n7 , 6070 ) ;
  edge_type * e7 = g.AddEdge( n7 , n1 , 7010 ) ;
  edge_type * e8 = g.AddEdge( n1 , n4 , 1040 ) ;
  edge_type * e9 = g.AddEdge( n3 , n5 , 3050 ) ;
  edge_type * e10 = g.AddEdge( n4 , n6 , 4060 ) ;

  EXPECT_EQ( 7 , g.NbNode() ) ;
  EXPECT_EQ( 10 , g.NbEdge() ) ;

  bool sok = Save( g , "test.bin" ) ;

  EXPECT_EQ( sok , true ) ;

  graph_type loaded ;
  bool lok = Load( loaded , "test.bin" ) ;

  EXPECT_EQ( lok , true ) ;

  EXPECT_EQ( 7 , loaded.NbNode() ) ;
  EXPECT_EQ( 10 , loaded.NbEdge() ) ;


  const std::vector< node_type * > & g_nodes = g.Nodes() ;
  const std::vector< node_type * > & l_nodes = loaded.Nodes() ;

  EXPECT_EQ( g_nodes.size() , l_nodes.size() ) ;

  for( size_t id = 0 ; id < g_nodes.size() ; ++id )
  {
    EXPECT_EQ( g_nodes[ id ]->Data() , l_nodes[ id ]->Data( ) ) ;
  }

  EXPECT_EQ( true , loaded.ExistEdgeBetween( l_nodes[0] , l_nodes[1] ) ) ;
  EXPECT_EQ( true , loaded.ExistEdgeBetween( l_nodes[1] , l_nodes[2] ) ) ;
  EXPECT_EQ( true , loaded.ExistEdgeBetween( l_nodes[2] , l_nodes[3] ) ) ;
  EXPECT_EQ( true , loaded.ExistEdgeBetween( l_nodes[3] , l_nodes[4] ) ) ;
  EXPECT_EQ( true , loaded.ExistEdgeBetween( l_nodes[4] , l_nodes[5] ) ) ;
  EXPECT_EQ( true , loaded.ExistEdgeBetween( l_nodes[5] , l_nodes[6] ) ) ;
  EXPECT_EQ( true , loaded.ExistEdgeBetween( l_nodes[6] , l_nodes[0] ) ) ;
  EXPECT_EQ( true , loaded.ExistEdgeBetween( l_nodes[0] , l_nodes[3] ) ) ;
  EXPECT_EQ( true , loaded.ExistEdgeBetween( l_nodes[2] , l_nodes[4] ) ) ;
  EXPECT_EQ( true , loaded.ExistEdgeBetween( l_nodes[3] , l_nodes[5] ) ) ;
}

TEST( GraphIO , saveAndLoadEmpty )
{
  typedef UndirectedGraph< int , int > graph_type ;
  typedef graph_type::node_type node_type ;
  typedef graph_type::edge_type edge_type ;

  graph_type g ;

  bool sok = Save( g , "test.bin" ) ;

  EXPECT_EQ( sok , true ) ;

  graph_type loaded ;
  bool lok = Load( loaded , "test.bin" ) ;

  EXPECT_EQ( lok , true ) ;

  EXPECT_EQ( 0 , loaded.NbNode() ) ;
  EXPECT_EQ( 0 , loaded.NbEdge() ) ;
}

TEST( GraphIO , saveAndLoadNullData )
{
  typedef UndirectedGraph<> graph_type ;
  typedef graph_type::node_type node_type ;
  typedef graph_type::edge_type edge_type ;

  graph_type g ;

  node_type * n1 = g.AddNode() ;
  node_type * n2 = g.AddNode() ;

  edge_type * e1 = g.AddEdge( n1 , n2 ) ;

  bool sok = Save( g , "test.bin" ) ;

  EXPECT_EQ( sok , true ) ;

  graph_type loaded ;
  bool lok = Load( loaded , "test.bin" ) ;

  EXPECT_EQ( lok , true ) ;

  EXPECT_EQ( 2 , loaded.NbNode() ) ;
  EXPECT_EQ( 1 , loaded.NbEdge() ) ;
}

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
