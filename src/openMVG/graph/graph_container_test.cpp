// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "testing/testing.h"

#include "openMVG/graph/graph.hpp"

using namespace openMVG::graph ;

TEST( graphContainer , Ctr )
{
  UndirectedGraph<> g ;

  EXPECT_EQ( 0, g.NbNode() );
  EXPECT_EQ( 0, g.NbEdge() );
}

TEST( graphContainer , AddNode )
{
  UndirectedGraph<> g ;

  UndirectedGraph<>::node_type * node = g.AddNode();

  EXPECT_EQ( 1, g.NbNode() );
  EXPECT_EQ( 0, g.NbEdge() );
}

TEST( graphContainer , RemoveNode )
{
  UndirectedGraph<> g ;

  UndirectedGraph<>::node_type * node = g.AddNode();

  g.RemoveNode( node ) ;

  EXPECT_EQ( 0, g.NbNode() );
  EXPECT_EQ( 0, g.NbEdge() );
}

TEST( graphContainer , AddEdge )
{
  UndirectedGraph<> g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;

  EXPECT_EQ( 2, g.NbNode() ) ;
  EXPECT_EQ( 1, g.NbEdge() ) ;
}

TEST( graphContainer , RemoveEdge )
{
  UndirectedGraph<> g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;

  g.RemoveEdge( e1 ) ;

  EXPECT_EQ( 2, g.NbNode() ) ;
  EXPECT_EQ( 0, g.NbEdge() ) ;
}

TEST( graphContainer , RemoveNodeWithEdge )
{
  UndirectedGraph<> g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;

  g.RemoveNode( n1 ) ;

  EXPECT_EQ( 1 , g.NbNode() ) ;
  EXPECT_EQ( 0 , g.NbEdge() ) ;
}

TEST( graphContainer , RemoveEdgeMultigraph )
{
  UndirectedGraph<> g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n1 ) ;

  g.RemoveEdge( e1 ) ;

  EXPECT_EQ( 1 , g.NbNode() ) ;
  EXPECT_EQ( 0 , g.NbEdge() ) ;
}

TEST( graphContainer , RemoveNodeMultigraph )
{
  UndirectedGraph<> g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n1 ) ;

  g.RemoveNode( n1 ) ;

  EXPECT_EQ( 0 , g.NbNode() ) ;
  EXPECT_EQ( 0 , g.NbEdge() ) ;
}

static inline std::string ToString( const int val )
{
  std::stringstream str ;
  str << val ;
  return str.str() ;
}

TEST( graphContainer , OutputToGraphViz )
{
  typedef UndirectedGraph<std::string, int > graph_type ;
  graph_type g ;

  std::map< graph_type::node_type * , std::string > map_node ;
  std::map< graph_type::edge_type * , std::string > map_edge ;

  graph_type::node_type * n1 = g.AddNode( "Paris" ) ;
  graph_type::node_type * n2 = g.AddNode( "Lyon" ) ;
  graph_type::node_type * n3 = g.AddNode( "Marseille" ) ;
  graph_type::node_type * n4 = g.AddNode( "Nice" ) ;
  graph_type::node_type * n5 = g.AddNode( "Bordeaux" ) ;
  graph_type::node_type * n6 = g.AddNode( "Nantes" ) ;
  graph_type::node_type * n7 = g.AddNode( "Strasbourg" ) ;
  graph_type::node_type * n8 = g.AddNode( "Lille" ) ;

  map_node[ n1 ] = n1->Data() ;
  map_node[ n2 ] = n2->Data() ;
  map_node[ n3 ] = n3->Data() ;
  map_node[ n4 ] = n4->Data() ;
  map_node[ n5 ] = n5->Data() ;
  map_node[ n6 ] = n6->Data() ;
  map_node[ n7 ] = n7->Data() ;
  map_node[ n8 ] = n8->Data() ;

  graph_type::edge_type * e1 = g.AddEdge( n1 , n2 , 470 ) ;
  graph_type::edge_type * e2 = g.AddEdge( n2 , n3 , 323 ) ;
  graph_type::edge_type * e3 = g.AddEdge( n3 , n4 , 197 ) ;
  graph_type::edge_type * e4 = g.AddEdge( n3 , n5 , 662 ) ;
  graph_type::edge_type * e5 = g.AddEdge( n5 , n6 , 329 ) ;
  graph_type::edge_type * e6 = g.AddEdge( n1 , n7 , 448 ) ;
  graph_type::edge_type * e7 = g.AddEdge( n1 , n8 , 223 ) ;
  graph_type::edge_type * e8 = g.AddEdge( n2 , n5 , 549 ) ;
  graph_type::edge_type * e9 = g.AddEdge( n1 , n6 , 393 ) ;
  graph_type::edge_type * e10 = g.AddEdge( n1 , n5 , 561 ) ;
  graph_type::edge_type * e11 = g.AddEdge( n2 , n7 , 434 ) ;
  graph_type::edge_type * e12 = g.AddEdge( n7 , n8 , 524 ) ;
  graph_type::edge_type * e13 = g.AddEdge( n6 , n2 , 607 ) ;

  map_edge[ e1 ] = ToString( e1->Data() ) ;
  map_edge[ e2 ] = ToString( e2->Data() ) ;
  map_edge[ e3 ] = ToString( e3->Data() ) ;
  map_edge[ e4 ] = ToString( e4->Data() ) ;
  map_edge[ e5 ] = ToString( e5->Data() ) ;
  map_edge[ e6 ] = ToString( e6->Data() ) ;
  map_edge[ e7 ] = ToString( e7->Data() ) ;
  map_edge[ e8 ] = ToString( e8->Data() ) ;
  map_edge[ e9 ] = ToString( e9->Data() ) ;
  map_edge[ e10 ] = ToString( e10->Data() ) ;

  const std::string g_viz = g.GetGraphVizString( map_node , map_edge ) ;

  std::ofstream file( "test.dot" );
  file << g_viz << std::endl ;
  file.close() ;
}

