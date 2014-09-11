/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/polar/PolarCurveParam.h>

using namespace sail;

TEST(PolarCurveParamTest, BasicTest) {
  PolarCurveParam param(5, 3, true);
  EXPECT_EQ(param.ctrlToParamIndex(0), 0);
  EXPECT_EQ(param.ctrlToParamIndex(1), 1);
  EXPECT_EQ(param.ctrlToParamIndex(2), 0);
}
