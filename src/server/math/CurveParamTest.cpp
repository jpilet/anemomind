/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/CurveParam.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(CurveParamTest, Test3Vertices) {
  int vertexCount = 5;
  Arrayi ctrlInds = Arrayi::args(0, 4);
  int regDeg = 2;
  bool open = true;
  MDArray2d P = parameterizeCurve(vertexCount, ctrlInds, regDeg, open);
  double tol = 1.0e-9;
  EXPECT_NEAR(P(0, 0), 1.0, tol);  EXPECT_NEAR(P(0, 1), 0.0, tol);
  EXPECT_NEAR(P(1, 0), 0.75, tol); EXPECT_NEAR(P(1, 1), 0.25, tol);
  EXPECT_NEAR(P(2, 0), 0.5, tol);  EXPECT_NEAR(P(2, 1), 0.5, tol);
  EXPECT_NEAR(P(3, 0), 0.25, tol); EXPECT_NEAR(P(3, 1), 0.75, tol);
  EXPECT_NEAR(P(4, 0), 0.0, tol);  EXPECT_NEAR(P(4, 1), 1.0, tol);
}



