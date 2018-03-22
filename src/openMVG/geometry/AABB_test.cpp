// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "testing/testing.h"

#include "AABB.hpp"


using namespace openMVG ;
using namespace openMVG::geometry ;

TEST( AABB , initNull )
{
  AABB box ;
  EXPECT_EQ( box.empty() , true ) ;

  openMVG::Vec3 center = box.center() ;
  EXPECT_EQ( center[0] , std::numeric_limits<double>::infinity() ) ;
  EXPECT_EQ( center[1] , std::numeric_limits<double>::infinity() ) ;
  EXPECT_EQ( center[2] , std::numeric_limits<double>::infinity() ) ;

  openMVG::Vec3 extends = box.extends() ;
  EXPECT_EQ( extends[0] , -1.0 );
  EXPECT_EQ( extends[1] , -1.0 );
  EXPECT_EQ( extends[2] , -1.0 );
}

TEST( AABB , initOnePoint )
{
  const openMVG::Vec3 pt = openMVG::Vec3( 1.2 , 1.3 , 1.4 ) ;
  std::vector< openMVG::Vec3 > pts ;
  pts.emplace_back( pt ) ;
  AABB box( pts ) ;

  EXPECT_EQ( box.empty() , false ) ;

  const openMVG::Vec3 center = box.center() ;
  EXPECT_EQ( center[0] , pt[0] ) ;
  EXPECT_EQ( center[1] , pt[1] ) ;
  EXPECT_EQ( center[2] , pt[2] ) ;

  const openMVG::Vec3 extends = box.extends() ;
  EXPECT_EQ( extends[0] , 0.0 ) ;
  EXPECT_EQ( extends[1] , 0.0 ) ;
  EXPECT_EQ( extends[2] , 0.0 ) ;

  EXPECT_EQ( box.min()[0] , pt[0] ) ;
  EXPECT_EQ( box.min()[1] , pt[1] ) ;
  EXPECT_EQ( box.min()[2] , pt[2] ) ;

  EXPECT_EQ( box.max()[0] , pt[0] ) ;
  EXPECT_EQ( box.max()[1] , pt[1] ) ;
  EXPECT_EQ( box.max()[2] , pt[2] ) ;
}

TEST( AABB , initMinMax )
{
  const openMVG::Vec3 min( -1.2 , -1.3 , -1.4 ) ;
  const openMVG::Vec3 max( 2.3 , 4.3 , 3.2 ) ;

  AABB box( min , max ) ;

  EXPECT_EQ( box.empty() , false ) ;

  EXPECT_EQ( min[0] , box.min()[0] ) ;
  EXPECT_EQ( min[1] , box.min()[1] ) ;
  EXPECT_EQ( min[2] , box.min()[2] ) ;

  EXPECT_EQ( max[0] , box.max()[0] ) ;
  EXPECT_EQ( max[1] , box.max()[1] ) ;
  EXPECT_EQ( max[2] , box.max()[2] ) ;

  const openMVG::Vec3 center = box.center() ;
  EXPECT_EQ( center[0] , ( min[0] + max[0] ) / 2.0 ) ;
  EXPECT_EQ( center[1] , ( min[1] + max[1] ) / 2.0 ) ;
  EXPECT_EQ( center[2] , ( min[2] + max[2] ) / 2.0 ) ;

  const openMVG::Vec3 extends = box.extends() ;
  EXPECT_EQ( extends[0] , max[0] - min[0] ) ;
  EXPECT_EQ( extends[1] , max[1] - min[1] ) ;
  EXPECT_EQ( extends[2] , max[2] - min[2] ) ;
}

