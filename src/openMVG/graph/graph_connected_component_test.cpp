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

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
