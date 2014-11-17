/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/synthtest/SynthBoatTrajectory.h>

using namespace sail;

namespace {
  SynthBoatTrajectory::WayPt makeWayPt(double x, double y, double r) {
    return SynthBoatTrajectory::WayPt(Vectorize<Length<double>, 2>{Length<double>::meters(x),
      Length<double>::meters(y)}, Length<double>::meters(r));
  }
}

TEST(SynthBoatTrajectoryTest, TangentTest1) {
  auto a = makeWayPt(0.5, 0.5, 0.5);
  auto b = makeWayPt(1.5, 1.5, 0.5);

  Angle<double> sol1, sol2;
  SynthBoatTrajectory::WayPt::solveCircleTangentLine(a, true, b, true, &sol1, &sol2);
  sol1 = sol1.normalizedAt0();
  sol2 = sol2.normalizedAt0();
  double minsol = std::min(sol1.degrees(), sol2.degrees());
  double maxsol = std::max(sol1.degrees(), sol2.degrees());


  EXPECT_NEAR(maxsol, 135.0, 1.0e-5);
  EXPECT_NEAR(minsol, -45, 1.0e-5);
}

TEST(SynthBoatTrajectoryTest, TangentTest2) {
  auto a = makeWayPt(0.5, 0.5, 0.5);
  auto b = makeWayPt(1.5, 1.5, 0.5);

  Angle<double> sol1, sol2;
  SynthBoatTrajectory::WayPt::solveCircleTangentLine(a, true, b, false, &sol1, &sol2);
  sol1 = sol1.positiveMinAngle();
  sol2 = sol2.positiveMinAngle();
  double minsol = std::min(sol1.degrees(), sol2.degrees());
  double maxsol = std::max(sol1.degrees(), sol2.degrees());


  EXPECT_NEAR(maxsol, 270, 1.0e-5);
  EXPECT_NEAR(minsol, 180, 1.0e-5);
}

TEST(SynthBoatTrajectoryTest, TangentTest3) {
  auto a = makeWayPt(0.5, 0.5, 0.5);
  auto b = makeWayPt(1.5, 1.5, 0.5);

  Angle<double> sol1, sol2;
  SynthBoatTrajectory::WayPt::solveCircleTangentLine(a, false, b, true, &sol1, &sol2);
  sol1 = sol1.positiveMinAngle();
  sol2 = sol2.positiveMinAngle();
  double minsol = std::min(sol1.degrees(), sol2.degrees());
  double maxsol = std::max(sol1.degrees(), sol2.degrees());


  EXPECT_NEAR(maxsol, 90, 1.0e-5);
  EXPECT_NEAR(minsol, 0, 1.0e-5);
}