TEST( AABB , initMultiplePoints )
{
  const openMVG::Vec3 pt1( 0.9 , -1.2 , 3.6 ) ;
  const openMVG::Vec3 pt2( -3.9 , 2.2 , 3.4 ) ;
  const openMVG::Vec3 pt3( 2.9 , 1.2 , 1.4 ) ;

  std::vector< openMVG::Vec3 > pts ;
  pts.emplace_back( pt1 ) ;
  pts.emplace_back( pt2 ) ;
  pts.emplace_back( pt3 ) ;
  AABB box( pts ) ;

  EXPECT_EQ( pts.empty() , false ) ;

  EXPECT_EQ( box.min()[0] , pt2[0] ) ;
  EXPECT_EQ( box.min()[1] , pt1[1] ) ;
  EXPECT_EQ( box.min()[2] , pt3[2] ) ;

  EXPECT_EQ( box.max()[0] , pt3[0] ) ;
  EXPECT_EQ( box.max()[1] , pt2[1] ) ;
  EXPECT_EQ( box.max()[2] , pt1[2] ) ;

  const openMVG::Vec3 center = box.center() ;
  EXPECT_EQ( center[0] , ( pt2[0] + pt3[0] ) / 2.0 ) ;
  EXPECT_EQ( center[1] , ( pt1[1] + pt2[1] ) / 2.0 ) ;
  EXPECT_EQ( center[2] , ( pt3[2] + pt1[2] ) / 2.0 ) ;

  const openMVG::Vec3 extends = box.extends() ;
  EXPECT_EQ( extends[0] , pt3[0] - pt2[0] ) ;
  EXPECT_EQ( extends[1] , pt2[1] - pt1[1] ) ;
  EXPECT_EQ( extends[2] , pt1[2] - pt3[2] ) ;
}

TEST( AABB , initNullUpdateOnePoint )
{
  const openMVG::Vec3 pt = openMVG::Vec3( 1.2 , 1.3 , 1.4 ) ;
  AABB box ;
  box.update( pt ) ;

  EXPECT_EQ( box.empty() , false ) ;

  const openMVG::Vec3 center = box.center() ;
  EXPECT_EQ( center[0] , pt[0] ) ;
  EXPECT_EQ( center[1] , pt[1] ) ;
  EXPECT_EQ( center[2] , pt[2] ) ;

  const openMVG::Vec3 extends = box.extends() ;
  EXPECT_EQ( extends[0] , 0.0 ) ;
  EXPECT_EQ( extends[1] , 0.0 ) ;
  EXPECT_EQ( extends[2] , 0.0 ) ;

  EXPECT_EQ( box.min()[0] , pt[0] ) ;
  EXPECT_EQ( box.min()[1] , pt[1] ) ;
  EXPECT_EQ( box.min()[2] , pt[2] ) ;

  EXPECT_EQ( box.max()[0] , pt[0] ) ;
  EXPECT_EQ( box.max()[1] , pt[1] ) ;
  EXPECT_EQ( box.max()[2] , pt[2] ) ;
}

TEST( AABB , initNullUpdateMulitplePoint )
{
  const openMVG::Vec3 pt1( 0.9 , -1.2 , 3.6 ) ;
  const openMVG::Vec3 pt2( -3.9 , 2.2 , 3.4 ) ;
  const openMVG::Vec3 pt3( 2.9 , 1.2 , 1.4 ) ;

  std::vector< openMVG::Vec3 > pts ;
  pts.emplace_back( pt1 ) ;
  pts.emplace_back( pt2 ) ;
  pts.emplace_back( pt3 ) ;

  AABB box ;
  box.update( pts ) ;

  EXPECT_EQ( pts.empty() , false ) ;

  EXPECT_EQ( box.min()[0] , pt2[0] ) ;
  EXPECT_EQ( box.min()[1] , pt1[1] ) ;
  EXPECT_EQ( box.min()[2] , pt3[2] ) ;

  EXPECT_EQ( box.max()[0] , pt3[0] ) ;
  EXPECT_EQ( box.max()[1] , pt2[1] ) ;
  EXPECT_EQ( box.max()[2] , pt1[2] ) ;

  const openMVG::Vec3 center = box.center() ;
  EXPECT_EQ( center[0] , ( pt2[0] + pt3[0] ) / 2.0 ) ;
  EXPECT_EQ( center[1] , ( pt1[1] + pt2[1] ) / 2.0 ) ;
  EXPECT_EQ( center[2] , ( pt3[2] + pt1[2] ) / 2.0 ) ;

  const openMVG::Vec3 extends = box.extends() ;
  EXPECT_EQ( extends[0] , pt3[0] - pt2[0] ) ;
  EXPECT_EQ( extends[1] , pt2[1] - pt1[1] ) ;
  EXPECT_EQ( extends[2] , pt1[2] - pt3[2] ) ;
}

