/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/polar/PolarSurfaceParam.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>
#include <iostream>

using namespace sail;

TEST(PolarSurfaceParamTest, BasicTest) {
  EXPECT_EQ(-1, int(floor(-0.1)));

  //PolarSurfaceParam(int segsPerCtrlSpan, int ctrlCount, bool mirrored)
  PolarSurfaceParam param(PolarCurveParam(15, 5, true),
        Velocity<double>::knots(40),
        40);

  Arrayd X = param.makeInitialParams();
  EXPECT_EQ(X.size(), 3*40);
  std::cout << EXPR_AND_VAL_AS_STRING(X) << std::endl;

}