TEST( graphContainer , TestPath )
{
  typedef UndirectedGraph<> graph_type ;
  graph_type g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;
  UndirectedGraph<>::node_type * n3 = g.AddNode() ;
  UndirectedGraph<>::node_type * n4 = g.AddNode() ;
  UndirectedGraph<>::node_type * n5 = g.AddNode() ;

  EXPECT_EQ( true , g.ExistPathBetween( n1 , n1 ) );
  EXPECT_EQ( false , g.ExistPathBetween( n1 , n2 ) ) ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;

  EXPECT_EQ( true , g.ExistPathBetween( n1 , n2 ) ) ;
  EXPECT_EQ( false , g.ExistPathBetween( n1 , n3 ) ) ;
  EXPECT_EQ( false , g.ExistPathBetween( n2 , n3 ) ) ;
  EXPECT_EQ( false , g.ExistPathBetween( n3 , n4 ) ) ;

  UndirectedGraph<>::edge_type * e2 = g.AddEdge( n2 , n3 ) ;

  EXPECT_EQ( true , g.ExistPathBetween( n1 , n2 ) ) ;
  EXPECT_EQ( true , g.ExistPathBetween( n2 , n3 ) ) ;
  EXPECT_EQ( true , g.ExistPathBetween( n1 , n3 ) ) ;
  EXPECT_EQ( true , g.ExistPathBetween( n3 , n1 ) ) ;
}

TEST( graphContainer , Loop )
{
  typedef UndirectedGraph<> graph_type ;
  graph_type g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;
  UndirectedGraph<>::node_type * n3 = g.AddNode() ;
  UndirectedGraph<>::node_type * n4 = g.AddNode() ;
  UndirectedGraph<>::node_type * n5 = g.AddNode() ;

  graph_type::edge_type * e1 = g.AddEdge( n1 , n2 ) ;
  graph_type::edge_type * e2 = g.AddEdge( n3 , n4 ) ;
  graph_type::edge_type * e3 = g.AddEdge( n2 , n5 ) ;

  EXPECT_EQ( false , g.HasLoopNode() ) ;

  graph_type::edge_type * e4 = g.AddEdge( n1 , n1 ) ;

  EXPECT_EQ( true , g.HasLoopNode() ) ;
}

TEST( graphContainer , MaxDegree )
{
  typedef UndirectedGraph<> graph_type ;
  graph_type g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;
  UndirectedGraph<>::node_type * n3 = g.AddNode() ;
  UndirectedGraph<>::node_type * n4 = g.AddNode() ;
  UndirectedGraph<>::node_type * n5 = g.AddNode() ;

  EXPECT_EQ( 0 , g.MaxDegree() ) ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;

  EXPECT_EQ( 1 , g.MaxDegree() ) ;

  UndirectedGraph<>::edge_type * e2 = g.AddEdge( n1 , n1 ) ;

  EXPECT_EQ( 2 , g.MaxDegree() ) ;

  UndirectedGraph<>::edge_type * e3 = g.AddEdge( n1 , n2 ) ;

  EXPECT_EQ( 3 , g.MaxDegree() ) ;

  g.RemoveNode( n1 ) ;

  EXPECT_EQ( 0 , g.MaxDegree() ) ; // (n2) (n3) (n4) (n5)

  UndirectedGraph<>::edge_type * e4 = g.AddEdge( n2 , n3 ) ;

  EXPECT_EQ( 1 , g.MaxDegree() ) ; // n2 - n3

  UndirectedGraph<>::edge_type * e5 = g.AddEdge( n3 , n4 ) ;

  EXPECT_EQ( 2 , g.MaxDegree() ) ; // n2 - n3 - n4

  UndirectedGraph<>::edge_type * e6 = g.AddEdge( n4 , n5 ) ;

  EXPECT_EQ( 2 , g.MaxDegree() ) ; // n2 - n3 - n4 - n5

  UndirectedGraph<>::edge_type * e7 = g.AddEdge( n3 , n5 ) ;

  EXPECT_EQ( 3 , g.MaxDegree() ) ;
}

