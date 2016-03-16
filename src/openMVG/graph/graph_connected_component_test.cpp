// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "testing/testing.h"

#include "openMVG/graph/graph.hpp"

using namespace openMVG::graph ;

TEST( graphConnectedComponent , TestCC )
{
  UndirectedGraph<> g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;
  UndirectedGraph<>::node_type * n3 = g.AddNode() ;
  UndirectedGraph<>::node_type * n4 = g.AddNode() ;
  UndirectedGraph<>::node_type * n5 = g.AddNode() ;

  GraphConnectedComponents< UndirectedGraph<> > cc_solver ;

  std::vector< UndirectedGraph<>::node_type * > cc = cc_solver.GetCC( g ) ;

  EXPECT_EQ( 5 , g.NbNode() ) ;
  EXPECT_EQ( 5 , cc.size() ) ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;
  cc = cc_solver.GetCC( g ) ;
  EXPECT_EQ( 5 , g.NbNode() ) ;
  EXPECT_EQ( 1 , g.NbEdge() ) ;
  EXPECT_EQ( 4 , cc.size() ) ;

  UndirectedGraph<>::edge_type * e2 = g.AddEdge( n2 , n3 ) ;
  cc = cc_solver.GetCC( g ) ;
  EXPECT_EQ( 5 , g.NbNode() ) ;
  EXPECT_EQ( 2 , g.NbEdge() ) ;
  EXPECT_EQ( 3 , cc.size() ) ; // (n1,n2,n3) (n4) (n5)

  UndirectedGraph<>::edge_type * e3 = g.AddEdge( n4 , n5 ) ;
  cc = cc_solver.GetCC( g ) ;
  EXPECT_EQ( 5 , g.NbNode() ) ;
  EXPECT_EQ( 3 , g.NbEdge() ) ;
  EXPECT_EQ( 2 , cc.size() ) ; // (n1,n2,n3) (n4,n5)


  UndirectedGraph<>::edge_type * e4 = g.AddEdge( n1 , n3 ) ;
  cc = cc_solver.GetCC( g ) ;
  EXPECT_EQ( 5 , g.NbNode() ) ;
  EXPECT_EQ( 4 , g.NbEdge() ) ;
  EXPECT_EQ( 2 , cc.size() ) ; // (n1,n2,n3) (n4,n5)

  UndirectedGraph<>::edge_type * e5 = g.AddEdge( n1 , n5 ) ;
  cc = cc_solver.GetCC( g ) ;
  EXPECT_EQ( 5 , g.NbNode() ) ;
  EXPECT_EQ( 5 , g.NbEdge() ) ;
  EXPECT_EQ( 1 , cc.size() ) ; // (n1,n2,n3,n4,n5)
}

TEST( graphConnectedComponent , connectedComponentsSize )
{
  UndirectedGraph<> g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;
  UndirectedGraph<>::node_type * n3 = g.AddNode() ;
  UndirectedGraph<>::node_type * n4 = g.AddNode() ;
  UndirectedGraph<>::node_type * n5 = g.AddNode() ;

  GraphConnectedComponents< UndirectedGraph<> > cc_solver ;

  std::vector< size_t > cc_count = cc_solver.GetCCNodeCount( g ) ;

  EXPECT_EQ( 5 , cc_count.size() ) ;
  EXPECT_EQ( 1 , cc_count[0] ) ;
  EXPECT_EQ( 1 , cc_count[1] ) ;
  EXPECT_EQ( 1 , cc_count[2] ) ;
  EXPECT_EQ( 1 , cc_count[3] ) ;
  EXPECT_EQ( 1 , cc_count[4] ) ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;
  UndirectedGraph<>::edge_type * e2 = g.AddEdge( n2 , n3 ) ;
  UndirectedGraph<>::edge_type * e3 = g.AddEdge( n4 , n5 ) ;

  cc_count = cc_solver.GetCCNodeCount( g ) ;

  EXPECT_EQ( 2 , cc_count.size() ) ;
  EXPECT_EQ( 5 , cc_count[0] + cc_count[1] ) ;

  EXPECT_EQ( true , cc_count[0] == 3 || cc_count[0] == 2 ) ;
  EXPECT_EQ( true , cc_count[1] == 2 || cc_count[1] == 3 ) ;
  EXPECT_EQ( true , cc_count[0] != cc_count[1] ) ;
}

TEST( graphConnectedComponent , largestCC )
{
  UndirectedGraph<> g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;
  UndirectedGraph<>::node_type * n3 = g.AddNode() ;
  UndirectedGraph<>::node_type * n4 = g.AddNode() ;
  UndirectedGraph<>::node_type * n5 = g.AddNode() ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;
  UndirectedGraph<>::edge_type * e2 = g.AddEdge( n2 , n3 ) ;
  UndirectedGraph<>::edge_type * e3 = g.AddEdge( n4 , n5 ) ;

  GraphConnectedComponents< UndirectedGraph<> > cc_solver ;

  UndirectedGraph<>::node_type * ref_big_cc = cc_solver.GetLargestCC( g ) ;

  EXPECT_EQ( true , n1 == ref_big_cc || n2 == ref_big_cc || n3 == ref_big_cc ) ;
}

