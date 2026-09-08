
// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2012, 2013 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/numeric/numeric.h"
#include "openMVG/numeric/poly.h"

#include "CppUnitLite/TestHarness.h"
#include "testing/testing.h"

using namespace openMVG;

// Find the polynomial coefficients of x in the equation
//
//   (x - a)(x - b)(x - c) == 0
//
// by expanding to
//
//   x^3 - (c+b+a) * x^2 + (a*b+(b+a)*c) * x - a*b*c = 0.
//           = p               = q              = r
void CoeffsForCubicZeros(std::array<double,3> coeff, // a,b,c
                         std::array<double,3> & res) {
  const double & a = coeff[0];
  const double & b = coeff[1];
  const double & c = coeff[2];
  res[0] = -(c + b + a);
  res[1] = (a * b + (b + a) * c);
  res[2] = -a * b * c;
}

TEST(Poly, SolveCubicPolynomial) {
  std::array<double,3> coeff;
  std::array<double,3> poly_coeff;
  double found_roots[3];

  const double &p = poly_coeff[0];
  const double &q = poly_coeff[1];
  const double &r = poly_coeff[2];

  const double epsilon = 1e-10; 

  coeff = {1,2,3};
  CoeffsForCubicZeros(coeff, poly_coeff);
  CHECK_EQUAL(3, SolveCubicPolynomial(p, q, r, found_roots));
  EXPECT_NEAR(coeff[0], found_roots[0], epsilon);
  EXPECT_NEAR(coeff[1], found_roots[2], epsilon);
  EXPECT_NEAR(coeff[2], found_roots[1], epsilon);

  coeff = {0,1,3};
  CoeffsForCubicZeros(coeff, poly_coeff);
  CHECK_EQUAL(3, SolveCubicPolynomial(p, q, r, found_roots));
  EXPECT_NEAR(coeff[0], found_roots[0], epsilon);
  EXPECT_NEAR(coeff[1], found_roots[2], epsilon);
  EXPECT_NEAR(coeff[2], found_roots[1], epsilon);

  coeff = {-10,0,1};
  CoeffsForCubicZeros(coeff, poly_coeff);
  CHECK_EQUAL(3, SolveCubicPolynomial(p,q,r, found_roots));
  EXPECT_NEAR(coeff[0], found_roots[0], epsilon);
  EXPECT_NEAR(coeff[1], found_roots[2], epsilon);
  EXPECT_NEAR(coeff[2], found_roots[1], epsilon);

  coeff = {-8,1,3};
  CoeffsForCubicZeros(coeff, poly_coeff);
  CHECK_EQUAL(3, SolveCubicPolynomial(p, q, r, found_roots));
  EXPECT_NEAR(coeff[0], found_roots[0], epsilon);
  EXPECT_NEAR(coeff[1], found_roots[2], epsilon);
  EXPECT_NEAR(coeff[2], found_roots[1], epsilon);

  coeff = {28,28,105};
  CoeffsForCubicZeros(coeff, poly_coeff);
  CHECK_EQUAL(3, SolveCubicPolynomial(p, q, r, found_roots));
  EXPECT_NEAR(coeff[0], found_roots[0], epsilon);
  EXPECT_NEAR(coeff[1], found_roots[2], epsilon);
  EXPECT_NEAR(coeff[2], found_roots[1], epsilon);
}

TEST( Poly, RobustQuadraticSolver )
{
  std::vector<double> roots;

  // Case 1: Standard roots (x - 1)(x - 2) = x^2 - 3x + 2 = 0
  CHECK_EQUAL( 2, RobustQuadraticSolver( 1.0, -3.0, 2.0, roots ) );
  std::sort( roots.begin(), roots.end() );
  EXPECT_NEAR( 1.0, roots[ 0 ], 1e-6 );
  EXPECT_NEAR( 2.0, roots[ 1 ], 1e-6 );

  // Case 2: Double root (x - 3)^2 = x^2 - 6x + 9 = 0
  CHECK_EQUAL( 1, RobustQuadraticSolver( 1.0, -6.0, 9.0, roots ) );
  EXPECT_NEAR( 3.0, roots[ 0 ], 1e-6 );

  // Case 3: No real roots x^2 + 1 = 0
  CHECK_EQUAL( 0, RobustQuadraticSolver( 1.0, 0.0, 1.0, roots ) );

  // Case 4: Degenerate linear equation a = 0: 2x - 6 = 0 => x = 3
  CHECK_EQUAL( 1, RobustQuadraticSolver( 0.0, 2.0, -6.0, roots ) );
  EXPECT_NEAR( 3.0, roots[ 0 ], 1e-6 );

  // Case 5: Catastrophic cancellation test case in float precision:
  // x^2 - 10000.0f * x + 1.0f = 0 => roots are ~ 10000.0f and ~ 0.0001f (1/10000)
  std::vector<float> roots_f;
  CHECK_EQUAL( 2, RobustQuadraticSolver( 1.0f, -10000.0f, 1.0f, roots_f, 1e-6f ) );
  std::sort( roots_f.begin(), roots_f.end() );
  EXPECT_NEAR( 1e-4f, roots_f[ 0 ], 1e-7f );
  EXPECT_NEAR( 10000.0f, roots_f[ 1 ], 1e-1f );
}

/* ************************************************************************* */
int main() { TestResult tr; return TestRegistry::runAllTests(tr);}
/* ************************************************************************* */

