/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/polar/PolarSurfaceParam.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>
#include <server/plot/extra.h>
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
  EXPECT_NEAR(X[0], 0.0, 1.0e-9);

  Arrayd vertices(param.vertexDim());
  param.paramToVertices(X, vertices);
  Vectorize<Velocity<double>, 3> x = param.computeSurfacePoint(vertices,
      Vectorize<double, 2>{0, 0});
  EXPECT_NEAR(x[0].knots(), 0.0, 1.0e-9);
  EXPECT_NEAR(x[1].knots(), 0.0, 1.0e-9);
  EXPECT_NEAR(x[2].knots(), 0.0, 1.0e-9);

  Vectorize<Velocity<double>, 3> y = param.computeSurfacePoint(vertices,
      Vectorize<double, 2>{0.5, 0.999999999});
  EXPECT_NEAR(y[0].knots(), 0.0, 1.0e-4);
  EXPECT_NEAR(y[1].knots(), -40.0, 1.0e-4);
  EXPECT_NEAR(y[2].knots(), 40.0, 1.0e-4);

  if (true) {
    GnuplotExtra plot;
    param.plot(vertices, &plot);
    plot.show();
  }
}