TEST( AABB , containsPoint )
{
  const openMVG::Vec3 pt1( 0.9 , -1.2 , 3.6 ) ;
  const openMVG::Vec3 pt2( -3.9 , 2.2 , 3.4 ) ;
  const openMVG::Vec3 pt3( 2.9 , 1.2 , 1.4 ) ;

  std::vector< openMVG::Vec3 > pts ;
  pts.emplace_back( pt1 ) ;
  pts.emplace_back( pt2 ) ;
  pts.emplace_back( pt3 ) ;

  AABB box ;
  box.update( pts ) ;

  const openMVG::Vec3 ptIn1( 0.1 , 0.3 , 1.6 ) ;
  const openMVG::Vec3 ptOut1( -4.0 , 0.3 , 1.6 ) ;
  const openMVG::Vec3 ptOut2( 3.0 , 0.3 , 1.6 ) ;
  const openMVG::Vec3 ptOut3( 0.1 , -1.3 , 1.6 ) ;
  const openMVG::Vec3 ptOut4( 0.1 , 2.3 , 1.6 ) ;
  const openMVG::Vec3 ptOut5( 0.1 , 0.3 , 1.0 ) ;
  const openMVG::Vec3 ptOut6( 0.1 , 0.3 , 4.0 ) ;
  const openMVG::Vec3 ptOut7( -5.0 , -5.0 , -5.0 ) ;
  const openMVG::Vec3 ptOut8( 5.0 , 5.0 , 5.0 ) ;

  EXPECT_EQ( box.contains( ptIn1 ) , true ) ;
  EXPECT_EQ( box.contains( ptOut1 ) , false ) ;
  EXPECT_EQ( box.contains( ptOut2 ) , false ) ;
  EXPECT_EQ( box.contains( ptOut3 ) , false ) ;
  EXPECT_EQ( box.contains( ptOut4 ) , false ) ;
  EXPECT_EQ( box.contains( ptOut5 ) , false ) ;
  EXPECT_EQ( box.contains( ptOut6 ) , false ) ;
  EXPECT_EQ( box.contains( ptOut7 ) , false ) ;
  EXPECT_EQ( box.contains( ptOut8 ) , false ) ;
}

TEST( AABB , containsAABB )
{
  const AABB box( openMVG::Vec3( -1.2 , -1.4 , -2.3 ) , openMVG::Vec3( 3.4 , 4.2 , 3.5 ) ) ;

  const AABB inBox( openMVG::Vec3( -0.5 , -0.6 , -0.7 ) , openMVG::Vec3( 1.2 , 1.3 , 1.4 ) ) ;

  const AABB overlapBox( openMVG::Vec3( -1.4 , -0.6 , -0.7 ) , openMVG::Vec3( 1.2 , 1.3 , 1.4 ) ) ;

  const AABB outBox( openMVG::Vec3( -10.0 , -11.0 , -12.0 ) , openMVG::Vec3( -5.0 , -5.1 , -4.3 ) ) ;

  EXPECT_EQ( box.contains( inBox ) , true ) ;
  EXPECT_EQ( box.contains( overlapBox ) , false ) ;
  EXPECT_EQ( box.contains( outBox ) , false ) ;
}

TEST( AABB , containsTriangle )
{
  const AABB box( openMVG::Vec3( -1.2 , -1.4 , -2.3 ) , openMVG::Vec3( 3.4 , 4.2 , 3.5 ) ) ;

  const openMVG::Vec3 triIn1( openMVG::Vec3( -1.1 , -1.0 , 0.3 ) );
  const openMVG::Vec3 triIn2( openMVG::Vec3( 0.0 , 0.2 , 0.4 ) ) ;
  const openMVG::Vec3 triIn3( openMVG::Vec3( 2.3 , 3.0 , 3.1 ) ) ;

  const openMVG::Vec3 triOut1( openMVG::Vec3( -1.3 , -0.9 , -2.0 ) ) ;

  const openMVG::Vec3 triOut2( openMVG::Vec3( -1.5 , -1.6 , -2.4 ) ) ;
  const openMVG::Vec3 triOut3( openMVG::Vec3( 3.5 , 4.3 , -2.5 ) ) ;
  const openMVG::Vec3 triOut4( openMVG::Vec3( 5.0 , 5.1 , 5.2 ) ) ;

  const openMVG::Vec3 triOut5( openMVG::Vec3( -20.3 , -13.4 , -12.3 ) ) ;
  const openMVG::Vec3 triOut6( openMVG::Vec3( -15.2 , -12.3 , -11.2 ) ) ;
  const openMVG::Vec3 triOut7( openMVG::Vec3( -8.7 , -3.2 , -4.5 ) ) ;

  EXPECT_EQ( box.contains( triIn1 , triIn2 , triIn3 ) , true ) ;
  EXPECT_EQ( box.contains( triOut1 , triIn2 , triIn3 ) , false ) ;
  EXPECT_EQ( box.contains( triIn1 , triOut1 , triIn3 ) , false ) ;
  EXPECT_EQ( box.contains( triIn1 , triIn2 , triOut1 ) , false ) ;
  EXPECT_EQ( box.contains( triOut1 , triOut2 , triOut3 ) , false ) ;
  EXPECT_EQ( box.contains( triOut5 , triOut6 , triOut7 ) , false ) ;
}