TEST( graphContainer , LoopRemoval )
{
  typedef UndirectedGraph<> graph_type ;
  graph_type g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;
  UndirectedGraph<>::node_type * n3 = g.AddNode() ;
  UndirectedGraph<>::node_type * n4 = g.AddNode() ;
  UndirectedGraph<>::node_type * n5 = g.AddNode() ;

  EXPECT_EQ( 5 , g.NbNode() );
  EXPECT_EQ( 0 , g.NbEdge() );
  EXPECT_EQ( false , g.HasLoopNode() ) ;

  g.RemoveNodeLoops() ;

  EXPECT_EQ( 5 , g.NbNode() );
  EXPECT_EQ( 0 , g.NbEdge() );
  EXPECT_EQ( false , g.HasLoopNode() ) ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;
  UndirectedGraph<>::edge_type * e2 = g.AddEdge( n1 , n3 ) ;
  UndirectedGraph<>::edge_type * e3 = g.AddEdge( n1 , n4 ) ;
  UndirectedGraph<>::edge_type * e4 = g.AddEdge( n1 , n5 ) ;

  EXPECT_EQ( 5 , g.NbNode() );
  EXPECT_EQ( 4 , g.NbEdge() );
  EXPECT_EQ( false , g.HasLoopNode() ) ;

  g.RemoveNodeLoops() ;

  EXPECT_EQ( 5 , g.NbNode() );
  EXPECT_EQ( 4 , g.NbEdge() );
  EXPECT_EQ( false , g.HasLoopNode() ) ;

  UndirectedGraph<>::edge_type * e5 = g.AddEdge( n1 , n1 ) ;
  UndirectedGraph<>::edge_type * e6 = g.AddEdge( n2 , n2 ) ;
  UndirectedGraph<>::edge_type * e7 = g.AddEdge( n3 , n3 ) ;
  UndirectedGraph<>::edge_type * e8 = g.AddEdge( n4 , n4 ) ;
  UndirectedGraph<>::edge_type * e9 = g.AddEdge( n5 , n5 ) ;

  EXPECT_EQ( 5 , g.NbNode() );
  EXPECT_EQ( 9 , g.NbEdge() );
  EXPECT_EQ( true , g.HasLoopNode() ) ;

  g.RemoveNodeLoops();

  EXPECT_EQ( 5 , g.NbNode() );
  EXPECT_EQ( 4 , g.NbEdge() );
  EXPECT_EQ( false , g.HasLoopNode() ) ;
}

TEST( graphContainer , MultipleLinks )
{
  typedef UndirectedGraph<> graph_type ;
  graph_type g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;
  UndirectedGraph<>::node_type * n3 = g.AddNode() ;
  UndirectedGraph<>::node_type * n4 = g.AddNode() ;
  UndirectedGraph<>::node_type * n5 = g.AddNode() ;

  EXPECT_EQ( false , g.HasMultipleEdgeBetweenNodes() ) ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n1 ) ;

  EXPECT_EQ( false , g.HasMultipleEdgeBetweenNodes() ) ;

  UndirectedGraph<>::edge_type * e2 = g.AddEdge( n1 , n2 ) ;

  EXPECT_EQ( false , g.HasMultipleEdgeBetweenNodes() ) ;

  UndirectedGraph<>::edge_type * e3 = g.AddEdge( n1 , n2 ) ;

  EXPECT_EQ( true , g.HasMultipleEdgeBetweenNodes() ) ;

  g.RemoveEdge( e3 ) ;

  EXPECT_EQ( false , g.HasMultipleEdgeBetweenNodes() ) ;
}

TEST( graphContainer , SimpleGraph )
{
  typedef UndirectedGraph<> graph_type ;
  graph_type g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;
  UndirectedGraph<>::node_type * n3 = g.AddNode() ;
  UndirectedGraph<>::node_type * n4 = g.AddNode() ;
  UndirectedGraph<>::node_type * n5 = g.AddNode() ;

  EXPECT_EQ( true , g.IsSimple() ) ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;
  UndirectedGraph<>::edge_type * e2 = g.AddEdge( n1 , n3 ) ;
  UndirectedGraph<>::edge_type * e3 = g.AddEdge( n1 , n4 ) ;
  UndirectedGraph<>::edge_type * e4 = g.AddEdge( n1 , n5 ) ;

  EXPECT_EQ( true , g.IsSimple() ) ;

  UndirectedGraph<>::edge_type * e5 = g.AddEdge( n1 , n1 ) ;

  EXPECT_EQ( false , g.IsSimple() ) ;

  g.RemoveEdge( e5 ) ;

  EXPECT_EQ( true , g.IsSimple() ) ;

  UndirectedGraph<>::edge_type * e6 = g.AddEdge( n1 , n5 ) ;

  EXPECT_EQ( false , g.IsSimple() ) ;

  g.RemoveEdge( e6 ) ;

  EXPECT_EQ( true , g.IsSimple() ) ;
}


/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
