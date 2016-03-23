// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "testing/testing.h"

#include "openMVG/graph/graph.hpp"

using namespace openMVG::graph ;

TEST( graphPathSearching , testDjikstra )
{
  typedef UndirectedGraph<std::string, int> graph_type ;
  typedef graph_type::node_type node_type ;
  typedef graph_type::edge_type edge_type ;

  /* Sample : http://optlab-server.sce.carleton.ca/POAnimations2007/DijkstrasAlgo.html */
  graph_type g ;

  node_type * nA = g.AddNode( "A" ) ;
  node_type * nB = g.AddNode( "B" ) ;
  node_type * nC = g.AddNode( "C" ) ;
  node_type * nD = g.AddNode( "D" ) ;
  node_type * nE = g.AddNode( "E" ) ;
  node_type * nF = g.AddNode( "F" ) ;
  node_type * nO = g.AddNode( "O" ) ;
  node_type * nT = g.AddNode( "T" ) ;

  edge_type * eOA = g.AddEdge( nO , nA , 2 ) ;
  edge_type * eOB = g.AddEdge( nO , nB , 5 ) ;
  edge_type * eOC = g.AddEdge( nO , nC , 4 ) ;
  edge_type * eAB = g.AddEdge( nA , nB , 2 ) ;
  edge_type * eBC = g.AddEdge( nB , nC , 1 ) ;
  edge_type * eAF = g.AddEdge( nA , nF , 12 ) ;
  edge_type * eAD = g.AddEdge( nA , nD , 7 ) ;
  edge_type * eBD = g.AddEdge( nB , nD , 5 ) ;
  edge_type * eBE = g.AddEdge( nB , nE , 3 ) ;
  edge_type * eCE = g.AddEdge( nC , nE , 4 ) ;
  edge_type * eFT = g.AddEdge( nF , nT , 3 ) ;
  edge_type * eDT = g.AddEdge( nD , nT , 5 ) ;
  edge_type * eDE = g.AddEdge( nD , nE , 1 ) ;
  edge_type * eET = g.AddEdge( nE , nT , 7 ) ;

  GraphShortestPath<graph_type> path_solver ;
  std::vector< edge_type * > shortest_path = path_solver.ShortestPathDijkstra( g , nO , nT ) ;


  EXPECT_EQ( 5 , shortest_path.size() ) ;
  // Shortest path is :
  // O (2) A (2) B (3) E (1) D (5) T
  //
  node_type * cur = nO ;
  edge_type * cur_edge = shortest_path[ 0 ] ;
  cur = cur_edge->Opposite( cur ) ;
  EXPECT_EQ( 2 , cur_edge->Data() ) ;
  EXPECT_EQ( nA , cur ) ;

  cur_edge = shortest_path[ 1 ] ;
  cur = cur_edge->Opposite( cur ) ;
  EXPECT_EQ( 2 , cur_edge->Data() ) ;
  EXPECT_EQ( nB , cur ) ;

  cur_edge = shortest_path[ 2 ] ;
  cur = cur_edge->Opposite( cur ) ;
  EXPECT_EQ( 3 , cur_edge->Data() ) ;
  EXPECT_EQ( nE , cur ) ;

  cur_edge = shortest_path[ 3 ] ;
  cur = cur_edge->Opposite( cur ) ;
  EXPECT_EQ( 1 , cur_edge->Data() ) ;
  EXPECT_EQ( nD , cur ) ;

  cur_edge = shortest_path[ 4 ] ;
  cur = cur_edge->Opposite( cur ) ;
  EXPECT_EQ( 5 , cur_edge->Data() ) ;
  EXPECT_EQ( nT , cur ) ;
}