TEST( AABB , overlapBox )
{
  const AABB box( openMVG::Vec3( -1.2 , -1.4 , -2.3 ) , openMVG::Vec3( 3.4 , 4.2 , 3.5 ) ) ;

  const AABB inBox( openMVG::Vec3( -0.5 , -0.6 , -0.7 ) , openMVG::Vec3( 1.2 , 1.3 , 1.4 ) ) ;

  const AABB overlapBox( openMVG::Vec3( -1.4 , -0.6 , -0.7 ) , openMVG::Vec3( 1.2 , 1.3 , 1.4 ) ) ;

  const AABB outBox( openMVG::Vec3( -10.0 , -11.0 , -12.0 ) , openMVG::Vec3( -5.0 , -5.1 , -4.3 ) ) ;

  EXPECT_EQ( box.overlap( inBox ) , true ) ;
  EXPECT_EQ( box.overlap( overlapBox ) , true ) ;
  EXPECT_EQ( box.contains( outBox ) , false ) ;
}

TEST( AABB , FrustumIncludesBox )
{
  const AABB box( openMVG::Vec3( -1.2 , -1.3 , -1.4 ) , openMVG::Vec3( 1.5 , 1.0 , 1.8 ) ) ;

  const double fov = D2R( 45 ) ;

  int w = 640 ;
  int h = 480 ;

  Mat3 K = Mat3::Identity() ;
  K( 0 , 0 ) = fov ;
  K( 0 , 2 ) = w / 2 ;
  K( 1 , 1 ) = fov ;
  K( 1 , 2 ) = h / 2 ;

  openMVG::Vec3 camC = openMVG::Vec3( 0 , 0 , -100 ) ;
  openMVG::Vec3 camC2 = openMVG::Vec3( 0 , 0 , -2 ) ;
  openMVG::Vec3 camC3 = openMVG::Vec3( 0 , 0 , 2 ) ;

  const Frustum fr( w , h , K , Mat3::Identity() , camC ) ;
  const Frustum fr2( w , h , K , Mat3::Identity() , camC2 ) ;
  const Frustum fr3( w , h , K , Mat3::Identity() , camC3 ) ;

  // Fully contains
  EXPECT_EQ( box.containedIn( fr ) , true ) ;
  // Overlap
  EXPECT_EQ( box.containedIn( fr2 ) , false ) ;
  // No intersection
  EXPECT_EQ( box.containedIn( fr3 ) , false ) ;
}

TEST( AABB , FrustumOverlap )
{
  const AABB box( openMVG::Vec3( -1.2 , -1.3 , -1.4 ) , openMVG::Vec3( 1.5 , 1.0 , 1.8 ) ) ;

  const double fov = D2R( 45 ) ;

  int w = 640 ;
  int h = 480 ;

  Mat3 K = Mat3::Identity() ;
  K( 0 , 0 ) = fov ;
  K( 0 , 2 ) = w / 2 ;
  K( 1 , 1 ) = fov ;
  K( 1 , 2 ) = h / 2 ;

  openMVG::Vec3 camC = openMVG::Vec3( 0 , 0 , -100 ) ;
  openMVG::Vec3 camC2 = openMVG::Vec3( 0 , 0 , -2 ) ;
  openMVG::Vec3 camC3 = openMVG::Vec3( 0 , 0 , 2.5 ) ;

  const Frustum fr( w , h , K , Mat3::Identity() , camC ) ;
  const Frustum fr2( w , h , K , Mat3::Identity() , camC2 ) ;
  const Frustum fr3( w , h , K , Mat3::Identity() , camC3 ) ;
  const Frustum fr4( w , h , K , Mat3::Identity() , camC3 , 1.0 , 10.0 ) ;

  // Fully contains
  EXPECT_EQ( box.overlap( fr ) , true ) ;
  // Overlap
  EXPECT_EQ( box.overlap( fr2 ) , true ) ;
  // No intersection but since no near or far defined -> intersect
  EXPECT_EQ( box.overlap( fr3 ) , true ) ;
  // No intersection
  EXPECT_EQ( box.overlap( fr4 ) , false ) ;
}

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
