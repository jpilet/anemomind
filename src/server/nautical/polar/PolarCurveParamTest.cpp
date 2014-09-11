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
  EXPECT_EQ(param.paramCount(), 3);
  EXPECT_EQ(param.vertexCount(), (4 + 2)*5 + 1);
}