TEST( graphPathSearching , testBellmanFord )
{
  typedef UndirectedGraph<std::string, int> graph_type ;
  typedef graph_type::node_type node_type ;
  typedef graph_type::edge_type edge_type ;

  /* Sample : http://optlab-server.sce.carleton.ca/POAnimations2007/DijkstrasAlgo.html */
  graph_type g ;

  node_type * nA = g.AddNode( "A" ) ;
  node_type * nB = g.AddNode( "B" ) ;
  node_type * nC = g.AddNode( "C" ) ;
  node_type * nD = g.AddNode( "D" ) ;
  node_type * nE = g.AddNode( "E" ) ;
  node_type * nF = g.AddNode( "F" ) ;
  node_type * nO = g.AddNode( "O" ) ;
  node_type * nT = g.AddNode( "T" ) ;

  edge_type * eOA = g.AddEdge( nO , nA , 2 ) ;
  edge_type * eOB = g.AddEdge( nO , nB , 5 ) ;
  edge_type * eOC = g.AddEdge( nO , nC , 4 ) ;
  edge_type * eAB = g.AddEdge( nA , nB , 2 ) ;
  edge_type * eBC = g.AddEdge( nB , nC , 1 ) ;
  edge_type * eAF = g.AddEdge( nA , nF , 12 ) ;
  edge_type * eAD = g.AddEdge( nA , nD , 7 ) ;
  edge_type * eBD = g.AddEdge( nB , nD , 5 ) ;
  edge_type * eBE = g.AddEdge( nB , nE , 3 ) ;
  edge_type * eCE = g.AddEdge( nC , nE , 4 ) ;
  edge_type * eFT = g.AddEdge( nF , nT , 3 ) ;
  edge_type * eDT = g.AddEdge( nD , nT , 5 ) ;
  edge_type * eDE = g.AddEdge( nD , nE , 1 ) ;
  edge_type * eET = g.AddEdge( nE , nT , 7 ) ;

  GraphShortestPath<graph_type> path_solver ;
  std::vector< edge_type * > shortest_path = path_solver.ShortestPathBellmanFord( g , nO , nT ) ;


  EXPECT_EQ( 5 , shortest_path.size() ) ;
  // Shortest path is :
  // O (2) A (2) B (3) E (1) D (5) T
  //
  node_type * cur = nO ;
  edge_type * cur_edge = shortest_path[ 0 ] ;
  cur = cur_edge->Opposite( cur ) ;
  EXPECT_EQ( 2 , cur_edge->Data() ) ;
  EXPECT_EQ( nA , cur ) ;

  cur_edge = shortest_path[ 1 ] ;
  cur = cur_edge->Opposite( cur ) ;
  EXPECT_EQ( 2 , cur_edge->Data() ) ;
  EXPECT_EQ( nB , cur ) ;

  cur_edge = shortest_path[ 2 ] ;
  cur = cur_edge->Opposite( cur ) ;
  EXPECT_EQ( 3 , cur_edge->Data() ) ;
  EXPECT_EQ( nE , cur ) ;

  cur_edge = shortest_path[ 3 ] ;
  cur = cur_edge->Opposite( cur ) ;
  EXPECT_EQ( 1 , cur_edge->Data() ) ;
  EXPECT_EQ( nD , cur ) ;

  cur_edge = shortest_path[ 4 ] ;
  cur = cur_edge->Opposite( cur ) ;
  EXPECT_EQ( 5 , cur_edge->Data() ) ;
  EXPECT_EQ( nT , cur ) ;
}

TEST( graphPathSearching , testBellmanFordNegCycle )
{
  typedef UndirectedGraph<std::string, int> graph_type ;
  typedef graph_type::node_type node_type ;
  typedef graph_type::edge_type edge_type ;

  /* Sample : http://optlab-server.sce.carleton.ca/POAnimations2007/DijkstrasAlgo.html */
  graph_type g ;

  node_type * nA = g.AddNode( "A" ) ;
  node_type * nB = g.AddNode( "B" ) ;
  node_type * nC = g.AddNode( "C" ) ;
  node_type * nD = g.AddNode( "D" ) ;
  node_type * nE = g.AddNode( "E" ) ;
  node_type * nF = g.AddNode( "F" ) ;
  node_type * nO = g.AddNode( "O" ) ;
  node_type * nT = g.AddNode( "T" ) ;

  edge_type * eOA = g.AddEdge( nO , nA , 2 ) ;
  edge_type * eOB = g.AddEdge( nO , nB , 5 ) ;
  edge_type * eOC = g.AddEdge( nO , nC , 4 ) ;
  edge_type * eAB = g.AddEdge( nA , nB , 2 ) ;
  edge_type * eBC = g.AddEdge( nB , nC , 1 ) ;
  edge_type * eAF = g.AddEdge( nA , nF , 12 ) ;
  edge_type * eAD = g.AddEdge( nA , nD , 7 ) ;
  edge_type * eBD = g.AddEdge( nB , nD , 5 ) ;
  edge_type * eBE = g.AddEdge( nB , nE , 3 ) ;
  edge_type * eCE = g.AddEdge( nC , nE , 4 ) ;
  edge_type * eFT = g.AddEdge( nF , nT , 3 ) ;
  edge_type * eDT = g.AddEdge( nD , nT , 5 ) ;
  edge_type * eDE = g.AddEdge( nD , nE , -1 ) ;
  edge_type * eET = g.AddEdge( nE , nT , 7 ) ;

  bool has_neg_cycle ;
  GraphShortestPath<graph_type> path_solver ;
  std::vector< edge_type * > shortest_path = path_solver.ShortestPathBellmanFord( g , nO , nT , &has_neg_cycle ) ;


  EXPECT_EQ( 0 , shortest_path.size() ) ;
  EXPECT_EQ( true , has_neg_cycle ) ;
}




/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
