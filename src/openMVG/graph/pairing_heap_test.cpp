// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "testing/testing.h"

#include "openMVG/graph/graph.hpp"

using namespace openMVG::graph ;

TEST( PairingHeap , Ctr )
{
  PairingHeap<int, float> heap( 10 ) ;

  EXPECT_EQ( 0 , heap.Size() ) ;
  EXPECT_EQ( true , heap.Empty() ) ;
  EXPECT_EQ( NULL , heap.FindMin() ) ;
}

TEST( PairingHeap , Insert )
{
  PairingHeap<int, float> heap( 10 ) ;

  heap.Insert( 10 , 3.14152 ) ;
  heap.Insert( 11 , 2.718 ) ;
  heap.Insert( 9 , 6.674 ) ;
  heap.Insert( 1 , 6.626 ) ;

  EXPECT_EQ( 4 , heap.Size() ) ;
  EXPECT_EQ( false , heap.Empty() ) ;
  EXPECT_TRUE( heap.FindMin() != nullptr ) ;
}

TEST( PairingHeap , FindMin )
{
  PairingHeap<int, float> heap( 10 ) ;

  heap.Insert( 10 , 3.14152 ) ;
  EXPECT_EQ( 10 , heap.GetKey( heap.FindMin() ) ) ;

  heap.Insert( 11 , 2.718 ) ;
  EXPECT_EQ( 10 , heap.GetKey( heap.FindMin() ) ) ;

  heap.Insert( 9 , 6.674 ) ;
  EXPECT_EQ( 9 , heap.GetKey( heap.FindMin() ) ) ;

  heap.Insert( 1 , 6.626 ) ;
  EXPECT_EQ( 1 , heap.GetKey( heap.FindMin() ) ) ;

  EXPECT_EQ( 4 , heap.Size() ) ;
  EXPECT_EQ( false , heap.Empty() ) ;
  EXPECT_TRUE( heap.FindMin() != nullptr ) ;
}

TEST( PairingHeap , DeleteMin )
{
  PairingHeap<int, float> heap( 10 ) ;

  heap.Insert( 10 , 3.14152 ) ;
  heap.Insert( 11 , 2.718 ) ;
  heap.Insert( 9 , 6.674 ) ;
  heap.Insert( 1 , 6.626 ) ;
  heap.Insert( -1 , 6.022 ) ;
  heap.Insert( 5 , 8.314 ) ;

  EXPECT_EQ( 6 , heap.Size() ) ;
  EXPECT_EQ( -1 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( 5 , heap.Size() ) ;
  EXPECT_EQ( 1 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( 4 , heap.Size() ) ;
  EXPECT_EQ( 5 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( 3 , heap.Size() ) ;
  EXPECT_EQ( 9 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( 2 , heap.Size() ) ;
  EXPECT_EQ( 10 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( 1 , heap.Size() ) ;
  EXPECT_EQ( 11 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( true , heap.Empty() ) ;
  EXPECT_EQ( NULL , heap.FindMin() ) ;
}

TEST( PairingHeap , MaxHeap )
{
  PairingHeap<int,float,std::greater<int>> heap( 10 , std::greater<int>() ) ;
  
  heap.Insert( 10 , 3.14152 ) ;
  heap.Insert( 11 , 2.718 ) ;
  heap.Insert( 9 , 6.674 ) ;
  heap.Insert( 1 , 6.626 ) ;
  heap.Insert( -1 , 6.022 ) ;
  heap.Insert( 5 , 8.314 ) ;

  EXPECT_EQ( 6 , heap.Size() ) ;
  EXPECT_EQ( 11 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( 5 , heap.Size() ) ;
  EXPECT_EQ( 10 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( 4 , heap.Size() ) ;
  EXPECT_EQ( 9 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( 3 , heap.Size() ) ;
  EXPECT_EQ( 5 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( 2 , heap.Size() ) ;
  EXPECT_EQ( 1 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( 1 , heap.Size() ) ;
  EXPECT_EQ( -1 , heap.GetKey( heap.FindMin() ) ) ;

  heap.DeleteMin() ;
  EXPECT_EQ( true , heap.Empty() ) ;
  EXPECT_EQ( NULL , heap.FindMin() ) ;
}


/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
