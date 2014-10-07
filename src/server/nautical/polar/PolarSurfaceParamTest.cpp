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

namespace {

  MDArray2d make3dPts(PolarSurfaceParam param,
      Arrayd vertices,
      Array<Vectorize<double, 2> > pts) {
    int count = pts.size();
    MDArray2d X(count, 3);
    for (int i = 0; i < count; i++) {
      Vectorize<Velocity<double>, 3> vel = param.computeSurfacePoint(vertices, pts[i]);
      for (int j = 0; j < 3; j++) {
        X(i, j) = vel[j].knots();
      }
    }
    return X;
  }

  void makeExamplePlot(PolarSurfaceParam param,
        Arrayd vertices) {
      Array<Vectorize<double, 2> > srfpts = param.generateSurfacePoints(300);
      MDArray2d pts3d = make3dPts(param, vertices, srfpts);

      GnuplotExtra plot;
      param.plot(vertices, &plot);
      plot.set_style("points");
      plot.plot(pts3d);
      plot.show();
  }
}

TEST(PolarSurfaceParamTest, BasicTest) {
  EXPECT_EQ(-1, int(floor(-0.1)));

  //PolarSurfaceParam(int segsPerCtrlSpan, int ctrlCount, bool mirrored)
  PolarSurfaceParam param(PolarCurveParam(15, 5, true),
        Velocity<double>::knots(40),
        40);
  EXPECT_FALSE(param.withCtrl());

  Arrayd X = param.makeInitialParams();
  EXPECT_EQ(X.size(), 3*40);

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
  EXPECT_LE(y[1].knots(), -10.0);
  EXPECT_NEAR(y[2].knots(), 40.0, 1.0e-4);

  EXPECT_NEAR(logline(expline(13.0)), 13.0, 1.0e-9);
  EXPECT_NEAR(logline(expline(-13.0)), -13.0, 1.0e-9);
  EXPECT_NEAR(expline(-1.0e-9), expline(1.0e-9), 1.0e-4);

  //makeExamplePlot(param, vertices);
}

TEST(PolarSurfaceParamTest, FullCtrl) {
  EXPECT_EQ(-1, int(floor(-0.1)));

  //PolarSurfaceParam(int segsPerCtrlSpan, int ctrlCount, bool mirrored)
  PolarSurfaceParam param(PolarCurveParam(15, 5, true),
        Velocity<double>::knots(40),
        40, 40);
  EXPECT_TRUE(param.withCtrl());

  Arrayd X = param.makeInitialParams();
  EXPECT_EQ(X.size(), 3*40);

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
  EXPECT_LE(y[1].knots(), -10.0);
  EXPECT_NEAR(y[2].knots(), 40.0, 1.0e-4);

  EXPECT_NEAR(logline(expline(13.0)), 13.0, 1.0e-9);
  EXPECT_NEAR(logline(expline(-13.0)), -13.0, 1.0e-9);
  EXPECT_NEAR(expline(-1.0e-9), expline(1.0e-9), 1.0e-4);

  //makeExamplePlot(param, vertices);
}

TEST(PolarSurfaceParamTest, Ctrl) {
  EXPECT_EQ(-1, int(floor(-0.1)));

  //PolarSurfaceParam(int segsPerCtrlSpan, int ctrlCount, bool mirrored)
  PolarSurfaceParam param(PolarCurveParam(15, 5, true),
        Velocity<double>::knots(40),
        40, 8);
  EXPECT_TRUE(param.withCtrl());

  Arrayd X = param.makeInitialParams();
  EXPECT_EQ(X.size(), 3*8);

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
  EXPECT_LE(y[1].knots(), -10.0);
  EXPECT_NEAR(y[2].knots(), 40.0, 1.0e-4);

  EXPECT_NEAR(logline(expline(13.0)), 13.0, 1.0e-9);
  EXPECT_NEAR(logline(expline(-13.0)), -13.0, 1.0e-9);
  EXPECT_NEAR(expline(-1.0e-9), expline(1.0e-9), 1.0e-4);

  //makeExamplePlot(param, vertices);
}
