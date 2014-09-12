/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/polar/PolarSurfaceParam.h>

using namespace sail;

TEST(PolarSurfaceParamTest, BasicTest) {
  EXPECT_EQ(-1, int(floor(-0.1)));

  //PolarSurfaceParam(int segsPerCtrlSpan, int ctrlCount, bool mirrored)
  PolarSurfaceParam(PolarCurveParam(),
        Velocity<double>::knots(40),
        40);
}
