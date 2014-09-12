/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/polar/PolarCurveParam.h>

using namespace sail;

TEST(PolarCurveParamTest, BasicTest) {
  PolarCurveParam param(5, 5, true);
  EXPECT_EQ(param.ctrlToParamIndex(0), 0);
  EXPECT_EQ(param.ctrlToParamIndex(1), 1);
  EXPECT_EQ(param.ctrlToParamIndex(2), 2);
  EXPECT_EQ(param.ctrlToParamIndex(3), 1);
  EXPECT_EQ(param.ctrlToParamIndex(4), 0);

  // Verify the vertex counts
  EXPECT_EQ(param.paramCount(), 3);
  EXPECT_EQ(param.vertexCount(), (4 + 2)*5 + 1);

  Arrayd params = param.makeInitialParameters();
  EXPECT_EQ(params.size(), 3);
  for (int i = 0; i < param.paramCount(); i++) {
    EXPECT_NEAR(params[i], 1.0, 1.0e-9);
  }

//  Arrayd vertices(param.vertexCount());
//  param.paramToVertices(params, vertices);
//  {
//    double v[2];
//    param.computeCurvePos(vertices, 0.5, v);
//    EXPECT_NEAR(v[0], 0.0, 1.0e-9);
//    EXPECT_NEAR(v[1], -1.0, 1.0e-9);
//  }
}
