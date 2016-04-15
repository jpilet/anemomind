/*
 * experimentalTest.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/calib/experiment/CalibExperiment.h>


using namespace sail;
using namespace sail::Experimental;

TEST(ExperimentalTest, Gaussian) {
  typedef Gaussian<double> Gd;

  Eigen::Vector2d a(3, 1);
  Eigen::Vector2d b(1, 3);

  Gd ga(a);
  Gd gb(b);
  auto gc = ga + gb;
  EXPECT_NEAR(gc.computeVariance(gc.computeMean()), 2.0, 1.0e-9);
}

TEST(ExperimentalTest, TestSomething) {
  Nav nav;
  nav.setWatSpeed(Velocity<double>::knots(4.3));
  nav.setMagHdg(Angle<double>::degrees(30.0));
  nav.setGpsSpeed(Velocity<double>::knots(9.4));
  nav.setGpsBearing(Angle<double>::degrees(74.4));
  auto x = makeCurrentMatrix(nav);
  EXPECT_EQ(x.A(0, 0), x.A(1, 1));
  EXPECT_EQ(x.A(0, 1), -x.A(1, 0));
  EXPECT_EQ(x.A(0, 2), x.A(1, 3));
  EXPECT_EQ(x.A(1, 2), -x.A(0, 3));
  EXPECT_TRUE(std::isfinite(x.B(0)));
  EXPECT_TRUE(std::isfinite(x.B(1)));
}