TEST( graphConnectedComponent , ccCopy )
{
  typedef UndirectedGraph<> graph_type ;
  graph_type g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;
  UndirectedGraph<>::node_type * n3 = g.AddNode() ;
  UndirectedGraph<>::node_type * n4 = g.AddNode() ;
  UndirectedGraph<>::node_type * n5 = g.AddNode() ;

  GraphConnectedComponents< graph_type > cc_solver ;

  std::vector< graph_type > graph_cc = cc_solver.GetCCCopy( g ) ;

  EXPECT_EQ( 5 , graph_cc.size() ) ;

  EXPECT_EQ( 1 , graph_cc[0].NbNode() ) ;
  EXPECT_EQ( 0 , graph_cc[0].NbEdge() ) ;

  EXPECT_EQ( 1 , graph_cc[1].NbNode() ) ;
  EXPECT_EQ( 0 , graph_cc[1].NbEdge() ) ;

  EXPECT_EQ( 1 , graph_cc[2].NbNode() ) ;
  EXPECT_EQ( 0 , graph_cc[2].NbEdge() ) ;

  EXPECT_EQ( 1 , graph_cc[3].NbNode() ) ;
  EXPECT_EQ( 0 , graph_cc[3].NbEdge() ) ;

  EXPECT_EQ( 1 , graph_cc[4].NbNode() ) ;
  EXPECT_EQ( 0 , graph_cc[4].NbEdge() ) ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;
  UndirectedGraph<>::edge_type * e2 = g.AddEdge( n2 , n3 ) ;
  UndirectedGraph<>::edge_type * e3 = g.AddEdge( n4 , n5 ) ;

  graph_cc = cc_solver.GetCCCopy( g ) ;

  EXPECT_EQ( 2 , graph_cc.size() ) ;

  EXPECT_EQ( 5 , graph_cc[0].NbNode() + graph_cc[1].NbNode() ) ;
  EXPECT_EQ( 3 , graph_cc[0].NbEdge() + graph_cc[1].NbEdge() ) ;

  EXPECT_EQ( true , graph_cc[0].NbNode() == 3 || graph_cc[1].NbNode() == 3 ) ;
  EXPECT_EQ( true , graph_cc[0].NbNode() == 2 || graph_cc[1].NbNode() == 2 ) ;
  EXPECT_EQ( true , graph_cc[0].NbNode() != graph_cc[1].NbNode() ) ;

  EXPECT_EQ( true , graph_cc[0].NbEdge() == 2 || graph_cc[1].NbEdge() == 2 ) ;
  EXPECT_EQ( true , graph_cc[0].NbEdge() == 1 || graph_cc[1].NbEdge() == 1 ) ;
  EXPECT_EQ( true , graph_cc[0].NbEdge() != graph_cc[1].NbEdge() ) ;

  EXPECT_EQ( 1 , cc_solver.GetCCCopy( graph_cc[0] ).size() ) ;
  EXPECT_EQ( 1 , cc_solver.GetCCCopy( graph_cc[1] ).size() ) ;
}

TEST( graphConnectedComponent , cutPoints )
{
  typedef UndirectedGraph<> graph_type ;
  graph_type g ;

  UndirectedGraph<>::node_type * n1 = g.AddNode() ;
  UndirectedGraph<>::node_type * n2 = g.AddNode() ;
  UndirectedGraph<>::node_type * n3 = g.AddNode() ;
  UndirectedGraph<>::node_type * n4 = g.AddNode() ;
  UndirectedGraph<>::node_type * n5 = g.AddNode() ;

  UndirectedGraph<>::edge_type * e12 = g.AddEdge( n1 , n2 ) ;
  UndirectedGraph<>::edge_type * e23 = g.AddEdge( n2 , n3 ) ;
  UndirectedGraph<>::edge_type * e34 = g.AddEdge( n3 , n4 ) ;
  UndirectedGraph<>::edge_type * e45 = g.AddEdge( n4 , n5 ) ;

  GraphConnectedComponents< graph_type > cc_solver ;
  std::vector< UndirectedGraph<>::node_type * > cuts = cc_solver.GetCutPoints( g ) ;

  EXPECT_EQ( 3 , cuts.size() ) ;

  EXPECT_TRUE( cuts[0] == n2 || cuts[0] == n3 || cuts[0] == n4 ) ;
  EXPECT_TRUE( cuts[1] == n2 || cuts[1] == n3 || cuts[1] == n4 ) ;
  EXPECT_TRUE( cuts[2] == n2 || cuts[2] == n3 || cuts[2] == n4 ) ;
  EXPECT_TRUE( cuts[0] != cuts[1] && cuts[0] != cuts[2] && cuts[1] != cuts[2] );

  g.RemoveEdge( e12 ) ;
  g.RemoveEdge( e23 ) ;
  g.RemoveEdge( e34 ) ;
  g.RemoveEdge( e45 ) ;

  UndirectedGraph<>::node_type * n6 = g.AddNode() ;
  UndirectedGraph<>::node_type * n7 = g.AddNode() ;

  UndirectedGraph<>::edge_type * e1 = g.AddEdge( n1 , n2 ) ;
  UndirectedGraph<>::edge_type * e2 = g.AddEdge( n1 , n3 ) ;
  UndirectedGraph<>::edge_type * e3 = g.AddEdge( n2 , n3 ) ;

  UndirectedGraph<>::edge_type * e4 = g.AddEdge( n2 , n4 ) ;
  UndirectedGraph<>::edge_type * e5 = g.AddEdge( n2 , n5 ) ;

  UndirectedGraph<>::edge_type * e6 = g.AddEdge( n2 , n7 ) ;
  UndirectedGraph<>::edge_type * e7 = g.AddEdge( n4 , n6 ) ;
  UndirectedGraph<>::edge_type * e8 = g.AddEdge( n5 , n6 ) ;

  cuts = cc_solver.GetCutPoints( g ) ;

  EXPECT_EQ( 1 , cuts.size() ) ;
  EXPECT_EQ( n2 , cuts[0] ) ;
}

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
